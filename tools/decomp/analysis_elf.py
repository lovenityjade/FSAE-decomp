#!/usr/bin/env python3
"""Wrap an extracted Nintendo DS executable in a deterministic analysis ELF.

The SRL stores autoload payloads after the static image even though those bytes
execute at unrelated addresses.  Importing the raw file in a disassembler maps
those bytes incorrectly.  This tool emits a small ELF32 container whose PT_LOAD
segments describe the runtime layout and whose symbols come from independently
verified SDK-library matches.  It is an analysis aid, never a matching build
artifact.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import re
import struct
import sys
from typing import Any, Iterable


ELF_HEADER_SIZE = 52
PROGRAM_HEADER_SIZE = 32
SECTION_HEADER_SIZE = 40
EM_ARM = 40


class AnalysisElfError(RuntimeError):
    """An inconsistent or unsafe analysis input was detected."""


@dataclass(frozen=True)
class Region:
    name: str
    file_offset: int
    size: int
    address: int

    @property
    def file_end(self) -> int:
        return self.file_offset + self.size

    @property
    def address_end(self) -> int:
        return self.address + self.size


@dataclass(frozen=True)
class Symbol:
    name: str
    address: int
    size: int
    thumb: bool
    section_index: int


def read_json(path: Path, label: str) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise AnalysisElfError(f"cannot read {label} {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise AnalysisElfError(f"{label} must be a JSON object: {path}")
    return value


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def align(value: int, alignment: int) -> int:
    return (value + alignment - 1) & -alignment


def executable(binary_map: dict[str, Any], image_id: str) -> dict[str, Any]:
    matches = [
        item
        for item in binary_map.get("executables", [])
        if isinstance(item, dict) and item.get("id") == image_id
    ]
    if len(matches) != 1:
        raise AnalysisElfError(f"binary map must contain exactly one {image_id} executable")
    return matches[0]


def _integer(value: Any, label: str) -> int:
    if not isinstance(value, int) or value < 0:
        raise AnalysisElfError(f"{label} must be a non-negative integer")
    return value


def runtime_layout(item: dict[str, Any], image_size: int) -> tuple[list[Region], int, bool]:
    image_id = str(item.get("id"))
    memory = item.get("memory")
    if not isinstance(memory, dict):
        raise AnalysisElfError(f"{image_id} memory metadata is missing")

    module = item.get("module_params")
    if isinstance(module, dict):
        load = _integer(memory.get("load_address"), f"{image_id} load address")
        autoload_start = _integer(module.get("autoload_data_start"), "autoload data start")
        table_start = _integer(module.get("autoload_list_start"), "autoload table start")
        static_size = autoload_start - load
        table_offset = table_start - load
        if not 0 <= static_size <= table_offset <= image_size:
            raise AnalysisElfError(f"{image_id} static/autoload offsets are inconsistent")
        regions = [Region("static", 0, static_size, load)]
        cursor = static_size
        records = module.get("autoload_records")
        if not isinstance(records, list):
            raise AnalysisElfError(f"{image_id} autoload records are missing")
        for index, record in enumerate(records):
            if not isinstance(record, dict):
                raise AnalysisElfError(f"{image_id} autoload record {index} is invalid")
            size = _integer(record.get("size"), f"autoload {index} size")
            destination = _integer(record.get("destination"), f"autoload {index} destination")
            regions.append(Region(f"autoload{index}", cursor, size, destination))
            cursor += size
        if cursor != table_offset:
            raise AnalysisElfError(
                f"{image_id} autoload payload ends at {cursor:#x}, table starts at {table_offset:#x}"
            )
        entry = _integer(memory.get("entry_address"), f"{image_id} entry address")
        thumb = str(item.get("entry_isa", "ARM")).lower() == "thumb"
        return regions, entry, thumb

    ltd = item.get("ltd_autoload")
    if not isinstance(ltd, dict):
        raise AnalysisElfError(f"{image_id} has no recognized autoload layout")
    table_offset = _integer(ltd.get("table_binary_offset"), "LTD table offset")
    records = ltd.get("records")
    if not isinstance(records, list) or not records:
        raise AnalysisElfError(f"{image_id} LTD records are missing")
    # LTD staging images reserve their first word.  Records consume the bytes
    # immediately after it and end exactly where the tail table begins.
    cursor = 4
    regions = []
    for index, record in enumerate(records):
        if not isinstance(record, dict):
            raise AnalysisElfError(f"{image_id} LTD record {index} is invalid")
        size = _integer(record.get("size"), f"LTD autoload {index} size")
        destination = _integer(record.get("destination"), f"LTD autoload {index} destination")
        regions.append(Region(f"autoload{index}", cursor, size, destination))
        cursor += size
    if cursor != table_offset or table_offset > image_size:
        raise AnalysisElfError(
            f"{image_id} LTD payload ends at {cursor:#x}, table starts at {table_offset:#x}"
        )
    code_offset = _integer(item.get("initial_code_binary_offset", 4), "initial code offset")
    first = next((region for region in regions if region.file_offset <= code_offset < region.file_end), None)
    if first is None:
        raise AnalysisElfError(f"{image_id} initial code is outside LTD payloads")
    entry = first.address + code_offset - first.file_offset
    thumb = str(item.get("initial_isa", "ARM")).lower() == "thumb"
    return regions, entry, thumb


def _clean_symbol(name: str) -> str:
    cleaned = re.sub(r"[^A-Za-z0-9_.$]", "_", name)
    if not cleaned or cleaned[0].isdigit():
        cleaned = "sdk_" + cleaned
    return cleaned


def runtime_address(regions: list[Region], file_offset: int, size: int) -> tuple[int, int]:
    """Return the runtime address and ELF section index for a file range."""
    for index, region in enumerate(regions, start=1):
        if region.file_offset <= file_offset and file_offset + size <= region.file_end:
            return region.address + file_offset - region.file_offset, index
    raise AnalysisElfError(
        f"file range {file_offset:#x}..{file_offset + size:#x} is outside loaded regions"
    )


def sdk_symbols(
    signature_data: dict[str, Any] | None,
    image_id: str,
    regions: list[Region],
    entry: int,
    entry_thumb: bool,
    build_plan: dict[str, Any] | None = None,
) -> list[Symbol]:
    symbols: list[Symbol] = []
    used_names: dict[str, int] = {}
    seen_addresses: set[int] = set()

    def append(name: str, address: int, size: int, thumb: bool) -> None:
        section_index = next(
            (
                index
                for index, region in enumerate(regions, start=1)
                if region.address <= address < region.address_end
            ),
            None,
        )
        if section_index is None or address in seen_addresses:
            return
        base = _clean_symbol(name)
        count = used_names.get(base, 0)
        used_names[base] = count + 1
        unique = base if count == 0 else f"{base}__{count + 1}"
        symbols.append(Symbol(unique, address, size, thumb, section_index))
        seen_addresses.add(address)

    append("_start", entry, 0, entry_thumb)
    if signature_data is None:
        return symbols
    if signature_data.get("image") != image_id:
        raise AnalysisElfError(
            f"signature image is {signature_data.get('image')!r}, expected {image_id!r}"
        )
    raw_matches = signature_data.get("matches")
    if not isinstance(raw_matches, list):
        raise AnalysisElfError("signature matches must be an array")
    # Stable address/name ordering makes the generated ELF byte-reproducible.
    candidates: list[tuple[int, str, int, bool]] = []
    for match in raw_matches:
        if not isinstance(match, dict):
            continue
        file_offset = _integer(match.get("binary_offset"), "SDK symbol file offset")
        address = _integer(match.get("address"), "SDK symbol address")
        name = str(match.get("name", "sdk_function"))
        size = _integer(match.get("size", 0), f"SDK symbol {name} size")
        thumb = str(match.get("isa", "ARM")).lower() == "thumb"
        expected_address, _ = runtime_address(regions, file_offset, size)
        if address != expected_address:
            raise AnalysisElfError(
                f"SDK symbol {name} address is {address:#x}, expected {expected_address:#x} "
                f"from file offset {file_offset:#x}"
            )
        candidates.append((address, name, size, thumb))
    if build_plan is not None:
        recovered = build_plan.get("recovered_functions", [])
        if not isinstance(recovered, list):
            raise AnalysisElfError("build-plan recovered_functions must be an array")
        for function in recovered:
            if not isinstance(function, dict):
                raise AnalysisElfError("build-plan recovered function is invalid")
            name = str(function.get("name", "recovered_function"))
            file_offset = _integer(function.get("offset"), f"recovered {name} offset")
            size = _integer(function.get("size"), f"recovered {name} size")
            address, _ = runtime_address(regions, file_offset, size)
            thumb = str(function.get("isa", "ARM")).lower() == "thumb"
            candidates.append((address, name, size, thumb))
    for address, name, size, thumb in sorted(candidates):
        append(name, address, size, thumb)
    return symbols


def _string_table(strings: Iterable[str]) -> tuple[bytes, dict[str, int]]:
    data = bytearray(b"\0")
    offsets: dict[str, int] = {"": 0}
    for value in strings:
        if value in offsets:
            continue
        offsets[value] = len(data)
        data.extend(value.encode("utf-8"))
        data.append(0)
    return bytes(data), offsets


def build_elf(image: bytes, regions: list[Region], entry: int, symbols: list[Symbol]) -> bytes:
    if not regions:
        raise AnalysisElfError("at least one runtime region is required")
    for region in regions:
        if region.size <= 0 or region.file_end > len(image):
            raise AnalysisElfError(f"region {region.name} lies outside the executable")

    phoff = ELF_HEADER_SIZE
    cursor = align(phoff + len(regions) * PROGRAM_HEADER_SIZE, 0x100)
    segment_offsets: list[int] = []
    contents = bytearray(cursor)
    for region in regions:
        segment_offsets.append(cursor)
        contents.extend(image[region.file_offset : region.file_end])
        cursor += region.size
        padding_end = align(cursor, 4)
        contents.extend(b"\0" * (padding_end - cursor))
        cursor = padding_end

    strtab, string_offsets = _string_table(symbol.name for symbol in symbols)
    strtab_offset = cursor
    contents.extend(strtab)
    cursor += len(strtab)
    cursor = align(cursor, 4)
    contents.extend(b"\0" * (cursor - len(contents)))

    symtab_offset = cursor
    contents.extend(b"\0" * 16)  # mandatory null symbol
    for symbol in symbols:
        value = symbol.address | (1 if symbol.thumb else 0)
        contents.extend(
            struct.pack(
                "<IIIBBH",
                string_offsets[symbol.name],
                value,
                symbol.size,
                0x12,  # STB_GLOBAL | STT_FUNC
                0,
                symbol.section_index,
            )
        )
    symtab_size = 16 * (len(symbols) + 1)
    cursor += symtab_size

    section_names = ["", *(f".{region.name}" for region in regions), ".strtab", ".symtab", ".shstrtab"]
    shstrtab, section_name_offsets = _string_table(section_names)
    shstrtab_offset = cursor
    contents.extend(shstrtab)
    cursor += len(shstrtab)
    shoff = align(cursor, 4)
    contents.extend(b"\0" * (shoff - cursor))

    section_count = 1 + len(regions) + 3
    strtab_index = 1 + len(regions)
    symtab_index = strtab_index + 1
    shstrtab_index = symtab_index + 1
    section_headers = [b"\0" * SECTION_HEADER_SIZE]
    for region, file_offset in zip(regions, segment_offsets, strict=True):
        section_headers.append(
            struct.pack(
                "<IIIIIIIIII",
                section_name_offsets[f".{region.name}"],
                1,  # SHT_PROGBITS
                0x6,  # SHF_ALLOC | SHF_EXECINSTR
                region.address,
                file_offset,
                region.size,
                0,
                0,
                4,
                0,
            )
        )
    section_headers.extend(
        (
            struct.pack(
                "<IIIIIIIIII",
                section_name_offsets[".strtab"], 3, 0, 0, strtab_offset, len(strtab), 0, 0, 1, 0
            ),
            struct.pack(
                "<IIIIIIIIII",
                section_name_offsets[".symtab"],
                2,
                0,
                0,
                symtab_offset,
                symtab_size,
                strtab_index,
                1,
                4,
                16,
            ),
            struct.pack(
                "<IIIIIIIIII",
                section_name_offsets[".shstrtab"],
                3,
                0,
                0,
                shstrtab_offset,
                len(shstrtab),
                0,
                0,
                1,
                0,
            ),
        )
    )
    if len(section_headers) != section_count:
        raise AssertionError("internal section count mismatch")
    for header in section_headers:
        contents.extend(header)

    program_headers = bytearray()
    for region, file_offset in zip(regions, segment_offsets, strict=True):
        program_headers.extend(
            struct.pack(
                "<IIIIIIII",
                1,  # PT_LOAD
                file_offset,
                region.address,
                region.address,
                region.size,
                region.size,
                0x5,  # PF_R | PF_X
                4,
            )
        )

    ident = b"\x7fELF" + bytes((1, 1, 1, 0, 0)) + b"\0" * 7
    elf_header = ident + struct.pack(
        "<HHIIIIIHHHHHH",
        2,
        EM_ARM,
        1,
        entry | (1 if any(symbol.name == "_start" and symbol.thumb for symbol in symbols) else 0),
        phoff,
        shoff,
        0x05000000,  # EABI5 container; no claim about the original object ABI
        ELF_HEADER_SIZE,
        PROGRAM_HEADER_SIZE,
        len(regions),
        SECTION_HEADER_SIZE,
        section_count,
        shstrtab_index,
    )
    contents[:ELF_HEADER_SIZE] = elf_header
    contents[phoff : phoff + len(program_headers)] = program_headers
    return bytes(contents)


def create_analysis_elf(
    image_path: Path,
    binary_map_path: Path,
    image_id: str,
    output_path: Path,
    signatures_path: Path | None,
    build_plan_path: Path | None = None,
) -> dict[str, Any]:
    image = image_path.read_bytes()
    binary_map = read_json(binary_map_path, "binary map")
    item = executable(binary_map, image_id)
    rom = item.get("rom")
    if not isinstance(rom, dict):
        raise AnalysisElfError(f"{image_id} ROM metadata is missing")
    expected_size = _integer(rom.get("size"), f"{image_id} size")
    if len(image) != expected_size:
        raise AnalysisElfError(f"{image_id} is {len(image)} bytes, expected {expected_size}")
    expected_sha = str(rom.get("sha256", ""))
    actual_sha = sha256(image)
    if actual_sha != expected_sha:
        raise AnalysisElfError(f"{image_id} SHA-256 is {actual_sha}, expected {expected_sha}")
    regions, entry, entry_thumb = runtime_layout(item, len(image))
    signatures = read_json(signatures_path, "SDK signatures") if signatures_path else None
    if signatures is not None and signatures.get("image_sha256") != actual_sha:
        raise AnalysisElfError("SDK signatures describe a different executable")
    build_plan = read_json(build_plan_path, "build plan") if build_plan_path else None
    if build_plan is not None:
        target = build_plan.get("target")
        if not isinstance(target, dict) or target.get("sha256") != actual_sha:
            raise AnalysisElfError("build plan describes a different executable")
    symbols = sdk_symbols(signatures, image_id, regions, entry, entry_thumb, build_plan)
    elf = build_elf(image, regions, entry, symbols)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_bytes(elf)
    return {
        "image": image_id,
        "input_sha256": actual_sha,
        "output": str(output_path),
        "output_size": len(elf),
        "output_sha256": sha256(elf),
        "entry": entry,
        "entry_isa": "Thumb" if entry_thumb else "ARM",
        "regions": [region.__dict__ for region in regions],
        "symbols": len(symbols),
        "purpose": "analysis-only; not a matching build artifact",
    }


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(description=__doc__)
    result.add_argument("--image", required=True, choices=("arm9", "arm7", "arm9i", "arm7i"))
    result.add_argument("--input", required=True, type=Path)
    result.add_argument("--binary-map", type=Path, default=Path("config/binary-map.json"))
    result.add_argument("--signatures", type=Path)
    result.add_argument("--build-plan", type=Path, help="optional recovered-function map")
    result.add_argument("--output", required=True, type=Path)
    result.add_argument("--manifest", type=Path, help="optional JSON report path")
    return result


def main(argv: list[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        report = create_analysis_elf(
            args.input,
            args.binary_map,
            args.image,
            args.output,
            args.signatures,
            args.build_plan,
        )
        if args.manifest:
            args.manifest.parent.mkdir(parents=True, exist_ok=True)
            args.manifest.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps(report, indent=2))
        return 0
    except (AnalysisElfError, OSError) as exc:
        print(f"analysis ELF error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
