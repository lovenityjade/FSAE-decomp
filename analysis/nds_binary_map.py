#!/usr/bin/env python3
"""Build reproducible metadata maps for a Nintendo DS/TWL SRL image.

The script only reads the ROM and emits hashes, offsets, sizes and filenames.
It never extracts executable or NitroFS payloads.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path
from typing import Any


def u16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def region(offset: int, size: int) -> dict[str, int]:
    return {"offset": offset, "size": size, "end": offset + size}


def parse_fnt(rom: bytes, fnt_offset: int, fnt_size: int, fat_offset: int,
              fat_size: int) -> tuple[list[dict[str, Any]], dict[str, Any]]:
    fnt = rom[fnt_offset:fnt_offset + fnt_size]
    file_count = fat_size // 8
    root_subtable, root_first_file_id, directory_count = struct.unpack_from(
        "<IHH", fnt, 0
    )
    if directory_count == 0 or directory_count * 8 > len(fnt):
        raise ValueError("invalid NitroFS directory table")

    directories: dict[int, tuple[int, int, int]] = {}
    for index in range(directory_count):
        subtable, first_file_id, parent = struct.unpack_from("<IHH", fnt, index * 8)
        directories[0xF000 + index] = (subtable, first_file_id, parent)

    def directory_entries(directory_id: int) -> list[tuple[str, str, int]]:
        subtable, file_id, _ = directories[directory_id]
        pos = subtable
        entries: list[tuple[str, str, int]] = []
        while True:
            if pos >= len(fnt):
                raise ValueError("unterminated NitroFS directory subtable")
            type_length = fnt[pos]
            pos += 1
            if type_length == 0:
                return entries
            name_length = type_length & 0x7F
            name = fnt[pos:pos + name_length].decode("ascii")
            pos += name_length
            if type_length & 0x80:
                child_id = u16(fnt, pos)
                pos += 2
                entries.append(("directory", name, child_id))
            else:
                entries.append(("file", name, file_id))
                file_id += 1

    files: list[dict[str, Any]] = []

    def walk(directory_id: int, prefix: str = "") -> None:
        for entry_type, name, value in directory_entries(directory_id):
            path = f"{prefix}{name}"
            if entry_type == "directory":
                walk(value, f"{path}/")
                continue
            if value >= file_count:
                raise ValueError(f"NitroFS file ID {value} exceeds FAT")
            start, end = struct.unpack_from("<II", rom, fat_offset + value * 8)
            if not (0 <= start <= end <= len(rom)):
                raise ValueError(f"invalid FAT range for NitroFS file ID {value}")
            payload = rom[start:end]
            files.append(
                {
                    "id": value,
                    "path": path,
                    "rom": region(start, end - start),
                    "sha256": sha256(payload),
                }
            )

    walk(0xF000)
    files.sort(key=lambda entry: entry["id"])
    if len(files) != file_count:
        raise ValueError(f"FNT names {len(files)} files, FAT contains {file_count}")
    metadata = {
        "file_count": file_count,
        "directory_count": directory_count,
        "root_directory_id": 0xF000,
        "root_subtable_offset": root_subtable,
        "root_first_file_id": root_first_file_id,
    }
    return files, metadata


def parse_module_params(image: bytes, offset: int, load_address: int) -> dict[str, Any]:
    if offset + 9 * 4 > len(image):
        raise ValueError("module-parameter offset lies outside executable")
    values = struct.unpack_from("<9I", image, offset)
    version_word = values[6]
    release_step = version_word & 0xFFFF
    release_class = release_step // 10000
    release_number = release_step % 10000
    release_names = {0: "test", 1: "pr", 2: "rc", 3: "release"}
    result: dict[str, Any] = {
        "binary_offset": offset,
        "memory_address": load_address + offset,
        "autoload_list_start": values[0],
        "autoload_list_end": values[1],
        "autoload_data_start": values[2],
        "static_bss_start": values[3],
        "static_bss_end": values[4],
        "compressed_static_end": values[5],
        "sdk_version_word": version_word,
        "sdk_version": {
            "major": (version_word >> 24) & 0xFF,
            "minor": (version_word >> 16) & 0xFF,
            "release_step": release_step,
            "release_class": release_names.get(release_class, "unknown"),
            "release_number": release_number,
        },
        "version_magic_be": values[7],
        "version_magic_le": values[8],
    }
    if values[1] < values[0] or values[0] < load_address:
        raise ValueError("invalid autoload list pointers in module parameters")
    table_offset = values[0] - load_address
    table_size = values[1] - values[0]
    if table_offset + table_size > len(image) or table_size % 16:
        raise ValueError("invalid autoload table in module parameters")
    records: list[dict[str, int]] = []
    for pos in range(table_offset, table_offset + table_size, 16):
        destination, size, static_init_start, bss_size = struct.unpack_from(
            "<IIII", image, pos
        )
        records.append(
            {
                "destination": destination,
                "size": size,
                "end": destination + size,
                "static_init_start": static_init_start,
                "bss_size": bss_size,
                "bss_end": destination + size + bss_size,
            }
        )
    result["autoload_records"] = records
    return result


def find_ltd_autoload_records(image: bytes) -> dict[str, Any] | None:
    """Locate a TwlSDK LTD autoload table packed at the image tail.

    Each record contains destination, initialized size, static-initializer start
    and BSS size.  The initialized payload precedes the table; up to 31 bytes
    of alignment are allowed between them.
    """
    for record_count in range(1, 9):
        table_offset = len(image) - record_count * 16
        if table_offset < 0:
            break
        records: list[dict[str, int]] = []
        total_payload = 0
        valid = True
        for pos in range(table_offset, len(image), 16):
            destination, size, static_init_start, bss_size = struct.unpack_from(
                "<IIII", image, pos
            )
            if not (0x01000000 <= destination < 0x04000000 and size > 0):
                valid = False
                break
            if not (destination <= static_init_start <= destination + size):
                valid = False
                break
            total_payload += size
            records.append(
                {
                    "destination": destination,
                    "size": size,
                    "end": destination + size,
                    "static_init_start": static_init_start,
                    "bss_size": bss_size,
                    "bss_end": destination + size + bss_size,
                }
            )
        alignment_gap = table_offset - total_payload
        if valid and 0 <= alignment_gap < 32:
            return {
                "table_binary_offset": table_offset,
                "table_size": record_count * 16,
                "payload_size": total_payload,
                "alignment_gap": alignment_gap,
                "records": records,
            }
    return None


def first_difference_window(left: bytes, right: bytes) -> dict[str, int] | None:
    differences = [index for index, pair in enumerate(zip(left, right)) if pair[0] != pair[1]]
    if not differences:
        return None
    return {
        "first_difference": differences[0],
        "last_difference_inclusive": differences[-1],
        "different_byte_count": len(differences),
    }


def build_maps(rom: bytes, encrypted_rom: bytes | None) -> tuple[dict[str, Any], dict[str, Any]]:
    if len(rom) < 0x4000:
        raise ValueError("input is too small to contain a TWL header")
    title = rom[0:12].rstrip(b"\0").decode("ascii")
    game_code = rom[12:16].decode("ascii")
    maker_code = rom[16:18].decode("ascii")

    arm9_offset, arm9_entry, arm9_load, arm9_size = struct.unpack_from("<IIII", rom, 0x20)
    arm7_offset, arm7_entry, arm7_load, arm7_size = struct.unpack_from("<IIII", rom, 0x30)
    fnt_offset, fnt_size, fat_offset, fat_size = struct.unpack_from("<IIII", rom, 0x40)
    arm9_ovt_offset, arm9_ovt_size, arm7_ovt_offset, arm7_ovt_size = struct.unpack_from(
        "<IIII", rom, 0x50
    )
    banner_offset = u32(rom, 0x68)
    arm9_module_params_offset = u32(rom, 0x88)
    arm7_module_params_offset = u32(rom, 0x8C)
    arm9i_offset, arm9i_header_word, arm9i_load, arm9i_size = struct.unpack_from(
        "<IIII", rom, 0x1C0
    )
    arm7i_offset, arm7i_header_word, arm7i_load, arm7i_size = struct.unpack_from(
        "<IIII", rom, 0x1D0
    )
    digest_ntr_offset, digest_ntr_size = struct.unpack_from("<II", rom, 0x1E0)
    digest_twl_offset, digest_twl_size = struct.unpack_from("<II", rom, 0x1E8)
    sector_hash_offset, sector_hash_size = struct.unpack_from("<II", rom, 0x1F0)
    block_hash_offset, block_hash_size = struct.unpack_from("<II", rom, 0x1F8)
    digest_sector_size, digest_block_sector_count = struct.unpack_from("<II", rom, 0x200)
    banner_size = u32(rom, 0x208)
    modcrypt1_offset, modcrypt1_size, modcrypt2_offset, modcrypt2_size = struct.unpack_from(
        "<IIII", rom, 0x220
    )

    images = {
        "arm9": rom[arm9_offset:arm9_offset + arm9_size],
        "arm7": rom[arm7_offset:arm7_offset + arm7_size],
        "arm9i": rom[arm9i_offset:arm9i_offset + arm9i_size],
        "arm7i": rom[arm7i_offset:arm7i_offset + arm7i_size],
    }
    for image_id, payload in images.items():
        expected = {"arm9": arm9_size, "arm7": arm7_size, "arm9i": arm9i_size,
                    "arm7i": arm7i_size}[image_id]
        if len(payload) != expected:
            raise ValueError(f"{image_id} extends beyond input")

    arm9_params = parse_module_params(images["arm9"], arm9_module_params_offset, arm9_load)
    arm7_params = parse_module_params(images["arm7"], arm7_module_params_offset, arm7_load)
    arm9i_autoload = find_ltd_autoload_records(images["arm9i"])
    arm7i_autoload = find_ltd_autoload_records(images["arm7i"])

    files, fs_metadata = parse_fnt(rom, fnt_offset, fnt_size, fat_offset, fat_size)
    nitrofs = {
        "schema_version": 1,
        "source_sha256": sha256(rom),
        **fs_metadata,
        "files": files,
    }

    executables: list[dict[str, Any]] = [
        {
            "id": "arm9",
            "processor": "ARM946E-S",
            "role": "NTR-compatible main image",
            "rom": {**region(arm9_offset, arm9_size), "sha256": sha256(images["arm9"])},
            "memory": {
                "load_address": arm9_load,
                "loaded_end": arm9_load + arm9_size,
                "entry_address": arm9_entry,
                "entry_binary_offset": arm9_entry - arm9_load,
            },
            "entry_isa": "ARM",
            "module_params": arm9_params,
            "encryption": {
                "scheme": "NTR secure-area Blowfish",
                "secure_area_size": 0x4000,
                "encrypted_prefix_size": 0x800,
                "state": "decrypted input required",
            },
        },
        {
            "id": "arm7",
            "processor": "ARM7TDMI",
            "role": "NTR-compatible sub image",
            "rom": {**region(arm7_offset, arm7_size), "sha256": sha256(images["arm7"])},
            "memory": {
                "load_address": arm7_load,
                "loaded_end": arm7_load + arm7_size,
                "entry_address": arm7_entry,
                "entry_binary_offset": arm7_entry - arm7_load,
            },
            "entry_isa": "ARM",
            "module_params": arm7_params,
            "encryption": {"scheme": None, "state": "plaintext in retail SRL"},
        },
        {
            "id": "arm9i",
            "processor": "ARM946E-S",
            "role": "TWL/LTD main extension staging image",
            "rom": {**region(arm9i_offset, arm9i_size), "sha256": sha256(images["arm9i"])},
            "memory": {"staging_load_address": arm9i_load,
                       "staging_loaded_end": arm9i_load + arm9i_size},
            "header_reserved_or_entry_word": arm9i_header_word,
            "initial_isa": "Thumb",
            "initial_code_binary_offset": 0x20,
            "ltd_autoload": arm9i_autoload,
            "encryption": {
                "scheme": "TWL modcrypt area 0 (AES-CTR)",
                "encrypted_region": region(modcrypt1_offset, modcrypt1_size),
                "state": "decrypted input required",
            },
        },
        {
            "id": "arm7i",
            "processor": "ARM7TDMI",
            "role": "TWL/LTD sub extension staging image",
            "rom": {**region(arm7i_offset, arm7i_size), "sha256": sha256(images["arm7i"])},
            "memory": {"staging_load_address": arm7i_load,
                       "staging_loaded_end": arm7i_load + arm7i_size},
            "header_reserved_or_entry_word": arm7i_header_word,
            "initial_isa": "ARM",
            "initial_code_binary_offset": 0x4,
            "ltd_autoload": arm7i_autoload,
            "encryption": {
                "scheme": "TWL modcrypt area 1 (AES-CTR)",
                "encrypted_region": region(modcrypt2_offset, modcrypt2_size),
                "state": "decrypted input required",
            },
        },
    ]

    if encrypted_rom is not None:
        if len(encrypted_rom) != len(rom) or encrypted_rom[:0x20] != rom[:0x20]:
            raise ValueError("encrypted comparison image is not the same SRL identity/size")
        comparison_ranges = {
            "arm9": (arm9_offset, arm9_size),
            "arm7": (arm7_offset, arm7_size),
            "arm9i": (arm9i_offset, arm9i_size),
            "arm7i": (arm7i_offset, arm7i_size),
        }
        by_id = {entry["id"]: entry for entry in executables}
        for image_id, (offset, size) in comparison_ranges.items():
            encrypted_payload = encrypted_rom[offset:offset + size]
            by_id[image_id]["encrypted_sha256"] = sha256(encrypted_payload)
            by_id[image_id]["encrypted_comparison"] = first_difference_window(
                encrypted_payload, images[image_id]
            )

    layout = [
        {"id": "header", **region(0, u32(rom, 0x84)), "kind": "metadata"},
        {"id": "arm9", **region(arm9_offset, arm9_size), "kind": "executable"},
        {"id": "arm7", **region(arm7_offset, arm7_size), "kind": "executable"},
        {"id": "fnt", **region(fnt_offset, fnt_size), "kind": "filesystem_table"},
        {"id": "fat", **region(fat_offset, fat_size), "kind": "filesystem_table"},
        {"id": "banner", **region(banner_offset, banner_size), "kind": "metadata"},
        {"id": "nitrofs_payload_span", **region(files[0]["rom"]["offset"],
                                                    files[-1]["rom"]["end"] - files[0]["rom"]["offset"]),
         "kind": "filesystem_payload"},
        {"id": "digest_sector_hash_table", **region(sector_hash_offset, sector_hash_size),
         "kind": "digest"},
        {"id": "digest_block_hash_table", **region(block_hash_offset, block_hash_size),
         "kind": "digest"},
        {"id": "arm9i", **region(arm9i_offset, arm9i_size), "kind": "executable"},
        {"id": "arm7i", **region(arm7i_offset, arm7i_size), "kind": "executable"},
    ]
    layout.sort(key=lambda entry: (entry["offset"], entry["size"]))

    binary_map: dict[str, Any] = {
        "schema_version": 1,
        "source": {"size": len(rom), "sha256": sha256(rom)},
        "identity": {
            "title": title,
            "game_code": game_code,
            "maker_code": maker_code,
            "unit_code": rom[0x12],
            "device_capacity_code": rom[0x14],
            "revision": rom[0x1E],
            "header_size": u32(rom, 0x84),
            "ntr_application_end": u32(rom, 0x80),
        },
        "executables": executables,
        "overlays": {
            "arm9": {"table": region(arm9_ovt_offset, arm9_ovt_size),
                     "count": arm9_ovt_size // 32},
            "arm7": {"table": region(arm7_ovt_offset, arm7_ovt_size),
                     "count": arm7_ovt_size // 32},
        },
        "filesystem": {
            "fnt": region(fnt_offset, fnt_size),
            "fat": region(fat_offset, fat_size),
            **fs_metadata,
        },
        "digests": {
            "ntr_covered_region": region(digest_ntr_offset, digest_ntr_size),
            "twl_covered_region": region(digest_twl_offset, digest_twl_size),
            "sector_hash_table": region(sector_hash_offset, sector_hash_size),
            "block_hash_table": region(block_hash_offset, block_hash_size),
            "sector_size": digest_sector_size,
            "block_sector_count": digest_block_sector_count,
        },
        "modcrypt": {
            "area_0": region(modcrypt1_offset, modcrypt1_size),
            "area_1": region(modcrypt2_offset, modcrypt2_size),
        },
        "layout": layout,
        "observations": [
            "All NTR and TWL overlay table sizes are zero.",
            "ARM9/ARM7 module parameters contain the TwlSDK version marker.",
            "ARM9i/ARM7i are staging images whose tail tables describe LTD autoload destinations.",
            "ISA labels identify confirmed entry/initial code only; interworking inside images remains to map.",
        ],
    }
    if encrypted_rom is not None:
        binary_map["source"]["encrypted_comparison_sha256"] = sha256(encrypted_rom)
    return binary_map, nitrofs


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("rom", type=Path, help="fully decrypted analysis SRL")
    parser.add_argument("--encrypted-rom", type=Path,
                        help="optional untouched SRL used only for difference metadata")
    parser.add_argument("--binary-map", type=Path, default=Path("config/binary-map.json"))
    parser.add_argument("--nitrofs-map", type=Path, default=Path("config/nitrofs.json"))
    args = parser.parse_args()

    binary_map, nitrofs = build_maps(
        args.rom.read_bytes(),
        args.encrypted_rom.read_bytes() if args.encrypted_rom else None,
    )
    for output, payload in ((args.binary_map, binary_map), (args.nitrofs_map, nitrofs)):
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(json.dumps(payload, indent=2, sort_keys=False) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
