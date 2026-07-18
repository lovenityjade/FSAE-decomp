#!/usr/bin/env python3
"""Unified public entry point for the FSAE decompilation workflow."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys
from typing import Any, Callable, Sequence


PROJECT_ROOT = Path(__file__).resolve().parents[2]
CANONICAL_SIZES = {
    "arm9": 0x123600,
    "arm7": 0x25860,
    "arm9i": 0x4794,
    "arm7i": 0x49710,
}
DEFAULT_MATCH_CONFIGS = (
    Path("config/build/arm7.json"),
    Path("config/build/arm7i.json"),
)
TEST_SUITES = (
    "analysis/tests",
    "tools/analysis/tests",
    "tools/rom/tests",
    "tools/match/tests",
    "tools/linker/tests",
    "tools/progress/tests",
    "tools/release/tests",
    "tools/sdk/tests",
    "tools/toolchain/tests",
    "tools/windows/tests",
    "tools/decomp/tests",
    "tools/build/tests",
)


class OrchestrationError(RuntimeError):
    """A safe, actionable error detected before or after delegation."""


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
        raise OrchestrationError(f"{label} is unavailable or invalid: {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise OrchestrationError(f"{label} must be a JSON object: {path}")
    return value


def require_file(path: Path, label: str) -> Path:
    if not path.is_file():
        raise OrchestrationError(f"missing {label}: {path}")
    return path


def require_directory(path: Path, label: str) -> Path:
    if not path.is_dir():
        raise OrchestrationError(f"missing {label}: {path}")
    return path


def resolve_path(root: Path, value: str | Path) -> Path:
    path = Path(value).expanduser()
    return path.resolve() if path.is_absolute() else (root / path).resolve()


def resolve_beneath(root: Path, value: str | Path, label: str) -> Path:
    path = resolve_path(root, value)
    try:
        path.relative_to(root.resolve())
    except ValueError as exc:
        raise OrchestrationError(f"{label} escapes the repository: {value}") from exc
    return path


def setting(cli_value: str | None, environment_name: str) -> str | None:
    return cli_value or os.environ.get(environment_name) or None


def required_setting(cli_value: str | None, environment_name: str, option: str) -> str:
    value = setting(cli_value, environment_name)
    if not value:
        raise OrchestrationError(f"missing {option}; pass {option} or set {environment_name}")
    return value


def display_command(command: Sequence[str]) -> None:
    print("+ " + shlex.join(str(item) for item in command), flush=True)


def invoke(root: Path, command: Sequence[str], runner: Runner = subprocess.run) -> int:
    command = [str(item) for item in command]
    display_command(command)
    try:
        completed = runner(command, cwd=root, check=False)
    except OSError as exc:
        raise OrchestrationError(f"cannot execute {command[0]}: {exc}") from exc
    return completed if isinstance(completed, int) else int(completed.returncode)


def python_tool(args: argparse.Namespace, relative: str, *tool_args: str) -> list[str]:
    return [args.python, str(args.root / relative), *map(str, tool_args)]


def private_rom_inputs(args: argparse.Namespace) -> tuple[Path, Path, Path]:
    values = (
        (args.rom, "FSAE_ROM", "--rom", "user-dumped ROM"),
        (args.bios7, "FSAE_BIOS7", "--bios7", "user-dumped BIOS7"),
        (args.twltool_zip, "TWLTOOL_ZIP", "--twltool-zip", "TWLTool archive"),
    )
    missing: list[str] = []
    resolved: list[Path] = []
    for cli_value, environment, option, label in values:
        value = setting(cli_value, environment)
        if not value:
            missing.append(f"{label}: pass {option} or set {environment}")
            continue
        path = resolve_path(args.root, value)
        if not path.is_file():
            missing.append(f"{label}: file not found: {path}")
        resolved.append(path)
    if missing:
        raise OrchestrationError("missing private inputs:\n- " + "\n- ".join(missing))
    return resolved[0], resolved[1], resolved[2]


def rom_command(args: argparse.Namespace, action: str) -> list[str]:
    rom, bios7, twltool = private_rom_inputs(args)
    output = resolve_path(args.root, args.output)
    command = python_tool(
        args,
        "tools/rom/pipeline.py",
        action,
        "--rom",
        str(rom),
        "--bios7",
        str(bios7),
        "--twltool-zip",
        str(twltool),
        "--output",
        str(output),
    )
    if args.wine:
        command.extend(("--wine", args.wine))
    if action == "prepare":
        if args.dry_run:
            command.append("--dry-run")
        if args.force:
            command.append("--force")
    return command


def handle_diagnose(args: argparse.Namespace, runner: Runner) -> int:
    return invoke(args.root, rom_command(args, "diagnose"), runner)


def handle_prepare(args: argparse.Namespace, runner: Runner) -> int:
    return invoke(args.root, rom_command(args, "prepare"), runner)


def expected_sizes(root: Path) -> dict[str, int]:
    config_path = root / "tools/progress/project.v2.json"
    config = read_json(config_path, "progress project config")
    sections = config.get("sections")
    if not isinstance(sections, list):
        raise OrchestrationError(f"progress project sections are invalid: {config_path}")
    observed: dict[str, int] = {}
    for section in sections:
        if not isinstance(section, dict) or section.get("dimension") != "section":
            continue
        targets = section.get("targets")
        if isinstance(targets, dict) and isinstance(targets.get("bytes"), int):
            observed[str(section.get("id"))] = targets["bytes"]
    if observed != CANONICAL_SIZES:
        raise OrchestrationError(
            f"progress byte targets are not canonical: got {observed!r}, expected {CANONICAL_SIZES!r}"
        )
    return observed


def _hex_digest(value: Any, field: str) -> str:
    if not isinstance(value, str) or len(value) != 64:
        raise OrchestrationError(f"{field} must be a SHA-256 digest")
    try:
        bytes.fromhex(value)
    except ValueError as exc:
        raise OrchestrationError(f"{field} must be hexadecimal") from exc
    return value.lower()


def validate_map_outputs(
    root: Path,
    binary_map_path: Path,
    nitrofs_map_path: Path,
    manifest_path: Path,
) -> dict[str, Any]:
    targets = expected_sizes(root)
    binary_map = read_json(require_file(binary_map_path, "binary map"), "binary map")
    nitrofs_map = read_json(require_file(nitrofs_map_path, "NitroFS map"), "NitroFS map")
    manifest = read_json(require_file(manifest_path, "ROM preparation manifest"), "ROM manifest")

    executables = binary_map.get("executables")
    if not isinstance(executables, list):
        raise OrchestrationError("binary map executables must be an array")
    mapped: dict[str, dict[str, Any]] = {}
    for item in executables:
        if not isinstance(item, dict) or not isinstance(item.get("id"), str):
            raise OrchestrationError("binary map contains an invalid executable entry")
        if item["id"] in mapped:
            raise OrchestrationError(f"binary map contains duplicate executable {item['id']}")
        mapped[item["id"]] = item
    if set(mapped) != set(targets):
        raise OrchestrationError(f"binary map executable set is {sorted(mapped)}, expected {sorted(targets)}")

    derived = manifest.get("derived")
    if not isinstance(derived, dict) or not isinstance(derived.get("sections"), dict):
        raise OrchestrationError("ROM manifest derived sections are invalid")
    manifest_sections = derived["sections"]
    decrypted = derived.get("decrypted_rom")
    source = binary_map.get("source")
    if not isinstance(decrypted, dict) or not isinstance(source, dict):
        raise OrchestrationError("ROM manifest or binary map source metadata is invalid")
    output_root = manifest_path.parent
    source_sha = _hex_digest(source.get("sha256"), "binary map source.sha256")
    if source_sha != _hex_digest(decrypted.get("sha256"), "manifest decrypted_rom.sha256"):
        raise OrchestrationError("binary map and preparation manifest describe different decrypted ROMs")
    decrypted_relative = decrypted.get("path")
    decrypted_size = decrypted.get("size")
    if not isinstance(decrypted_relative, str) or not isinstance(decrypted_size, int):
        raise OrchestrationError("ROM manifest decrypted file metadata is invalid")
    decrypted_path = resolve_beneath(output_root, decrypted_relative, "manifest decrypted ROM path")
    require_file(decrypted_path, "prepared decrypted ROM")
    if decrypted_path.stat().st_size != decrypted_size or sha256_file(decrypted_path) != source_sha:
        raise OrchestrationError("prepared decrypted ROM does not match manifest/map metadata")
    if source.get("size") != decrypted_size:
        raise OrchestrationError("binary map and manifest decrypted ROM sizes differ")

    section_report: dict[str, dict[str, Any]] = {}
    for section_id, expected_size in targets.items():
        mapped_rom = mapped[section_id].get("rom")
        manifest_section = manifest_sections.get(section_id)
        if not isinstance(mapped_rom, dict) or not isinstance(manifest_section, dict):
            raise OrchestrationError(f"missing metadata for {section_id}")
        mapped_size = mapped_rom.get("size")
        manifest_size = manifest_section.get("size")
        if mapped_size != expected_size or manifest_size != expected_size:
            raise OrchestrationError(
                f"{section_id} size mismatch: map={mapped_size}, manifest={manifest_size}, expected={expected_size}"
            )
        mapped_sha = _hex_digest(mapped_rom.get("sha256"), f"{section_id} map SHA-256")
        manifest_sha = _hex_digest(manifest_section.get("sha256"), f"{section_id} manifest SHA-256")
        if mapped_sha != manifest_sha:
            raise OrchestrationError(f"{section_id} map and manifest hashes differ")
        relative = manifest_section.get("path")
        if not isinstance(relative, str):
            raise OrchestrationError(f"{section_id} manifest path is invalid")
        section_path = resolve_beneath(output_root, relative, f"{section_id} manifest path")
        require_file(section_path, f"prepared {section_id}")
        if section_path.stat().st_size != expected_size or sha256_file(section_path) != mapped_sha:
            raise OrchestrationError(f"prepared {section_id} bytes do not match map metadata")
        section_report[section_id] = {"size": expected_size, "sha256": mapped_sha}

    files = nitrofs_map.get("files")
    manifest_files = derived.get("nitrofs")
    if not isinstance(files, list) or not isinstance(manifest_files, list):
        raise OrchestrationError("NitroFS map or manifest file list is invalid")
    declared_count = nitrofs_map.get("file_count")
    filesystem = binary_map.get("filesystem")
    mapped_count = filesystem.get("file_count") if isinstance(filesystem, dict) else None
    if declared_count != len(files) or mapped_count != len(files) or len(manifest_files) != len(files):
        raise OrchestrationError(
            f"NitroFS counts disagree: map={declared_count}, binary-map={mapped_count}, "
            f"manifest={len(manifest_files)}, entries={len(files)}"
        )
    by_path: dict[str, dict[str, Any]] = {}
    for item in files:
        if not isinstance(item, dict) or not isinstance(item.get("path"), str):
            raise OrchestrationError("NitroFS map contains an invalid file entry")
        if item["path"] in by_path:
            raise OrchestrationError(f"duplicate NitroFS path: {item['path']}")
        by_path[item["path"]] = item
    manifest_by_path = {
        item.get("path"): item for item in manifest_files if isinstance(item, dict) and isinstance(item.get("path"), str)
    }
    if set(by_path) != set(manifest_by_path):
        raise OrchestrationError("NitroFS map and manifest paths differ")
    for path_name, item in by_path.items():
        manifest_item = manifest_by_path[path_name]
        rom_metadata = item.get("rom")
        if not isinstance(rom_metadata, dict):
            raise OrchestrationError(f"NitroFS ROM metadata is invalid for {path_name}")
        mapped_size = rom_metadata.get("size")
        mapped_sha = item.get("sha256")
        if mapped_size != manifest_item.get("size") or mapped_sha != manifest_item.get("sha256"):
            raise OrchestrationError(f"NitroFS metadata differs for {path_name}")
        output = manifest_item.get("output")
        if not isinstance(output, str):
            raise OrchestrationError(f"NitroFS output path is invalid for {path_name}")
        file_path = resolve_beneath(output_root, output, f"NitroFS output {path_name}")
        require_file(file_path, f"prepared NitroFS file {path_name}")
        if file_path.stat().st_size != mapped_size or sha256_file(file_path) != mapped_sha:
            raise OrchestrationError(f"prepared NitroFS bytes do not match metadata for {path_name}")

    return {
        "status": "valid",
        "schema_version": 1,
        "decrypted_rom_sha256": source_sha,
        "sections": section_report,
        "nitrofs_file_count": len(files),
    }


def map_paths(args: argparse.Namespace) -> tuple[Path, Path, Path, Path]:
    decrypted_value = setting(args.decrypted_rom, "FSAE_DECRYPTED_ROM") or "build/rom/decrypted.nds"
    decrypted = resolve_path(args.root, decrypted_value)
    binary_map = resolve_path(args.root, args.binary_map)
    nitrofs_map = resolve_path(args.root, args.nitrofs_map)
    manifest = resolve_path(args.root, args.manifest)
    return decrypted, binary_map, nitrofs_map, manifest


def handle_map(args: argparse.Namespace, runner: Runner) -> int:
    decrypted, binary_map, nitrofs_map, manifest = map_paths(args)
    require_file(decrypted, "prepared decrypted ROM; run prepare first")
    command = python_tool(
        args,
        "analysis/nds_binary_map.py",
        str(decrypted),
        "--binary-map",
        str(binary_map),
        "--nitrofs-map",
        str(nitrofs_map),
    )
    encrypted_value = setting(args.encrypted_rom, "FSAE_ROM")
    if encrypted_value:
        encrypted = resolve_path(args.root, encrypted_value)
        require_file(encrypted, "encrypted comparison ROM")
        command.extend(("--encrypted-rom", str(encrypted)))
    status = invoke(args.root, command, runner)
    if status:
        return status
    report = validate_map_outputs(args.root, binary_map, nitrofs_map, manifest)
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


def handle_validate(args: argparse.Namespace, runner: Runner) -> int:
    del runner
    _, binary_map, nitrofs_map, manifest = map_paths(args)
    report = validate_map_outputs(args.root, binary_map, nitrofs_map, manifest)
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


def validate_match_outputs(root: Path, sdk_root: Path, config_paths: Sequence[Path]) -> dict[str, Any]:
    results: dict[str, dict[str, Any]] = {}
    for config_path in config_paths:
        config = read_json(require_file(config_path, "match config"), "match config")
        section = config.get("section")
        if section not in CANONICAL_SIZES or section in results:
            raise OrchestrationError(f"invalid or duplicate match section in {config_path}: {section!r}")
        reference = config.get("reference")
        target = config.get("target")
        normalized = config.get("normalized")
        output = config.get("output")
        if not all(isinstance(value, dict) for value in (reference, target, normalized, output)):
            raise OrchestrationError(f"match config objects are invalid: {config_path}")
        target_size = target.get("size")
        reference_size = reference.get("size")
        if isinstance(reference_size, bool) or not isinstance(reference_size, int) or reference_size < 1:
            raise OrchestrationError(f"{section} reference size is invalid")
        reference_sha = _hex_digest(reference.get("sha256"), f"{section} reference SHA-256")
        target_sha = _hex_digest(target.get("sha256"), f"{section} target SHA-256")
        normalized_sha = _hex_digest(normalized.get("sha256"), f"{section} normalized SHA-256")
        if target_size != CANONICAL_SIZES[section] or normalized.get("size") != target_size:
            raise OrchestrationError(f"{section} match config does not use the canonical target size")
        if normalized_sha != target_sha:
            raise OrchestrationError(f"{section} normalized hash differs from the target hash")

        reference_path = resolve_beneath(sdk_root, str(reference.get("path", "")), f"{section} reference")
        target_path = resolve_beneath(root, str(target.get("path", "")), f"{section} target")
        artifact_path = resolve_beneath(root, str(output.get("artifact", "")), f"{section} artifact")
        proof_path = resolve_beneath(root, str(output.get("proof", "")), f"{section} proof")
        progress_path = resolve_beneath(root, str(output.get("progress", "")), f"{section} progress")
        for path, label in (
            (reference_path, f"{section} official SDK reference"),
            (target_path, f"{section} target"),
            (artifact_path, f"{section} matched artifact"),
            (proof_path, f"{section} proof"),
            (progress_path, f"{section} progress evidence"),
        ):
            require_file(path, label)
        if reference_path.stat().st_size != reference_size or sha256_file(reference_path) != reference_sha:
            raise OrchestrationError(f"{section} official SDK reference no longer matches its config")
        if target_path.stat().st_size != target_size or sha256_file(target_path) != target_sha:
            raise OrchestrationError(f"{section} target no longer matches its config")
        if artifact_path.stat().st_size != target_size or sha256_file(artifact_path) != target_sha:
            raise OrchestrationError(f"{section} generated artifact is not byte-identical to the target")

        proof = read_json(proof_path, f"{section} proof")
        progress = read_json(progress_path, f"{section} progress evidence")
        comparison = proof.get("comparison")
        proof_target = proof.get("target")
        proof_reference = proof.get("reference")
        proof_normalization = proof.get("normalization")
        if proof.get("kind") != "binary-match-proof" or proof.get("section") != section:
            raise OrchestrationError(f"{section} proof identity is invalid")
        if not isinstance(comparison, dict) or comparison != {
            "byte_for_byte_equal": True,
            "mismatch_bytes": 0,
            "first_mismatch_offset": None,
        }:
            raise OrchestrationError(f"{section} proof does not establish exact byte equality")
        for label, value in (
            ("reference", proof_reference),
            ("target", proof_target),
            ("normalization", proof_normalization),
        ):
            if not isinstance(value, dict) or value.get("verified") is not True:
                raise OrchestrationError(f"{section} proof {label} is not verified")
        if proof_target.get("size") != target_size or proof_target.get("sha256") != target_sha:
            raise OrchestrationError(f"{section} proof target metadata is stale")
        if proof_reference.get("size") != reference_size or proof_reference.get("sha256") != reference_sha:
            raise OrchestrationError(f"{section} proof reference metadata is stale")
        if proof_normalization.get("size") != target_size or proof_normalization.get("sha256") != target_sha:
            raise OrchestrationError(f"{section} proof normalization metadata is stale")
        if proof.get("dashboard_evidence") != progress:
            raise OrchestrationError(f"{section} proof and progress evidence disagree")
        metrics = progress.get("metrics")
        byte_metrics = metrics.get("bytes") if isinstance(metrics, dict) else None
        if (
            progress.get("schema_version") != 2
            or progress.get("kind") != "evidence"
            or progress.get("track") != "matching"
            or progress.get("section") != section
            or progress.get("category") != "sdk"
            or not isinstance(metrics, dict)
            or metrics.get("units") != {"matched": 1, "total": 1}
            or metrics.get("functions") != {"matched": 0, "total": 0}
            or byte_metrics != {"matched": target_size, "total": target_size}
        ):
            raise OrchestrationError(f"{section} progress evidence is not an exact SDK matching claim")
        results[section] = {"matched": target_size, "total": target_size, "sha256": target_sha}
    return {"status": "matched", "sections": results}


def match_configs(args: argparse.Namespace) -> list[Path]:
    values = args.config or list(DEFAULT_MATCH_CONFIGS)
    return [resolve_path(args.root, value) for value in values]


def handle_match(args: argparse.Namespace, runner: Runner) -> int:
    sdk_value = required_setting(args.twlsdk_root, "TWLSDK_ROOT", "--twlsdk-root")
    sdk_root = require_directory(resolve_path(args.root, sdk_value), "external TwlSDK root")
    configs = match_configs(args)
    command = python_tool(args, "tools/match/arm7_match.py", "--twlsdk-root", str(sdk_root))
    for config in configs:
        command.extend(("--config", str(config)))
    status = invoke(args.root, command, runner)
    if status:
        return status
    report = validate_match_outputs(args.root, sdk_root, configs)
    progress_status = invoke(
        args.root,
        python_tool(args, "tools/progress/progress.py", "validate"),
        runner,
    )
    if progress_status:
        raise OrchestrationError("matching proofs passed but progress validation failed")
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


def handle_progress_validate(args: argparse.Namespace, runner: Runner) -> int:
    return invoke(args.root, python_tool(args, "tools/progress/progress.py", "validate"), runner)


def handle_serve(args: argparse.Namespace, runner: Runner) -> int:
    return invoke(
        args.root,
        python_tool(
            args,
            "tools/progress/progress.py",
            "serve",
            "--host",
            args.host,
            "--port",
            str(args.port),
        ),
        runner,
    )


def handle_test(args: argparse.Namespace, runner: Runner) -> int:
    for suite in TEST_SUITES:
        status = invoke(
            args.root,
            [args.python, "-m", "unittest", "discover", "-s", suite, "-v"],
            runner,
        )
        if status:
            return status
    return handle_progress_validate(args, runner)


def windows_common(args: argparse.Namespace, *, needs_toolchain: bool) -> list[str]:
    host = required_setting(args.host, "FSAE_WINDOWS_HOST", "--host")
    workspace = required_setting(
        args.remote_workspace, "FSAE_WINDOWS_WORKSPACE", "--remote-workspace"
    )
    result = ("--host", host, "--remote-workspace", workspace)
    command = list(result)
    if needs_toolchain:
        toolchain = required_setting(
            args.toolchain_root, "FSAE_WINDOWS_TOOLCHAIN_ROOT", "--toolchain-root"
        )
        environment = required_setting(
            args.environment_script,
            "FSAE_WINDOWS_ENVIRONMENT_SCRIPT",
            "--environment-script",
        )
        command.extend(("--toolchain-root", toolchain, "--environment-script", environment))
        mwccarm = setting(args.mwccarm, "MWCCARM")
        mwldarm = setting(args.mwldarm, "MWLDARM")
        if mwccarm:
            command.extend(("--mwccarm", mwccarm))
        if mwldarm:
            command.extend(("--mwldarm", mwldarm))
    return command


def handle_windows_sync(args: argparse.Namespace, runner: Runner) -> int:
    command = python_tool(args, "tools/windows/windows_toolchain.py", "sync")
    command.extend(windows_common(args, needs_toolchain=False))
    command.extend(("--source", str(args.root)))
    if args.dry_run:
        command.append("--dry-run")
    if args.delete_stale:
        command.append("--delete-stale")
    return invoke(args.root, command, runner)


def handle_windows_check(args: argparse.Namespace, runner: Runner) -> int:
    command = python_tool(args, "tools/windows/windows_toolchain.py", "check")
    command.extend(windows_common(args, needs_toolchain=True))
    return invoke(args.root, command, runner)


def handle_windows_build(args: argparse.Namespace, runner: Runner) -> int:
    build_command = required_setting(args.command, "FSAE_WINDOWS_BUILD_COMMAND", "--command")
    command = python_tool(args, "tools/windows/windows_toolchain.py", "build")
    command.extend(windows_common(args, needs_toolchain=True))
    command.extend(("--command", build_command))
    return invoke(args.root, command, runner)


def add_rom_arguments(parser: argparse.ArgumentParser, *, prepare: bool = False) -> None:
    parser.add_argument("--rom", help="user-dumped ROM (or FSAE_ROM)")
    parser.add_argument("--bios7", help="user-dumped BIOS7 (or FSAE_BIOS7)")
    parser.add_argument("--twltool-zip", help="TWLTool archive (or TWLTOOL_ZIP)")
    parser.add_argument("--output", default="build/rom")
    parser.add_argument("--wine")
    if prepare:
        parser.add_argument("--dry-run", action="store_true")
        parser.add_argument("--force", action="store_true")


def add_map_arguments(parser: argparse.ArgumentParser, *, include_input: bool) -> None:
    if include_input:
        parser.add_argument("--decrypted-rom", help="prepared SRL (or FSAE_DECRYPTED_ROM)")
        parser.add_argument("--encrypted-rom", help="optional untouched comparison ROM (or FSAE_ROM)")
    else:
        parser.set_defaults(decrypted_rom=None, encrypted_rom=None)
    parser.add_argument("--binary-map", default="config/binary-map.json")
    parser.add_argument("--nitrofs-map", default="config/nitrofs.json")
    parser.add_argument("--manifest", default="build/rom/manifest.json")


def add_windows_arguments(parser: argparse.ArgumentParser, *, needs_toolchain: bool) -> None:
    parser.add_argument("--host", help="SSH alias (or FSAE_WINDOWS_HOST)")
    parser.add_argument(
        "--remote-workspace", help="absolute Windows workspace (or FSAE_WINDOWS_WORKSPACE)"
    )
    if needs_toolchain:
        parser.add_argument(
            "--toolchain-root", help="private toolchain root (or FSAE_WINDOWS_TOOLCHAIN_ROOT)"
        )
        parser.add_argument(
            "--environment-script",
            help="private environment script path (or FSAE_WINDOWS_ENVIRONMENT_SCRIPT)",
        )
        parser.add_argument("--mwccarm", help="optional explicit compiler path (or MWCCARM)")
        parser.add_argument("--mwldarm", help="optional explicit linker path (or MWLDARM)")
    else:
        parser.set_defaults(toolchain_root=None, environment_script=None, mwccarm=None, mwldarm=None)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=PROJECT_ROOT, help=argparse.SUPPRESS)
    parser.add_argument("--python", default=sys.executable, help=argparse.SUPPRESS)
    subparsers = parser.add_subparsers(dest="action", required=True)

    diagnose = subparsers.add_parser("diagnose", help="verify private ROM preparation inputs")
    add_rom_arguments(diagnose)
    diagnose.set_defaults(handler=handle_diagnose)

    prepare = subparsers.add_parser("prepare", help="prepare verified local analysis artifacts")
    add_rom_arguments(prepare, prepare=True)
    prepare.set_defaults(handler=handle_prepare)

    map_parser = subparsers.add_parser("map", help="regenerate and validate binary/NitroFS maps")
    add_map_arguments(map_parser, include_input=True)
    map_parser.set_defaults(handler=handle_map)

    validate = subparsers.add_parser("validate", help="validate maps against prepared artifacts")
    add_map_arguments(validate, include_input=False)
    validate.set_defaults(handler=handle_validate)

    match = subparsers.add_parser("match", help="run and independently validate exact match proofs")
    match.add_argument("--twlsdk-root", help="external TwlSDK root (or TWLSDK_ROOT)")
    match.add_argument("--config", action="append", type=Path)
    match.set_defaults(handler=handle_match)

    progress = subparsers.add_parser("progress-validate", help="validate progress inputs")
    progress.set_defaults(handler=handle_progress_validate)

    serve = subparsers.add_parser("serve", help="serve the live local dashboard")
    serve.add_argument("--host", default="127.0.0.1")
    serve.add_argument("--port", type=int, default=8765)
    serve.set_defaults(handler=handle_serve)

    test = subparsers.add_parser("test", help="run all public unit tests and progress validation")
    test.set_defaults(handler=handle_test)

    windows_sync = subparsers.add_parser("windows-sync", help="sync public files to Windows")
    add_windows_arguments(windows_sync, needs_toolchain=False)
    windows_sync.add_argument("--dry-run", action="store_true")
    windows_sync.add_argument("--delete-stale", action="store_true")
    windows_sync.set_defaults(handler=handle_windows_sync)

    windows_check = subparsers.add_parser("windows-check", help="verify the remote private toolchain")
    add_windows_arguments(windows_check, needs_toolchain=True)
    windows_check.set_defaults(handler=handle_windows_check)

    windows_build = subparsers.add_parser("windows-build", help="run an explicit remote build command")
    add_windows_arguments(windows_build, needs_toolchain=True)
    windows_build.add_argument("--command", help="remote cmd.exe command (or FSAE_WINDOWS_BUILD_COMMAND)")
    windows_build.set_defaults(handler=handle_windows_build)
    return parser


def main(argv: Sequence[str] | None = None, runner: Runner = subprocess.run) -> int:
    args = build_parser().parse_args(argv)
    args.root = args.root.expanduser().resolve()
    if not args.root.is_dir():
        print(f"orchestration error: repository root does not exist: {args.root}", file=sys.stderr)
        return 2
    try:
        return int(args.handler(args, runner))
    except OrchestrationError as exc:
        print(f"orchestration error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
