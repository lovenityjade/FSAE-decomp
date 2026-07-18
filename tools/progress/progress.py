#!/usr/bin/env python3
"""Dependency-free progress collector, writer and local dashboard server."""

from __future__ import annotations

import argparse
import hashlib
import json
import mimetypes
import os
import re
import sys
import tempfile
import uuid
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Any, Iterable
from urllib.parse import urlsplit


SCHEMA_VERSION = 2
KINDS = {"evidence", "worker", "change"}
METRIC_NAMES = ("units", "functions", "bytes")
PROVENANCE_NAMES = ("source", "sdk", "fallback", "unmatched")
UNIT_OUTCOME_NAMES = ("probe", "promoted", "rejected")
WORKER_STATUSES = {"working", "idle", "blocked", "done", "offline"}
SAFE_ID = re.compile(r"^[A-Za-z0-9][A-Za-z0-9_.-]*$")
DEFAULT_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_CONFIG = Path("tools/progress/project.v2.json")
STATIC_FILES = {
    "/": "index.html",
    "/index.html": "index.html",
    "/app.js": "app.js",
    "/styles.css": "styles.css",
}
DASHBOARD_ASSET_TOKEN = "__DASHBOARD_ASSET_VERSION__"
NO_STORE_POLICY = "no-store, no-cache, must-revalidate, max-age=0"


class ProgressError(Exception):
    """A user-facing validation or input error."""


def utc_now() -> datetime:
    return datetime.now(timezone.utc)


def iso_now() -> str:
    return utc_now().isoformat(timespec="seconds").replace("+00:00", "Z")


def parse_timestamp(value: Any, field: str) -> datetime:
    if not isinstance(value, str):
        raise ProgressError(f"{field} must be an ISO-8601 string")
    try:
        parsed = datetime.fromisoformat(value.replace("Z", "+00:00"))
    except ValueError as exc:
        raise ProgressError(f"{field} is not a valid ISO-8601 timestamp") from exc
    if parsed.tzinfo is None:
        raise ProgressError(f"{field} must include a timezone")
    return parsed.astimezone(timezone.utc)


def read_json(path: Path) -> dict[str, Any]:
    try:
        with path.open("r", encoding="utf-8") as stream:
            value = json.load(stream)
    except (OSError, json.JSONDecodeError) as exc:
        raise ProgressError(f"{path}: {exc}") from exc
    if not isinstance(value, dict):
        raise ProgressError(f"{path}: root value must be an object")
    return value


def atomic_write_json(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    try:
        mode = path.stat().st_mode & 0o777
    except OSError:
        mode = 0o644
    handle = tempfile.NamedTemporaryFile(
        mode="w", encoding="utf-8", dir=path.parent, prefix=f".{path.name}.", delete=False
    )
    temporary = Path(handle.name)
    try:
        with handle:
            os.fchmod(handle.fileno(), mode)
            json.dump(value, handle, indent=2, ensure_ascii=False)
            handle.write("\n")
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary, path)
        directory_flags = os.O_RDONLY | getattr(os, "O_DIRECTORY", 0)
        try:
            directory_fd = os.open(path.parent, directory_flags)
        except OSError:
            directory_fd = None
        if directory_fd is not None:
            try:
                os.fsync(directory_fd)
            finally:
                os.close(directory_fd)
    finally:
        if temporary.exists():
            temporary.unlink()


def git_commit(root: Path) -> str:
    """Resolve HEAD without invoking git; return unknown for an unborn tree."""
    git_directory = root / ".git"
    if git_directory.is_file():
        try:
            marker = git_directory.read_text(encoding="utf-8").strip()
        except OSError:
            return "unknown"
        if not marker.startswith("gitdir:"):
            return "unknown"
        git_directory = (root / marker.split(":", 1)[1].strip()).resolve()
    try:
        head = (git_directory / "HEAD").read_text(encoding="ascii").strip()
    except OSError:
        return "unknown"
    if not head.startswith("ref:"):
        return head if re.fullmatch(r"[0-9a-fA-F]{7,64}", head) else "unknown"
    reference = head.split(":", 1)[1].strip()
    try:
        commit = (git_directory / reference).read_text(encoding="ascii").strip()
    except OSError:
        commit = ""
    if not commit:
        try:
            packed = (git_directory / "packed-refs").read_text(encoding="ascii")
        except OSError:
            packed = ""
        for line in packed.splitlines():
            fields = line.split()
            if len(fields) == 2 and fields[1] == reference:
                commit = fields[0]
                break
    return commit if re.fullmatch(r"[0-9a-fA-F]{7,64}", commit) else "unknown"


def dashboard_asset_version(dashboard_root: Path) -> str:
    """Return a stable cache-busting token for the executable dashboard assets."""
    digest = hashlib.sha256()
    for name in ("app.js", "styles.css"):
        digest.update(name.encode("utf-8"))
        digest.update(b"\0")
        digest.update((dashboard_root / name).read_bytes())
        digest.update(b"\0")
    return digest.hexdigest()[:16]


def count_key(track: str) -> str:
    return "matched" if track == "matching" else "covered"


def empty_metrics(track: str) -> dict[str, dict[str, int]]:
    key = count_key(track)
    return {name: {key: 0, "total": 0} for name in METRIC_NAMES}


def validate_counter(value: Any, field: str, track: str) -> None:
    key_name = count_key(track)
    if not isinstance(value, dict) or set(value) != {key_name, "total"}:
        raise ProgressError(f"{field} must contain exactly {key_name} and total")
    for key in (key_name, "total"):
        if isinstance(value[key], bool) or not isinstance(value[key], int) or value[key] < 0:
            raise ProgressError(f"{field}.{key} must be a non-negative integer")
    if value[key_name] > value["total"]:
        raise ProgressError(f"{field}.{key_name} cannot exceed total")


def validate_metrics(value: Any, track: str, field: str = "metrics") -> None:
    if not isinstance(value, dict) or set(value) != set(METRIC_NAMES):
        raise ProgressError(f"{field} must contain exactly units, functions and bytes")
    for name in METRIC_NAMES:
        validate_counter(value[name], f"{field}.{name}", track)


