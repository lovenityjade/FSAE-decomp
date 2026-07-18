#!/usr/bin/env python3
"""Verify and prepare the Four Swords Anniversary Edition analysis inputs.

This module intentionally contains no Nintendo data.  The user supplies their
own ROM and BIOS dump; every mutation is performed on a build-directory copy.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
from typing import Any, Iterable, Sequence
import zipfile


TARGET_ROM_SHA256 = "3a880dce73ace38f923eb2c3f3e497ca00749c7349ab14c99ad88710c45a8be8"
TARGET_ROM_SIZE = 14_848_000
TARGET_TITLE = b"ZELDA 4SWORD"
TARGET_GAME_CODE = b"KQ9V"
TARGET_UNIT_CODE = 0x03
TARGET_DECRYPTED_ROM_SHA256 = "070edd820627d25749c1d24817c36fcb08a082e87cd3f4e0ad39ac41af84ded8"

BIOS7_SHA256 = "ba65f690eb04ec92db67c0e299e21ad71de087d6d5de8a9cb17a62eaab563c17"
BIOS7_SIZE = 16_384
BLOWFISH_OFFSET = 0x30
BLOWFISH_SIZE = 0x1048
BLOWFISH_SHA256 = "bedd20bd7f9cac742ad760e2448d4043e0d37121b67a1be3a6b8afbb8a34f08e"

TWLTOOL_ZIP_SHA256 = "a242eba6b10b5a7527a2df88a00c4029d65ebc69bbfb80045a73311e0071296b"
TWLTOOL_EXE_SHA256 = "bdadc1d4797dffe931b9c0dd12e99fe19e2d5680adca69b57b8d479273de5b52"
TWLTOOL_MEMBER = "twltool.exe"

SECURE_AREA_OFFSET = 0x4000
SECURE_AREA_CRYPT_SIZE = 0x800
MASK32 = 0xFFFF_FFFF

SECTION_FIELDS = {
    "arm9": 0x20,
    "arm7": 0x30,
    "arm9i": 0x1C0,
    "arm7i": 0x1D0,
}


class PipelineError(RuntimeError):
    """A user-facing validation or preparation error."""


def sha256_bytes(data: bytes | bytearray | memoryview) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _check_regular_file(path: Path, label: str) -> None:
    if not path.is_file():
        raise PipelineError(f"{label} not found or not a regular file: {path}")


def _verify_file(
    path: Path,
    *,
    label: str,
    expected_sha256: str,
    expected_size: int | None = None,
) -> dict[str, Any]:
    _check_regular_file(path, label)
    size = path.stat().st_size
    if expected_size is not None and size != expected_size:
        raise PipelineError(
            f"{label} has size {size}, expected {expected_size}; "
            "use a verified dump for this project"
        )
    actual = sha256_file(path)
    if actual != expected_sha256:
        raise PipelineError(
            f"{label} SHA-256 mismatch: got {actual}, expected {expected_sha256}"
        )
    return {"sha256": actual, "size": size}


def read_header(rom: bytes | bytearray | memoryview) -> dict[str, Any]:
    if len(rom) < 0x1000:
        raise PipelineError("ROM is too small to contain a DSi extended header")
    title = bytes(rom[0:12])
    game_code = bytes(rom[0x0C:0x10])
    maker_code = bytes(rom[0x10:0x12])
    unit_code = rom[0x12]

    sections: dict[str, dict[str, int]] = {}
    for name, offset in SECTION_FIELDS.items():
        file_offset, entry, load, size = struct.unpack_from("<IIII", rom, offset)
        sections[name] = {
            "offset": file_offset,
            "entry": entry,
            "load": load,
            "size": size,
        }

    fnt_offset, fnt_size, fat_offset, fat_size = struct.unpack_from("<IIII", rom, 0x40)
    modcrypt = []
    for index, offset in enumerate((0x220, 0x228)):
        area_offset, area_size = struct.unpack_from("<II", rom, offset)
        modcrypt.append({"index": index, "offset": area_offset, "size": area_size})

    return {
        "title": title,
        "game_code": game_code,
        "maker_code": maker_code,
        "unit_code": unit_code,
        "sections": sections,
        "nitrofs": {
            "fnt_offset": fnt_offset,
            "fnt_size": fnt_size,
            "fat_offset": fat_offset,
            "fat_size": fat_size,
        },
        "modcrypt": modcrypt,
    }


def _checked_range(total: int, offset: int, size: int, label: str) -> tuple[int, int]:
    if offset < 0 or size < 0 or offset > total or size > total - offset:
        raise PipelineError(
            f"{label} range 0x{offset:X}+0x{size:X} exceeds input size 0x{total:X}"
        )
    return offset, offset + size


def validate_target_header(header: dict[str, Any], rom_size: int) -> None:
    if header["title"] != TARGET_TITLE:
        raise PipelineError(f"unexpected ROM title: {header['title']!r}")
    if header["game_code"] != TARGET_GAME_CODE:
        raise PipelineError(f"unexpected game code: {header['game_code']!r}")
    if header["unit_code"] != TARGET_UNIT_CODE:
        raise PipelineError(f"unexpected unit code: 0x{header['unit_code']:02X}")
    for name, section in header["sections"].items():
        _checked_range(rom_size, section["offset"], section["size"], name)
    nitrofs = header["nitrofs"]
    _checked_range(rom_size, nitrofs["fnt_offset"], nitrofs["fnt_size"], "FNT")
    _checked_range(rom_size, nitrofs["fat_offset"], nitrofs["fat_size"], "FAT")
    if nitrofs["fat_size"] % 8:
        raise PipelineError("FAT size is not a multiple of eight bytes")
    for area in header["modcrypt"]:
        if area["offset"]:
            _checked_range(rom_size, area["offset"], area["size"], f"modcrypt {area['index']}")
            if area["size"] % 16:
                raise PipelineError(f"modcrypt {area['index']} size is not AES-block aligned")


def extract_blowfish_table(bios7: bytes) -> bytes:
    if len(bios7) != BIOS7_SIZE:
        raise PipelineError(f"bios7.bin has size {len(bios7)}, expected {BIOS7_SIZE}")
    table = bios7[BLOWFISH_OFFSET : BLOWFISH_OFFSET + BLOWFISH_SIZE]
    if len(table) != BLOWFISH_SIZE:
        raise PipelineError("bios7.bin does not contain a complete Blowfish table at 0x30")
    return table


def _lookup(magic: list[int], value: int) -> int:
    a = magic[18 + ((value >> 24) & 0xFF)]
    b = magic[18 + 256 + ((value >> 16) & 0xFF)]
    c = magic[18 + 512 + ((value >> 8) & 0xFF)]
    d = magic[18 + 768 + (value & 0xFF)]
    return (d + (c ^ ((b + a) & MASK32))) & MASK32


def _encrypt_block(magic: list[int], arg1: int, arg2: int) -> tuple[int, int]:
    a, b = arg1, arg2
    for index in range(16):
        c = magic[index] ^ a
        a = (b ^ _lookup(magic, c)) & MASK32
        b = c
    return (b ^ magic[17]) & MASK32, (a ^ magic[16]) & MASK32


def _decrypt_block(magic: list[int], arg1: int, arg2: int) -> tuple[int, int]:
    a, b = arg1, arg2
    for index in range(17, 1, -1):
        c = magic[index] ^ a
        a = (b ^ _lookup(magic, c)) & MASK32
        b = c
    return (b ^ magic[0]) & MASK32, (a ^ magic[1]) & MASK32


def _update_hashtable(magic: list[int], args: list[int]) -> None:
    raw_args = struct.pack("<III", *args)
    for outer in range(18):
        value = 0
        for inner in range(4):
            value = ((value << 8) | raw_args[(outer * 4 + inner) & 7]) & MASK32
        magic[outer] ^= value

    tmp1 = tmp2 = 0
    for index in range(0, 18, 2):
        tmp1, tmp2 = _encrypt_block(magic, tmp1, tmp2)
        magic[index : index + 2] = (tmp1, tmp2)
    for index in range(0, 0x400, 2):
        tmp1, tmp2 = _encrypt_block(magic, tmp1, tmp2)
        magic[18 + index : 20 + index] = (tmp1, tmp2)


def _init2(magic: list[int], args: list[int]) -> None:
    args[2], args[1] = _encrypt_block(magic, args[2], args[1])
    args[1], args[0] = _encrypt_block(magic, args[1], args[0])
    _update_hashtable(magic, args)


def _init1(table: bytes, game_code_u32: int) -> tuple[list[int], list[int]]:
    if len(table) != BLOWFISH_SIZE:
        raise PipelineError(
            f"Blowfish table has size {len(table)}, expected {BLOWFISH_SIZE}"
        )
    magic = list(struct.unpack("<1042I", table))
    args = [game_code_u32, game_code_u32 >> 1, (game_code_u32 << 1) & MASK32]
    _init2(magic, args)
    _init2(magic, args)
    return magic, args


def decrypt_secure_area(rom: bytearray, table: bytes) -> None:
    """Decrypt the first 0x800 ARM9 bytes without modifying the DSi header."""

    _checked_range(len(rom), SECURE_AREA_OFFSET, SECURE_AREA_CRYPT_SIZE, "secure area")
    game_code = struct.unpack_from("<I", rom, 0x0C)[0]
    magic, args = _init1(table, game_code)

    word0, word1 = struct.unpack_from("<II", rom, SECURE_AREA_OFFSET)
    word1, word0 = _decrypt_block(magic, word1, word0)
    args[1] = (args[1] << 1) & MASK32
    args[2] >>= 1
    _init2(magic, args)
    word1, word0 = _decrypt_block(magic, word1, word0)
    if (word0, word1) != (0x72636E65, 0x6A624F79):
        raise PipelineError(
            "secure-area decryption check failed; ROM and BIOS table may not match"
        )

    struct.pack_into("<II", rom, SECURE_AREA_OFFSET, 0xE7FFDEFF, 0xE7FFDEFF)
    end = SECURE_AREA_OFFSET + SECURE_AREA_CRYPT_SIZE
    for offset in range(SECURE_AREA_OFFSET + 8, end, 8):
        word0, word1 = struct.unpack_from("<II", rom, offset)
        word1, word0 = _decrypt_block(magic, word1, word0)
        struct.pack_into("<II", rom, offset, word0, word1)


def _safe_component(raw_name: bytes) -> str:
    try:
        name = raw_name.decode("ascii")
    except UnicodeDecodeError as error:
        raise PipelineError("NitroFS contains a non-ASCII name") from error
    if not name or name in {".", ".."} or "/" in name or "\\" in name or "\x00" in name:
        raise PipelineError(f"unsafe NitroFS path component: {name!r}")
    return name


def list_nitrofs(rom: bytes | bytearray | memoryview) -> list[dict[str, Any]]:
    header = read_header(rom)
    nitrofs = header["nitrofs"]
    fnt_start, fnt_end = _checked_range(
        len(rom), nitrofs["fnt_offset"], nitrofs["fnt_size"], "FNT"
    )
    fat_start, fat_end = _checked_range(
        len(rom), nitrofs["fat_offset"], nitrofs["fat_size"], "FAT"
    )
    fnt = memoryview(rom)[fnt_start:fnt_end]
    fat = memoryview(rom)[fat_start:fat_end]
    if len(fnt) < 8 or len(fat) % 8:
        raise PipelineError("invalid NitroFS FNT/FAT structure")
    directory_count = struct.unpack_from("<H", fnt, 6)[0]
    if not 1 <= directory_count <= 0x1000 or directory_count * 8 > len(fnt):
        raise PipelineError(f"invalid NitroFS directory count: {directory_count}")

    files: list[dict[str, Any]] = []
    active: set[int] = set()
    visited: set[int] = set()

    def walk(directory_id: int, prefix: tuple[str, ...]) -> None:
        index = directory_id - 0xF000
        if not 0 <= index < directory_count:
            raise PipelineError(f"invalid NitroFS directory id: 0x{directory_id:04X}")
        if directory_id in active:
            raise PipelineError("cycle detected in NitroFS directory tree")
        if directory_id in visited:
            raise PipelineError("NitroFS directory is referenced more than once")
        active.add(directory_id)
        visited.add(directory_id)
        subtable, file_id, _parent = struct.unpack_from("<IHH", fnt, index * 8)
        if subtable >= len(fnt):
            raise PipelineError("NitroFS subtable offset is outside the FNT")

        cursor = subtable
        while True:
            if cursor >= len(fnt):
                raise PipelineError("unterminated NitroFS directory subtable")
            length = fnt[cursor]
            cursor += 1
            if length == 0:
                break
            is_directory = bool(length & 0x80)
            name_length = length & 0x7F
            if name_length == 0 or cursor + name_length > len(fnt):
                raise PipelineError("invalid NitroFS name length")
            name = _safe_component(bytes(fnt[cursor : cursor + name_length]))
            cursor += name_length
            if is_directory:
                if cursor + 2 > len(fnt):
                    raise PipelineError("truncated NitroFS child directory id")
                child_id = struct.unpack_from("<H", fnt, cursor)[0]
                cursor += 2
                walk(child_id, prefix + (name,))
            else:
                if (file_id + 1) * 8 > len(fat):
                    raise PipelineError(f"NitroFS file id {file_id} is outside the FAT")
                start, end = struct.unpack_from("<II", fat, file_id * 8)
                if end < start:
                    raise PipelineError(f"NitroFS file id {file_id} has a reversed range")
                _checked_range(len(rom), start, end - start, f"NitroFS file {file_id}")
                files.append(
                    {
                        "id": file_id,
                        "path": "/".join(prefix + (name,)),
                        "offset": start,
                        "size": end - start,
                    }
                )
                file_id += 1
        active.remove(directory_id)

    walk(0xF000, ())
    return sorted(files, key=lambda item: item["id"])


def extract_sections(
    rom: bytes | bytearray | memoryview, destination: Path
) -> dict[str, dict[str, Any]]:
    header = read_header(rom)
    destination.mkdir(parents=True, exist_ok=True)
    result: dict[str, dict[str, Any]] = {}
    for name, section in header["sections"].items():
        start, end = _checked_range(len(rom), section["offset"], section["size"], name)
        data = bytes(rom[start:end])
        output = destination / f"{name}.bin"
        output.write_bytes(data)
        result[name] = {
            **section,
            "path": f"sections/{output.name}",
            "sha256": sha256_bytes(data),
        }
    return result


def extract_nitrofs(
    rom: bytes | bytearray | memoryview, destination: Path
) -> list[dict[str, Any]]:
    files = list_nitrofs(rom)
    destination.mkdir(parents=True, exist_ok=True)
    for item in files:
        output = destination.joinpath(*item["path"].split("/"))
        output.parent.mkdir(parents=True, exist_ok=True)
        start = item["offset"]
        data = bytes(rom[start : start + item["size"]])
        output.write_bytes(data)
        item["sha256"] = sha256_bytes(data)
        item["output"] = f"nitrofs/{item['path']}"
    return files


def _find_zip_member(archive: zipfile.ZipFile, basename: str) -> str:
    matches = [name for name in archive.namelist() if Path(name).name.lower() == basename.lower()]
    if len(matches) != 1:
        raise PipelineError(f"TWLTool archive must contain exactly one {basename}")
    return matches[0]


def extract_twltool(twltool_zip: Path, destination: Path) -> dict[str, Any]:
    zip_info = _verify_file(
        twltool_zip,
        label="TWLTool v1.6 archive",
        expected_sha256=TWLTOOL_ZIP_SHA256,
    )
    try:
        with zipfile.ZipFile(twltool_zip) as archive:
            member = _find_zip_member(archive, TWLTOOL_MEMBER)
            executable = archive.read(member)
    except (OSError, zipfile.BadZipFile, KeyError) as error:
        raise PipelineError(f"cannot read TWLTool archive: {error}") from error
    executable_sha = sha256_bytes(executable)
    if executable_sha != TWLTOOL_EXE_SHA256:
        raise PipelineError(
            f"TWLTool executable SHA-256 mismatch: got {executable_sha}, "
            f"expected {TWLTOOL_EXE_SHA256}"
        )
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(executable)
    return {
        "archive_sha256": zip_info["sha256"],
        "executable_sha256": executable_sha,
        "version": "1.6",
    }


def _resolve_wine(requested: str | None) -> str:
    if requested:
        resolved = shutil.which(requested) if os.sep not in requested else requested
    else:
        resolved = shutil.which("wine")
    if not resolved or not Path(resolved).is_file():
        raise PipelineError("Wine is required to run the supplied TWLTool v1.6 executable")
    return str(Path(resolved).resolve())


def run_twltool(
    wine: str,
    executable: Path,
    source_copy: Path,
    output: Path,
) -> str:
    # Both paths are disposable build products.  The original ROM is never
    # handed to a third-party executable that opens its input read/write.
    command = [
        wine,
        str(executable.resolve()),
        "modcrypt",
        "--in",
        str(source_copy.resolve()),
        "--out",
        str(output.resolve()),
    ]
    try:
        completed = subprocess.run(
            command,
            check=False,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
    except OSError as error:
        raise PipelineError(f"failed to launch TWLTool: {error}") from error
    log = completed.stdout.replace("\r\n", "\n")
    if completed.returncode != 0:
        raise PipelineError(
            f"TWLTool failed with exit code {completed.returncode}:\n{log.rstrip()}"
        )
    if not output.is_file():
        raise PipelineError("TWLTool reported success but did not produce its output")
    return log


def _changed_offsets(before: bytes, after: bytes) -> Iterable[int]:
    if len(before) != len(after):
        raise PipelineError("decrypted ROM size differs from the verified source ROM")
    return (index for index, pair in enumerate(zip(before, after)) if pair[0] != pair[1])


def validate_decryption_changes(
    original: bytes,
    decrypted: bytes,
    header: dict[str, Any],
) -> None:
    allowed = [
        (SECURE_AREA_OFFSET, SECURE_AREA_OFFSET + SECURE_AREA_CRYPT_SIZE),
        *[
            (area["offset"], area["offset"] + area["size"])
            for area in header["modcrypt"]
            if area["offset"] and area["size"]
        ],
    ]
    for offset in _changed_offsets(original, decrypted):
        if not any(start <= offset < end for start, end in allowed):
            raise PipelineError(
                f"decryption unexpectedly changed ROM offset 0x{offset:X}"
            )
    if original[:0x1000] != decrypted[:0x1000]:
        raise PipelineError("DSi extended header changed during preparation")
    for label, start, end in [
        ("secure area", SECURE_AREA_OFFSET, SECURE_AREA_OFFSET + SECURE_AREA_CRYPT_SIZE),
        *[
            (f"modcrypt {area['index']}", area["offset"], area["offset"] + area["size"])
            for area in header["modcrypt"]
            if area["offset"] and area["size"]
        ],
    ]:
        if original[start:end] == decrypted[start:end]:
            raise PipelineError(f"{label} was not decrypted")


def _default_twltool_zip() -> Path:
    configured = os.environ.get("TWLTOOL_ZIP")
    if configured:
        return Path(configured).expanduser()
    return Path.home() / "Downloads" / "twltool-v1.6.zip"


def _resolve_inputs(args: argparse.Namespace) -> tuple[Path, Path, Path, Path]:
    return (
        args.rom.expanduser().resolve(),
        args.bios7.expanduser().resolve(),
        args.twltool_zip.expanduser().resolve(),
        args.output.expanduser().resolve(),
    )


def diagnose(args: argparse.Namespace) -> dict[str, Any]:
    rom_path, bios7_path, twltool_zip, output = _resolve_inputs(args)
    rom_info = _verify_file(
        rom_path,
        label="Four Swords Anniversary Edition EUR ROM",
        expected_sha256=TARGET_ROM_SHA256,
        expected_size=TARGET_ROM_SIZE,
    )
    bios_info = _verify_file(
        bios7_path,
        label="bios7.bin",
        expected_sha256=BIOS7_SHA256,
        expected_size=BIOS7_SIZE,
    )
    twltool_info = _verify_file(
        twltool_zip,
        label="TWLTool v1.6 archive",
        expected_sha256=TWLTOOL_ZIP_SHA256,
    )
    wine = _resolve_wine(args.wine)

    rom = rom_path.read_bytes()
    header = read_header(rom)
    validate_target_header(header, len(rom))
    table = extract_blowfish_table(bios7_path.read_bytes())
    table_sha = sha256_bytes(table)
    if table_sha != BLOWFISH_SHA256:
        raise PipelineError(
            f"extracted Blowfish table SHA-256 mismatch: got {table_sha}, "
            f"expected {BLOWFISH_SHA256}"
        )
    nitrofs_files = list_nitrofs(rom)
    return {
        "status": "ready",
        "inputs": {"rom": rom_info, "bios7": bios_info, "twltool_zip": twltool_info},
        "header": {
            "title": header["title"].decode("ascii"),
            "game_code": header["game_code"].decode("ascii"),
            "unit_code": header["unit_code"],
            "sections": header["sections"],
            "modcrypt": header["modcrypt"],
        },
        "blowfish_table": {"offset": BLOWFISH_OFFSET, "size": len(table), "sha256": table_sha},
        "nitrofs_file_count": len(nitrofs_files),
        "wine": wine,
        "output": str(output),
    }


def prepare(args: argparse.Namespace) -> dict[str, Any]:
    diagnostic = diagnose(args)
    if args.dry_run:
        diagnostic["status"] = "dry-run"
        return diagnostic

    rom_path, bios7_path, twltool_zip, output = _resolve_inputs(args)
    output_parent = output.parent
    output_parent.mkdir(parents=True, exist_ok=True)
    temporary = Path(tempfile.mkdtemp(prefix=".rom-pipeline-", dir=output_parent))
    try:
        tools_directory = temporary / "tools"
        keys_directory = temporary / "keys"
        keys_directory.mkdir(parents=True)
        executable = tools_directory / TWLTOOL_MEMBER
        tool_info = extract_twltool(twltool_zip, executable)

        original = rom_path.read_bytes()
        working = bytearray(original)
        table = extract_blowfish_table(bios7_path.read_bytes())
        table_path = keys_directory / "nds-blowfish.bin"
        table_path.write_bytes(table)
        decrypt_secure_area(working, table)

        secure_copy = temporary / "secure-area-decrypted.nds"
        secure_copy.write_bytes(working)
        decrypted_path = temporary / "decrypted.nds"
        log = run_twltool(diagnostic["wine"], executable, secure_copy, decrypted_path)
        (temporary / "twltool.log").write_text(log, encoding="utf-8")
        decrypted = decrypted_path.read_bytes()
        header = read_header(original)
        validate_decryption_changes(original, decrypted, header)
        decrypted_sha = sha256_bytes(decrypted)
        if decrypted_sha != TARGET_DECRYPTED_ROM_SHA256:
            raise PipelineError(
                f"decrypted ROM SHA-256 mismatch: got {decrypted_sha}, "
                f"expected {TARGET_DECRYPTED_ROM_SHA256}"
            )

        section_info = extract_sections(decrypted, temporary / "sections")
        nitrofs_info = extract_nitrofs(decrypted, temporary / "nitrofs")
        secure_copy.unlink()
        executable.unlink()
        tools_directory.rmdir()

        manifest = {
            "schema_version": 1,
            "target": {
                "title": header["title"].decode("ascii"),
                "game_code": header["game_code"].decode("ascii"),
                "unit_code": header["unit_code"],
            },
            "inputs": diagnostic["inputs"],
            "tools": {"twltool": tool_info},
            "derived": {
                "blowfish_table": {
                    "path": "keys/nds-blowfish.bin",
                    "offset": BLOWFISH_OFFSET,
                    "size": len(table),
                    "sha256": sha256_bytes(table),
                },
                "decrypted_rom": {
                    "path": "decrypted.nds",
                    "size": len(decrypted),
                    "sha256": decrypted_sha,
                },
                "sections": section_info,
                "nitrofs": nitrofs_info,
            },
        }
        (temporary / "manifest.json").write_text(
            json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8"
        )

        if output.exists():
            if not args.force:
                raise PipelineError(f"output already exists (pass --force to replace it): {output}")
            if output.is_dir() and not output.is_symlink():
                shutil.rmtree(output)
            else:
                output.unlink()
        temporary.rename(output)
        diagnostic["status"] = "prepared"
        diagnostic["manifest"] = str(output / "manifest.json")
        diagnostic["decrypted_rom_sha256"] = manifest["derived"]["decrypted_rom"]["sha256"]
        diagnostic["sections"] = len(section_info)
        diagnostic["nitrofs_file_count"] = len(nitrofs_info)
        return diagnostic
    except Exception:
        shutil.rmtree(temporary, ignore_errors=True)
        raise


def _add_common_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        "--rom",
        type=Path,
        default=Path("Zelda-_Four_Swords-Anniversary_Edition-Nintendo (EUR).nds"),
        help="path to the user-dumped EUR ROM",
    )
    parser.add_argument(
        "--bios7", type=Path, default=Path("bios7.bin"), help="path to a user-dumped DS bios7.bin"
    )
    parser.add_argument(
        "--twltool-zip",
        type=Path,
        default=_default_twltool_zip(),
        help="path to the official twltool-v1.6.zip archive",
    )
    parser.add_argument(
        "--output", type=Path, default=Path("build/rom"), help="generated output directory"
    )
    parser.add_argument("--wine", help="Wine executable (default: discover wine on PATH)")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    diagnose_parser = subparsers.add_parser(
        "diagnose", help="verify every input and print the extraction plan without writing"
    )
    _add_common_arguments(diagnose_parser)

    prepare_parser = subparsers.add_parser(
        "prepare", help="decrypt copies and extract code plus NitroFS"
    )
    _add_common_arguments(prepare_parser)
    prepare_parser.add_argument(
        "--dry-run", action="store_true", help="perform the same checks as diagnose without writing"
    )
    prepare_parser.add_argument(
        "--force", action="store_true", help="replace an existing generated output directory"
    )
    return parser


def _print_summary(result: dict[str, Any]) -> None:
    print(json.dumps(result, indent=2, sort_keys=True))


def main(argv: Sequence[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        if args.command == "diagnose":
            result = diagnose(args)
        else:
            result = prepare(args)
        _print_summary(result)
        return 0
    except PipelineError as error:
        print(f"ROM pipeline error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
