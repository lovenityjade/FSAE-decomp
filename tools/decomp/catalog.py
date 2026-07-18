#!/usr/bin/env python3
"""Classify a Ghidra ARM9 export into SDK, game, and switch-artifact records.

The generated C shards remain analysis material under ``build/``.  They are
deliberately not treated as compilable or matching source.
"""

from __future__ import annotations

import argparse
from collections import defaultdict
import csv
import hashlib
import json
from pathlib import Path
import re
from typing import Any, Sequence


PROJECT_ROOT = Path(__file__).resolve().parents[2]
FUNCTION_BLOCK = re.compile(
    r"(?ms)^/\* ={64}\n"
    r" \* (?P<name>.+?) @ (?P<entry>[0-9a-fA-F]+)\n"
    r" \* ={64} \*/\n"
    r"(?P<body>.*?)"
    r"(?=^/\* ={64}\n \* |\Z)"
)


class CatalogError(RuntimeError):
    """An export or signature file is stale or malformed."""


def read_json(path: Path, label: str) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise CatalogError(f"cannot read {label} {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise CatalogError(f"{label} must be a JSON object: {path}")
    return value


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def parse_entry(value: Any, label: str) -> int:
    if not isinstance(value, str) or not re.fullmatch(r"[0-9a-fA-F]+", value):
        raise CatalogError(f"{label} must be a hexadecimal address")
    return int(value, 16)


def parse_c_blocks(text: str) -> dict[int, str]:
    blocks: dict[int, str] = {}
    for match in FUNCTION_BLOCK.finditer(text):
        address = int(match.group("entry"), 16)
        if address in blocks:
            raise CatalogError(f"duplicate recovered C block at {address:#010x}")
        blocks[address] = match.group(0).rstrip() + "\n"
    if not blocks:
        raise CatalogError("recovered C contains no recognized function blocks")
    return blocks


def signature_boundaries(signatures: dict[str, Any]) -> dict[int, list[dict[str, Any]]]:
    if signatures.get("schema_version") != 1 or signatures.get("image") != "arm9":
        raise CatalogError("SDK signature identity is invalid")
    matches = signatures.get("matches")
    if not isinstance(matches, list):
        raise CatalogError("SDK signature matches must be an array")
    boundaries: dict[int, list[dict[str, Any]]] = defaultdict(list)
    for index, match in enumerate(matches):
        if not isinstance(match, dict):
            raise CatalogError(f"SDK signature match {index} is invalid")
        address = match.get("address")
        size = match.get("size")
        name = match.get("name")
        if (
            isinstance(address, bool)
            or not isinstance(address, int)
            or isinstance(size, bool)
            or not isinstance(size, int)
            or size < 1
            or not isinstance(name, str)
        ):
            raise CatalogError(f"SDK signature match {index} has invalid boundary metadata")
        boundaries[address].append(match)
    return dict(boundaries)


def classify_export(
    export: dict[str, Any], signatures: dict[str, Any], c_blocks: dict[int, str]
) -> tuple[dict[str, Any], dict[int, list[dict[str, Any]]]]:
    if export.get("schema_version") != 1 or export.get("program") != "arm9.analysis.elf":
        raise CatalogError("Ghidra export identity is invalid")
    functions = export.get("functions")
    if not isinstance(functions, list):
        raise CatalogError("Ghidra export functions must be an array")
    if export.get("selected_functions") != len(functions):
        raise CatalogError("Ghidra selected function count is stale")

    sdk = signature_boundaries(signatures)
    seen: set[int] = set()
    classified: list[dict[str, Any]] = []
    class_counts: dict[str, int] = defaultdict(int)
    class_bytes: dict[str, int] = defaultdict(int)
    shards: dict[int, list[dict[str, Any]]] = defaultdict(list)

    for index, function in enumerate(functions):
        if not isinstance(function, dict):
            raise CatalogError(f"Ghidra function {index} is invalid")
        address = parse_entry(function.get("entry"), f"Ghidra function {index} entry")
        if address in seen:
            raise CatalogError(f"duplicate Ghidra function at {address:#010x}")
        seen.add(address)
        if address not in c_blocks:
            raise CatalogError(f"missing recovered C block at {address:#010x}")
        name = function.get("name")
        body_bytes = function.get("body_bytes")
        if not isinstance(name, str) or isinstance(body_bytes, bool) or not isinstance(body_bytes, int):
            raise CatalogError(f"Ghidra function {index} metadata is invalid")
        if address in sdk:
            classification = "sdk"
        elif name.startswith("caseD_"):
            classification = "switch_artifact"
        else:
            classification = "game_candidate"
        item = {
            "name": name,
            "entry": f"0x{address:08x}",
            "file_offset": address - int(signatures.get("load_address", 0)),
            "body_bytes": body_bytes,
            "decompiled": function.get("decompiled") is True,
            "classification": classification,
        }
        if classification == "sdk":
            candidates = sorted({str(match["name"]) for match in sdk[address]})
            item["sdk_candidates"] = candidates
        elif classification == "game_candidate":
            shard = address >> 16
            item["raw_shard"] = f"modules/game_raw_{shard:04x}.c"
            shards[shard].append(item)
        classified.append(item)
        class_counts[classification] += 1
        class_bytes[classification] += body_bytes

    missing_sdk = [
        {
            "entry": f"0x{address:08x}",
            "candidates": sorted({str(match["name"]) for match in matches}),
            "reason": "outside the exported static address range",
        }
        for address, matches in sorted(sdk.items())
        if address not in seen
    ]
    report = {
        "schema_version": 1,
        "kind": "arm9-decompilation-catalog",
        "source_program": export["program"],
        "address_range": {
            "minimum": export.get("minimum_address"),
            "maximum": export.get("maximum_address"),
        },
        "summary": {
            "exported_functions": len(classified),
            "decompiled_functions": sum(item["decompiled"] for item in classified),
            "sdk_functions": class_counts["sdk"],
            "game_candidates": class_counts["game_candidate"],
            "switch_artifacts": class_counts["switch_artifact"],
            "sdk_boundaries_outside_export": len(missing_sdk),
            "game_candidate_body_bytes": class_bytes["game_candidate"],
            "raw_module_shards": len(shards),
        },
        "classification_notes": {
            "sdk": "entry address has an independently verified SDK-library boundary",
            "game_candidate": "decompiled non-SDK function; semantic rewrite and later matching remain",
            "switch_artifact": "Ghidra caseD pseudo-function excluded from the game function total",
        },
        "missing_sdk_boundaries": missing_sdk,
        "functions": classified,
    }
    return report, dict(shards)


def write_outputs(
    output: Path,
    report: dict[str, Any],
    shards: dict[int, list[dict[str, Any]]],
    c_blocks: dict[int, str],
) -> None:
    output.mkdir(parents=True, exist_ok=True)
    module_dir = output / "modules"
    module_dir.mkdir(parents=True, exist_ok=True)
    expected_shards: set[Path] = set()
    for shard, functions in sorted(shards.items()):
        path = module_dir / f"game_raw_{shard:04x}.c"
        expected_shards.add(path)
        with path.open("w", encoding="utf-8", newline="\n") as stream:
            stream.write("/*\n")
            stream.write(" * Raw Ghidra recovery shard. Analysis only: not yet compilable or matching source.\n")
            stream.write(" */\n\n")
            for function in functions:
                stream.write(c_blocks[int(function["entry"], 16)])
                stream.write("\n")
    for stale in module_dir.glob("game_raw_*.c"):
        if stale not in expected_shards:
            stale.unlink()

    catalog_path = output / "catalog.json"
    catalog_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    with (output / "game_candidates.csv").open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(
            stream,
            fieldnames=("entry", "file_offset", "name", "body_bytes", "raw_shard"),
        )
        writer.writeheader()
        for function in report["functions"]:
            if function["classification"] == "game_candidate":
                writer.writerow({key: function[key] for key in writer.fieldnames})


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--export-json", type=Path, default=PROJECT_ROOT / "build/decomp/arm9/recovered.json"
    )
    parser.add_argument(
        "--export-c", type=Path, default=PROJECT_ROOT / "build/decomp/arm9/recovered.c"
    )
    parser.add_argument(
        "--signatures", type=Path, default=PROJECT_ROOT / "config/sdk-signatures-arm9.json"
    )
    parser.add_argument("--output", type=Path, default=PROJECT_ROOT / "build/decomp/arm9")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        export = read_json(args.export_json, "Ghidra export")
        signatures = read_json(args.signatures, "SDK signatures")
        c_blocks = parse_c_blocks(args.export_c.read_text(encoding="utf-8"))
        report, shards = classify_export(export, signatures, c_blocks)
        report["inputs"] = {
            "export_json_sha256": sha256_file(args.export_json),
            "export_c_sha256": sha256_file(args.export_c),
            "sdk_signatures_sha256": sha256_file(args.signatures),
        }
        write_outputs(args.output, report, shards, c_blocks)
    except (CatalogError, OSError) as exc:
        print(f"decompilation catalog error: {exc}", file=__import__("sys").stderr)
        return 2
    print(json.dumps(report["summary"], indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
