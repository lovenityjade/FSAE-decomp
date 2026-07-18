#!/usr/bin/env python3
"""Build and verify the complete ARM9i object/function map.

Official bytes stay in an external TWLSDK_ROOT.  The tool emits metadata and
dashboard evidence only; it never copies an SDK member into the repository.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import struct
import sys
import tempfile
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


PROJECT_ROOT = Path(__file__).resolve().parents[2]
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

from analysis.scan_sdk_libraries import ar_members, elf_functions  # noqa: E402


class Arm9iError(Exception):
    """A reproducibility precondition failed."""


def iso_now() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="seconds").replace("+00:00", "Z")


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def read_json(path: Path) -> dict[str, Any]:
    try:
        with path.open("r", encoding="utf-8") as stream:
            value = json.load(stream)
    except (OSError, json.JSONDecodeError) as exc:
        raise Arm9iError(f"{path}: {exc}") from exc
    if not isinstance(value, dict):
        raise Arm9iError(f"{path}: JSON root must be an object")
    return value


def atomic_write_json(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle = tempfile.NamedTemporaryFile(
        mode="w", encoding="utf-8", dir=path.parent, prefix=f".{path.name}.", delete=False
    )
    temporary = Path(handle.name)
    try:
        with handle:
            json.dump(value, handle, indent=2, ensure_ascii=False)
            handle.write("\n")
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary, path)
    finally:
        if temporary.exists():
            temporary.unlink()


def resolve_beneath(root: Path, value: str, field: str) -> Path:
    relative = Path(value)
    if relative.is_absolute() or ".." in relative.parts:
        raise Arm9iError(f"{field} must remain relative to its root")
    root = root.resolve()
    result = (root / relative).resolve()
    if not result.is_relative_to(root):
        raise Arm9iError(f"{field} escapes its root")
    return result


def validate_partition(ranges: list[dict[str, Any]], size: int) -> None:
    cursor = 0
    for index, entry in enumerate(ranges):
        start, end = entry.get("offset"), entry.get("end")
        if not isinstance(start, int) or not isinstance(end, int) or start != cursor or end <= start:
            raise Arm9iError(f"object_ranges[{index}] does not continue the exact partition at {cursor}")
        cursor = end
    if cursor != size:
        raise Arm9iError(f"object_ranges end at {cursor}, expected {size}")


def compute_uncovered(
    coverage: list[dict[str, Any]], payload_start: int, payload_size: int
) -> list[tuple[int, int]]:
    payload_end = payload_start + payload_size
    intervals = sorted(
        (max(payload_start, item["offset"]), min(payload_end, item["end"]))
        for item in coverage
        if item["end"] > payload_start and item["offset"] < payload_end
    )
    result: list[tuple[int, int]] = []
    cursor = payload_start
    for start, end in intervals:
        if cursor < start:
            result.append((cursor, start))
        cursor = max(cursor, end)
    if cursor < payload_end:
        result.append((cursor, payload_end))
    return result


def masked_equal(target: bytes, code: bytes, mask: bytes) -> bool:
    return len(target) == len(code) and all(
        target[index] == code[index] for index, keep in enumerate(mask) if keep
    )


def classify_function(target: bytes, function: dict[str, Any]) -> dict[str, Any]:
    code, mask = function["code"], function["mask"]
    if len(target) != len(code):
        raise Arm9iError("function size differs from configured boundary")
    exact = target == code
    if not exact and not masked_equal(target, code, mask):
        raise Arm9iError("reference function does not match even with relocations masked")
    return {
        "match_kind": "byte_exact_reference" if exact else "relocation_masked_reference",
        "byte_exact": exact,
        "reference_equal_bytes": sum(a == b for a, b in zip(target, code)),
        "reference_different_bytes": sum(a != b for a, b in zip(target, code)),
        "relocation_masked_bytes": len(mask) - sum(mask),
    }


def elf_section_bytes(data: bytes) -> dict[str, bytes]:
    if data[:7] != b"\x7fELF\x01\x01\x01" or len(data) < 52:
        raise Arm9iError("layout object is not a 32-bit little-endian ELF")
    section_offset = struct.unpack_from("<I", data, 32)[0]
    section_entry_size, section_count, names_index = struct.unpack_from("<HHH", data, 46)
    if section_entry_size < 40 or section_offset + section_entry_size * section_count > len(data):
        raise Arm9iError("layout ELF section table is invalid")
    sections = [
        struct.unpack_from("<10I", data, section_offset + index * section_entry_size)
        for index in range(section_count)
    ]
    if names_index >= len(sections):
        raise Arm9iError("layout ELF section-name table is invalid")
    names_section = sections[names_index]
    names = data[names_section[4] : names_section[4] + names_section[5]]
    result: dict[str, bytes] = {}
    for section in sections:
        end = names.find(b"\0", section[0])
        name = names[section[0] : end if end >= 0 else len(names)].decode("utf-8", "replace")
        if section[1] == 1:  # SHT_PROGBITS
            result[name] = data[section[4] : section[4] + section[5]]
    return result


def layout_matches(
    target: bytes, object_path: Path, section_specs: list[dict[str, Any]]
) -> tuple[int, list[dict[str, Any]]]:
    if not object_path.is_file():
        return 0, []
    sections = elf_section_bytes(object_path.read_bytes())
    records: list[dict[str, Any]] = []
    matched = 0
    for spec in section_specs:
        name, offset, size = spec["name"], spec["target_offset"], spec["size"]
        section = sections.get(name)
        if section is None:
            raise Arm9iError(f"layout object is missing section {name}")
        equal = len(section) == size and section == target[offset : offset + size]
        records.append(
            {
                "section": name,
                "target_offset": offset,
                "size": size,
                "sha256": sha256(section),
                "byte_for_byte_equal": equal,
            }
        )
        if equal:
            matched += size
    return matched, records


def matching_evidence(matched: int, total: int, timestamp: str, full: bool) -> dict[str, Any]:
    return {
        "$schema": "../../tools/progress/schema-v2.json#/$defs/evidence",
        "schema_version": 2,
        "kind": "evidence",
        "id": "arm9i-rebuilt-byte-proof",
        "track": "matching",
        "section": "arm9i",
        "category": "sdk",
        "metrics": {
            "units": {"matched": int(full), "total": 1},
            "functions": {"matched": 0, "total": 0},
            "bytes": {"matched": matched, "total": total},
        },
        "updated_at": timestamp,
        "worker": "binary-map",
        "summary": (
            "ARM9i rebuilt image is byte-identical"
            if full
            else f"ARM9i repository-owned layout rebuild matches {matched} structural bytes"
        ),
    }


def load_archives(
    sdk_root: Path, specs: list[dict[str, Any]]
) -> tuple[dict[str, dict[str, dict[str, dict[str, Any]]]], list[dict[str, Any]]]:
    index: dict[str, dict[str, dict[str, dict[str, Any]]]] = {}
    proof: list[dict[str, Any]] = []
    for spec in specs:
        name = spec["name"]
        path = resolve_beneath(sdk_root, spec["path"], f"archives[{name}].path")
        try:
            data = path.read_bytes()
        except OSError as exc:
            raise Arm9iError(str(exc)) from exc
        actual_hash = sha256(data)
        if actual_hash != spec["sha256"]:
            raise Arm9iError(f"{name}: SHA-256 {actual_hash} != expected {spec['sha256']}")
        members: dict[str, dict[str, dict[str, Any]]] = {}
        for member_name, member_data in ar_members(data):
            functions: dict[str, dict[str, Any]] = {}
            for function in elf_functions(member_data):
                functions[function["name"]] = function
            members[member_name] = functions
        index[name] = members
        proof.append({"name": name, "path": spec["path"], "sha256": actual_hash, "verified": True})
    return index, proof


def function_reference(
    archive_index: dict[str, dict[str, dict[str, dict[str, Any]]]],
    archive: str,
    member: str,
    name: str,
) -> dict[str, Any]:
    try:
        return archive_index[archive][member][name]
    except KeyError as exc:
        raise Arm9iError(f"missing SDK function {archive}:{member}:{name}") from exc


def analyze(project_root: Path, sdk_root: Path, config_path: Path, candidate: Path | None) -> dict[str, Any]:
    config = read_json(config_path)
    if config.get("schema_version") != 1 or config.get("kind") != "arm9i-build-plan":
        raise Arm9iError("unsupported ARM9i build plan")
    target_spec = config["target"]
    target_path = resolve_beneath(project_root, target_spec["path"], "target.path")
    target = target_path.read_bytes()
    if len(target) != target_spec["size"] or sha256(target) != target_spec["sha256"]:
        raise Arm9iError("ARM9i target size or SHA-256 does not match the build plan")

    layout = config["layout"]
    payload_start, payload_size = layout["payload_file_offset"], layout["payload_size"]
    if payload_start + payload_size != layout["autoload_table_offset"]:
        raise Arm9iError("payload must end exactly at the autoload table")
    table_offset = layout["autoload_table_offset"]
    table_words = list(struct.unpack_from("<4I", target, table_offset))
    if table_words != layout["autoload_table_words"]:
        raise Arm9iError("autoload table does not match the configured record")
    validate_partition(config["object_ranges"], len(target))

    signature_path = resolve_beneath(project_root, config["signature_map"], "signature_map")
    signatures = read_json(signature_path)
    if signatures.get("image_sha256") != target_spec["sha256"]:
        raise Arm9iError("signature map targets a different ARM9i image")
    uncovered = compute_uncovered(signatures["coverage_ranges"], payload_start, payload_size)
    expected_uncovered = [(entry["offset"], entry["end"]) for entry in config["previously_uncovered_ranges"]]
    if uncovered != expected_uncovered:
        raise Arm9iError(f"derived uncovered ranges {uncovered!r} differ from the build plan")
    if sum(end - start for start, end in uncovered) != 1232:
        raise Arm9iError("uncovered range total is not 1,232 bytes")
    uncovered_proof: list[dict[str, Any]] = []
    for entry in config["previously_uncovered_ranges"]:
        data = target[entry["offset"] : entry["end"]]
        actual_hash = sha256(data)
        if actual_hash != entry["sha256"]:
            raise Arm9iError(f"uncovered range at {entry['offset']} changed")
        uncovered_proof.append({**entry, "size": entry["end"] - entry["offset"], "verified": True})

    archive_index, archive_proof = load_archives(sdk_root, config["archives"])
    boundary_groups: dict[tuple[int, int, str], list[dict[str, Any]]] = {}
    for match in signatures["matches"]:
        boundary_groups.setdefault(
            (match["binary_offset"], match["size"], match["isa"]), []
        ).append(match)

    functions: list[dict[str, Any]] = []
    for (offset, size, isa), candidates in sorted(boundary_groups.items()):
        possible: list[tuple[dict[str, Any], dict[str, Any], dict[str, str]]] = []
        for match in candidates:
            for source in match["sources"]:
                if source["library"] not in archive_index:
                    continue
                try:
                    function = function_reference(
                        archive_index, source["library"], source["object"], match["name"]
                    )
                    classification = classify_function(target[offset : offset + size], function)
                except Arm9iError:
                    continue
                possible.append((classification, match, source))
        if not possible:
            raise Arm9iError(f"no selected SDK reference proves boundary at 0x{offset:x}")
        position_owned = [
            item
            for item in possible
            if any(
                entry.get("kind") == "sdk_object"
                and entry.get("archive") == item[2]["library"]
                and entry.get("object") == item[2]["object"]
                and entry["offset"] <= offset
                and offset + size <= entry["end"]
                for entry in config["object_ranges"]
            )
        ]
        if not position_owned:
            raise Arm9iError(f"no configured SDK object owns boundary at 0x{offset:x}")
        possible = position_owned
        possible.sort(
            key=lambda item: (
                not item[0]["byte_exact"],
                item[0]["reference_different_bytes"],
                item[1]["name"],
            )
        )
        classification, selected, source = possible[0]
        functions.append(
            {
                "offset": offset,
                "address": layout["load_address"] + offset - payload_start,
                "size": size,
                "isa": isa,
                "names": sorted({item["name"] for item in candidates}),
                "archive": source["library"],
                "object": source["object"],
                **classification,
                "discovery": "signature_map",
            }
        )

    for recovered in config["recovered_functions"]:
        function = function_reference(
            archive_index, recovered["archive"], recovered["object"], recovered["name"]
        )
        if len(function["code"]) != recovered["size"] or function["isa"] != recovered["isa"]:
            raise Arm9iError(f"recovered SDK metadata changed for {recovered['name']}")
        offset, size = recovered["offset"], recovered["size"]
        classification = classify_function(target[offset : offset + size], function)
        functions.append(
            {
                "offset": offset,
                "address": layout["load_address"] + offset - payload_start,
                "size": size,
                "isa": recovered["isa"],
                "names": [recovered["name"]],
                "archive": recovered["archive"],
                "object": recovered["object"],
                **classification,
                "discovery": "fixed-gap object recovery",
            }
        )
    functions.sort(key=lambda item: (item["offset"], item["names"][0]))
    seen_boundaries: set[tuple[int, int, str]] = set()
    previous_end = payload_start
    for function in functions:
        key = (function["offset"], function["size"], function["isa"])
        if key in seen_boundaries:
            raise Arm9iError(f"duplicate function boundary at 0x{function['offset']:x}")
        end = function["offset"] + function["size"]
        if function["offset"] < previous_end or end > table_offset:
            raise Arm9iError(f"overlapping or out-of-payload function at 0x{function['offset']:x}")
        owners = [
            entry
            for entry in config["object_ranges"]
            if entry.get("kind") == "sdk_object"
            and entry.get("archive") == function["archive"]
            and entry.get("object") == function["object"]
            and entry["offset"] <= function["offset"]
            and end <= entry["end"]
        ]
        if len(owners) != 1:
            raise Arm9iError(
                f"function at 0x{function['offset']:x} does not belong to exactly one configured object"
            )
        function["object_range"] = {"offset": owners[0]["offset"], "end": owners[0]["end"]}
        seen_boundaries.add(key)
        previous_end = end

    timestamp = iso_now()
    exact_functions = [function for function in functions if function["byte_exact"]]
    function_bytes = sum(item["size"] for item in functions)
    analysis_counts = {
        "function_boundaries": len(functions),
        "function_bytes": function_bytes,
        "byte_exact_reference_boundaries": len(exact_functions),
        "byte_exact_reference_bytes": sum(item["size"] for item in exact_functions),
        "relocation_masked_reference_boundaries": len(functions) - len(exact_functions),
        "relocation_masked_reference_bytes": sum(
            item["size"] for item in functions if not item["byte_exact"]
        ),
        "non_function_payload_bytes": payload_size - function_bytes,
        "proprietary_game_bytes": 0,
    }
    if analysis_counts != config["expected_analysis"]:
        raise Arm9iError(
            f"complete function/object totals changed: {analysis_counts!r} != "
            f"{config['expected_analysis']!r}"
        )
    analysis_proof = {
        "schema_version": 1,
        "kind": "arm9i-complete-object-map",
        "generated_at": timestamp,
        "target": {
            "path": target_spec["path"],
            "size": len(target),
            "sha256": sha256(target),
        },
        "runtime_mapping": {
            "payload_file_offset": payload_start,
            "payload_size": payload_size,
            "load_address": layout["load_address"],
            "formula": "runtime_address = load_address + file_offset - payload_file_offset",
            "autoload_table_offset": table_offset,
            "autoload_table_words": table_words,
        },
        "archives": archive_proof,
        "objects": config["object_ranges"],
        "previously_uncovered": {
            "bytes": sum(item["size"] for item in uncovered_proof),
            "proprietary_game_bytes": 0,
            "ranges": uncovered_proof,
        },
        "function_summary": {
            "boundaries": analysis_counts["function_boundaries"],
            "function_bytes": analysis_counts["function_bytes"],
            "byte_exact_reference_boundaries": analysis_counts[
                "byte_exact_reference_boundaries"
            ],
            "relocation_masked_reference_boundaries": analysis_counts[
                "relocation_masked_reference_boundaries"
            ],
            "byte_exact_reference_bytes": analysis_counts["byte_exact_reference_bytes"],
            "relocation_masked_reference_bytes": analysis_counts[
                "relocation_masked_reference_bytes"
            ],
            "non_function_payload_bytes": analysis_counts["non_function_payload_bytes"],
            "note": "reference equality is analysis evidence, not rebuilt matching credit",
        },
        "functions": functions,
    }

    layout_spec = config["layout_object"]
    layout_object_path = resolve_beneath(project_root, layout_spec["path"], "layout_object.path")
    rebuilt_layout_bytes, layout_records = layout_matches(
        target, layout_object_path, layout_spec["sections"]
    )
    full_equal = False
    candidate_result: dict[str, Any] | None = None
    if candidate is not None:
        candidate_data = candidate.read_bytes()
        full_equal = candidate_data == target
        candidate_result = {
            "path": str(candidate),
            "size": len(candidate_data),
            "sha256": sha256(candidate_data),
            "byte_for_byte_equal": full_equal,
            "different_bytes": (
                sum(a != b for a, b in zip(candidate_data, target))
                + abs(len(candidate_data) - len(target))
            ),
        }
    matched_bytes = len(target) if full_equal else rebuilt_layout_bytes
    progress = matching_evidence(matched_bytes, len(target), timestamp, full_equal)
    matching_proof = {
        "schema_version": 1,
        "kind": "arm9i-rebuild-proof",
        "generated_at": timestamp,
        "layout_object": {
            "path": layout_spec["path"],
            "available": layout_object_path.is_file(),
            "matched_bytes": rebuilt_layout_bytes,
            "sections": layout_records,
        },
        "candidate": candidate_result,
        "matched_bytes": matched_bytes,
        "total_bytes": len(target),
        "full_image_match": full_equal,
        "dashboard_evidence": progress,
    }
    output = config["output"]
    atomic_write_json(resolve_beneath(project_root, output["analysis_proof"], "output.analysis_proof"), analysis_proof)
    atomic_write_json(resolve_beneath(project_root, output["matching_proof"], "output.matching_proof"), matching_proof)
    atomic_write_json(resolve_beneath(project_root, output["matching_progress"], "output.matching_progress"), progress)
    return {"analysis": analysis_proof, "matching": matching_proof}


def make_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--twlsdk-root", help="external TwlSDK root; defaults to TWLSDK_ROOT")
    parser.add_argument("--config", type=Path, default=Path("config/build/arm9i.json"))
    parser.add_argument("--candidate", type=Path, help="optional full rebuilt ARM9i image")
    parser.add_argument("--project-root", type=Path, default=PROJECT_ROOT, help=argparse.SUPPRESS)
    return parser


def main(argv: list[str] | None = None) -> int:
    args = make_parser().parse_args(argv)
    sdk_value = args.twlsdk_root or os.environ.get("TWLSDK_ROOT")
    if not sdk_value:
        print("error: set TWLSDK_ROOT or pass --twlsdk-root", file=sys.stderr)
        return 2
    project_root = args.project_root.resolve()
    config_path = args.config if args.config.is_absolute() else project_root / args.config
    try:
        proof = analyze(
            project_root,
            Path(sdk_value).expanduser().resolve(),
            config_path.resolve(),
            args.candidate.resolve() if args.candidate else None,
        )
    except (Arm9iError, OSError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    summary = proof["analysis"]["function_summary"]
    matching = proof["matching"]
    print(
        f"ARM9i: {summary['boundaries']} SDK functions, 1232/1232 gap bytes classified, "
        f"{matching['matched_bytes']}/{matching['total_bytes']} rebuilt bytes matched"
    )
    if args.candidate and not matching["full_image_match"]:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
