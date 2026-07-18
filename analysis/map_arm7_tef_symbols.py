#!/usr/bin/env python3
"""Map symbols from the official racoon TEF onto ARM7/ARM7i images.

TwlSDK TEF files retain a complete ELF symbol table.  This utility consumes
only that metadata and the project's binary map; it does not copy SDK code.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
from bisect import bisect_right
from pathlib import Path
from typing import Any


def c_string(data: bytes, offset: int) -> str:
    end = data.find(b"\0", offset)
    if end < 0:
        end = len(data)
    return data[offset:end].decode("utf-8", errors="replace")


def parse_elf32_symbols(data: bytes) -> list[dict[str, Any]]:
    if data[:7] != b"\x7fELF\x01\x01\x01":
        raise ValueError("expected a 32-bit little-endian ELF file")
    if struct.unpack_from("<H", data, 18)[0] != 40:
        raise ValueError("expected an ARM ELF file")
    section_offset = struct.unpack_from("<I", data, 32)[0]
    section_entry_size, section_count = struct.unpack_from("<HH", data, 46)
    sections = [
        struct.unpack_from("<10I", data, section_offset + index * section_entry_size)
        for index in range(section_count)
    ]
    symbols: list[dict[str, Any]] = []
    for section_index, section in enumerate(sections):
        section_type, offset, size, link, entry_size = (
            section[1], section[4], section[5], section[6], section[9]
        )
        if section_type != 2:  # SHT_SYMTAB
            continue
        if not entry_size:
            entry_size = 16
        string_section = sections[link]
        strings = data[string_section[4]:string_section[4] + string_section[5]]
        for pos in range(offset, offset + size, entry_size):
            name_offset, value, symbol_size, info, other, shndx = struct.unpack_from(
                "<IIIBBH", data, pos
            )
            symbols.append(
                {
                    "name": c_string(strings, name_offset),
                    "address": value,
                    "size": symbol_size,
                    "type": info & 0xF,
                    "bind": info >> 4,
                    "visibility": other & 0x3,
                    "section_index": shndx,
                    "symbol_table_section": section_index,
                }
            )
    if not symbols:
        raise ValueError("ELF contains no symbol table")
    return symbols


def executable(binary_map: dict[str, Any], image_id: str) -> dict[str, Any]:
    return next(entry for entry in binary_map["executables"] if entry["id"] == image_id)


def build_memory_maps(binary_map: dict[str, Any]) -> list[dict[str, Any]]:
    arm7 = executable(binary_map, "arm7")
    arm7i = executable(binary_map, "arm7i")
    mappings: list[dict[str, Any]] = []

    load = arm7["memory"]["load_address"]
    static_end = arm7["module_params"]["static_bss_start"]
    mappings.append(
        {
            "image": "arm7",
            "memory_start": load,
            "memory_end": static_end,
            "binary_offset": 0,
            "kind": "static",
        }
    )
    payload_offset = arm7["module_params"]["autoload_data_start"] - load
    for record in arm7["module_params"]["autoload_records"]:
        mappings.append(
            {
                "image": "arm7",
                "memory_start": record["destination"],
                "memory_end": record["end"],
                "binary_offset": payload_offset,
                "kind": "autoload",
            }
        )
        payload_offset += record["size"]

    payload_offset = 0
    for record in arm7i["ltd_autoload"]["records"]:
        mappings.append(
            {
                "image": "arm7i",
                "memory_start": record["destination"],
                "memory_end": record["end"],
                "binary_offset": payload_offset,
                "kind": "ltd_autoload",
            }
        )
        payload_offset += record["size"]
    return mappings


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("tef", type=Path, help="official racoon Rom TEF")
    parser.add_argument("--binary-map", type=Path, default=Path("config/binary-map.json"))
    parser.add_argument("--output", type=Path, default=Path("config/sdk-symbols-arm7.json"))
    args = parser.parse_args()

    tef_data = args.tef.read_bytes()
    symbols = parse_elf32_symbols(tef_data)
    binary_map = json.loads(args.binary_map.read_text())
    memory_maps = build_memory_maps(binary_map)

    mapping_symbols: dict[int, list[tuple[int, str]]] = {}
    for symbol in symbols:
        if symbol["name"] in ("$a", "$t", "$d"):
            mapping_symbols.setdefault(symbol["section_index"], []).append(
                (symbol["address"], symbol["name"])
            )
    for entries in mapping_symbols.values():
        entries.sort()

    bind_names = {0: "local", 1: "global", 2: "weak"}
    isa_names = {"$a": "ARM", "$t": "Thumb", "$d": "data"}
    output_symbols: list[dict[str, Any]] = []
    for symbol in symbols:
        if symbol["type"] != 2 or symbol["size"] == 0 or symbol["name"].startswith("$"):
            continue
        address = symbol["address"]
        mapping = next(
            (entry for entry in memory_maps
             if entry["memory_start"] <= address < entry["memory_end"]),
            None,
        )
        if mapping is None or address + symbol["size"] > mapping["memory_end"]:
            continue
        markers = mapping_symbols.get(symbol["section_index"], [])
        marker_addresses = [entry[0] for entry in markers]
        marker_index = bisect_right(marker_addresses, address) - 1
        isa = "unknown"
        if marker_index >= 0:
            isa = isa_names.get(markers[marker_index][1], "unknown")
        output_symbols.append(
            {
                "name": symbol["name"],
                "image": mapping["image"],
                "address": address,
                "size": symbol["size"],
                "end": address + symbol["size"],
                "binary_offset": mapping["binary_offset"] + address - mapping["memory_start"],
                "isa": isa,
                "bind": bind_names.get(
                    symbol["bind"], f"processor_specific_{symbol['bind']}"
                ),
                "memory_region_kind": mapping["kind"],
            }
        )
    output_symbols.sort(key=lambda entry: (entry["image"], entry["address"], entry["name"]))

    payload = {
        "schema_version": 1,
        "source_tef_sha256": hashlib.sha256(tef_data).hexdigest(),
        "source_component": "TwlSDK 5.5 patch 5 racoon/ARM7-TS.LTD/Rom/racoon.tef",
        "mapping_basis": {
            "arm7": "Patch-5 FLX image matches the target prefix except the SDK patch byte; target omits a 16-byte SDK footer.",
            "arm7i": "Patch-5 LTD sbin SHA-256 is identical to the decrypted target ARM7i image.",
        },
        "memory_maps": memory_maps,
        "symbol_count": len(output_symbols),
        "symbols": output_symbols,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(payload, indent=2) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
