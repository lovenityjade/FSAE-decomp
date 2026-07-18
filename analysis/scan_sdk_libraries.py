#!/usr/bin/env python3
"""Find relocation-masked TwlSDK/TwlSystem library functions in a raw image.

The scanner understands GNU ar archives and 32-bit little-endian ARM ELF
objects using only the Python standard library.  Relocation words are ignored
during comparison, so linked calls and pointers do not prevent a match.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path
from typing import Any, Iterable


def c_string(data: bytes, offset: int) -> str:
    end = data.find(b"\0", offset)
    if end < 0:
        end = len(data)
    return data[offset:end].decode("utf-8", errors="replace")


def ar_members(data: bytes) -> Iterable[tuple[str, bytes]]:
    if not data.startswith(b"!<arch>\n"):
        raise ValueError("not an ar archive")
    pos = 8
    raw_members: list[tuple[str, bytes]] = []
    long_names = b""
    while pos + 60 <= len(data):
        header = data[pos:pos + 60]
        if header[58:60] != b"`\n":
            raise ValueError(f"invalid ar member header at {pos:#x}")
        raw_name = header[:16].decode("ascii", errors="replace").strip()
        size = int(header[48:58].decode("ascii").strip())
        payload = data[pos + 60:pos + 60 + size]
        if raw_name == "//":
            long_names = payload
        else:
            raw_members.append((raw_name, payload))
        pos += 60 + size
        if pos & 1:
            pos += 1
    for raw_name, payload in raw_members:
        if raw_name in ("/", "/SYM64/"):
            continue
        if raw_name.startswith("#1/"):
            name_size = int(raw_name[3:])
            name = payload[:name_size].decode("utf-8", errors="replace").rstrip("\0")
            payload = payload[name_size:]
        elif raw_name.startswith("/") and raw_name[1:].isdigit():
            name = c_string(long_names, int(raw_name[1:])).rstrip("/")
        else:
            name = raw_name.rstrip("/")
        yield name, payload


def elf_functions(data: bytes) -> list[dict[str, Any]]:
    if data[:7] != b"\x7fELF\x01\x01\x01" or len(data) < 52:
        return []
    if struct.unpack_from("<H", data, 18)[0] != 40:
        return []
    section_offset = struct.unpack_from("<I", data, 32)[0]
    section_entry_size, section_count = struct.unpack_from("<HH", data, 46)
    if section_entry_size < 40 or section_offset + section_entry_size * section_count > len(data):
        return []
    sections = [
        struct.unpack_from("<10I", data, section_offset + index * section_entry_size)
        for index in range(section_count)
    ]

    symbols: list[dict[str, Any]] = []
    for section in sections:
        if section[1] != 2:  # SHT_SYMTAB
            continue
        offset, size, string_index, entry_size = section[4], section[5], section[6], section[9] or 16
        strings_section = sections[string_index]
        strings = data[strings_section[4]:strings_section[4] + strings_section[5]]
        for pos in range(offset, offset + size, entry_size):
            name_offset, value, symbol_size, info, _, shndx = struct.unpack_from(
                "<IIIBBH", data, pos
            )
            symbols.append(
                {
                    "name": c_string(strings, name_offset),
                    "value": value,
                    "size": symbol_size,
                    "type": info & 0xF,
                    "section_index": shndx,
                }
            )

    relocation_offsets: dict[int, set[int]] = {}
    for section in sections:
        if section[1] not in (4, 9):  # SHT_RELA / SHT_REL
            continue
        offset, size, target_index = section[4], section[5], section[7]
        entry_size = section[9] or (12 if section[1] == 4 else 8)
        offsets = relocation_offsets.setdefault(target_index, set())
        for pos in range(offset, offset + size, entry_size):
            relocation_offset = struct.unpack_from("<I", data, pos)[0]
            offsets.update(range(relocation_offset, relocation_offset + 4))

    markers: dict[int, list[tuple[int, str]]] = {}
    for symbol in symbols:
        if symbol["name"] in ("$a", "$t", "$d"):
            markers.setdefault(symbol["section_index"], []).append(
                (symbol["value"], symbol["name"])
            )
    for entries in markers.values():
        entries.sort()

    functions: list[dict[str, Any]] = []
    for symbol in symbols:
        section_index = symbol["section_index"]
        if symbol["type"] != 2 or symbol["size"] == 0 or symbol["name"].startswith("$"):
            continue
        if not (0 < section_index < len(sections)):
            continue
        section = sections[section_index]
        if section[1] != 1 or not (section[2] & 0x4):  # PROGBITS + SHF_EXECINSTR
            continue
        start, size = symbol["value"], symbol["size"]
        if start + size > section[5]:
            continue
        code = data[section[4] + start:section[4] + start + size]
        mask = bytearray(b"\x01" * size)
        for relocated in relocation_offsets.get(section_index, set()):
            if start <= relocated < start + size:
                mask[relocated - start] = 0
        isa = "unknown"
        for marker_address, marker_name in markers.get(section_index, []):
            if marker_address > start:
                break
            isa = {"$a": "ARM", "$t": "Thumb", "$d": "data"}[marker_name]
        functions.append(
            {"name": symbol["name"], "code": code, "mask": bytes(mask), "isa": isa}
        )
    return functions


def clean_ranges(mask: bytes) -> list[tuple[int, int]]:
    ranges: list[tuple[int, int]] = []
    start: int | None = None
    for index, keep in enumerate(mask + b"\0"):
        if keep and start is None:
            start = index
        elif not keep and start is not None:
            ranges.append((start, index))
            start = None
    return ranges


def find_matches(target: bytes, code: bytes, mask: bytes, min_anchor: int) -> list[int]:
    ranges = clean_ranges(mask)
    if not ranges:
        return []
    anchor_start, anchor_end = max(ranges, key=lambda item: item[1] - item[0])
    if anchor_end - anchor_start < min_anchor:
        return []
    anchor = code[anchor_start:anchor_end]
    matches: list[int] = []
    search_pos = 0
    while True:
        found = target.find(anchor, search_pos)
        if found < 0:
            break
        candidate = found - anchor_start
        if candidate >= 0 and candidate + len(code) <= len(target):
            if all(target[candidate + left:candidate + right] == code[left:right]
                   for left, right in ranges):
                matches.append(candidate)
        search_pos = found + 1
    return matches


def runtime_mapping(binary_map: dict[str, Any], image_id: str,
                    binary_offset: int, size: int) -> tuple[int, str] | None:
    image = next(
        (entry for entry in binary_map["executables"] if entry["id"] == image_id),
        None,
    )
    if image is None:
        return None
    mappings: list[tuple[int, int, int, str]] = []
    if "module_params" in image:
        load = image["memory"]["load_address"]
        payload_offset = image["module_params"]["autoload_data_start"] - load
        mappings.append((0, payload_offset, load, "static"))
        for record in image["module_params"]["autoload_records"]:
            mappings.append(
                (payload_offset, payload_offset + record["size"],
                 record["destination"], "autoload")
            )
            payload_offset += record["size"]
    elif image.get("ltd_autoload"):
        ltd = image["ltd_autoload"]
        records = ltd["records"]
        # TWL staging files begin with a repository/header word that is not
        # part of the loaded payload.  Derive the payload start from the tail
        # table instead of assuming file offset zero.  This is 4 for KQ9V's
        # ARM9i and ARM7i images.
        payload_offset = ltd["table_binary_offset"] - sum(
            record["size"] for record in records
        )
        if payload_offset < 0:
            return None
        for record in records:
            mappings.append(
                (payload_offset, payload_offset + record["size"],
                 record["destination"], "ltd_autoload")
            )
            payload_offset += record["size"]
    for start, end, memory_start, kind in mappings:
        if start <= binary_offset and binary_offset + size <= end:
            return memory_start + binary_offset - start, kind
    return None


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("image", type=Path)
    parser.add_argument("archives", nargs="+", type=Path)
    parser.add_argument("--image-id", required=True)
    parser.add_argument("--load-address", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--min-size", type=int, default=12)
    parser.add_argument("--min-anchor", type=int, default=8)
    parser.add_argument("--binary-map", type=Path,
                        help="optional map used to translate autoload runtime addresses")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    target = args.image.read_bytes()
    binary_map = json.loads(args.binary_map.read_text()) if args.binary_map else None
    raw_matches: list[dict[str, Any]] = []
    archive_metadata: list[dict[str, Any]] = []
    functions_scanned = 0
    for archive_path in args.archives:
        archive_data = archive_path.read_bytes()
        archive_metadata.append(
            {"name": archive_path.name, "sha256": hashlib.sha256(archive_data).hexdigest()}
        )
        for member_name, member in ar_members(archive_data):
            for function in elf_functions(member):
                if len(function["code"]) < args.min_size:
                    continue
                functions_scanned += 1
                offsets = find_matches(target, function["code"], function["mask"], args.min_anchor)
                if len(offsets) != 1:
                    continue
                offset = offsets[0]
                mapped = runtime_mapping(binary_map, args.image_id, offset,
                                         len(function["code"])) if binary_map else None
                address = mapped[0] if mapped else args.load_address + offset
                raw_matches.append(
                    {
                        "name": function["name"],
                        "binary_offset": offset,
                        "address": address,
                        "size": len(function["code"]),
                        "end": address + len(function["code"]),
                        "memory_region_kind": mapped[1] if mapped else "linear_load",
                        "isa": function["isa"],
                        "matched_bytes": sum(function["mask"]),
                        "relocation_masked_bytes": len(function["mask"]) - sum(function["mask"]),
                        "library": archive_path.name,
                        "object": member_name,
                    }
                )

    collapsed: dict[tuple[Any, ...], dict[str, Any]] = {}
    for match in raw_matches:
        key = (match["name"], match["binary_offset"], match["size"], match["isa"])
        if key not in collapsed:
            collapsed[key] = {**match, "sources": []}
            del collapsed[key]["library"]
            del collapsed[key]["object"]
        collapsed[key]["sources"].append(
            {"library": match["library"], "object": match["object"]}
        )
    matches = sorted(collapsed.values(), key=lambda entry: (entry["binary_offset"], entry["name"]))
    for match in matches:
        match["sources"] = sorted(match["sources"], key=lambda entry: (entry["library"], entry["object"]))

    boundary_groups: dict[tuple[int, int, str], list[str]] = {}
    for match in matches:
        key = (match["binary_offset"], match["size"], match["isa"])
        boundary_groups.setdefault(key, []).append(match["name"])
    for match in matches:
        key = (match["binary_offset"], match["size"], match["isa"])
        candidates = sorted(set(boundary_groups[key]))
        match["boundary_candidate_count"] = len(candidates)
        if len(candidates) > 1:
            match["boundary_candidate_names"] = candidates

    intervals = sorted((offset, offset + size) for offset, size, _ in boundary_groups)
    coverage_ranges: list[list[int]] = []
    for start, end in intervals:
        if not coverage_ranges or start > coverage_ranges[-1][1]:
            coverage_ranges.append([start, end])
        else:
            coverage_ranges[-1][1] = max(coverage_ranges[-1][1], end)
    covered_bytes = sum(end - start for start, end in coverage_ranges)

    payload = {
        "schema_version": 1,
        "image": args.image_id,
        "image_sha256": hashlib.sha256(target).hexdigest(),
        "load_address": args.load_address,
        "method": "unique relocation-masked function match",
        "minimum_function_size": args.min_size,
        "minimum_contiguous_anchor": args.min_anchor,
        "archive_count": len(args.archives),
        "archives": archive_metadata,
        "functions_scanned": functions_scanned,
        "unique_match_count": len(matches),
        "unique_boundary_count": len(boundary_groups),
        "covered_bytes": covered_bytes,
        "coverage_ranges": [
            {"offset": start, "size": end - start, "end": end}
            for start, end in coverage_ranges
        ],
        "unique_matched_bytes_sum": sum(match["size"] for match in matches),
        "matches": matches,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(payload, indent=2) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