def validate_unit_outcomes(value: Any, field: str = "units") -> None:
    if not isinstance(value, dict) or set(value) != set(UNIT_OUTCOME_NAMES):
        raise ProgressError(f"{field} must contain exactly probe, promoted and rejected")
    for name in UNIT_OUTCOME_NAMES:
        count = value[name]
        if isinstance(count, bool) or not isinstance(count, int) or count < 0:
            raise ProgressError(f"{field}.{name} must be a non-negative integer")
    if value["promoted"] + value["rejected"] > value["probe"]:
        raise ProgressError(f"{field}.promoted plus rejected cannot exceed probe")


def empty_named_counts(names: Iterable[str]) -> dict[str, int]:
    return {name: 0 for name in names}


def validate_targets(value: Any, field: str) -> None:
    if not isinstance(value, dict) or set(value) != set(METRIC_NAMES):
        raise ProgressError(f"{field} must contain exactly units, functions and bytes")
    for name in METRIC_NAMES:
        target = value[name]
        if isinstance(target, bool) or not isinstance(target, int) or target < 0:
            raise ProgressError(f"{field}.{name} must be a non-negative integer")


def require_string(value: dict[str, Any], field: str, *, allow_empty: bool = False) -> str:
    result = value.get(field)
    if not isinstance(result, str) or (not allow_empty and not result.strip()):
        raise ProgressError(f"{field} must be a string" + ("" if allow_empty else " with content"))
    return result


def validate_id(value: Any, field: str = "id") -> str:
    if not isinstance(value, str) or not SAFE_ID.fullmatch(value):
        raise ProgressError(f"{field} must match {SAFE_ID.pattern}")
    return value


def validate_config(config: dict[str, Any]) -> None:
    allowed_config = {
        "$schema",
        "schema_version",
        "kind",
        "project",
        "refresh_seconds",
        "worker_stale_seconds",
        "recent_change_limit",
        "sections",
        "inputs",
        "expected_workers",
    }
    unknown = set(config) - allowed_config
    if unknown:
        raise ProgressError(f"unknown project config fields: {', '.join(sorted(unknown))}")
    if config.get("schema_version") != SCHEMA_VERSION or config.get("kind") != "project":
        raise ProgressError(f"project config must have schema_version {SCHEMA_VERSION} and kind project")
    project = config.get("project")
    if not isinstance(project, dict):
        raise ProgressError("project must be an object")
    if set(project) != {"name", "short_name"}:
        raise ProgressError("project must contain exactly name and short_name")
    require_string(project, "name")
    require_string(project, "short_name")
    for field in ("refresh_seconds", "worker_stale_seconds", "recent_change_limit"):
        value = config.get(field)
        if isinstance(value, bool) or not isinstance(value, int) or value < 1:
            raise ProgressError(f"{field} must be a positive integer")

    sections = config.get("sections")
    if not isinstance(sections, list) or not sections:
        raise ProgressError("sections must be a non-empty array")
    seen: set[tuple[str, str]] = set()
    for index, section in enumerate(sections):
        prefix = f"sections[{index}]"
        if not isinstance(section, dict):
            raise ProgressError(f"{prefix} must be an object")
        expected_section_fields = {
            "id",
            "label",
            "dimension",
            "include_in_total",
            "optional",
            "targets",
        }
        if set(section) != expected_section_fields:
            raise ProgressError(f"{prefix} fields do not match schema v{SCHEMA_VERSION}")
        section_id = validate_id(section.get("id"), f"{prefix}.id")
        require_string(section, "label")
        dimension = section.get("dimension")
        if dimension not in {"section", "category"}:
            raise ProgressError(f"{prefix}.dimension must be section or category")
        key = (dimension, section_id)
        if key in seen:
            raise ProgressError(f"duplicate {dimension} id: {section_id}")
        seen.add(key)
        for field in ("include_in_total", "optional"):
            if not isinstance(section.get(field), bool):
                raise ProgressError(f"{prefix}.{field} must be boolean")
        validate_targets(section.get("targets"), f"{prefix}.targets")

    inputs = config.get("inputs")
    if not isinstance(inputs, dict):
        raise ProgressError("inputs must be an object")
    required_inputs = {"evidence_globs", "worker_globs", "change_globs"}
    allowed_inputs = required_inputs | {"proof_link_globs"}
    if not required_inputs <= set(inputs) or set(inputs) - allowed_inputs:
        raise ProgressError(
            "inputs must contain evidence_globs, worker_globs and change_globs; "
            "proof_link_globs is optional"
        )
    for field in (*sorted(required_inputs), "proof_link_globs"):
        if field not in inputs:
            continue
        globs = inputs.get(field)
        if not isinstance(globs, list) or not all(isinstance(item, str) and item for item in globs):
            raise ProgressError(f"inputs.{field} must be an array of non-empty strings")
        if any(Path(item).is_absolute() or ".." in Path(item).parts for item in globs):
            raise ProgressError(f"inputs.{field} paths must remain under the project root")

    expected = config.get("expected_workers")
    if not isinstance(expected, list):
        raise ProgressError("expected_workers must be an array")
    expected_ids: set[str] = set()
    for index, worker in enumerate(expected):
        if not isinstance(worker, dict):
            raise ProgressError(f"expected_workers[{index}] must be an object")
        if set(worker) != {"id", "label"}:
            raise ProgressError(f"expected_workers[{index}] must contain exactly id and label")
        worker_id = validate_id(worker.get("id"), f"expected_workers[{index}].id")
        require_string(worker, "label")
        if worker_id in expected_ids:
            raise ProgressError(f"duplicate expected worker id: {worker_id}")
        expected_ids.add(worker_id)


def section_ids(config: dict[str, Any], dimension: str) -> set[str]:
    return {item["id"] for item in config["sections"] if item["dimension"] == dimension}


def normalize_input(value: dict[str, Any]) -> tuple[dict[str, Any], bool]:
    """Convert v1 inputs to v2 in memory; legacy evidence is analysis-only."""
    if value.get("schema_version") == SCHEMA_VERSION:
        return value, False
    if value.get("schema_version") != 1:
        raise ProgressError(f"schema_version must be 1 or {SCHEMA_VERSION}")
    normalized = dict(value)
    normalized["schema_version"] = SCHEMA_VERSION
    kind = normalized.get("kind")
    if kind == "evidence":
        normalized["$schema"] = "../schema-v2.json#/$defs/evidence"
        normalized["track"] = "analysis"
        metrics = normalized.get("metrics")
        if isinstance(metrics, dict):
            normalized["metrics"] = {
                name: {
                    "covered": counter.get("matched") if isinstance(counter, dict) else None,
                    "total": counter.get("total") if isinstance(counter, dict) else None,
                }
                for name, counter in metrics.items()
            }
    elif kind in {"worker", "change"}:
        normalized["$schema"] = f"../schema-v2.json#/$defs/{kind}"
    return normalized, True


