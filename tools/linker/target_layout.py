#!/usr/bin/env python3
"""Validate the ARM9/ARM9i link layout against the prepared target bytes."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import struct
from typing import Any, Sequence


PROJECT_ROOT = Path(__file__).resolve().parents[2]


class LayoutError(RuntimeError):
    """The configured linker layout is stale or internally inconsistent."""


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def read_json(path: Path, label: str) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise LayoutError(f"cannot read {label} {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise LayoutError(f"{label} must be a JSON object: {path}")
    return value


def integer(value: Any, label: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < 0:
        raise LayoutError(f"{label} must be a non-negative integer")
    return value


def object_value(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise LayoutError(f"{label} must be an object")
    return value


def array_value(value: Any, label: str) -> list[Any]:
    if not isinstance(value, list):
        raise LayoutError(f"{label} must be an array")
    return value


def metadata_for(executables: Any, image_id: str) -> dict[str, Any]:
    entries = array_value(executables, "binary map executables")
    matches = [entry for entry in entries if isinstance(entry, dict) and entry.get("id") == image_id]
    if len(matches) != 1:
        raise LayoutError(f"binary map must contain exactly one {image_id} entry")
    return matches[0]


def verify_target(target: dict[str, Any], data: bytes, label: str) -> None:
    size = integer(target.get("size"), f"{label} target size")
    digest = target.get("sha256")
    if not isinstance(digest, str) or len(digest) != 64:
        raise LayoutError(f"{label} target SHA-256 is invalid")
    if len(data) != size:
        raise LayoutError(f"{label} target size is {len(data)}, expected {size}")
    if sha256_bytes(data) != digest.lower():
        raise LayoutError(f"{label} target SHA-256 does not match the layout config")


def record_words(records: list[Any], label: str) -> list[int]:
    words: list[int] = []
    for index, value in enumerate(records):
        record = object_value(value, f"{label} record {index}")
        destination = integer(record.get("destination"), f"{label} record {index} destination")
        size = integer(record.get("size"), f"{label} record {index} size")
        static_init = integer(
            record.get("static_init_start"), f"{label} record {index} static init start"
        )
        bss_size = integer(record.get("bss_size"), f"{label} record {index} BSS size")
        if record.get("end") != destination + size:
            raise LayoutError(f"{label} record {index} initialized end is inconsistent")
        if record.get("bss_end") != destination + size + bss_size:
            raise LayoutError(f"{label} record {index} BSS end is inconsistent")
        words.extend((destination, size, static_init, bss_size))
    return words


def unpack_words(data: bytes, offset: int, count: int, label: str) -> list[int]:
    size = count * 4
    if offset + size > len(data):
        raise LayoutError(f"{label} extends beyond its target image")
    return list(struct.unpack_from(f"<{count}I", data, offset))


def validate_layout(
    config: dict[str, Any], binary_map: dict[str, Any], arm9: bytes, arm9i: bytes
) -> dict[str, Any]:
    if config.get("schema_version") != 1 or config.get("kind") != "arm9-link-layout":
        raise LayoutError("linker layout identity is invalid")

    targets = object_value(config.get("targets"), "layout targets")
    arm9_target = object_value(targets.get("arm9"), "ARM9 target")
    arm9i_target = object_value(targets.get("arm9i"), "ARM9i target")
    verify_target(arm9_target, arm9, "ARM9")
    verify_target(arm9i_target, arm9i, "ARM9i")

    mapped_arm9 = metadata_for(binary_map.get("executables"), "arm9")
    mapped_arm9i = metadata_for(binary_map.get("executables"), "arm9i")
    for label, target, mapped in (
        ("ARM9", arm9_target, mapped_arm9),
        ("ARM9i", arm9i_target, mapped_arm9i),
    ):
        rom = object_value(mapped.get("rom"), f"mapped {label} ROM metadata")
        if rom.get("size") != target.get("size") or rom.get("sha256") != target.get("sha256"):
            raise LayoutError(f"{label} layout target disagrees with the binary map")

    static = object_value(config.get("static"), "static layout")
    load = integer(static.get("load_address"), "static load address")
    entry = integer(static.get("entry_address"), "static entry address")
    initialized_size = integer(static.get("initialized_size"), "static initialized size")
    initialized_end = integer(static.get("initialized_end"), "static initialized end")
    bss_start = integer(static.get("bss_start"), "static BSS start")
    bss_size = integer(static.get("bss_size"), "static BSS size")
    bss_end = integer(static.get("bss_end"), "static BSS end")
    if load + initialized_size != initialized_end or initialized_end != bss_start:
        raise LayoutError("static initialized image and BSS boundary are inconsistent")
    if bss_start + bss_size != bss_end:
        raise LayoutError("static BSS end is inconsistent")
    memory = object_value(mapped_arm9.get("memory"), "mapped ARM9 memory metadata")
    module = object_value(mapped_arm9.get("module_params"), "mapped ARM9 module parameters")
    if memory.get("load_address") != load or memory.get("entry_address") != entry:
        raise LayoutError("static addresses disagree with the ARM9 binary map")
    if module.get("autoload_data_start") != initialized_end:
        raise LayoutError("static initialized end disagrees with ARM9 module parameters")
    if module.get("static_bss_start") != bss_start or module.get("static_bss_end") != bss_end:
        raise LayoutError("static BSS disagrees with ARM9 module parameters")

    autoload = object_value(config.get("autoload"), "ARM9 autoload layout")
    payload_offset = integer(autoload.get("payload_file_offset"), "ARM9 autoload payload offset")
    table_offset = integer(autoload.get("table_file_offset"), "ARM9 autoload table offset")
    table_size = integer(autoload.get("table_size"), "ARM9 autoload table size")
    records = array_value(autoload.get("records"), "ARM9 autoload records")
    words = record_words(records, "ARM9 autoload")
    if payload_offset != initialized_size:
        raise LayoutError("ARM9 autoload payload does not follow the initialized static image")
    if payload_offset + sum(integer(r.get("size"), "autoload size") for r in records) != table_offset:
        raise LayoutError("ARM9 autoload records do not end at the table")
    if table_size != len(words) * 4 or table_offset + table_size != len(arm9):
        raise LayoutError("ARM9 autoload table does not end at the image boundary")
    if unpack_words(arm9, table_offset, len(words), "ARM9 autoload table") != words:
        raise LayoutError("ARM9 autoload table bytes disagree with the configured records")
    if module.get("autoload_list_start") != load + table_offset:
        raise LayoutError("ARM9 autoload table address disagrees with module parameters")

    ltd = object_value(config.get("ltdautoload"), "LTD autoload layout")
    prefix_size = integer(ltd.get("file_prefix_size"), "LTD prefix size")
    ltd_payload_offset = integer(ltd.get("payload_file_offset"), "LTD payload offset")
    ltd_payload_size = integer(ltd.get("payload_size"), "LTD payload size")
    ltd_table_offset = integer(ltd.get("table_file_offset"), "LTD table offset")
    ltd_table_size = integer(ltd.get("table_size"), "LTD table size")
    ltd_records = array_value(ltd.get("records"), "LTD records")
    ltd_words = record_words(ltd_records, "LTD autoload")
    if prefix_size != ltd_payload_offset or any(arm9i[:prefix_size]):
        raise LayoutError("LTD file prefix is not the expected zero-filled container prefix")
    if ltd_payload_offset + ltd_payload_size != ltd_table_offset:
        raise LayoutError("LTD payload does not end at its autoload table")
    if ltd_table_size != len(ltd_words) * 4 or ltd_table_offset + ltd_table_size != len(arm9i):
        raise LayoutError("LTD table does not end at the image boundary")
    if unpack_words(arm9i, ltd_table_offset, len(ltd_words), "LTD table") != ltd_words:
        raise LayoutError("LTD table bytes disagree with the configured record")
    if not ltd_records or ltd_records[0].get("destination") != bss_end:
        raise LayoutError("LTDMAIN is not placed immediately after the static ARM9 BSS")
    mapped_ltd = object_value(mapped_arm9i.get("ltd_autoload"), "mapped ARM9i LTD metadata")
    if mapped_ltd.get("table_binary_offset") != ltd_table_offset:
        raise LayoutError("LTD table offset disagrees with the binary map")
    if mapped_ltd.get("payload_size") != ltd_payload_size:
        raise LayoutError("LTD payload size disagrees with the binary map")
    if mapped_ltd.get("records") != ltd_records:
        raise LayoutError("LTD record disagrees with the binary map")

    return {
        "status": "valid",
        "static_initialized_size": initialized_size,
        "static_bss_size": bss_size,
        "autoload_payload_size": table_offset - payload_offset,
        "ltd_payload_size": ltd_payload_size,
        "ltd_bss_size": integer(ltd_records[0].get("bss_size"), "LTD BSS size"),
    }


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--config", type=Path, default=PROJECT_ROOT / "config/linker/arm9.json")
    parser.add_argument("--binary-map", type=Path, default=PROJECT_ROOT / "config/binary-map.json")
    parser.add_argument("--arm9", type=Path, default=PROJECT_ROOT / "build/rom/sections/arm9.bin")
    parser.add_argument("--arm9i", type=Path, default=PROJECT_ROOT / "build/rom/sections/arm9i.bin")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        report = validate_layout(
            read_json(args.config, "layout config"),
            read_json(args.binary_map, "binary map"),
            args.arm9.read_bytes(),
            args.arm9i.read_bytes(),
        )
    except (LayoutError, OSError) as exc:
        print(f"linker layout error: {exc}", file=__import__("sys").stderr)
        return 2
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
