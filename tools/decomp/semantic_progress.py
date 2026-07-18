#!/usr/bin/env python3
"""Measure semantic ARM9 recovery and refresh its dashboard evidence.

The raw Ghidra export is the first analysis stage.  Semantic C recovery is the
second stage.  Its recovered body bytes are projected over the part of the
ARM9 game-code budget not already covered by the raw export, so the dashboard
advances monotonically from the raw-analysis baseline to 100% only when every
catalogued game function has a semantic source counterpart.
"""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import re
from typing import Any, Iterable, Sequence


PROJECT_ROOT = Path(__file__).resolve().parents[2]
# Semantic source functions carry a standalone leading address annotation.
# Anchoring after the C comment marker deliberately excludes incidental ROM
# addresses in prose, vtable initializers, RTTI notes and unresolved externs.
ADDRESS_RE = re.compile(r"(?m)^\s*(?:/\*+|\*)\s*0x(02[0-9a-fA-F]{6})\b")


class SemanticProgressError(RuntimeError):
    """Raised when semantic progress cannot be measured safely."""


def read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise SemanticProgressError(f"cannot read {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise SemanticProgressError(f"expected a JSON object in {path}")
    return value


def source_addresses(paths: Iterable[Path]) -> set[int]:
    addresses: set[int] = set()
    for path in paths:
        try:
            text = path.read_text(encoding="utf-8")
        except OSError as exc:
            raise SemanticProgressError(f"cannot read semantic source {path}: {exc}") from exc
        addresses.update(int(match.group(1), 16) for match in ADDRESS_RE.finditer(text))
    return addresses


def semantic_source_paths(source_dir: Path) -> list[Path]:
    """Return reviewed semantic units, excluding address-named raw drafts."""
    return sorted(
        path for path in source_dir.rglob("*.c") if not path.name.endswith("_raw.c")
    )


def measure(catalog: dict[str, Any], addresses: set[int], game_total_bytes: int) -> dict[str, int]:
    summary = catalog.get("summary")
    functions = catalog.get("functions")
    if not isinstance(summary, dict) or not isinstance(functions, list):
        raise SemanticProgressError("catalog is missing summary/functions")

    raw_bytes = summary.get("game_candidate_body_bytes")
    raw_functions = summary.get("game_candidates")
    raw_units = summary.get("raw_module_shards")
    if not all(isinstance(value, int) and value > 0 for value in (raw_bytes, raw_functions, raw_units)):
        raise SemanticProgressError("catalog summary counters are invalid")
    if game_total_bytes < raw_bytes:
        raise SemanticProgressError("game byte target is smaller than the raw function bodies")

    game_functions: dict[int, int] = {}
    for item in functions:
        if not isinstance(item, dict) or item.get("classification") != "game_candidate":
            continue
        entry = item.get("entry")
        body_bytes = item.get("body_bytes")
        if not isinstance(entry, str) or not isinstance(body_bytes, int) or body_bytes < 0:
            raise SemanticProgressError("catalog contains an invalid game function")
        game_functions[int(entry, 16)] = body_bytes

    semantic = addresses & set(game_functions)
    semantic_bytes = sum(game_functions[address] for address in semantic)
    remaining_budget = game_total_bytes - raw_bytes
    projected_semantic_bytes = (remaining_budget * semantic_bytes) // raw_bytes
    return {
        "raw_bytes": raw_bytes,
        "raw_functions": raw_functions,
        "raw_units": raw_units,
        "semantic_bytes": semantic_bytes,
        "semantic_functions": len(semantic),
        "covered_bytes": raw_bytes + projected_semantic_bytes,
        "total_bytes": game_total_bytes,
    }


def evidence(metrics: dict[str, int], source_files: int, timestamp: str) -> dict[str, Any]:
    return {
        "$schema": "../schema-v2.json#/$defs/evidence",
        "schema_version": 2,
        "kind": "evidence",
        "id": "coordinator-arm9-ghidra-catalog",
        "track": "analysis",
        "section": "arm9",
        "metrics": {
            "units": {
                "covered": metrics["raw_units"] + source_files,
                "total": metrics["raw_units"] + source_files,
            },
            "functions": {
                "covered": metrics["raw_functions"] + metrics["semantic_functions"],
                "total": metrics["raw_functions"] * 2,
            },
            "bytes": {
                "covered": metrics["covered_bytes"],
                "total": metrics["total_bytes"],
            },
        },
        "updated_at": timestamp,
        "category": "game",
        "worker": "coordinator",
        "summary": (
            f"Ghidra exported {metrics['raw_functions']} game candidates; "
            f"{metrics['semantic_functions']} functions / {metrics['semantic_bytes']} body bytes "
            "now have reviewed semantic C"
        ),
    }


def atomic_write_json(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")
    temporary.replace(path)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=PROJECT_ROOT, help=argparse.SUPPRESS)
    parser.add_argument("--catalog", type=Path, default=Path("build/decomp/arm9/catalog.json"))
    parser.add_argument("--source-dir", type=Path, default=Path("src/arm9"))
    parser.add_argument("--game-total-bytes", type=int, default=1_082_378)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("tools/progress/evidence/coordinator-arm9-ghidra-catalog.json"),
    )
    return parser


def resolve(root: Path, path: Path) -> Path:
    return path.resolve() if path.is_absolute() else (root / path).resolve()


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    root = args.root.resolve()
    catalog = read_json(resolve(root, args.catalog))
    source_dir = resolve(root, args.source_dir)
    sources = semantic_source_paths(source_dir)
    metrics = measure(catalog, source_addresses(sources), args.game_total_bytes)
    timestamp = datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")
    value = evidence(metrics, len(sources), timestamp)
    atomic_write_json(resolve(root, args.output), value)
    print(json.dumps({"status": "updated", **metrics, "source_files": len(sources)}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