def validate_input(value: dict[str, Any], config: dict[str, Any]) -> None:
    if value.get("schema_version") != SCHEMA_VERSION:
        raise ProgressError(f"schema_version must be {SCHEMA_VERSION}")
    kind = value.get("kind")
    if kind not in KINDS:
        raise ProgressError(f"kind must be one of {', '.join(sorted(KINDS))}")
    validate_id(value.get("id"))
    parse_timestamp(value.get("updated_at"), "updated_at")

    if kind == "evidence":
        allowed = {
            "$schema",
            "schema_version",
            "kind",
            "id",
            "track",
            "section",
            "category",
            "metrics",
            "updated_at",
            "worker",
            "summary",
            "provenance",
            "units",
            "linked",
            "build_id",
            "commit",
        }
        unknown = set(value) - allowed
        if unknown:
            raise ProgressError(f"unknown evidence fields: {', '.join(sorted(unknown))}")
        track = value.get("track")
        if track not in {"matching", "analysis"}:
            raise ProgressError("track must be matching or analysis")
        section = require_string(value, "section")
        if section not in section_ids(config, "section"):
            raise ProgressError(f"unknown section: {section}")
        category = value.get("category")
        if category is not None and category not in section_ids(config, "category"):
            raise ProgressError(f"unknown category: {category}")
        validate_metrics(value.get("metrics"), track)
        provenance = value.get("provenance")
        if provenance is not None and provenance not in PROVENANCE_NAMES:
            raise ProgressError(
                f"provenance must be one of {', '.join(PROVENANCE_NAMES)}"
            )
        if "units" in value:
            validate_unit_outcomes(value["units"])
        linked = value.get("linked", False)
        if not isinstance(linked, bool):
            raise ProgressError("linked must be boolean")
        for field in ("build_id", "commit"):
            if field in value:
                require_string(value, field)
        if linked and ("build_id" not in value or "commit" not in value):
            raise ProgressError("linked evidence requires build_id and commit")
        if "summary" in value:
            require_string(value, "summary")
        if "worker" in value:
            require_string(value, "worker")
    elif kind == "worker":
        allowed = {"$schema", "schema_version", "kind", "id", "label", "status", "section", "task", "updated_at"}
        unknown = set(value) - allowed
        if unknown:
            raise ProgressError(f"unknown worker fields: {', '.join(sorted(unknown))}")
        require_string(value, "label")
        if value.get("status") not in WORKER_STATUSES:
            raise ProgressError(f"status must be one of {', '.join(sorted(WORKER_STATUSES))}")
        require_string(value, "task", allow_empty=True)
        if "section" in value and value["section"] not in section_ids(config, "section") | section_ids(config, "category"):
            raise ProgressError(f"unknown section: {value['section']}")
    else:
        allowed = {"$schema", "schema_version", "kind", "id", "summary", "section", "worker", "updated_at"}
        unknown = set(value) - allowed
        if unknown:
            raise ProgressError(f"unknown change fields: {', '.join(sorted(unknown))}")
        require_string(value, "summary")
        if "section" in value and value["section"] not in section_ids(config, "section") | section_ids(config, "category"):
            raise ProgressError(f"unknown section: {value['section']}")
        if "worker" in value:
            require_string(value, "worker")


def discover(root: Path, patterns: Iterable[str]) -> list[Path]:
    paths: set[Path] = set()
    for pattern in patterns:
        paths.update(path for path in root.glob(pattern) if path.is_file())
    return sorted(paths)


def load_inputs(
    root: Path, config: dict[str, Any], field: str, expected_kind: str
) -> tuple[list[tuple[Path, dict[str, Any]]], list[str], int]:
    records: list[tuple[Path, dict[str, Any]]] = []
    issues: list[str] = []
    legacy_count = 0
    ids: dict[str, Path] = {}
    for path in discover(root, config["inputs"][field]):
        try:
            value, legacy = normalize_input(read_json(path))
            validate_input(value, config)
            if value["kind"] != expected_kind:
                raise ProgressError(f"expected kind {expected_kind}, got {value['kind']}")
            previous = ids.get(value["id"])
            if previous is not None:
                raise ProgressError(f"duplicate id {value['id']} (also in {previous})")
            ids[value["id"]] = path
            records.append((path, value))
            legacy_count += int(legacy)
        except ProgressError as exc:
            issues.append(f"{path.relative_to(root)}: {exc}")
    return records, issues, legacy_count


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(chunk)
    except OSError as exc:
        raise ProgressError(f"{path}: {exc}") from exc
    return digest.hexdigest()


def proof_path(root: Path, relative: str, field: str) -> Path:
    path = Path(relative)
    if path.is_absolute() or ".." in path.parts:
        raise ProgressError(f"{field} must remain under the project root")
    resolved = (root / path).resolve()
    try:
        resolved.relative_to(root.resolve())
    except ValueError as exc:
        raise ProgressError(f"{field} must remain under the project root") from exc
    return resolved


