#!/usr/bin/env python3
"""Plan CodeWarrior ARM9 source builds and stage linked unit candidates.

Compilation objects are evidence that public C inputs were accepted by the
target compiler; they are not ROM units.  Only externally linked raw unit
candidates are staged for the incremental probe, which remains the sole exact
selection gate.  This provider never awards matching credit.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import subprocess
import sys
from typing import Any, Callable, Sequence

PROJECT_ROOT = Path(__file__).resolve().parents[2]
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

from tools.linker.codewarrior_driver import (
    LICENSE_MARKERS,
    validate_arm_relocatable_elf,
)
from tools.linker.incremental import (
    DEFAULT_BUILD_DIR,
    DEFAULT_MANIFEST,
    IncrementalLinkError,
    LinkUnit,
    UnitManifest,
    load_manifest,
    probe,
    read_json_object,
    sha256_bytes,
    write_json,
)


MAX_PROMOTION_BYTES = 0x4000
ARTIFACT_REGISTRY = Path("source-provider/staged-artifacts.v1.json")
COMPILER_IDENTITY = "Freescale CodeWarrior ARM 4.0 build 1051"
COMPILER_TOKEN = "mwccarm"
COMMON_FLAGS = (
    "-proc", "arm946e", "-nothumb", "-nopic", "-nopid",
    "-interworking", "-lang", "c99",
)
DEFINE_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*(?:=[A-Za-z0-9_+.-]+)?\Z")
Runner = Callable[[Sequence[str], Path], subprocess.CompletedProcess[str]]


class SourceProviderError(RuntimeError):
    """The public promotion or generated provider state is invalid."""


class SourceProviderLicenseError(SourceProviderError):
    """The private compiler reported a license failure."""


def canonical_sha256(value: Any) -> str:
    data = json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")
    return hashlib.sha256(data).hexdigest()


def strict_keys(
    value: Any,
    label: str,
    required: set[str],
    optional: set[str] | None = None,
) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise SourceProviderError(f"{label} must be an object")
    optional = optional or set()
    keys = set(value)
    missing = required - keys
    unknown = keys - required - optional
    if missing or unknown:
        raise SourceProviderError(
            f"{label} fields are invalid; missing={sorted(missing)}, "
            f"unknown={sorted(unknown)}"
        )
    return value


def strict_list(value: Any, label: str) -> list[Any]:
    if not isinstance(value, list):
        raise SourceProviderError(f"{label} must be a list")
    return value


def relative_path(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value or "\\" in value or "\0" in value:
        raise SourceProviderError(f"{label} must be a safe relative POSIX path")
    path = PurePosixPath(value)
    if path.is_absolute() or any(part in ("", ".", "..") for part in path.parts):
        raise SourceProviderError(f"{label} must be a safe relative POSIX path")
    return path.as_posix()


def project_file(project_root: Path, relative: str, label: str) -> Path:
    project = project_root.resolve()
    path = (project / PurePosixPath(relative)).resolve()
    if not path.is_relative_to(project) or not path.is_file() or path.is_symlink():
        raise SourceProviderError(f"{label} is missing, symlinked, or outside the project")
    return path


def project_directory(project_root: Path, relative: str, label: str) -> Path:
    project = project_root.resolve()
    path = (project / PurePosixPath(relative)).resolve()
    if not path.is_relative_to(project) or not path.is_dir() or path.is_symlink():
        raise SourceProviderError(f"{label} is missing, symlinked, or outside the project")
    return path


def build_root(project_root: Path, build_dir: Path) -> Path:
    project = project_root.resolve()
    requested = build_dir if build_dir.is_absolute() else project / build_dir
    result = requested.resolve()
    repository_build = (project / "build").resolve()
    if not result.is_relative_to(repository_build):
        raise SourceProviderError("linker build directory must remain beneath project build/")
    result.mkdir(parents=True, exist_ok=True)
    return result


def default_runner(
    command: Sequence[str], cwd: Path
) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        list(command), cwd=cwd,
        env={**os.environ, "CCACHE_DISABLE": "1"},
        text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False,
    )


def run_compiler(
    command: Sequence[str], project_root: Path, runner: Runner
) -> None:
    result = runner(command, project_root)
    combined = f"{result.stdout or ''}\n{result.stderr or ''}".lower()
    if result.returncode == 0:
        return
    detail = (result.stderr or result.stdout or "no diagnostic").strip()
    if any(marker in combined for marker in LICENSE_MARKERS):
        raise SourceProviderLicenseError(
            f"mwccarm license failure ({result.returncode}): {detail}"
        )
    raise SourceProviderError(
        f"mwccarm failed ({result.returncode}): {detail}"
    )


def manifest_units(manifest: UnitManifest) -> dict[tuple[str, str], LinkUnit]:
    return {
        (unit.image, unit.unit_id): unit
        for target in manifest.targets
        for unit in target.units
    }


def target_sha256(manifest: UnitManifest, image: str) -> str:
    return next(target.sha256 for target in manifest.targets if target.image == image)


def load_source_promotions(
    promotions_path: Path,
    manifest: UnitManifest,
    project_root: Path,
    build_dir: Path,
) -> dict[str, Any]:
    value, raw = read_json_object(promotions_path, "ARM9 source promotions")
    strict_keys(
        value,
        "ARM9 source promotions",
        {"schema_version", "kind", "units"},
    )
    if value["schema_version"] != 1 or value["kind"] != "arm9-source-promotions":
        raise SourceProviderError("ARM9 source promotions identity is invalid")
    raw_units = strict_list(value["units"], "ARM9 source promotion units")
    if not raw_units:
        raise SourceProviderError("ARM9 source promotions must select at least one unit")

    available = manifest_units(manifest)
    selected: list[dict[str, Any]] = []
    selected_keys: set[tuple[str, str]] = set()
    total_size = 0
    for unit_index, raw_unit in enumerate(raw_units):
        label = f"promotion unit {unit_index}"
        entry = strict_keys(
            raw_unit,
            label,
            {"image", "unit_id", "sources"},
            {"include_dirs", "defines"},
        )
        image = entry["image"]
        unit_id = entry["unit_id"]
        if not isinstance(image, str) or not isinstance(unit_id, str):
            raise SourceProviderError(f"{label} image and unit_id must be strings")
        key = (image, unit_id)
        if image != "arm9" or key not in available:
            raise SourceProviderError(f"{label} must reference a configured ARM9 unit")
        if key in selected_keys:
            raise SourceProviderError(f"duplicate promotion unit: {image}/{unit_id}")
        unit = available[key]
        if unit.size > MAX_PROMOTION_BYTES:
            raise SourceProviderError(f"{label} exceeds the 16 KiB unit cap")
        total_size += unit.size
        if total_size > MAX_PROMOTION_BYTES:
            raise SourceProviderError("source promotion batch exceeds the 16 KiB cap")

        include_values = strict_list(
            entry.get("include_dirs", []), f"{label} include_dirs"
        )
        include_dirs = sorted({
            relative_path(item, f"{label} include directory")
            for item in include_values
        })
        for include in include_dirs:
            project_directory(project_root, include, f"{label} include directory")
        define_values = strict_list(entry.get("defines", []), f"{label} defines")
        defines: list[str] = []
        for define in define_values:
            if not isinstance(define, str) or DEFINE_RE.fullmatch(define) is None:
                raise SourceProviderError(f"{label} contains an unsafe define")
            defines.append(define)
        if len(defines) != len(set(defines)):
            raise SourceProviderError(f"{label} contains duplicate defines")
        defines.sort()

        raw_sources = strict_list(entry["sources"], f"{label} sources")
        if not raw_sources:
            raise SourceProviderError(f"{label} must contain at least one source")
        sources: list[dict[str, Any]] = []
        source_paths: set[str] = set()
        for source_index, raw_source in enumerate(raw_sources):
            source_label = f"{label} sources[{source_index}]"
            source_entry = strict_keys(raw_source, source_label, {"path", "proof"})
            source_relative = relative_path(source_entry["path"], f"{source_label}.path")
            proof_relative = relative_path(source_entry["proof"], f"{source_label}.proof")
            if source_relative in source_paths or not source_relative.endswith(".c"):
                raise SourceProviderError(f"{source_label} is duplicate or not C source")
            source_path = project_file(project_root, source_relative, source_label)
            proof_path = project_file(project_root, proof_relative, f"{source_label} proof")
            proof, proof_raw = read_json_object(proof_path, f"{source_label} proof")
            if proof.get("kind") != "arm9-raw-recovery-block":
                raise SourceProviderError(f"{source_label} proof kind is unsupported")
            proof_outputs = proof.get("outputs")
            proof_inputs = proof.get("inputs")
            if not isinstance(proof_outputs, dict) or proof_outputs.get("source") != source_relative:
                raise SourceProviderError(f"{source_label} proof does not own its source")
            if not isinstance(proof_inputs, dict) or proof_inputs.get("target_sha256") != target_sha256(manifest, image):
                raise SourceProviderError(f"{source_label} proof target is stale")
            dependencies: list[dict[str, Any]] = []
            header = proof_outputs.get("header")
            if header is not None:
                header_relative = relative_path(header, f"{source_label} proof header")
                header_path = project_file(
                    project_root, header_relative, f"{source_label} proof header"
                )
                dependencies.append({
                    "path": header_relative,
                    "sha256": sha256_bytes(header_path.read_bytes()),
                })
            sources.append({
                "path": source_relative,
                "sha256": sha256_bytes(source_path.read_bytes()),
                "proof": proof_relative,
                "proof_sha256": sha256_bytes(proof_raw),
                "dependencies": dependencies,
            })
            source_paths.add(source_relative)

        object_directory = (
            build_dir / "source-provider/objects" /
            unit.image / unit.region_id / unit.chunk_id
        )
        commands: list[list[str]] = []
        objects: list[str] = []
        for source_index, source in enumerate(sources):
            stem = Path(source["path"]).stem
            output = object_directory / f"{source_index:03d}-{stem}.o"
            output_relative = output.relative_to(build_dir).as_posix()
            project_output = output.relative_to(project_root).as_posix()
            command = [COMPILER_TOKEN, *COMMON_FLAGS]
            for include in include_dirs:
                command.extend(("-I", include))
            for define in defines:
                command.extend(("-D", define))
            command.extend(("-c", source["path"], "-o", project_output))
            commands.append(command)
            objects.append(output_relative)
        selected.append({
            "image": unit.image,
            "unit_id": unit.unit_id,
            "region_id": unit.region_id,
            "chunk_id": unit.chunk_id,
            "target_offset": unit.target_offset,
            "size": unit.size,
            "target_sha256": unit.target_sha256,
            "provider_path": unit.source_path,
            "include_dirs": include_dirs,
            "defines": defines,
            "sources": sources,
            "commands": commands,
            "objects": objects,
        })
        selected_keys.add(key)

    return {
        "manifest_sha256": manifest.sha256,
        "promotions_sha256": sha256_bytes(raw),
        "units": selected,
        "total_bytes": total_size,
    }


def build_plan(
    manifest_path: Path,
    promotions_path: Path,
    project_root: Path,
    build_dir: Path,
) -> dict[str, Any]:
    project = project_root.resolve()
    build = build_root(project, build_dir)
    manifest = load_manifest(manifest_path)
    promotions = load_source_promotions(
        promotions_path, manifest, project, build
    )
    basis = {
        "manifest_sha256": promotions["manifest_sha256"],
        "promotions_sha256": promotions["promotions_sha256"],
        "compiler_identity": COMPILER_IDENTITY,
        "compiler_flags": list(COMMON_FLAGS),
        "units": promotions["units"],
    }
    report = {
        "schema_version": 1,
        "kind": "arm9-source-provider-plan",
        **basis,
        "plan_sha256": canonical_sha256(basis),
        "unit_count": len(promotions["units"]),
        "command_count": sum(len(unit["commands"]) for unit in promotions["units"]),
        "total_bytes": promotions["total_bytes"],
        "credited_matching_bytes": 0,
        "fallback_credited_bytes": 0,
        "status": "planned-unverified",
    }
    write_json(build / "source-provider/plan.v1.json", report)
    return report


def compile_sources(
    manifest_path: Path,
    promotions_path: Path,
    project_root: Path,
    build_dir: Path,
    mwccarm: str,
    *,
    runner: Runner = default_runner,
) -> dict[str, Any]:
    project = project_root.resolve()
    build = build_root(project, build_dir)
    plan = build_plan(manifest_path, promotions_path, project, build)
    records: list[dict[str, Any]] = []
    for unit in plan["units"]:
        for command_index, (command, relative) in enumerate(
            zip(unit["commands"], unit["objects"], strict=True)
        ):
            output = (build / PurePosixPath(relative)).resolve()
            if not output.is_relative_to(build):
                raise SourceProviderError("planned object escapes the linker build directory")
            output.parent.mkdir(parents=True, exist_ok=True)
            output.unlink(missing_ok=True)
            runtime_command = [mwccarm, *command[1:]]
            run_compiler(runtime_command, project, runner)
            if not output.is_file() or output.is_symlink():
                raise SourceProviderError(
                    f"mwccarm did not create planned object for {unit['unit_id']}"
                )
            data = output.read_bytes()
            validate_arm_relocatable_elf(
                data, f"source object {unit['unit_id']}[{command_index}]"
            )
            records.append({
                "image": unit["image"],
                "unit_id": unit["unit_id"],
                "command_index": command_index,
                "command": command,
                "object": relative,
                "object_sha256": sha256_bytes(data),
                "size": len(data),
            })
    report = {
        "schema_version": 1,
        "kind": "arm9-source-provider-compile",
        "plan_sha256": plan["plan_sha256"],
        "compiler_identity": COMPILER_IDENTITY,
        "object_count": len(records),
        "objects": records,
        "credited_matching_bytes": 0,
        "fallback_credited_bytes": 0,
        "status": "compiled-unlinked-uncredited",
    }
    write_json(build / "source-provider/compile.v1.json", report)
    return report


def validate_compile_report(build: Path, plan: dict[str, Any]) -> dict[str, Any]:
    report, _ = read_json_object(
        build / "source-provider/compile.v1.json", "source provider compile report"
    )
    if report.get("schema_version") != 1 or report.get("kind") != "arm9-source-provider-compile":
        raise SourceProviderError("source provider compile report identity is invalid")
    if report.get("plan_sha256") != plan["plan_sha256"]:
        raise SourceProviderError("source provider compile report is stale")
    expected = [
        (unit["image"], unit["unit_id"], index, relative)
        for unit in plan["units"]
        for index, relative in enumerate(unit["objects"])
    ]
    records = report.get("objects")
    if not isinstance(records, list) or len(records) != len(expected):
        raise SourceProviderError("source provider compile report has wrong objects")
    for expected_entry, record in zip(expected, records, strict=True):
        if not isinstance(record, dict):
            raise SourceProviderError("source provider compile object is invalid")
        image, unit_id, command_index, relative = expected_entry
        if (
            record.get("image"), record.get("unit_id"),
            record.get("command_index"), record.get("object")
        ) != (image, unit_id, command_index, relative):
            raise SourceProviderError("source provider compile object order is stale")
        path = (build / PurePosixPath(relative)).resolve()
        if not path.is_relative_to(build) or not path.is_file() or path.is_symlink():
            raise SourceProviderError("source provider compiled object is missing")
        data = path.read_bytes()
        if record.get("size") != len(data) or record.get("object_sha256") != sha256_bytes(data):
            raise SourceProviderError("source provider compiled object integrity changed")
        validate_arm_relocatable_elf(data, f"source object {unit_id}[{command_index}]")
    return report


def _load_artifact_registry(
    build: Path,
    source_root: Path,
    manifest: UnitManifest,
) -> dict[str, dict[str, Any]]:
    registry_path = build / ARTIFACT_REGISTRY
    source_relative = source_root.relative_to(build).as_posix()
    if not registry_path.exists():
        return {}
    if registry_path.is_symlink():
        raise SourceProviderError("source provider artifact registry may not be a symlink")
    value, _ = read_json_object(registry_path, "source provider artifact registry")
    strict_keys(
        value,
        "source provider artifact registry",
        {
            "schema_version", "kind", "manifest_sha256", "source_root",
            "artifact_count", "artifacts",
        },
    )
    if (
        value["schema_version"] != 1
        or value["kind"] != "arm9-source-provider-artifacts"
    ):
        raise SourceProviderError("source provider artifact registry identity is invalid")
    if value["manifest_sha256"] != manifest.sha256:
        raise SourceProviderError("source provider artifact registry manifest is stale")
    if value["source_root"] != source_relative:
        raise SourceProviderError("source provider artifact registry root is stale")
    raw_artifacts = strict_list(
        value["artifacts"], "source provider artifact registry artifacts"
    )
    artifact_count = value["artifact_count"]
    if (
        not isinstance(artifact_count, int)
        or isinstance(artifact_count, bool)
        or artifact_count != len(raw_artifacts)
    ):
        raise SourceProviderError("source provider artifact registry count is stale")

    available = manifest_units(manifest)
    artifacts: dict[str, dict[str, Any]] = {}
    registered_units: set[tuple[str, str]] = set()
    for index, raw_artifact in enumerate(raw_artifacts):
        label = f"source provider artifact registry artifacts[{index}]"
        artifact = strict_keys(
            raw_artifact,
            label,
            {
                "image", "unit_id", "artifact", "artifact_sha256", "size",
                "target_sha256",
            },
        )
        image = artifact["image"]
        unit_id = artifact["unit_id"]
        if not isinstance(image, str) or not isinstance(unit_id, str):
            raise SourceProviderError(f"{label} unit identity is invalid")
        unit = available.get((image, unit_id))
        relative = relative_path(artifact["artifact"], f"{label}.artifact")
        digest = artifact["artifact_sha256"]
        size = artifact["size"]
        if unit is None or unit.image != "arm9":
            raise SourceProviderError(f"{label} does not reference a configured ARM9 unit")
        if (
            relative != unit.source_path
            or artifact["target_sha256"] != unit.target_sha256
            or not isinstance(size, int)
            or isinstance(size, bool)
            or size != unit.size
            or not isinstance(digest, str)
            or re.fullmatch(r"[0-9a-f]{64}", digest) is None
        ):
            raise SourceProviderError(f"{label} metadata is stale or invalid")
        key = (image, unit_id)
        if relative in artifacts or key in registered_units:
            raise SourceProviderError("source provider artifact registry has duplicates")
        artifacts[relative] = dict(artifact)
        registered_units.add(key)
    if list(artifacts) != sorted(artifacts):
        raise SourceProviderError("source provider artifact registry is not ordered")
    return artifacts


def _validate_managed_source_tree(
    source_root: Path,
    artifacts: dict[str, dict[str, Any]],
) -> None:
    if source_root.is_symlink():
        raise SourceProviderError("source provider root may not be a symbolic link")
    if not source_root.exists():
        if artifacts:
            raise SourceProviderError("registered source provider artifacts are missing")
        return
    symlinks = [path for path in source_root.rglob("*") if path.is_symlink()]
    if symlinks:
        raise SourceProviderError("source provider tree may not contain symbolic links")
    actual = {
        path.relative_to(source_root).as_posix(): path
        for path in source_root.rglob("*")
        if path.is_file()
    }
    extra = set(actual) - set(artifacts)
    missing = set(artifacts) - set(actual)
    if extra:
        raise SourceProviderError(
            "source provider tree contains artifacts not present in its registry"
        )
    if missing:
        raise SourceProviderError("registered source provider artifacts are missing")
    for relative, artifact in artifacts.items():
        data = actual[relative].read_bytes()
        if (
            len(data) != artifact["size"]
            or sha256_bytes(data) != artifact["artifact_sha256"]
        ):
            raise SourceProviderError(
                f"registered source provider artifact integrity changed: {relative}"
            )


def _write_artifact_registry(
    build: Path,
    source_root: Path,
    manifest: UnitManifest,
    artifacts: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    value = {
        "schema_version": 1,
        "kind": "arm9-source-provider-artifacts",
        "manifest_sha256": manifest.sha256,
        "source_root": source_root.relative_to(build).as_posix(),
        "artifact_count": len(artifacts),
        "artifacts": [artifacts[path] for path in sorted(artifacts)],
    }
    registry_path = build / ARTIFACT_REGISTRY
    if registry_path.is_symlink():
        raise SourceProviderError("source provider artifact registry may not be a symlink")
    write_json(registry_path, value)
    return value


def stage_candidates(
    manifest_path: Path,
    promotions_path: Path,
    project_root: Path,
    build_dir: Path,
    candidate_root: Path,
    source_root: Path,
) -> dict[str, Any]:
    project = project_root.resolve()
    build = build_root(project, build_dir)
    plan = build_plan(manifest_path, promotions_path, project, build)
    validate_compile_report(build, plan)
    manifest = load_manifest(manifest_path)

    candidate_requested = (
        candidate_root if candidate_root.is_absolute() else project / candidate_root
    )
    source_requested = source_root if source_root.is_absolute() else project / source_root
    if candidate_requested.is_symlink() or source_requested.is_symlink():
        raise SourceProviderError("candidate and source roots may not be symbolic links")
    candidates = candidate_requested.resolve()
    sources = source_requested.resolve()
    if not candidates.is_relative_to(build) or not sources.is_relative_to(build):
        raise SourceProviderError("candidate and source roots must remain beneath linker build")

    managed_artifacts = _load_artifact_registry(build, sources, manifest)
    _validate_managed_source_tree(sources, managed_artifacts)
    staged: list[dict[str, Any]] = []
    for unit in plan["units"]:
        relative = PurePosixPath(unit["provider_path"])
        candidate_lexical = candidates / relative
        output_lexical = sources / relative
        if candidate_lexical.is_symlink() or output_lexical.is_symlink():
            raise SourceProviderError("unit provider paths may not be symbolic links")
        candidate = candidate_lexical.resolve()
        output = output_lexical.resolve()
        if not candidate.is_relative_to(candidates) or not output.is_relative_to(sources):
            raise SourceProviderError("unit candidate path escapes its configured root")
        if not candidate.is_file() or candidate.is_symlink():
            raise SourceProviderError(f"linked unit candidate is missing: {unit['unit_id']}")
        data = candidate.read_bytes()
        if len(data) != unit["size"] or len(data) > MAX_PROMOTION_BYTES:
            raise SourceProviderError(
                f"linked unit candidate has wrong size for {unit['unit_id']}"
            )
        digest = sha256_bytes(data)
        previous = managed_artifacts.get(unit["provider_path"])
        if (
            previous is not None
            and previous["artifact_sha256"] == unit["target_sha256"]
            and digest != unit["target_sha256"]
        ):
            raise SourceProviderError(
                f"non-exact candidate may not replace an exact managed artifact: "
                f"{unit['unit_id']}"
            )
        output.parent.mkdir(parents=True, exist_ok=True)
        temporary = output.with_name(f".{output.name}.tmp")
        temporary.write_bytes(data)
        temporary.replace(output)
        record = {
            "image": unit["image"],
            "unit_id": unit["unit_id"],
            "artifact": unit["provider_path"],
            "artifact_sha256": digest,
            "size": len(data),
            "target_sha256": unit["target_sha256"],
        }
        staged.append(record)
        managed_artifacts[unit["provider_path"]] = record
    _validate_managed_source_tree(sources, managed_artifacts)
    registry = _write_artifact_registry(build, sources, manifest, managed_artifacts)

    probe_report = probe(
        manifest.path, project, build, sources, None
    )
    candidates_by_unit = {
        (record["image"], record["unit_id"]): record
        for record in probe_report["candidates"]
        if record["provider"] == "source"
    }
    selections_by_unit = {
        (record["image"], record["unit_id"]): record
        for record in probe_report["selections"]
    }
    results: list[dict[str, Any]] = []
    for record in staged:
        key = (record["image"], record["unit_id"])
        candidate = candidates_by_unit[key]
        selection = selections_by_unit[key]
        results.append({
            **record,
            "exact": candidate["exact"],
            "selected_provider": selection["provider"],
            "credited_bytes": 0,
        })
    report = {
        "schema_version": 1,
        "kind": "arm9-source-provider-stage",
        "plan_sha256": plan["plan_sha256"],
        "probe_manifest_sha256": probe_report["manifest_sha256"],
        "unit_count": len(results),
        "managed_artifact_count": registry["artifact_count"],
        "artifact_registry": ARTIFACT_REGISTRY.as_posix(),
        "exact_candidate_units": sum(1 for item in results if item["exact"]),
        "units": results,
        "credited_matching_bytes": 0,
        "fallback_credited_bytes": 0,
        "status": "probed-uncredited",
    }
    write_json(build / "source-provider/stage.v1.json", report)
    return report


def add_common(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--promotions", type=Path, required=True)
    parser.add_argument("--project-root", type=Path, default=PROJECT_ROOT)
    parser.add_argument("--build-dir", type=Path, default=DEFAULT_BUILD_DIR)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    plan = commands.add_parser("plan", help="emit deterministic unit compile commands")
    add_common(plan)
    compile_parser = commands.add_parser("compile", help="run the planned mwccarm commands")
    add_common(compile_parser)
    compile_parser.add_argument("--mwccarm")
    stage = commands.add_parser("stage", help="stage linked raw units and run probe only")
    add_common(stage)
    stage.add_argument(
        "--candidate-root", type=Path,
        default=Path("build/linker/source-provider/candidates"),
    )
    stage.add_argument(
        "--source-root", type=Path, default=Path("build/linker/source"),
    )
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        if args.command == "plan":
            report = build_plan(
                args.manifest, args.promotions, args.project_root, args.build_dir
            )
        elif args.command == "compile":
            compiler = args.mwccarm or os.environ.get("MWCCARM")
            if not compiler:
                raise SourceProviderError("--mwccarm or MWCCARM is required")
            report = compile_sources(
                args.manifest, args.promotions, args.project_root,
                args.build_dir, compiler,
            )
        else:
            report = stage_candidates(
                args.manifest, args.promotions, args.project_root,
                args.build_dir, args.candidate_root, args.source_root,
            )
    except SourceProviderLicenseError as error:
        print(f"ARM9 source provider license error: {error}", file=sys.stderr)
        return 3
    except (SourceProviderError, IncrementalLinkError, OSError) as error:
        print(f"ARM9 source provider error: {error}", file=sys.stderr)
        return 2
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
