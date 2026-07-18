#!/usr/bin/env python3
"""Validate and apply public ARM9 semantic names through Ghidra headless."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys
from typing import Any, Callable, Sequence


PROJECT_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_CONFIG = Path("config/decomp/arm9-symbols.json")
DEFAULT_BINARY_MAP = Path("config/binary-map.json")
DEFAULT_ANALYSIS_MANIFEST = Path("build/decomp/arm9.analysis.json")
DEFAULT_PROJECT_DIR = Path("build/ghidra")
DEFAULT_REPORT = Path("build/decomp/arm9/semantic-symbols-report.json")
PURPOSE = "analysis-only semantic function names; never matching evidence"
ADDRESS_RE = re.compile(r"^0x[0-9a-fA-F]{8}$")
NAME_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
CONFIDENCE = {"verified", "descriptive", "provisional"}


class SemanticSymbolError(RuntimeError):
    """A semantic-name config or headless application error."""


Runner = Callable[..., Any]


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def read_json(path: Path, label: str) -> dict[str, Any]:
    try:
        with path.open("r", encoding="utf-8") as stream:
            value = json.load(stream)
    except (OSError, json.JSONDecodeError) as exc:
        raise SemanticSymbolError(f"cannot read {label} {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise SemanticSymbolError(f"{label} must be a JSON object: {path}")
    return value


def _string(value: dict[str, Any], field: str) -> str:
    result = value.get(field)
    if not isinstance(result, str) or not result.strip():
        raise SemanticSymbolError(f"{field} must be a non-empty string")
    return result


def _sha256(value: dict[str, Any], field: str) -> str:
    result = _string(value, field)
    if not SHA256_RE.fullmatch(result):
        raise SemanticSymbolError(f"{field} must be a lowercase SHA-256 digest")
    return result


def validate_config_data(value: dict[str, Any]) -> list[dict[str, Any]]:
    allowed = {
        "$schema",
        "schema_version",
        "image",
        "program",
        "program_sha256",
        "target_image_sha256",
        "purpose",
        "symbols",
    }
    unknown = set(value) - allowed
    if unknown:
        raise SemanticSymbolError(f"unknown config fields: {', '.join(sorted(unknown))}")
    if value.get("schema_version") != 1:
        raise SemanticSymbolError("schema_version must be 1")
    if value.get("image") != "arm9":
        raise SemanticSymbolError("image must be arm9")
    if value.get("program") != "arm9.analysis.elf":
        raise SemanticSymbolError("program must be arm9.analysis.elf")
    _sha256(value, "program_sha256")
    _sha256(value, "target_image_sha256")
    if value.get("purpose") != PURPOSE:
        raise SemanticSymbolError(f"purpose must be exactly {PURPOSE!r}")
    symbols = value.get("symbols")
    if not isinstance(symbols, list):
        raise SemanticSymbolError("symbols must be an array")

    addresses: set[int] = set()
    names: set[str] = set()
    normalized: list[dict[str, Any]] = []
    required_symbol_fields = {"address", "name", "expected_names", "confidence", "rationale"}
    allowed_symbol_fields = required_symbol_fields | {"create_if_missing"}
    for index, symbol in enumerate(symbols):
        prefix = f"symbols[{index}]"
        if (
            not isinstance(symbol, dict)
            or not required_symbol_fields.issubset(symbol)
            or not set(symbol).issubset(allowed_symbol_fields)
        ):
            raise SemanticSymbolError(f"{prefix} fields do not match schema v1")
        create_if_missing = symbol.get("create_if_missing", False)
        if "create_if_missing" in symbol and create_if_missing is not True:
            raise SemanticSymbolError(f"{prefix}.create_if_missing may only be true")
        address_text = _string(symbol, "address")
        if not ADDRESS_RE.fullmatch(address_text):
            raise SemanticSymbolError(f"{prefix}.address must be 0x followed by eight hex digits")
        address = int(address_text, 16)
        if address in addresses:
            raise SemanticSymbolError(f"duplicate function address: {address_text}")
        addresses.add(address)
        name = _string(symbol, "name")
        if not NAME_RE.fullmatch(name):
            raise SemanticSymbolError(f"{prefix}.name is not a portable Ghidra identifier")
        if name in names:
            raise SemanticSymbolError(f"duplicate semantic name: {name}")
        names.add(name)
        expected = symbol.get("expected_names")
        if (
            not isinstance(expected, list)
            or not expected
            or not all(isinstance(item, str) and NAME_RE.fullmatch(item) for item in expected)
            or len(set(expected)) != len(expected)
        ):
            raise SemanticSymbolError(f"{prefix}.expected_names must contain unique identifiers")
        if name not in expected:
            raise SemanticSymbolError(f"{prefix}.expected_names must include the desired name for idempotence")
        if symbol.get("confidence") not in CONFIDENCE:
            raise SemanticSymbolError(f"{prefix}.confidence is invalid")
        rationale = _string(symbol, "rationale")
        normalized.append(
            {
                "address": address,
                "address_text": f"0x{address:08x}",
                "name": name,
                "expected_names": list(expected),
                "confidence": symbol["confidence"],
                "rationale": rationale,
                "create_if_missing": create_if_missing,
            }
        )
    return normalized


def validate_external_identity(
    config: dict[str, Any],
    binary_map_path: Path,
    analysis_manifest_path: Path,
    root: Path = PROJECT_ROOT,
) -> None:
    binary_map = read_json(binary_map_path, "binary map")
    executables = binary_map.get("executables")
    if not isinstance(executables, list):
        raise SemanticSymbolError("binary map executables must be an array")
    arm9 = [item for item in executables if isinstance(item, dict) and item.get("id") == "arm9"]
    if len(arm9) != 1 or not isinstance(arm9[0].get("rom"), dict):
        raise SemanticSymbolError("binary map must contain exactly one ARM9 executable")
    if arm9[0]["rom"].get("sha256") != config["target_image_sha256"]:
        raise SemanticSymbolError("target_image_sha256 does not match config/binary-map.json")

    manifest = read_json(analysis_manifest_path, "analysis ELF manifest")
    if manifest.get("image") != "arm9":
        raise SemanticSymbolError("analysis manifest image is not arm9")
    if manifest.get("input_sha256") != config["target_image_sha256"]:
        raise SemanticSymbolError("analysis manifest does not use the configured ARM9 target")
    if manifest.get("output_sha256") != config["program_sha256"]:
        raise SemanticSymbolError("program_sha256 does not match the analysis ELF manifest")
    output = manifest.get("output")
    if not isinstance(output, str):
        raise SemanticSymbolError("analysis manifest output path is invalid")
    program_path = (root / output).resolve() if not Path(output).is_absolute() else Path(output)
    if not program_path.is_file() or sha256_file(program_path) != config["program_sha256"]:
        raise SemanticSymbolError("analysis ELF bytes do not match program_sha256")


def validate_config(
    config_path: Path,
    binary_map_path: Path,
    analysis_manifest_path: Path,
    root: Path = PROJECT_ROOT,
) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    config = read_json(config_path, "semantic-symbol config")
    symbols = validate_config_data(config)
    validate_external_identity(config, binary_map_path, analysis_manifest_path, root)
    return config, symbols


def build_ghidra_command(
    headless: str,
    project_dir: Path,
    project_name: str,
    program: str,
    script_dir: Path,
    config_path: Path,
    report_path: Path,
    dry_run: bool,
) -> list[str]:
    command = [
        headless,
        str(project_dir),
        project_name,
        "-process",
        program,
        "-noanalysis",
        "-scriptPath",
        str(script_dir),
        "-postScript",
        "ApplySemanticSymbols.java",
        str(config_path),
        str(report_path),
    ]
    if dry_run:
        command.append("--dry-run")
    return command


def validate_report(
    report_path: Path,
    config_path: Path,
    config: dict[str, Any],
    requested: int,
    dry_run: bool,
) -> dict[str, Any]:
    report = read_json(report_path, "Ghidra semantic-symbol report")
    expected_status = "dry-run" if dry_run else "applied"
    if report.get("schema_version") != 1 or report.get("status") != expected_status:
        raise SemanticSymbolError("Ghidra report status is invalid")
    if report.get("program") != config["program"]:
        raise SemanticSymbolError("Ghidra report names a different program")
    if report.get("program_sha256") != config["program_sha256"]:
        raise SemanticSymbolError("Ghidra report program hash is stale")
    if report.get("purpose") != PURPOSE:
        raise SemanticSymbolError("Ghidra report purpose is invalid")
    if report.get("config_sha256") != sha256_file(config_path):
        raise SemanticSymbolError("Ghidra report config hash is stale")
    if report.get("requested") != requested:
        raise SemanticSymbolError("Ghidra report request count is stale")
    renamed = report.get("renamed")
    created = report.get("created", 0)
    unchanged = report.get("unchanged")
    if (
        type(renamed) is not int
        or type(created) is not int
        or type(unchanged) is not int
        or renamed + created + unchanged != requested
    ):
        raise SemanticSymbolError("Ghidra report result counts are inconsistent")
    entries = report.get("entries")
    if not isinstance(entries, list) or len(entries) != requested:
        raise SemanticSymbolError("Ghidra report entry count is inconsistent")
    actions = [entry.get("action") for entry in entries if isinstance(entry, dict)]
    if (
        len(actions) != requested
        or actions.count("rename") != renamed
        or actions.count("create") != created
        or actions.count("unchanged") != unchanged
    ):
        raise SemanticSymbolError("Ghidra report actions are inconsistent")
    return report


def resolve(root: Path, value: Path) -> Path:
    return value.expanduser().resolve() if value.is_absolute() else (root / value).resolve()


def handle_validate(args: argparse.Namespace) -> int:
    config_path = resolve(args.root, args.config)
    binary_map = resolve(args.root, args.binary_map)
    manifest = resolve(args.root, args.analysis_manifest)
    config, symbols = validate_config(config_path, binary_map, manifest, args.root)
    print(
        json.dumps(
            {
                "status": "valid",
                "schema_version": config["schema_version"],
                "image": config["image"],
                "program": config["program"],
                "symbols": len(symbols),
                "purpose": config["purpose"],
            },
            indent=2,
            sort_keys=True,
        )
    )
    return 0


def handle_apply(args: argparse.Namespace, runner: Runner) -> int:
    config_path = resolve(args.root, args.config)
    binary_map = resolve(args.root, args.binary_map)
    manifest = resolve(args.root, args.analysis_manifest)
    project_dir = resolve(args.root, args.project_dir)
    report_path = resolve(args.root, args.report)
    config, symbols = validate_config(config_path, binary_map, manifest, args.root)
    report_root = (args.root / "build/decomp").resolve()
    if not report_path.is_relative_to(report_root):
        raise SemanticSymbolError(f"report must stay under {report_root}")
    manifest_data = read_json(manifest, "analysis ELF manifest")
    program_output = manifest_data.get("output")
    if not isinstance(program_output, str):
        raise SemanticSymbolError("analysis manifest output path is invalid")
    program_path = resolve(args.root, Path(program_output))
    if report_path in {config_path, binary_map, manifest, program_path}:
        raise SemanticSymbolError("report path collides with a protected ARM9 input")
    if not project_dir.is_dir():
        raise SemanticSymbolError(f"Ghidra project directory is missing: {project_dir}")
    headless = args.ghidra_headless
    resolved_headless = shutil.which(headless) if not Path(headless).is_absolute() else headless
    if not resolved_headless or not Path(resolved_headless).is_file():
        raise SemanticSymbolError(f"Ghidra headless executable is unavailable: {headless}")
    report_path.parent.mkdir(parents=True, exist_ok=True)
    if report_path.exists() and not report_path.is_file():
        raise SemanticSymbolError(f"report path is not a regular file: {report_path}")
    if report_path.exists():
        report_path.unlink()
    command = build_ghidra_command(
        str(resolved_headless),
        project_dir,
        args.project_name,
        config["program"],
        args.root / "tools/decomp/ghidra_scripts",
        config_path,
        report_path,
        args.dry_run,
    )
    print("+ " + shlex.join(command), flush=True)
    try:
        completed = runner(command, cwd=args.root, check=False)
    except OSError as exc:
        raise SemanticSymbolError(f"cannot execute Ghidra headless: {exc}") from exc
    status = completed if isinstance(completed, int) else completed.returncode
    if status:
        return int(status)
    report = validate_report(report_path, config_path, config, len(symbols), args.dry_run)
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


def add_identity_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--config", type=Path, default=DEFAULT_CONFIG)
    parser.add_argument("--binary-map", type=Path, default=DEFAULT_BINARY_MAP)
    parser.add_argument("--analysis-manifest", type=Path, default=DEFAULT_ANALYSIS_MANIFEST)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=PROJECT_ROOT, help=argparse.SUPPRESS)
    subparsers = parser.add_subparsers(dest="action", required=True)
    validate = subparsers.add_parser("validate", help="validate config and ARM9 program identity")
    add_identity_arguments(validate)
    validate.set_defaults(handler=handle_validate)

    apply = subparsers.add_parser("apply", help="apply guarded semantic names in Ghidra")
    add_identity_arguments(apply)
    apply.add_argument("--project-dir", type=Path, default=DEFAULT_PROJECT_DIR)
    apply.add_argument("--project-name", default="fsae_decomp")
    apply.add_argument("--report", type=Path, default=DEFAULT_REPORT)
    apply.add_argument("--ghidra-headless", default="ghidra-headless")
    apply.add_argument("--dry-run", action="store_true")
    apply.set_defaults(handler=handle_apply)
    return parser


def main(argv: Sequence[str] | None = None, runner: Runner = subprocess.run) -> int:
    args = build_parser().parse_args(argv)
    args.root = args.root.expanduser().resolve()
    try:
        if args.action == "validate":
            return handle_validate(args)
        return handle_apply(args, runner)
    except SemanticSymbolError as exc:
        print(f"semantic symbol error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