def validate_proof_link(
    root: Path,
    value: dict[str, Any],
    record: dict[str, Any],
) -> dict[str, Any]:
    allowed = {
        "$schema",
        "schema_version",
        "kind",
        "id",
        "evidence_id",
        "proof",
        "proof_sha256",
        "provenance",
        "build_id",
        "commit",
        "units",
    }
    if set(value) - allowed:
        raise ProgressError(
            f"unknown proof-link fields: {', '.join(sorted(set(value) - allowed))}"
        )
    if value.get("schema_version") != SCHEMA_VERSION or value.get("kind") != "proof-link":
        raise ProgressError(f"proof link must have schema_version {SCHEMA_VERSION} and kind proof-link")
    validate_id(value.get("id"))
    evidence_id = validate_id(value.get("evidence_id"), "evidence_id")
    if evidence_id != record["id"] or record.get("track") != "matching":
        raise ProgressError("proof link must reference matching evidence with the same id")
    provenance = value.get("provenance")
    if provenance not in {"source", "sdk"}:
        raise ProgressError("proof-link provenance must be source or sdk")
    build_id = require_string(value, "build_id")
    commit = require_string(value, "commit")
    validate_unit_outcomes(value.get("units"), "units")
    expected_hash = require_string(value, "proof_sha256").lower()
    if re.fullmatch(r"[0-9a-f]{64}", expected_hash) is None:
        raise ProgressError("proof_sha256 must be a lowercase SHA-256 digest")
    proof_file = proof_path(root, require_string(value, "proof"), "proof")
    if sha256_file(proof_file) != expected_hash:
        raise ProgressError("proof_sha256 does not match the linked proof file")

    proof = read_json(proof_file)
    embedded = proof.get("dashboard_evidence")
    if not isinstance(embedded, dict):
        raise ProgressError("linked proof must contain dashboard_evidence")
    for field in ("id", "track", "section", "category", "metrics"):
        if embedded.get(field) != record.get(field):
            raise ProgressError(f"linked proof dashboard_evidence.{field} does not match")

    comparison = proof.get("comparison")
    layout = proof.get("layout_object")
    if isinstance(comparison, dict):
        exact = (
            comparison.get("byte_for_byte_equal") is True
            and comparison.get("mismatch_bytes") == 0
        )
    elif isinstance(layout, dict):
        sections = layout.get("sections")
        exact = (
            isinstance(sections, list)
            and bool(sections)
            and all(
                isinstance(section, dict)
                and section.get("byte_for_byte_equal") is True
                and isinstance(section.get("size"), int)
                for section in sections
            )
            and sum(section["size"] for section in sections)
            == record["metrics"]["bytes"]["matched"]
            and proof.get("matched_bytes") == record["metrics"]["bytes"]["matched"]
        )
    else:
        exact = False
    if not exact:
        raise ProgressError("linked proof is not an exact byte comparison")

    linked = dict(record)
    linked.update(
        {
            "provenance": provenance,
            "units": {name: value["units"][name] for name in UNIT_OUTCOME_NAMES},
            "linked": True,
            "build_id": build_id,
            "commit": commit,
            "_proof_verified": True,
        }
    )
    return linked


def apply_proof_links(
    root: Path,
    config: dict[str, Any],
    evidence: list[tuple[Path, dict[str, Any]]],
) -> tuple[list[tuple[Path, dict[str, Any]]], list[str], int]:
    patterns = config["inputs"].get("proof_link_globs", [])
    if not patterns:
        return evidence, [], 0
    by_id = {record["id"]: (path, record) for path, record in evidence}
    applied: dict[str, dict[str, Any]] = {}
    issues: list[str] = []
    for path in discover(root, patterns):
        try:
            link = read_json(path)
            evidence_id = link.get("evidence_id")
            if evidence_id not in by_id:
                continue
            if evidence_id in applied:
                raise ProgressError(f"duplicate proof link for evidence {evidence_id}")
            applied[evidence_id] = validate_proof_link(
                root,
                link,
                by_id[evidence_id][1],
            )
        except ProgressError as exc:
            issues.append(f"{path.relative_to(root)}: {exc}")
    linked_evidence = [
        (path, applied.get(record["id"], record)) for path, record in evidence
    ]
    return linked_evidence, issues, len(applied)


def add_metrics(
    target: dict[str, dict[str, int]], source: dict[str, dict[str, int]], track: str
) -> None:
    key = count_key(track)
    for name in METRIC_NAMES:
        target[name][key] += source[name][key]
        target[name]["total"] += source[name]["total"]


def merge_target_and_evidence(
    target: dict[str, int], evidence: dict[str, dict[str, int]], track: str
) -> dict[str, dict[str, int]]:
    result = empty_metrics(track)
    key = count_key(track)
    for name in METRIC_NAMES:
        result[name][key] = evidence[name][key]
        result[name]["total"] = max(target[name], evidence[name]["total"])
        result[name][key] = min(result[name][key], result[name]["total"])
    return result


def metric_progress(metrics: dict[str, dict[str, int]], track: str) -> tuple[float, str]:
    key = count_key(track)
    for name in ("bytes", "functions", "units"):
        counter = metrics[name]
        if counter["total"] > 0:
            rounded = round(100 * counter[key] / counter["total"], 2)
            if counter[key] < counter["total"] and rounded >= 100:
                rounded = 99.99
            return rounded, name
    return 0.0, "unmeasured"


def matching_evidence_is_linked(record: dict[str, Any]) -> bool:
    """Only explicit linked source/SDK proofs may credit matching metrics."""
    return (
        record.get("track") == "matching"
        and record.get("linked") is True
        and record.get("provenance") in {"source", "sdk"}
        and isinstance(record.get("build_id"), str)
        and bool(record["build_id"].strip())
        and isinstance(record.get("commit"), str)
        and (
            record.get("_proof_verified") is True
            or re.fullmatch(r"[0-9a-fA-F]{7,64}", record["commit"].strip()) is not None
        )
    )


def has_positive_matching_claim(record: dict[str, Any]) -> bool:
    if record.get("track") != "matching":
        return False
    return any(record["metrics"][name]["matched"] > 0 for name in METRIC_NAMES)


def evidence_unit_outcomes(
    record: dict[str, Any], matching_credited: bool
) -> dict[str, int]:
    explicit = record.get("units")
    if isinstance(explicit, dict):
        return {name: explicit[name] for name in UNIT_OUTCOME_NAMES}
    track = record["track"]
    key = count_key(track)
    measured = record["metrics"]["units"][key]
    if track == "matching":
        return {
            "probe": measured,
            "promoted": measured if matching_credited else 0,
            "rejected": 0 if matching_credited else measured,
        }
    return {"probe": measured, "promoted": 0, "rejected": 0}


def add_named_counts(target: dict[str, int], source: dict[str, int]) -> None:
    for name in target:
        target[name] += source[name]


