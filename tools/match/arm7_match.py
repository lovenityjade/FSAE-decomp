#!/usr/bin/env python3
"""Prove ARM7/ARM7i matches from an external TWL-SDK installation.

The repository contains only configuration and comparison logic.  Official SDK
binaries are read from TWLSDK_ROOT and generated artifacts stay under build/.
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
DEFAULT_CONFIGS = (
    Path("config/build/arm7.json"),
    Path("config/build/arm7i.json"),
)
STATIC_FOOTER_MAGIC = 0xDEC00621


class MatchError(Exception):
    """A configuration, input, or normalization precondition failed."""


def iso_now() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="seconds").replace("+00:00", "Z")


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def read_json(path: Path) -> dict[str, Any]:
    try:
        with path.open("r", encoding="utf-8") as stream:
            value = json.load(stream)
    except (OSError, json.JSONDecodeError) as exc:
        raise MatchError(f"{path}: {exc}") from exc
    if not isinstance(value, dict):
        raise MatchError(f"{path}: root must be a JSON object")
    return value


def atomic_write(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle = tempfile.NamedTemporaryFile(dir=path.parent, prefix=f".{path.name}.", delete=False)
    temporary = Path(handle.name)
    try:
        with handle:
            handle.write(data)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary, path)
    finally:
        if temporary.exists():
            temporary.unlink()


def atomic_write_json(path: Path, value: dict[str, Any]) -> None:
    payload = (json.dumps(value, indent=2, ensure_ascii=False) + "\n").encode("utf-8")
    atomic_write(path, payload)


def resolve_beneath(root: Path, relative: str, field: str) -> Path:
    candidate_value = Path(relative)
    if candidate_value.is_absolute() or ".." in candidate_value.parts:
        raise MatchError(f"{field} must be a relative path without '..'")
    root = root.resolve()
    candidate = (root / candidate_value).resolve()
    if not candidate.is_relative_to(root):
        raise MatchError(f"{field} escapes its root")
    return candidate


def require_object(value: Any, field: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise MatchError(f"{field} must be an object")
    return value


def require_string(value: dict[str, Any], field: str) -> str:
    result = value.get(field)
    if not isinstance(result, str) or not result:
        raise MatchError(f"{field} must be a non-empty string")
    return result


def require_size_and_hash(spec: dict[str, Any], field: str) -> tuple[int, str]:
    size = spec.get("size")
    digest = spec.get("sha256")
    if isinstance(size, bool) or not isinstance(size, int) or size < 0:
        raise MatchError(f"{field}.size must be a non-negative integer")
    if not isinstance(digest, str) or len(digest) != 64:
        raise MatchError(f"{field}.sha256 must be a SHA-256 hex digest")
    try:
        bytes.fromhex(digest)
    except ValueError as exc:
        raise MatchError(f"{field}.sha256 must be hexadecimal") from exc
    return size, digest.lower()


def verify_blob(data: bytes, spec: dict[str, Any], field: str) -> dict[str, Any]:
    expected_size, expected_hash = require_size_and_hash(spec, field)
    actual_hash = sha256(data)
    if len(data) != expected_size:
        raise MatchError(f"{field}: size {len(data)} != expected {expected_size}")
    if actual_hash != expected_hash:
        raise MatchError(f"{field}: SHA-256 {actual_hash} != expected {expected_hash}")
    return {"size": len(data), "sha256": actual_hash, "verified": True}


def parse_footer(operation: dict[str, Any], data: bytes) -> tuple[bytes, dict[str, Any]]:
    footer_hex = require_string(operation, "footer_hex")
    try:
        footer = bytes.fromhex(footer_hex)
    except ValueError as exc:
        raise MatchError("footer_hex must be hexadecimal") from exc
    if len(footer) != 16:
        raise MatchError("strip_static_footer requires exactly 16 footer bytes")
    if len(data) < len(footer) or data[-len(footer) :] != footer:
        raise MatchError("official input does not end with the configured static footer")
    magic, module_params, digest, ltd_module_params = struct.unpack("<4I", footer)
    if magic != STATIC_FOOTER_MAGIC:
        raise MatchError(f"static footer magic 0x{magic:08x} is not 0x{STATIC_FOOTER_MAGIC:08x}")
    expected_fields = require_object(operation.get("fields"), "fields")
    observed = {
        "magic": f"0x{magic:08x}",
        "module_params_offset": module_params,
        "digest_offset": digest,
        "ltd_module_params_offset": ltd_module_params,
    }
    if observed != expected_fields:
        raise MatchError(f"static footer fields {observed!r} do not match configured fields")
    return data[:-len(footer)], observed


def apply_operations(data: bytes, operations: Any) -> tuple[bytes, list[dict[str, Any]]]:
    if not isinstance(operations, list):
        raise MatchError("normalization must be an array")
    result = data
    applied: list[dict[str, Any]] = []
    seen: set[str] = set()
    for index, raw_operation in enumerate(operations):
        operation = require_object(raw_operation, f"normalization[{index}]")
        operation_type = require_string(operation, "type")
        if operation_type in seen:
            raise MatchError(f"normalization operation {operation_type} is duplicated")
        seen.add(operation_type)
        if operation_type == "sdk_version_patch":
            offset = operation.get("offset")
            if isinstance(offset, bool) or not isinstance(offset, int) or offset < 0:
                raise MatchError("sdk_version_patch.offset must be a non-negative integer")
            try:
                before = bytes.fromhex(require_string(operation, "before_hex"))
                after = bytes.fromhex(require_string(operation, "after_hex"))
            except ValueError as exc:
                raise MatchError("sdk_version_patch values must be hexadecimal") from exc
            if len(before) != 4 or len(after) != 4:
                raise MatchError("sdk_version_patch must replace one exact 32-bit marker")
            if offset + 4 > len(result) or result[offset : offset + 4] != before:
                actual = result[offset : offset + 4].hex()
                raise MatchError(
                    f"SDK version marker at 0x{offset:x} is {actual}, expected {before.hex()}"
                )
            result = result[:offset] + after + result[offset + 4 :]
            applied.append(
                {
                    "type": operation_type,
                    "offset": offset,
                    "before_hex": before.hex(),
                    "after_hex": after.hex(),
                }
            )
        elif operation_type == "strip_static_footer":
            result, fields = parse_footer(operation, result)
            applied.append({"type": operation_type, "bytes_removed": 16, "fields": fields})
        else:
            raise MatchError(f"unsupported normalization operation: {operation_type}")
    return result, applied


def compare_bytes(actual: bytes, target: bytes) -> dict[str, Any]:
    shared = min(len(actual), len(target))
    mismatch_offsets = [index for index in range(shared) if actual[index] != target[index]]
    mismatch_count = len(mismatch_offsets) + abs(len(actual) - len(target))
    first_mismatch = mismatch_offsets[0] if mismatch_offsets else (shared if len(actual) != len(target) else None)
    return {
        "byte_for_byte_equal": mismatch_count == 0,
        "mismatch_bytes": mismatch_count,
        "first_mismatch_offset": first_mismatch,
    }


def make_progress_evidence(
    section: str, target_size: int, equal: bool, timestamp: str
) -> dict[str, Any]:
    matched = target_size if equal else 0
    return {
        "$schema": "../../tools/progress/schema-v2.json#/$defs/evidence",
        "schema_version": 2,
        "kind": "evidence",
        "id": f"arm7-match-{section}",
        "track": "matching",
        "section": section,
        "category": "sdk",
        "metrics": {
            "units": {"matched": int(equal), "total": 1},
            "functions": {"matched": 0, "total": 0},
            "bytes": {"matched": matched, "total": target_size},
        },
        "updated_at": timestamp,
        "worker": "binary-map",
        "summary": (
            f"{section.upper()} official racoon component reproduces the target byte-for-byte"
            if equal
            else f"{section.upper()} official racoon component does not match the target"
        ),
    }


def run_match(config_path: Path, project_root: Path, sdk_root: Path) -> dict[str, Any]:
    config = read_json(config_path)
    if config.get("schema_version") != 1 or config.get("kind") != "arm7-match-config":
        raise MatchError(f"{config_path}: unsupported match configuration")
    section = require_string(config, "section")
    if section not in {"arm7", "arm7i"}:
        raise MatchError("section must be arm7 or arm7i")

    reference_spec = require_object(config.get("reference"), "reference")
    target_spec = require_object(config.get("target"), "target")
    output_spec = require_object(config.get("output"), "output")
    normalized_spec = require_object(config.get("normalized"), "normalized")

    reference_path = resolve_beneath(
        sdk_root, require_string(reference_spec, "path"), "reference.path"
    )
    target_path = resolve_beneath(
        project_root, require_string(target_spec, "path"), "target.path"
    )
    try:
        reference_data = reference_path.read_bytes()
        target_data = target_path.read_bytes()
    except OSError as exc:
        raise MatchError(str(exc)) from exc

    reference_result = verify_blob(reference_data, reference_spec, "reference")
    target_result = verify_blob(target_data, target_spec, "target")
    normalized, applied = apply_operations(reference_data, config.get("normalization"))
    normalized_result = verify_blob(normalized, normalized_spec, "normalized")
    comparison = compare_bytes(normalized, target_data)

    artifact_path = resolve_beneath(
        project_root, require_string(output_spec, "artifact"), "output.artifact"
    )
    proof_path = resolve_beneath(
        project_root, require_string(output_spec, "proof"), "output.proof"
    )
    progress_path = resolve_beneath(
        project_root, require_string(output_spec, "progress"), "output.progress"
    )
    timestamp = iso_now()
    progress = make_progress_evidence(section, len(target_data), comparison["byte_for_byte_equal"], timestamp)
    proof = {
        "schema_version": 1,
        "kind": "binary-match-proof",
        "section": section,
        "generated_at": timestamp,
        "reference": {
            "path": reference_spec["path"],
            **reference_result,
        },
        "target": {
            "path": target_spec["path"],
            **target_result,
        },
        "normalization": {
            "operations": applied,
            **normalized_result,
        },
        "artifact": {
            "path": output_spec["artifact"],
            "contains_official_sdk_material": True,
            "git_ignored": artifact_path.is_relative_to(project_root / "build"),
        },
        "comparison": comparison,
        "dashboard_evidence": progress,
    }
    atomic_write(artifact_path, normalized)
    atomic_write_json(proof_path, proof)
    atomic_write_json(progress_path, progress)
    return proof


def make_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--twlsdk-root",
        help="external TwlSDK root (defaults to TWLSDK_ROOT)",
    )
    parser.add_argument(
        "--project-root",
        type=Path,
        default=PROJECT_ROOT,
        help=argparse.SUPPRESS,
    )
    parser.add_argument(
        "--config",
        action="append",
        type=Path,
        help="match configuration; may be repeated (defaults to ARM7 and ARM7i)",
    )
    return parser


def main(argv: list[str] | None = None) -> int:
    args = make_parser().parse_args(argv)
    sdk_value = args.twlsdk_root or os.environ.get("TWLSDK_ROOT")
    if not sdk_value:
        print("error: set TWLSDK_ROOT or pass --twlsdk-root", file=sys.stderr)
        return 2
    project_root = args.project_root.resolve()
    sdk_root = Path(sdk_value).expanduser().resolve()
    config_values = args.config or list(DEFAULT_CONFIGS)
    all_equal = True
    try:
        for config_value in config_values:
            config_path = config_value if config_value.is_absolute() else project_root / config_value
            proof = run_match(config_path.resolve(), project_root, sdk_root)
            equal = proof["comparison"]["byte_for_byte_equal"]
            all_equal &= equal
            state = "MATCH" if equal else "MISMATCH"
            print(
                f"{proof['section']}: {state} "
                f"({proof['target']['size']} bytes, {proof['normalization']['sha256']})"
            )
    except MatchError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    return 0 if all_equal else 1


if __name__ == "__main__":
    raise SystemExit(main())
