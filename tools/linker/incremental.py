#!/usr/bin/env python3
"""Bootstrap, select and compare incremental ARM9/ARM9i link units.

The checked-in manifest contains only target metadata.  ROM-derived fallback
bytes and every selection/comparison report are generated below ``build/``.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import sys
from typing import Any, Sequence


PROJECT_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_MANIFEST = PROJECT_ROOT / "config/linker/units.v1.json"
DEFAULT_BUILD_DIR = PROJECT_ROOT / "build/linker"
ID_PATTERN = re.compile(r"^[a-z0-9][a-z0-9._-]*$")
SHA256_PATTERN = re.compile(r"^[0-9a-f]{64}$")
TARGET_IDS = ("arm9", "arm9i")
UNIT_KINDS = {
    "autoload",
    "autoload-table",
    "container-prefix",
    "ltdautoload",
    "ltdautoload-table",
    "static",
}


class IncrementalLinkError(RuntimeError):
    """A manifest, artifact or provenance contract is invalid."""


@dataclass(frozen=True)
class LinkUnit:
    image: str
    region_id: str
    chunk_id: str
    unit_id: str
    kind: str
    target_offset: int
    size: int
    target_sha256: str
    source_path: str
    sdk_path: str


@dataclass(frozen=True)
class LinkRegion:
    image: str
    region_id: str
    kind: str
    target_offset: int
    size: int
    target_sha256: str
    linker_mode: str
    linker_block: str | None
    linker_name: str | None
    linker_address: int | None
    linker_after: str | None
    bss_size: int
    units: tuple[LinkUnit, ...]


@dataclass(frozen=True)
class LinkTarget:
    image: str
    path: str
    size: int
    sha256: str
    regions: tuple[LinkRegion, ...]

    @property
    def units(self) -> tuple[LinkUnit, ...]:
        return tuple(unit for region in self.regions for unit in region.units)


@dataclass(frozen=True)
class UnitManifest:
    path: Path
    sha256: str
    targets: tuple[LinkTarget, ...]


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def strict_object(
    value: Any,
    label: str,
    *,
    required: set[str],
    optional: set[str] | None = None,
) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise IncrementalLinkError(f"{label} must be an object")
    optional = optional or set()
    missing = required - set(value)
    unknown = set(value) - required - optional
    if missing:
        raise IncrementalLinkError(
            f"{label} is missing keys: {', '.join(sorted(missing))}"
        )
    if unknown:
        raise IncrementalLinkError(
            f"{label} has unknown keys: {', '.join(sorted(unknown))}"
        )
    return value


def strict_array(value: Any, label: str) -> list[Any]:
    if not isinstance(value, list):
        raise IncrementalLinkError(f"{label} must be an array")
    return value


def non_negative_integer(value: Any, label: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < 0:
        raise IncrementalLinkError(f"{label} must be a non-negative integer")
    return value


def non_empty_string(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value:
        raise IncrementalLinkError(f"{label} must be a non-empty string")
    return value


def identifier(value: Any, label: str) -> str:
    result = non_empty_string(value, label)
    if ID_PATTERN.fullmatch(result) is None:
        raise IncrementalLinkError(f"{label} is not a safe identifier")
    return result


def digest(value: Any, label: str) -> str:
    result = non_empty_string(value, label)
    if SHA256_PATTERN.fullmatch(result) is None:
        raise IncrementalLinkError(f"{label} must be a lowercase SHA-256")
    return result


def relative_path(value: Any, label: str) -> str:
    result = non_empty_string(value, label)
    if "\\" in result:
        raise IncrementalLinkError(f"{label} must use POSIX separators")
    path = PurePosixPath(result)
    if path.is_absolute() or any(part in ("", ".", "..") for part in path.parts):
        raise IncrementalLinkError(f"{label} must be a normalized relative path")
    if str(path) != result:
        raise IncrementalLinkError(f"{label} must be a normalized relative path")
    return result


def read_json_object(path: Path, label: str) -> tuple[dict[str, Any], bytes]:
    try:
        raw = path.read_bytes()
        value = json.loads(raw.decode("utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise IncrementalLinkError(f"cannot read {label} {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise IncrementalLinkError(f"{label} must be a JSON object: {path}")
    return value, raw


def load_manifest(path: Path) -> UnitManifest:
    value, raw = read_json_object(path, "unit manifest")
    strict_object(
        value,
        "unit manifest",
        required={"schema_version", "kind", "targets"},
    )
    if value["schema_version"] != 1:
        raise IncrementalLinkError("unit manifest schema_version must be 1")
    if value["kind"] != "arm9-incremental-link-units":
        raise IncrementalLinkError("unit manifest kind is invalid")

    targets: list[LinkTarget] = []
    seen_targets: set[str] = set()
    for target_index, target_value in enumerate(
        strict_array(value["targets"], "manifest targets")
    ):
        label = f"target {target_index}"
        target = strict_object(
            target_value,
            label,
            required={"id", "path", "size", "sha256", "regions"},
        )
        image = identifier(target["id"], f"{label} id")
        if image not in TARGET_IDS:
            raise IncrementalLinkError(f"{label} id must be arm9 or arm9i")
        if image in seen_targets:
            raise IncrementalLinkError(f"duplicate target id: {image}")
        seen_targets.add(image)
        target_path = relative_path(target["path"], f"{label} path")
        target_size = non_negative_integer(target["size"], f"{label} size")
        target_digest = digest(target["sha256"], f"{label} SHA-256")

        regions: list[LinkRegion] = []
        next_region_offset = 0
        seen_regions: set[str] = set()
        for region_index, region_value in enumerate(
            strict_array(target["regions"], f"{label} regions")
        ):
            region_label = f"{label} region {region_index}"
            region = strict_object(
                region_value,
                region_label,
                required={
                    "id",
                    "kind",
                    "target_offset",
                    "size",
                    "sha256",
                    "linker",
                    "unit_size",
                    "unit_sha256",
                },
            )
            region_id = identifier(region["id"], f"{region_label} id")
            if region_id in seen_regions:
                raise IncrementalLinkError(
                    f"duplicate region id for {image}: {region_id}"
                )
            seen_regions.add(region_id)
            region_kind = non_empty_string(
                region["kind"], f"{region_label} kind"
            )
            if region_kind not in UNIT_KINDS:
                raise IncrementalLinkError(
                    f"{region_label} kind is unsupported: {region_kind}"
                )
            region_offset = non_negative_integer(
                region["target_offset"], f"{region_label} target offset"
            )
            region_size = non_negative_integer(
                region["size"], f"{region_label} size"
            )
            if region_size == 0:
                raise IncrementalLinkError(
                    f"{region_label} size must be positive"
                )
            if region_offset != next_region_offset:
                raise IncrementalLinkError(
                    f"{region_label} starts at {region_offset}, expected contiguous "
                    f"offset {next_region_offset}"
                )
            next_region_offset += region_size
            region_digest = digest(
                region["sha256"], f"{region_label} SHA-256"
            )
            linker = strict_object(
                region["linker"],
                f"{region_label} linker",
                required={
                    "mode",
                    "block",
                    "name",
                    "address",
                    "after",
                    "bss_size",
                },
            )
            linker_mode = non_empty_string(
                linker["mode"], f"{region_label} linker mode"
            )
            if linker_mode not in ("generated", "input"):
                raise IncrementalLinkError(
                    f"{region_label} linker mode must be generated or input"
                )

            def optional_string(value: Any, field: str) -> str | None:
                if value is None:
                    return None
                return non_empty_string(value, f"{region_label} {field}")

            def optional_integer(value: Any, field: str) -> int | None:
                if value is None:
                    return None
                return non_negative_integer(value, f"{region_label} {field}")

            linker_block = optional_string(linker["block"], "linker block")
            linker_name = optional_string(linker["name"], "linker name")
            linker_address = optional_integer(
                linker["address"], "linker address"
            )
            linker_after = optional_string(linker["after"], "linker after")
            bss_size = non_negative_integer(
                linker["bss_size"], f"{region_label} linker BSS size"
            )
            if linker_mode == "generated":
                if any(
                    value is not None
                    for value in (
                        linker_block,
                        linker_name,
                        linker_address,
                        linker_after,
                    )
                ) or bss_size != 0:
                    raise IncrementalLinkError(
                        f"{region_label} generated linker metadata must be empty"
                    )
            else:
                if linker_block not in ("Static", "Autoload", "Ltdautoload"):
                    raise IncrementalLinkError(
                        f"{region_label} linker block is invalid"
                    )
                if linker_name is None:
                    raise IncrementalLinkError(
                        f"{region_label} input linker name is required"
                    )
                if linker_block in ("Static", "Autoload"):
                    if linker_address is None or linker_after is not None:
                        raise IncrementalLinkError(
                            f"{region_label} requires address and no after placement"
                        )
                elif linker_after is None or linker_address is not None:
                    raise IncrementalLinkError(
                        f"{region_label} requires after and no address placement"
                    )

            unit_size = non_negative_integer(
                region["unit_size"], f"{region_label} unit size"
            )
            if unit_size == 0 or unit_size > 0x4000:
                raise IncrementalLinkError(
                    f"{region_label} unit size must be in 1..16384"
                )
            final_unit_size = region_size % unit_size or unit_size
            if region_size >= 0x2000 and (
                unit_size < 0x2000 or final_unit_size < 0x2000
            ):
                raise IncrementalLinkError(
                    f"{region_label} large-region units must each stay in "
                    "8192..16384 bytes"
                )
            unit_hashes = strict_array(
                region["unit_sha256"], f"{region_label} unit SHA-256 list"
            )
            expected_unit_count = (region_size + unit_size - 1) // unit_size
            if len(unit_hashes) != expected_unit_count:
                raise IncrementalLinkError(
                    f"{region_label} has {len(unit_hashes)} unit hashes, expected "
                    f"{expected_unit_count}"
                )
            units: list[LinkUnit] = []
            for unit_index, unit_hash in enumerate(unit_hashes):
                chunk_id = f"u{unit_index:03d}"
                offset = region_offset + unit_index * unit_size
                size = min(unit_size, region_offset + region_size - offset)
                unit_id = f"{region_id}-{chunk_id}"
                provider_path = f"{image}/{region_id}/{chunk_id}.bin"
                units.append(
                    LinkUnit(
                        image=image,
                        region_id=region_id,
                        chunk_id=chunk_id,
                        unit_id=unit_id,
                        kind=region_kind,
                        target_offset=offset,
                        size=size,
                        target_sha256=digest(
                            unit_hash,
                            f"{region_label} {chunk_id} SHA-256",
                        ),
                        source_path=provider_path,
                        sdk_path=provider_path,
                    )
                )
            regions.append(
                LinkRegion(
                    image=image,
                    region_id=region_id,
                    kind=region_kind,
                    target_offset=region_offset,
                    size=region_size,
                    target_sha256=region_digest,
                    linker_mode=linker_mode,
                    linker_block=linker_block,
                    linker_name=linker_name,
                    linker_address=linker_address,
                    linker_after=linker_after,
                    bss_size=bss_size,
                    units=tuple(units),
                )
            )
        if not regions:
            raise IncrementalLinkError(f"{label} must contain at least one region")
        if next_region_offset != target_size:
            raise IncrementalLinkError(
                f"{label} regions cover {next_region_offset} bytes, expected "
                f"{target_size}"
            )
        targets.append(
            LinkTarget(
                image=image,
                path=target_path,
                size=target_size,
                sha256=target_digest,
                regions=tuple(regions),
            )
        )
    if seen_targets != set(TARGET_IDS):
        raise IncrementalLinkError("manifest must contain exactly arm9 and arm9i")
    targets.sort(key=lambda item: TARGET_IDS.index(item.image))
    return UnitManifest(path=path, sha256=sha256_bytes(raw), targets=tuple(targets))


def safe_join(root: Path, relative: str, label: str) -> Path:
    root = root.resolve()
    candidate = (root / relative).resolve()
    try:
        candidate.relative_to(root)
    except ValueError as exc:
        raise IncrementalLinkError(f"{label} escapes its configured root") from exc
    return candidate


def read_regular_file(path: Path, label: str) -> bytes:
    if not path.is_file():
        raise IncrementalLinkError(f"{label} is not a regular file: {path}")
    try:
        return path.read_bytes()
    except OSError as exc:
        raise IncrementalLinkError(f"cannot read {label} {path}: {exc}") from exc


def target_bytes(manifest: UnitManifest, project_root: Path) -> dict[str, bytes]:
    result: dict[str, bytes] = {}
    for target in manifest.targets:
        path = safe_join(project_root, target.path, f"{target.image} target path")
        data = read_regular_file(path, f"{target.image} target")
        if len(data) != target.size:
            raise IncrementalLinkError(
                f"{target.image} target size is {len(data)}, expected {target.size}"
            )
        if sha256_bytes(data) != target.sha256:
            raise IncrementalLinkError(
                f"{target.image} target SHA-256 does not match the manifest"
            )
        for region in target.regions:
            body = data[
                region.target_offset : region.target_offset + region.size
            ]
            if sha256_bytes(body) != region.target_sha256:
                raise IncrementalLinkError(
                    f"{target.image}/{region.region_id} region SHA-256 is stale"
                )
        for unit in target.units:
            body = data[unit.target_offset : unit.target_offset + unit.size]
            if sha256_bytes(body) != unit.target_sha256:
                raise IncrementalLinkError(
                    f"{target.image}/{unit.unit_id} target SHA-256 is stale"
                )
        result[target.image] = data
    return result


def write_json(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_text(
        json.dumps(value, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    temporary.replace(path)


def fallback_relative(unit: LinkUnit) -> str:
    return (
        f"fallback/{unit.image}/{unit.region_id}/{unit.chunk_id}.bin"
    )


def bootstrap(
    manifest_path: Path,
    project_root: Path,
    build_dir: Path,
    *,
    force: bool = False,
) -> dict[str, Any]:
    manifest = load_manifest(manifest_path)
    targets = target_bytes(manifest, project_root)
    units: list[dict[str, Any]] = []

    for target in manifest.targets:
        target_data = targets[target.image]
        for unit in target.units:
            body = target_data[
                unit.target_offset : unit.target_offset + unit.size
            ]
            relative = fallback_relative(unit)
            artifact = safe_join(
                build_dir, relative, f"{target.image}/{unit.unit_id} fallback"
            )
            if artifact.exists():
                if not artifact.is_file():
                    raise IncrementalLinkError(
                        f"fallback path is not a regular file: {artifact}"
                    )
                existing = artifact.read_bytes()
                if existing != body and not force:
                    raise IncrementalLinkError(
                        f"fallback differs for {target.image}/{unit.unit_id}; "
                        "rerun bootstrap with --force to regenerate it"
                    )
            if not artifact.exists() or artifact.read_bytes() != body:
                artifact.parent.mkdir(parents=True, exist_ok=True)
                artifact.write_bytes(body)
            units.append(
                {
                    "artifact": relative,
                    "artifact_sha256": sha256_bytes(body),
                    "image": target.image,
                    "size": unit.size,
                    "target_offset": unit.target_offset,
                    "target_sha256": unit.target_sha256,
                    "unit_id": unit.unit_id,
                }
            )

    report = {
        "schema_version": 1,
        "kind": "arm9-incremental-fallback-index",
        "manifest_sha256": manifest.sha256,
        "targets": {
            target.image: {
                "sha256": target.sha256,
                "size": target.size,
            }
            for target in manifest.targets
        },
        "units": units,
    }
    write_json(build_dir / "fallback-index.v1.json", report)
    return report


def validate_fallback_index(
    manifest: UnitManifest,
    build_dir: Path,
) -> dict[tuple[str, str], dict[str, Any]]:
    index_path = build_dir / "fallback-index.v1.json"
    value, _ = read_json_object(index_path, "fallback index")
    strict_object(
        value,
        "fallback index",
        required={
            "schema_version",
            "kind",
            "manifest_sha256",
            "targets",
            "units",
        },
    )
    if value["schema_version"] != 1 or value["kind"] != (
        "arm9-incremental-fallback-index"
    ):
        raise IncrementalLinkError("fallback index identity is invalid")
    if value["manifest_sha256"] != manifest.sha256:
        raise IncrementalLinkError("fallback index was built for another manifest")
    targets = strict_object(
        value["targets"],
        "fallback targets",
        required=set(TARGET_IDS),
    )
    for target in manifest.targets:
        metadata = strict_object(
            targets[target.image],
            f"fallback {target.image} metadata",
            required={"size", "sha256"},
        )
        if metadata != {"size": target.size, "sha256": target.sha256}:
            raise IncrementalLinkError(
                f"fallback target metadata is stale for {target.image}"
            )

    expected = {
        (unit.image, unit.unit_id): unit
        for target in manifest.targets
        for unit in target.units
    }
    records: dict[tuple[str, str], dict[str, Any]] = {}
    for index, record_value in enumerate(
        strict_array(value["units"], "fallback units")
    ):
        label = f"fallback unit {index}"
        record = strict_object(
            record_value,
            label,
            required={
                "artifact",
                "artifact_sha256",
                "image",
                "size",
                "target_offset",
                "target_sha256",
                "unit_id",
            },
        )
        image = identifier(record["image"], f"{label} image")
        unit_id = identifier(record["unit_id"], f"{label} id")
        key = (image, unit_id)
        if key not in expected:
            raise IncrementalLinkError(f"unexpected fallback unit: {image}/{unit_id}")
        if key in records:
            raise IncrementalLinkError(f"duplicate fallback unit: {image}/{unit_id}")
        unit = expected[key]
        expected_record = {
            "artifact": fallback_relative(unit),
            "artifact_sha256": unit.target_sha256,
            "image": image,
            "size": unit.size,
            "target_offset": unit.target_offset,
            "target_sha256": unit.target_sha256,
            "unit_id": unit_id,
        }
        if record != expected_record:
            raise IncrementalLinkError(
                f"fallback provenance is stale for {image}/{unit_id}"
            )
        artifact = safe_join(
            build_dir, record["artifact"], f"{image}/{unit_id} fallback artifact"
        )
        data = read_regular_file(artifact, f"{image}/{unit_id} fallback artifact")
        if len(data) != unit.size or sha256_bytes(data) != unit.target_sha256:
            raise IncrementalLinkError(
                f"fallback artifact is invalid for {image}/{unit_id}"
            )
        records[key] = record
    if set(records) != set(expected):
        missing = sorted(set(expected) - set(records))
        raise IncrementalLinkError(f"fallback index is missing units: {missing}")
    return records


def provider_candidate(
    root: Path | None,
    relative: str,
    unit: LinkUnit,
    provider: str,
) -> tuple[Path, bytes] | None:
    if root is None:
        return None
    path = safe_join(root, relative, f"{unit.image}/{unit.unit_id} {provider}")
    if not path.exists():
        return None
    data = read_regular_file(path, f"{unit.image}/{unit.unit_id} {provider}")
    if len(data) != unit.size:
        raise IncrementalLinkError(
            f"{provider} artifact size for {unit.image}/{unit.unit_id} is "
            f"{len(data)}, expected {unit.size}"
        )
    return path, data


def probe(
    manifest_path: Path,
    project_root: Path,
    build_dir: Path,
    source_dir: Path,
    sdk_dir: Path | None,
) -> dict[str, Any]:
    manifest = load_manifest(manifest_path)
    targets = target_bytes(manifest, project_root)
    fallbacks = validate_fallback_index(manifest, build_dir)
    selections: list[dict[str, Any]] = []
    candidates: list[dict[str, Any]] = []

    for target in manifest.targets:
        for unit in target.units:
            target_body = targets[target.image][
                unit.target_offset : unit.target_offset + unit.size
            ]
            selected: tuple[Path, bytes] | None = None
            provider = "fallback"
            relative = fallback_relative(unit)
            root: Path | None = build_dir
            for candidate_provider, candidate_root, candidate_path in (
                ("source", source_dir, unit.source_path),
                ("sdk", sdk_dir, unit.sdk_path),
            ):
                candidate = provider_candidate(
                    candidate_root,
                    candidate_path,
                    unit,
                    candidate_provider,
                )
                if candidate is None:
                    continue
                candidate_file, candidate_data = candidate
                candidate_digest = sha256_bytes(candidate_data)
                exact = candidate_digest == unit.target_sha256
                candidates.append(
                    {
                        "artifact_path": candidate_path,
                        "artifact_sha256": candidate_digest,
                        "exact": exact,
                        "image": unit.image,
                        "provider": candidate_provider,
                        "provider_root": str(candidate_root.resolve()),
                        "size": unit.size,
                        "target_offset": unit.target_offset,
                        "target_sha256": unit.target_sha256,
                        "unit_id": unit.unit_id,
                    }
                )
                if exact:
                    selected = (candidate_file, candidate_data)
                    provider = candidate_provider
                    relative = candidate_path
                    root = candidate_root
                    break
            if selected is None:
                fallback = fallbacks[(unit.image, unit.unit_id)]
                relative = fallback["artifact"]
                root = build_dir
                selected = (
                    safe_join(
                        build_dir,
                        relative,
                        f"{unit.image}/{unit.unit_id} fallback",
                    ),
                    read_regular_file(
                        safe_join(
                            build_dir,
                            relative,
                            f"{unit.image}/{unit.unit_id} fallback",
                        ),
                        f"{unit.image}/{unit.unit_id} fallback",
                    ),
                )
                provider = "fallback"
            path, data = selected
            artifact_digest = sha256_bytes(data)
            if artifact_digest != unit.target_sha256 or data != target_body:
                raise IncrementalLinkError(
                    f"non-exact artifact cannot be promoted for "
                    f"{unit.image}/{unit.unit_id}"
                )
            selections.append(
                {
                    "artifact_path": relative,
                    "artifact_sha256": artifact_digest,
                    "exact": data == target_body,
                    "image": unit.image,
                    "provider": provider,
                    "provider_root": str(root.resolve()) if root else "",
                    "size": unit.size,
                    "target_offset": unit.target_offset,
                    "target_sha256": unit.target_sha256,
                    "unit_id": unit.unit_id,
                }
            )
            if path.resolve() != safe_join(
                root, relative, f"{unit.image}/{unit.unit_id} selected artifact"
            ):
                raise IncrementalLinkError("selected artifact path changed unexpectedly")

    report = {
        "schema_version": 1,
        "kind": "arm9-incremental-link-probe",
        "manifest_sha256": manifest.sha256,
        "roots": {
            "build": str(build_dir.resolve()),
            "sdk": str(sdk_dir.resolve()) if sdk_dir else None,
            "source": str(source_dir.resolve()),
        },
        "candidates": candidates,
        "selections": selections,
    }
    write_json(build_dir / "probe.v1.json", report)
    return report


def load_probe(
    path: Path,
    manifest: UnitManifest,
    build_dir: Path,
    source_dir: Path,
    sdk_dir: Path | None,
) -> tuple[
    dict[tuple[str, str], dict[str, Any]],
    list[dict[str, Any]],
]:
    value, _ = read_json_object(path, "probe report")
    strict_object(
        value,
        "probe report",
        required={
            "schema_version",
            "kind",
            "manifest_sha256",
            "roots",
            "candidates",
            "selections",
        },
    )
    if value["schema_version"] != 1 or value["kind"] != (
        "arm9-incremental-link-probe"
    ):
        raise IncrementalLinkError("probe report identity is invalid")
    if value["manifest_sha256"] != manifest.sha256:
        raise IncrementalLinkError("probe report was generated for another manifest")
    roots = strict_object(
        value["roots"],
        "probe roots",
        required={"build", "source", "sdk"},
    )
    expected_roots = {
        "build": str(build_dir.resolve()),
        "source": str(source_dir.resolve()),
        "sdk": str(sdk_dir.resolve()) if sdk_dir else None,
    }
    if roots != expected_roots:
        raise IncrementalLinkError("probe provider roots differ from compare roots")

    expected = {
        (unit.image, unit.unit_id): unit
        for target in manifest.targets
        for unit in target.units
    }
    records: dict[tuple[str, str], dict[str, Any]] = {}
    required = {
        "artifact_path",
        "artifact_sha256",
        "exact",
        "image",
        "provider",
        "provider_root",
        "size",
        "target_offset",
        "target_sha256",
        "unit_id",
    }
    for index, record_value in enumerate(
        strict_array(value["selections"], "probe selections")
    ):
        label = f"probe selection {index}"
        record = strict_object(record_value, label, required=required)
        image = identifier(record["image"], f"{label} image")
        unit_id = identifier(record["unit_id"], f"{label} unit id")
        key = (image, unit_id)
        if key not in expected:
            raise IncrementalLinkError(f"unexpected probe unit: {image}/{unit_id}")
        if key in records:
            raise IncrementalLinkError(f"duplicate probe unit: {image}/{unit_id}")
        unit = expected[key]
        provider = record["provider"]
        if provider not in ("source", "sdk", "fallback"):
            raise IncrementalLinkError(f"{label} provider is invalid")
        if not isinstance(record["exact"], bool):
            raise IncrementalLinkError(f"{label} exact must be a boolean")
        if record["exact"] is not True:
            raise IncrementalLinkError(f"{label} promotes a non-exact artifact")
        digest(record["artifact_sha256"], f"{label} artifact SHA-256")
        relative_path(record["artifact_path"], f"{label} artifact path")
        expected_path = {
            "source": unit.source_path,
            "sdk": unit.sdk_path,
            "fallback": fallback_relative(unit),
        }[provider]
        expected_root = {
            "source": source_dir,
            "sdk": sdk_dir,
            "fallback": build_dir,
        }[provider]
        if expected_root is None:
            raise IncrementalLinkError(f"{label} selects unavailable SDK root")
        if record["artifact_path"] != expected_path:
            raise IncrementalLinkError(f"{label} artifact path is inconsistent")
        if record["provider_root"] != str(expected_root.resolve()):
            raise IncrementalLinkError(f"{label} provider root is inconsistent")
        if (
            record["size"] != unit.size
            or record["target_offset"] != unit.target_offset
            or record["target_sha256"] != unit.target_sha256
        ):
            raise IncrementalLinkError(f"{label} unit metadata is inconsistent")
        records[key] = record
    if set(records) != set(expected):
        raise IncrementalLinkError("probe report does not select every manifest unit")

    candidate_records: list[dict[str, Any]] = []
    seen_candidates: set[tuple[str, str, str]] = set()
    for index, record_value in enumerate(
        strict_array(value["candidates"], "probe candidates")
    ):
        label = f"probe candidate {index}"
        record = strict_object(record_value, label, required=required)
        image = identifier(record["image"], f"{label} image")
        unit_id = identifier(record["unit_id"], f"{label} unit id")
        key = (image, unit_id)
        if key not in expected:
            raise IncrementalLinkError(f"unexpected probe candidate: {image}/{unit_id}")
        unit = expected[key]
        provider = record["provider"]
        if provider not in ("source", "sdk"):
            raise IncrementalLinkError(f"{label} provider must be source or sdk")
        candidate_key = (image, unit_id, provider)
        if candidate_key in seen_candidates:
            raise IncrementalLinkError(
                f"duplicate probe candidate: {image}/{unit_id}/{provider}"
            )
        seen_candidates.add(candidate_key)
        if not isinstance(record["exact"], bool):
            raise IncrementalLinkError(f"{label} exact must be a boolean")
        digest(record["artifact_sha256"], f"{label} artifact SHA-256")
        expected_path = (
            unit.source_path if provider == "source" else unit.sdk_path
        )
        expected_root = source_dir if provider == "source" else sdk_dir
        if expected_root is None:
            raise IncrementalLinkError(f"{label} refers to an unavailable SDK root")
        if record["artifact_path"] != expected_path:
            raise IncrementalLinkError(f"{label} artifact path is inconsistent")
        if record["provider_root"] != str(expected_root.resolve()):
            raise IncrementalLinkError(f"{label} provider root is inconsistent")
        if (
            record["size"] != unit.size
            or record["target_offset"] != unit.target_offset
            or record["target_sha256"] != unit.target_sha256
        ):
            raise IncrementalLinkError(f"{label} unit metadata is inconsistent")
        candidate_records.append(record)
    return records, candidate_records


def compare(
    manifest_path: Path,
    project_root: Path,
    build_dir: Path,
    source_dir: Path,
    sdk_dir: Path | None,
    probe_path: Path | None = None,
) -> dict[str, Any]:
    manifest = load_manifest(manifest_path)
    targets = target_bytes(manifest, project_root)
    validate_fallback_index(manifest, build_dir)
    selections, probed_candidates = load_probe(
        probe_path or build_dir / "probe.v1.json",
        manifest,
        build_dir,
        source_dir,
        sdk_dir,
    )
    provider_summary = {
        provider: {"bytes": 0, "matching_bytes": 0, "units": 0}
        for provider in ("source", "sdk", "fallback")
    }
    images: dict[str, Any] = {}
    comparison_units: list[dict[str, Any]] = []
    total_matching = 0
    total_size = 0
    authored_matching = 0
    authored_size = 0
    candidate_comparisons: list[dict[str, Any]] = []
    probed_bytes = 0
    probed_matching = 0

    units_by_key = {
        (unit.image, unit.unit_id): unit
        for target in manifest.targets
        for unit in target.units
    }
    for record in probed_candidates:
        unit = units_by_key[(record["image"], record["unit_id"])]
        root = source_dir if record["provider"] == "source" else sdk_dir
        if root is None:
            raise IncrementalLinkError(
                f"SDK root is unavailable for {unit.image}/{unit.unit_id}"
            )
        artifact = safe_join(
            root,
            record["artifact_path"],
            f"{unit.image}/{unit.unit_id} probed candidate",
        )
        data = read_regular_file(
            artifact, f"{unit.image}/{unit.unit_id} probed candidate"
        )
        if len(data) != unit.size or sha256_bytes(data) != record["artifact_sha256"]:
            raise IncrementalLinkError(
                f"probed candidate changed after probe for "
                f"{unit.image}/{unit.unit_id}/{record['provider']}"
            )
        target_body = targets[unit.image][
            unit.target_offset : unit.target_offset + unit.size
        ]
        matching = sum(left == right for left, right in zip(data, target_body))
        exact = matching == unit.size
        if exact != record["exact"]:
            raise IncrementalLinkError(
                f"candidate exactness changed for {unit.image}/{unit.unit_id}"
            )
        probed_bytes += unit.size
        probed_matching += matching
        candidate_comparisons.append(
            {
                "artifact_sha256": record["artifact_sha256"],
                "exact": exact,
                "image": unit.image,
                "matching_bytes": matching,
                "provider": record["provider"],
                "size": unit.size,
                "target_offset": unit.target_offset,
                "unit_id": unit.unit_id,
            }
        )

    candidate_dir = build_dir / "candidate"
    candidate_dir.mkdir(parents=True, exist_ok=True)
    for target in manifest.targets:
        target_data = targets[target.image]
        candidate = bytearray(target.size)
        image_matching = 0
        image_credited = 0
        image_fallback = 0
        for unit in target.units:
            record = selections[(unit.image, unit.unit_id)]
            provider = record["provider"]
            root = {
                "source": source_dir,
                "sdk": sdk_dir,
                "fallback": build_dir,
            }[provider]
            if root is None:
                raise IncrementalLinkError(
                    f"SDK root is unavailable for {unit.image}/{unit.unit_id}"
                )
            artifact = safe_join(
                root,
                record["artifact_path"],
                f"{unit.image}/{unit.unit_id} selected artifact",
            )
            data = read_regular_file(
                artifact, f"{unit.image}/{unit.unit_id} selected artifact"
            )
            if len(data) != unit.size:
                raise IncrementalLinkError(
                    f"selected artifact size changed for {unit.image}/{unit.unit_id}"
                )
            artifact_digest = sha256_bytes(data)
            if artifact_digest != record["artifact_sha256"]:
                raise IncrementalLinkError(
                    f"selected artifact changed after probe for "
                    f"{unit.image}/{unit.unit_id}"
                )
            target_body = target_data[
                unit.target_offset : unit.target_offset + unit.size
            ]
            matching = sum(
                left == right for left, right in zip(data, target_body)
            )
            exact = matching == unit.size
            if exact != record["exact"]:
                raise IncrementalLinkError(
                    f"probe exactness is inconsistent for {unit.image}/{unit.unit_id}"
                )
            candidate[
                unit.target_offset : unit.target_offset + unit.size
            ] = data
            image_matching += matching
            provider_summary[provider]["units"] += 1
            provider_summary[provider]["bytes"] += unit.size
            provider_summary[provider]["matching_bytes"] += matching
            if provider != "fallback":
                authored_size += unit.size
                authored_matching += matching
                image_credited += matching
            else:
                image_fallback += unit.size
            comparison_units.append(
                {
                    "artifact_sha256": artifact_digest,
                    "exact": exact,
                    "image": unit.image,
                    "matching_bytes": matching,
                    "provider": provider,
                    "size": unit.size,
                    "target_offset": unit.target_offset,
                    "unit_id": unit.unit_id,
                }
            )
        candidate_bytes = bytes(candidate)
        candidate_path = candidate_dir / f"{target.image}.bin"
        candidate_path.write_bytes(candidate_bytes)
        images[target.image] = {
            "candidate": str(candidate_path.relative_to(build_dir)),
            "candidate_sha256": sha256_bytes(candidate_bytes),
            "credited_matching_bytes": image_credited,
            "credit_exact": image_credited == target.size,
            "exact": image_matching == target.size,
            "fallback_bytes": image_fallback,
            "reconstruction_matching_bytes": image_matching,
            "reconstruction_mismatched_bytes": target.size - image_matching,
            "size": target.size,
            "target_sha256": target.sha256,
        }
        total_size += target.size
        total_matching += image_matching

    report = {
        "schema_version": 1,
        "kind": "arm9-incremental-link-comparison",
        "manifest_sha256": manifest.sha256,
        "status": "exact" if total_matching == total_size else "different",
        "images": images,
        "providers": provider_summary,
        "candidates": candidate_comparisons,
        "summary": {
            "authored_bytes": authored_size,
            "authored_matching_bytes": authored_matching,
            "credited_matching_bytes": authored_matching,
            "fallback_bytes": provider_summary["fallback"]["bytes"],
            "fallback_credited_bytes": 0,
            "probed_bytes": probed_bytes,
            "probed_matching_bytes": probed_matching,
            "reconstruction_matching_bytes": total_matching,
            "reconstruction_mismatched_bytes": total_size - total_matching,
            "total_bytes": total_size,
        },
        "units": comparison_units,
    }
    write_json(build_dir / "compare.v1.json", report)
    return report


def add_common_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--project-root", type=Path, default=PROJECT_ROOT)
    parser.add_argument("--build-dir", type=Path, default=DEFAULT_BUILD_DIR)


def add_provider_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--source-dir", type=Path)
    parser.add_argument("--sdk-dir", type=Path)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)

    bootstrap_parser = commands.add_parser(
        "bootstrap", help="generate private target fallback units"
    )
    add_common_arguments(bootstrap_parser)
    bootstrap_parser.add_argument("--force", action="store_true")

    probe_parser = commands.add_parser(
        "probe", help="select source, SDK or fallback for every unit"
    )
    add_common_arguments(probe_parser)
    add_provider_arguments(probe_parser)

    compare_parser = commands.add_parser(
        "compare", help="rebuild selected images and compare target bytes"
    )
    add_common_arguments(compare_parser)
    add_provider_arguments(compare_parser)
    compare_parser.add_argument("--probe", type=Path)
    return parser


def provider_roots(args: argparse.Namespace) -> tuple[Path, Path | None]:
    source = args.source_dir or args.build_dir / "source"
    sdk_setting = args.sdk_dir or os.environ.get("FSAE_LINKER_SDK_UNITS")
    sdk = Path(sdk_setting) if sdk_setting else None
    return source, sdk


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        if args.command == "bootstrap":
            report = bootstrap(
                args.manifest,
                args.project_root,
                args.build_dir,
                force=args.force,
            )
        elif args.command == "probe":
            source, sdk = provider_roots(args)
            report = probe(
                args.manifest,
                args.project_root,
                args.build_dir,
                source,
                sdk,
            )
        else:
            source, sdk = provider_roots(args)
            report = compare(
                args.manifest,
                args.project_root,
                args.build_dir,
                source,
                sdk,
                args.probe,
            )
    except (IncrementalLinkError, OSError) as exc:
        print(f"incremental linker error: {exc}", file=sys.stderr)
        return 2
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