def collect_progress(root: Path, config_path: Path | None = None) -> dict[str, Any]:
    root = root.resolve()
    config_file = config_path or root / DEFAULT_CONFIG
    if not config_file.is_absolute():
        config_file = root / config_file
    config = read_json(config_file)
    validate_config(config)

    evidence, evidence_issues, legacy_evidence = load_inputs(root, config, "evidence_globs", "evidence")
    evidence, proof_link_issues, applied_proof_links = apply_proof_links(
        root, config, evidence
    )
    workers, worker_issues, legacy_workers = load_inputs(root, config, "worker_globs", "worker")
    changes, change_issues, legacy_changes = load_inputs(root, config, "change_globs", "change")
    issues = evidence_issues + proof_link_issues + worker_issues + change_issues

    aggregates: dict[tuple[str, str], dict[str, Any]] = {}
    for section in config["sections"]:
        aggregates[(section["dimension"], section["id"])] = {
            "matching": empty_metrics("matching"),
            "analysis": empty_metrics("analysis"),
            "records": {"matching": 0, "analysis": 0},
            "provenance": empty_named_counts(PROVENANCE_NAMES),
            "pipeline_units": empty_named_counts(UNIT_OUTCOME_NAMES),
            "matching_rejected": 0,
        }
    evidence_changes: list[dict[str, Any]] = []
    for _, record in evidence:
        track = record["track"]
        matching_credited = matching_evidence_is_linked(record)
        provenance = record.get("provenance", "unmatched")
        outcomes = evidence_unit_outcomes(record, matching_credited)
        section_key = ("section", record["section"])
        if track != "matching" or matching_credited:
            add_metrics(aggregates[section_key][track], record["metrics"], track)
        elif has_positive_matching_claim(record):
            aggregates[section_key]["matching_rejected"] += 1
        aggregates[section_key]["records"][track] += 1
        aggregates[section_key]["provenance"][provenance] += outcomes["probe"]
        add_named_counts(aggregates[section_key]["pipeline_units"], outcomes)
        if record.get("category"):
            category_key = ("category", record["category"])
            if track != "matching" or matching_credited:
                add_metrics(aggregates[category_key][track], record["metrics"], track)
            elif has_positive_matching_claim(record):
                aggregates[category_key]["matching_rejected"] += 1
            aggregates[category_key]["records"][track] += 1
            aggregates[category_key]["provenance"][provenance] += outcomes["probe"]
            add_named_counts(aggregates[category_key]["pipeline_units"], outcomes)
        if record.get("summary"):
            evidence_changes.append(
                {
                    "id": f"evidence:{record['id']}",
                    "summary": record["summary"],
                    "section": record["section"],
                    "worker": record.get("worker", ""),
                    "track": track,
                    "updated_at": record["updated_at"],
                    "source": "evidence",
                    "provenance": provenance,
                    "matching_credited": matching_credited,
                }
            )

    section_results: list[dict[str, Any]] = []
    for section in config["sections"]:
        aggregate = aggregates[(section["dimension"], section["id"])]
        matching_metrics = merge_target_and_evidence(section["targets"], aggregate["matching"], "matching")
        analysis_metrics = merge_target_and_evidence(section["targets"], aggregate["analysis"], "analysis")
        matching_percent, matching_basis = metric_progress(matching_metrics, "matching")
        analysis_percent, analysis_basis = metric_progress(analysis_metrics, "analysis")
        has_measurement = any(
            matching_metrics[name]["total"] > 0 or analysis_metrics[name]["total"] > 0
            for name in METRIC_NAMES
        )
        section_results.append(
            {
                "id": section["id"],
                "label": section["label"],
                "dimension": section["dimension"],
                "include_in_total": section["include_in_total"],
                "optional": section["optional"],
                "available": not section["optional"] or has_measurement or sum(aggregate["records"].values()) > 0,
                # The unqualified fields are the dashboard's primary progress track.
                # Byte matching remains available below as an explicit secondary
                # track while the active work is decompilation and analysis.
                "percent": analysis_percent,
                "basis": analysis_basis,
                "metrics": analysis_metrics,
                "matching": {
                    "percent": matching_percent,
                    "basis": matching_basis,
                    "metrics": matching_metrics,
                    "evidence_records": aggregate["records"]["matching"],
                },
                "analysis": {
                    "percent": analysis_percent,
                    "basis": analysis_basis,
                    "metrics": analysis_metrics,
                    "evidence_records": aggregate["records"]["analysis"],
                },
                "provenance": aggregate["provenance"],
                "pipeline_units": aggregate["pipeline_units"],
                "matching_rejected": aggregate["matching_rejected"],
            }
        )

    total_matching = empty_metrics("matching")
    total_analysis = empty_metrics("analysis")
    total_provenance = empty_named_counts(PROVENANCE_NAMES)
    total_pipeline_units = empty_named_counts(UNIT_OUTCOME_NAMES)
    total_matching_rejected = 0
    included_sections: list[dict[str, Any]] = []
    for section in section_results:
        if not section["include_in_total"]:
            continue
        included_sections.append(section)
        add_metrics(total_matching, section["matching"]["metrics"], "matching")
        add_metrics(total_analysis, section["analysis"]["metrics"], "analysis")
        add_named_counts(total_provenance, section["provenance"])
        add_named_counts(total_pipeline_units, section["pipeline_units"])
        total_matching_rejected += section["matching_rejected"]
    matching_percent, matching_basis = metric_progress(total_matching, "matching")
    analysis_percent, analysis_basis = metric_progress(total_analysis, "analysis")

    now = utc_now()
    worker_values: dict[str, dict[str, Any]] = {
        item["id"]: {
            "id": item["id"],
            "label": item["label"],
            "status": "offline",
            "reported_status": "offline",
            "section": "",
            "task": "No heartbeat yet",
            "updated_at": None,
            "stale": True,
        }
        for item in config["expected_workers"]
    }
    for _, worker in workers:
        updated = parse_timestamp(worker["updated_at"], "updated_at")
        age_seconds = max(0, int((now - updated).total_seconds()))
        stale = age_seconds > config["worker_stale_seconds"]
        effective = worker["status"]
        if stale and effective in {"working", "idle"}:
            effective = "offline"
        worker_values[worker["id"]] = {
            "id": worker["id"],
            "label": worker["label"],
            "status": effective,
            "reported_status": worker["status"],
            "section": worker.get("section", ""),
            "task": worker["task"],
            "updated_at": worker["updated_at"],
            "age_seconds": age_seconds,
            "stale": stale,
        }

    recent_changes = evidence_changes
    recent_changes.extend(
        {
            "id": value["id"],
            "summary": value["summary"],
            "section": value.get("section", ""),
            "worker": value.get("worker", ""),
            "updated_at": value["updated_at"],
            "source": "change",
        }
        for _, value in changes
    )
    recent_changes.sort(key=lambda item: parse_timestamp(item["updated_at"], "updated_at"), reverse=True)
    recent_changes = recent_changes[: config["recent_change_limit"]]

    linked_matching_records = sum(
        1 for _, record in evidence if matching_evidence_is_linked(record)
    )

    return {
        "schema_version": SCHEMA_VERSION,
        "generated_at": iso_now(),
        "build": {
            "id": "live",
            "commit": git_commit(root),
            "published_at": None,
            "last_error": None,
        },
        "project": config["project"],
        "refresh_seconds": config["refresh_seconds"],
        "overall": {
            "percent": analysis_percent,
            "basis": analysis_basis,
            "metrics": total_analysis,
            "matching": {
                "percent": matching_percent,
                "basis": matching_basis,
                "metrics": total_matching,
            },
            "analysis": {
                "percent": analysis_percent,
                "basis": analysis_basis,
                "metrics": total_analysis,
            },
            "provenance": total_provenance,
            "pipeline_units": total_pipeline_units,
            "matching_rejected": total_matching_rejected,
            "total_sections": len(included_sections),
        },
        "sections": section_results,
        "workers": sorted(worker_values.values(), key=lambda item: item["label"].casefold()),
        "recent_changes": recent_changes,
        "issues": issues,
        "sources": {
            "evidence": len(evidence),
            "workers": len(workers),
            "changes": len(changes),
            "legacy_inputs": legacy_evidence + legacy_workers + legacy_changes,
            "matching_linked": linked_matching_records,
            "matching_rejected": sum(
                1
                for _, record in evidence
                if record["track"] == "matching"
                and not matching_evidence_is_linked(record)
                and has_positive_matching_claim(record)
            ),
            "proof_links": applied_proof_links,
        },
    }


