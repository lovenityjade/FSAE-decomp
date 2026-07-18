#!/usr/bin/env python3
"""Recover Itanium C++ RTTI classes and virtual tables from the ARM9 image."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import shutil
import struct
import subprocess
from typing import Any, Callable, Sequence


PROJECT_ROOT = Path(__file__).resolve().parents[2]
TYPE_NAME = re.compile(rb"N[0-9][A-Za-z0-9_]*E\x00")


class RttiError(RuntimeError):
    """The image contains inconsistent RTTI metadata."""


def words(image: bytes) -> list[int]:
    return [struct.unpack_from("<I", image, offset)[0] for offset in range(0, len(image) - 3, 4)]


def default_demangle(raw_name: str) -> str:
    tool = shutil.which("c++filt")
    if not tool:
        return raw_name
    completed = subprocess.run(
        [tool, "_ZTS" + raw_name], text=True, capture_output=True, check=False
    )
    value = completed.stdout.strip()
    prefix = "typeinfo name for "
    return value.removeprefix(prefix) if completed.returncode == 0 and value else raw_name


def function_index(export: dict[str, Any] | None) -> dict[int, dict[str, Any]]:
    if export is None:
        return {}
    functions = export.get("functions")
    if not isinstance(functions, list):
        raise RttiError("Ghidra export functions must be an array")
    result: dict[int, dict[str, Any]] = {}
    for item in functions:
        if not isinstance(item, dict) or not isinstance(item.get("entry"), str):
            raise RttiError("Ghidra export contains invalid function metadata")
        try:
            address = int(item["entry"], 16)
        except ValueError as exc:
            raise RttiError(f"invalid Ghidra function entry {item['entry']!r}") from exc
        result[address] = item
    return result


def recover_rtti(
    image: bytes,
    load_address: int,
    code_start: int,
    code_end: int,
    export: dict[str, Any] | None = None,
    demangle: Callable[[str], str] = default_demangle,
) -> dict[str, Any]:
    if load_address % 4 or code_start >= code_end:
        raise RttiError("invalid image or code address range")
    image_words = words(image)
    functions = function_index(export)
    classes: list[dict[str, Any]] = []

    def occurrences(value: int) -> list[int]:
        return [load_address + index * 4 for index, word in enumerate(image_words) if word == value]

    def read_word(address: int) -> int:
        offset = address - load_address
        if offset < 0 or offset + 4 > len(image) or offset % 4:
            raise RttiError(f"unaligned RTTI address outside image: {address:#010x}")
        return image_words[offset // 4]

    for match in TYPE_NAME.finditer(image):
        raw_name = match.group()[:-1].decode("ascii")
        name_address = load_address + match.start()
        name_references = occurrences(name_address)
        if not name_references:
            continue
        for name_reference in name_references:
            typeinfo_address = name_reference - 4
            headers = [
                reference - 4
                for reference in occurrences(typeinfo_address)
                if reference >= load_address + 4 and read_word(reference - 4) == 0
            ]
            for header in headers:
                address_point = header + 8
                virtuals: list[dict[str, Any]] = []
                cursor = address_point
                while cursor + 4 <= load_address + len(image):
                    target = read_word(cursor)
                    canonical_target = target & ~1
                    if not code_start <= canonical_target < code_end:
                        break
                    recovered = functions.get(canonical_target, {})
                    item: dict[str, Any] = {
                        "slot": len(virtuals),
                        "address": canonical_target,
                        "isa": "Thumb" if target & 1 else "ARM",
                    }
                    if isinstance(recovered.get("name"), str):
                        item["recovered_name"] = recovered["name"]
                    if isinstance(recovered.get("body_bytes"), int):
                        item["body_bytes"] = recovered["body_bytes"]
                    virtuals.append(item)
                    cursor += 4
                if not virtuals:
                    continue
                classes.append(
                    {
                        "name": demangle(raw_name),
                        "encoded_name": raw_name,
                        "type_name_address": name_address,
                        "typeinfo_address": typeinfo_address,
                        "vtable_header": header,
                        "vtable_address_point": address_point,
                        "virtual_functions": virtuals,
                    }
                )

    classes.sort(key=lambda item: (item["vtable_header"], item["name"]))
    unique_virtuals = {
        function["address"] for item in classes for function in item["virtual_functions"]
    }
    return {
        "schema_version": 1,
        "kind": "arm9-rtti-catalog",
        "load_address": load_address,
        "code_range": {"start": code_start, "end": code_end},
        "summary": {
            "classes": len(classes),
            "vtable_slots": sum(len(item["virtual_functions"]) for item in classes),
            "unique_virtual_functions": len(unique_virtuals),
        },
        "classes": classes,
    }


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--image", type=Path, default=PROJECT_ROOT / "build/rom/sections/arm9.bin")
    parser.add_argument(
        "--export", type=Path, default=PROJECT_ROOT / "build/decomp/arm9/recovered.json"
    )
    parser.add_argument("--load-address", type=lambda value: int(value, 0), default=0x02004000)
    parser.add_argument("--code-start", type=lambda value: int(value, 0), default=0x02004000)
    parser.add_argument("--code-end", type=lambda value: int(value, 0), default=0x020DF530)
    parser.add_argument("--output", type=Path, default=PROJECT_ROOT / "build/decomp/arm9/rtti.json")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        export = json.loads(args.export.read_text(encoding="utf-8"))
        if not isinstance(export, dict):
            raise RttiError("Ghidra export must be a JSON object")
        report = recover_rtti(
            args.image.read_bytes(),
            args.load_address,
            args.code_start,
            args.code_end,
            export,
        )
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    except (OSError, json.JSONDecodeError, RttiError) as exc:
        print(f"RTTI catalog error: {exc}", file=__import__("sys").stderr)
        return 2
    print(json.dumps(report["summary"], indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