def dashboard_progress_value(
    root: Path,
    config_path: Path,
    dashboard_root: Path | None = None,
) -> dict[str, Any]:
    """Serve the last validated publication, or a live v2 view before the first one."""
    snapshot = (dashboard_root or root / "dashboard") / "progress.json"
    if snapshot.is_file():
        try:
            value = read_json(snapshot)
            build = value.get("build")
            if (
                value.get("schema_version") == SCHEMA_VERSION
                and isinstance(build, dict)
                and isinstance(build.get("id"), str)
                and isinstance(build.get("commit"), str)
                and "published_at" in build
                and "last_error" in build
                and isinstance(value.get("overall"), dict)
                and isinstance(value["overall"].get("provenance"), dict)
                and isinstance(value["overall"].get("pipeline_units"), dict)
                and isinstance(value["overall"].get("matching_rejected"), int)
                and isinstance(value.get("sections"), list)
                and isinstance(value.get("sources"), dict)
                and isinstance(value["sources"].get("matching_linked"), int)
                and isinstance(value["sources"].get("matching_rejected"), int)
            ):
                return value
        except ProgressError:
            pass
    return collect_progress(root, config_path)


def config_for(root: Path, config_arg: str | None) -> tuple[Path, dict[str, Any]]:
    path = Path(config_arg) if config_arg else DEFAULT_CONFIG
    if not path.is_absolute():
        path = root / path
    value = read_json(path)
    validate_config(value)
    return path, value


def validate_installation(root: Path, config_arg: str | None) -> list[str]:
    issues: list[str] = []
    schema_path = root / "tools/progress/schema-v2.json"
    try:
        schema = read_json(schema_path)
        if schema.get("$schema") != "https://json-schema.org/draft/2020-12/schema":
            issues.append("schema-v2.json is not declared as JSON Schema draft 2020-12")
        definitions = schema.get("$defs")
        if not isinstance(definitions, dict) or not {"project", "evidence", "worker", "change"} <= set(definitions):
            issues.append("schema-v2.json is missing required definitions")
    except ProgressError as exc:
        issues.append(str(exc))
    try:
        config_path, _ = config_for(root, config_arg)
        result = collect_progress(root, config_path)
        issues.extend(result["issues"])
    except ProgressError as exc:
        return issues + [str(exc)]
    for relative in (
        "dashboard/index.html",
        "dashboard/app.js",
        "dashboard/styles.css",
    ):
        if not (root / relative).is_file():
            issues.append(f"missing required file: {relative}")
    return issues


class DashboardHandler(BaseHTTPRequestHandler):
    server_version = "FSAEProgress/2"

    def do_GET(self) -> None:  # noqa: N802 - required by BaseHTTPRequestHandler
        self.serve_request(send_body=True)

    def do_HEAD(self) -> None:  # noqa: N802 - required by BaseHTTPRequestHandler
        self.serve_request(send_body=False)

    def serve_request(self, send_body: bool) -> None:
        path = urlsplit(self.path).path
        if path == "/api/progress":
            self.send_progress(send_body)
            return
        if path == "/healthz":
            self.send_health(send_body)
            return
        asset = STATIC_FILES.get(path)
        if asset is None:
            self.send_error(404, "Not found")
            return
        self.send_asset(asset, send_body)

    def send_no_store_headers(self) -> None:
        self.send_header("Cache-Control", NO_STORE_POLICY)
        self.send_header("Pragma", "no-cache")
        self.send_header("Expires", "0")
        self.send_header("Surrogate-Control", "no-store")

    def send_json(
        self,
        status: int,
        value: dict[str, Any],
        send_body: bool = True,
        dashboard_version: str | None = None,
    ) -> None:
        body = json.dumps(value, ensure_ascii=False, separators=(",", ":")).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_no_store_headers()
        if dashboard_version is not None:
            self.send_header("X-Dashboard-Version", dashboard_version)
        self.send_header("X-Content-Type-Options", "nosniff")
        self.end_headers()
        if send_body:
            self.wfile.write(body)

    def send_progress(self, send_body: bool = True) -> None:
        try:
            value = dashboard_progress_value(  # type: ignore[attr-defined]
                self.server.project_root,
                self.server.config_path,
                self.server.dashboard_root,
            )
            version = dashboard_asset_version(self.server.dashboard_root)  # type: ignore[attr-defined]
            value["dashboard_version"] = version
            self.send_json(200, value, send_body, version)
        except ProgressError as exc:
            self.send_json(500, {"schema_version": SCHEMA_VERSION, "error": str(exc)}, send_body)

    def send_health(self, send_body: bool = True) -> None:
        try:
            value = collect_progress(self.server.project_root, self.server.config_path)  # type: ignore[attr-defined]
            status = "ok" if not value["issues"] else "degraded"
            version = dashboard_asset_version(self.server.dashboard_root)  # type: ignore[attr-defined]
            self.send_json(
                200,
                {"status": status, "issues": value["issues"], "dashboard_version": version},
                send_body,
                version,
            )
        except ProgressError as exc:
            self.send_json(500, {"status": "error", "error": str(exc)}, send_body)

    def send_asset(self, name: str, send_body: bool = True) -> None:
        path = self.server.dashboard_root / name  # type: ignore[attr-defined]
        try:
            body = path.read_bytes()
        except OSError:
            self.send_error(404, "Not found")
            return
        version = dashboard_asset_version(self.server.dashboard_root)  # type: ignore[attr-defined]
        if name == "index.html":
            token = version.encode("ascii")
            body = body.replace(DASHBOARD_ASSET_TOKEN.encode("ascii"), token)
        content_type = mimetypes.guess_type(path.name)[0] or "application/octet-stream"
        self.send_response(200)
        self.send_header("Content-Type", f"{content_type}; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_no_store_headers()
        self.send_header("X-Dashboard-Version", version)
        if name == "index.html":
            self.send_header("Clear-Site-Data", '"cache", "storage"')
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header("Content-Security-Policy", "default-src 'self'; style-src 'self'; script-src 'self'; connect-src 'self'")
        self.end_headers()
        if send_body:
            self.wfile.write(body)

    def log_message(self, fmt: str, *args: Any) -> None:
        sys.stderr.write(f"[{self.log_date_time_string()}] {fmt % args}\n")


def make_server(root: Path, config_path: Path, host: str, port: int) -> ThreadingHTTPServer:
    server = ThreadingHTTPServer((host, port), DashboardHandler)
    server.daemon_threads = True
    server.project_root = root.resolve()  # type: ignore[attr-defined]
    server.dashboard_root = (root / "dashboard").resolve()  # type: ignore[attr-defined]
    server.config_path = config_path.resolve()  # type: ignore[attr-defined]
    return server


def counter_from_args(args: argparse.Namespace, name: str, track: str) -> dict[str, int]:
    key = count_key(track)
    wrong_key = "covered" if key == "matched" else "matched"
    value = getattr(args, f"{name}_{key}")
    wrong_value = getattr(args, f"{name}_{wrong_key}")
    if wrong_value is not None:
        raise ProgressError(f"--{name}-{wrong_key} is not valid for the {track} track")
    return {key: value or 0, "total": getattr(args, f"{name}_total")}


def record_publication_error(
    output: Path,
    build_id: str,
    commit: str,
    message: str,
) -> None:
    """Attach an error to the last good snapshot without replacing its metrics."""
    try:
        previous = read_json(output)
    except ProgressError:
        return
    build = previous.get("build")
    if not isinstance(build, dict):
        build = {
            "id": "unknown",
            "commit": "unknown",
            "published_at": None,
        }
        previous["build"] = build
    build["last_error"] = {
        "message": message,
        "at": iso_now(),
        "build_id": build_id,
        "commit": commit,
    }
    atomic_write_json(output, previous)


def publish_dashboard_snapshot(
    root: Path,
    config_path: Path,
    output: Path,
    build_id: str,
    commit: str,
) -> dict[str, Any]:
    value = collect_progress(root, config_path)
    if value["issues"]:
        raise ProgressError(
            "publication refused because progress inputs are invalid: "
            + "; ".join(value["issues"])
        )
    value["build"] = {
        "id": build_id,
        "commit": commit,
        "published_at": iso_now(),
        "last_error": None,
    }
    atomic_write_json(output, value)
    return value


def cmd_collect(args: argparse.Namespace, root: Path) -> int:
    config_path, _ = config_for(root, args.config)
    value = collect_progress(root, config_path)
    rendered = json.dumps(value, indent=2 if args.pretty else None, ensure_ascii=False)
    if args.output:
        output = Path(args.output)
        if not output.is_absolute():
            output = root / output
        atomic_write_json(output, value)
    else:
        print(rendered)
    return 1 if value["issues"] else 0


def cmd_publish(args: argparse.Namespace, root: Path) -> int:
    output = Path(args.output)
    if not output.is_absolute():
        output = root / output
    build_id = args.build_id
    commit = args.commit or git_commit(root)
    try:
        build_id = validate_id(build_id, "build_id")
        if not isinstance(commit, str) or not commit.strip():
            raise ProgressError("commit must be a string with content")
        config_path, _ = config_for(root, args.config)
        publish_dashboard_snapshot(
            root,
            config_path,
            output,
            build_id,
            commit,
        )
    except (OSError, ProgressError) as exc:
        try:
            record_publication_error(output, build_id, commit, str(exc))
        except (OSError, ProgressError):
            pass
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2
    print(f"Published dashboard build {build_id} atomically to {output}.")
    return 0


def cmd_validate(args: argparse.Namespace, root: Path) -> int:
    issues = validate_installation(root, args.config)
    if issues:
        for issue in issues:
            print(f"ERROR: {issue}", file=sys.stderr)
        return 1
    print("Progress dashboard validation passed.")
    return 0


def cmd_serve(args: argparse.Namespace, root: Path) -> int:
    config_path, _ = config_for(root, args.config)
    initial = collect_progress(root, config_path)
    missing = [
        relative
        for relative in (
            "tools/progress/schema-v2.json",
            "dashboard/index.html",
            "dashboard/app.js",
            "dashboard/styles.css",
        )
        if not (root / relative).is_file()
    ]
    if missing:
        raise ProgressError("dashboard validation failed:\n- " + "\n- ".join(f"missing required file: {item}" for item in missing))
    if initial["issues"]:
        print(f"WARNING: serving with {len(initial['issues'])} ignored input(s); see /healthz", file=sys.stderr)
    server = make_server(root, config_path, args.host, args.port)
    address, port = server.server_address[:2]
    print(f"Progress dashboard: http://{address}:{port}", flush=True)
    try:
        server.serve_forever(poll_interval=0.25)
    except KeyboardInterrupt:
        print("\nStopping progress dashboard.")
    finally:
        server.server_close()
    return 0


def cmd_set_worker(args: argparse.Namespace, root: Path) -> int:
    _, config = config_for(root, args.config)
    worker_id = validate_id(args.id)
    value: dict[str, Any] = {
        "$schema": "../schema-v2.json#/$defs/worker",
        "schema_version": SCHEMA_VERSION,
        "kind": "worker",
        "id": worker_id,
        "label": args.label,
        "status": args.status,
        "task": args.task,
        "updated_at": iso_now(),
    }
    if args.section:
        value["section"] = args.section
    validate_input(value, config)
    atomic_write_json(root / "tools/progress/workers" / f"{worker_id}.json", value)
    print(f"Updated worker {worker_id}.")
    return 0


def cmd_heartbeat_worker(args: argparse.Namespace, root: Path) -> int:
    _, config = config_for(root, args.config)
    worker_id = validate_id(args.id)
    path = root / "tools/progress/workers" / f"{worker_id}.json"
    value = read_json(path)
    if value.get("kind") != "worker" or value.get("id") != worker_id:
        raise ProgressError(f"worker heartbeat identity mismatch: {path}")
    validate_input(value, config)
    value["updated_at"] = iso_now()
    validate_input(value, config)
    atomic_write_json(path, value)
    print(f"Refreshed worker {worker_id}.")
    return 0


def cmd_set_evidence(args: argparse.Namespace, root: Path) -> int:
    _, config = config_for(root, args.config)
    evidence_id = validate_id(args.id)
    metrics = {name: counter_from_args(args, name, args.track) for name in METRIC_NAMES}
    value: dict[str, Any] = {
        "$schema": "../schema-v2.json#/$defs/evidence",
        "schema_version": SCHEMA_VERSION,
        "kind": "evidence",
        "id": evidence_id,
        "track": args.track,
        "section": args.section,
        "metrics": metrics,
        "updated_at": iso_now(),
        "provenance": args.provenance,
    }
    outcome_values = {
        "probe": args.probe_units,
        "promoted": args.promoted_units,
        "rejected": args.rejected_units,
    }
    if any(count is not None for count in outcome_values.values()):
        value["units"] = {
            name: count or 0 for name, count in outcome_values.items()
        }
    if args.linked:
        value["linked"] = True
        value["build_id"] = args.build_id or ""
        value["commit"] = args.commit or git_commit(root)
    matching_claimed = has_positive_matching_claim(value)
    if matching_claimed and not matching_evidence_is_linked(value):
        raise ProgressError(
            "matching counters require --linked, --provenance source|sdk, "
            "--build-id and --commit"
        )
    for field in ("category", "worker", "summary"):
        argument = getattr(args, field)
        if argument:
            value[field] = argument
    validate_input(value, config)
    atomic_write_json(root / "tools/progress/evidence" / f"{evidence_id}.json", value)
    print(f"Updated evidence {evidence_id}.")
    return 0


def cmd_add_change(args: argparse.Namespace, root: Path) -> int:
    _, config = config_for(root, args.config)
    change_id = validate_id(args.id or f"change-{uuid.uuid4().hex[:12]}")
    value: dict[str, Any] = {
        "$schema": "../schema-v2.json#/$defs/change",
        "schema_version": SCHEMA_VERSION,
        "kind": "change",
        "id": change_id,
        "summary": args.summary,
        "updated_at": iso_now(),
    }
    for field in ("section", "worker"):
        argument = getattr(args, field)
        if argument:
            value[field] = argument
    validate_input(value, config)
    atomic_write_json(root / "tools/progress/changes" / f"{change_id}.json", value)
    print(f"Added change {change_id}.")
    return 0


def add_counter_arguments(parser: argparse.ArgumentParser) -> None:
    for name in METRIC_NAMES:
        parser.add_argument(f"--{name}-matched", type=int)
        parser.add_argument(f"--{name}-covered", type=int)
        parser.add_argument(f"--{name}-total", type=int, default=0)


def make_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", help="project root (defaults to the script's repository)")
    parser.add_argument("--config", help="config path, relative to project root")
    subparsers = parser.add_subparsers(dest="command", required=True)

    collect = subparsers.add_parser("collect", help="recalculate and print progress JSON")
    collect.add_argument("--pretty", action="store_true")
    collect.add_argument("--output", help="atomically write the result to a file")

    publish = subparsers.add_parser(
        "publish",
        help="validate and atomically publish one dashboard snapshot",
    )
    publish.add_argument("--build-id", required=True)
    publish.add_argument("--commit")
    publish.add_argument("--output", default="dashboard/progress.json")

    subparsers.add_parser("validate", help="validate config, inputs and dashboard assets")

    serve = subparsers.add_parser("serve", help="serve the live dashboard")
    serve.add_argument("--host", default="127.0.0.1")
    serve.add_argument("--port", type=int, default=8765)

    worker = subparsers.add_parser("set-worker", help="write or refresh a worker heartbeat")
    worker.add_argument("--id", required=True)
    worker.add_argument("--label", required=True)
    worker.add_argument("--status", choices=sorted(WORKER_STATUSES), required=True)
    worker.add_argument("--section")
    worker.add_argument("--task", default="")

    heartbeat = subparsers.add_parser(
        "heartbeat-worker",
        help="refresh only updated_at for an existing worker",
    )
    heartbeat.add_argument("--id", required=True)

    evidence = subparsers.add_parser("set-evidence", help="atomically write one progress fragment")
    evidence.add_argument("--id", required=True)
    evidence.add_argument("--track", choices=("matching", "analysis"), required=True)
    evidence.add_argument("--section", required=True)
    evidence.add_argument("--category")
    evidence.add_argument("--worker")
    evidence.add_argument("--summary")
    evidence.add_argument("--provenance", choices=PROVENANCE_NAMES, default="unmatched")
    evidence.add_argument("--probe-units", type=int)
    evidence.add_argument("--promoted-units", type=int)
    evidence.add_argument("--rejected-units", type=int)
    evidence.add_argument("--linked", action="store_true")
    evidence.add_argument("--build-id")
    evidence.add_argument("--commit")
    add_counter_arguments(evidence)

    change = subparsers.add_parser("add-change", help="append a dashboard activity entry")
    change.add_argument("--id")
    change.add_argument("--summary", required=True)
    change.add_argument("--section")
    change.add_argument("--worker")
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = make_parser()
    args = parser.parse_args(argv)
    root = Path(args.root).resolve() if args.root else DEFAULT_ROOT
    commands = {
        "collect": cmd_collect,
        "publish": cmd_publish,
        "validate": cmd_validate,
        "serve": cmd_serve,
        "set-worker": cmd_set_worker,
        "heartbeat-worker": cmd_heartbeat_worker,
        "set-evidence": cmd_set_evidence,
        "add-change": cmd_add_change,
    }
    try:
        return commands[args.command](args, root)
    except ProgressError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
