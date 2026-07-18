#!/usr/bin/env python3
"""Prepare and run the real CodeWarrior ARM9/ARM9i production link.

Unlike :mod:`tools.linker.incremental`, this module does not reconstruct an
image by concatenation.  It creates ARM ELF input objects, generates a linker
specification/response and invokes private ``makelcf`` and ``mwldarm`` tools.
The extracted ARM9i SDK objects are opt-in through ``--arm9i-sdk-set``; without
that option the existing exact fallback wrappers remain the link inputs.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
from typing import Any, Callable, Sequence

PROJECT_ROOT = Path(__file__).resolve().parents[2]
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

from tools.linker.incremental import (
    DEFAULT_BUILD_DIR,
    DEFAULT_MANIFEST,
    IncrementalLinkError,
    LinkRegion,
    LinkUnit,
    UnitManifest,
    load_manifest,
    load_probe,
    read_json_object,
    read_regular_file,
    safe_join,
    sha256_bytes,
    strict_object,
    validate_fallback_index,
    write_json,
)
from tools.linker.arm9i_sdk_prepare import (
    PreparationError,
    parse_plan as parse_arm9i_plan,
    validate_extraction as validate_arm9i_extraction,
)


DEFAULT_LAYOUT = PROJECT_ROOT / "config/linker/arm9.json"
DEFAULT_ARM9I_PLAN = PROJECT_ROOT / "config/build/arm9i.json"
LICENSE_MARKERS = (
    "flexlm",
    "license checkout",
    "license error",
    "licensed number",
    "win32_plugins_arm",
    "-10,32",
)
Runner = Callable[[Sequence[str], Path], subprocess.CompletedProcess[str]]


class CodeWarriorDriverError(RuntimeError):
    """The production-link contract or a private tool invocation failed."""


class CodeWarriorLicenseError(CodeWarriorDriverError):
    """A private CodeWarrior tool reported a license failure."""


def default_runner(
    command: Sequence[str],
    cwd: Path,
) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        list(command),
        cwd=cwd,
        env={**os.environ, "CCACHE_DISABLE": "1"},
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )


def command_text(command: Sequence[str]) -> str:
    return " ".join(str(part) for part in command)


def run_checked(
    command: Sequence[str],
    cwd: Path,
    runner: Runner,
    label: str,
) -> subprocess.CompletedProcess[str]:
    result = runner(tuple(str(part) for part in command), cwd)
    combined = f"{result.stdout or ''}\n{result.stderr or ''}".lower()
    if result.returncode != 0:
        detail = (result.stderr or result.stdout or "no diagnostic").strip()
        if any(marker in combined for marker in LICENSE_MARKERS):
            raise CodeWarriorLicenseError(
                f"{label} license failure ({result.returncode}): {detail}"
            )
        raise CodeWarriorDriverError(
            f"{label} failed ({result.returncode}): {detail}; "
            f"command: {command_text(command)}"
        )
    return result


def section_token(value: str) -> str:
    return value.replace("-", "_").replace(".", "_")


def unit_section(unit: LinkUnit) -> str:
    return (
        f".fsae.{section_token(unit.image)}."
        f"{section_token(unit.region_id)}.{section_token(unit.chunk_id)}"
    )


def bss_section(region: LinkRegion) -> str:
    return (
        f".fsae.{section_token(region.image)}."
        f"{section_token(region.region_id)}.bss"
    )


def escaped_assembly_path(path: Path) -> str:
    return str(path.resolve()).replace("\\", "\\\\").replace('"', '\\"')


def data_assembly(section: str, artifact: Path) -> str:
    return (
        f'.section {section},"a",%progbits\n'
        ".balign 1\n"
        f'.incbin "{escaped_assembly_path(artifact)}"\n'
    )


def bss_assembly(section: str, size: int) -> str:
    return (
        f'.section {section},"aw",%nobits\n'
        ".balign 4\n"
        f".space {size}\n"
    )


def read_elf_sections(path: Path, expected_type: int) -> dict[str, tuple[int, bytes]]:
    data = read_regular_file(path, "ARM ELF file")
    if len(data) < 52 or data[:4] != b"\x7fELF":
        raise CodeWarriorDriverError(f"not an ELF file: {path}")
    if data[4] != 1 or data[5] != 1 or data[6] != 1:
        raise CodeWarriorDriverError(f"ELF must be 32-bit little-endian v1: {path}")
    elf_type, machine = struct.unpack_from("<HH", data, 16)
    if elf_type != expected_type or machine != 40:
        raise CodeWarriorDriverError(
            f"ELF type/machine is {elf_type}/{machine}, expected "
            f"{expected_type}/ARM: {path}"
        )
    section_offset = struct.unpack_from("<I", data, 32)[0]
    entry_size, count, names_index = struct.unpack_from("<HHH", data, 46)
    if entry_size < 40 or count == 0 or names_index >= count:
        raise CodeWarriorDriverError(f"invalid ELF section table: {path}")
    if section_offset + entry_size * count > len(data):
        raise CodeWarriorDriverError(f"truncated ELF section table: {path}")

    headers: list[tuple[int, int, int, int]] = []
    for index in range(count):
        offset = section_offset + index * entry_size
        name, section_type = struct.unpack_from("<II", data, offset)
        file_offset, size = struct.unpack_from("<II", data, offset + 16)
        if section_type != 8 and file_offset + size > len(data):
            raise CodeWarriorDriverError(f"truncated ELF section {index}: {path}")
        headers.append((name, section_type, file_offset, size))
    _, names_type, names_offset, names_size = headers[names_index]
    if names_type != 3 or names_offset + names_size > len(data):
        raise CodeWarriorDriverError(f"invalid ELF section names: {path}")
    names = data[names_offset : names_offset + names_size]

    sections: dict[str, tuple[int, bytes]] = {}
    for name_offset, section_type, file_offset, size in headers:
        if name_offset >= len(names):
            raise CodeWarriorDriverError(f"invalid ELF section name offset: {path}")
        end = names.find(b"\0", name_offset)
        if end < 0:
            raise CodeWarriorDriverError(f"unterminated ELF section name: {path}")
        name = names[name_offset:end].decode("ascii", errors="strict")
        body = b"" if section_type == 8 else data[file_offset : file_offset + size]
        if name in sections:
            raise CodeWarriorDriverError(f"duplicate ELF section {name}: {path}")
        sections[name] = (section_type, body if section_type != 8 else bytes(size))
    return sections


def validate_data_object(
    path: Path,
    section: str,
    expected: bytes,
) -> None:
    sections = read_elf_sections(path, 1)
    if section not in sections:
        raise CodeWarriorDriverError(f"object is missing section {section}: {path}")
    section_type, body = sections[section]
    if section_type != 1 or body != expected:
        raise CodeWarriorDriverError(
            f"object section {section} does not preserve its selected bytes: {path}"
        )


def validate_bss_object(path: Path, section: str, expected_size: int) -> None:
    sections = read_elf_sections(path, 1)
    if section not in sections:
        raise CodeWarriorDriverError(f"object is missing BSS section {section}: {path}")
    section_type, body = sections[section]
    if section_type != 8 or len(body) != expected_size:
        raise CodeWarriorDriverError(
            f"object BSS section {section} has the wrong type or size: {path}"
        )


def validate_arm_relocatable_elf(data: bytes, label: str) -> None:
    """Validate a CodeWarrior-compatible ARM ET_REL without indexing names.

    CodeWarrior objects may legally contain repeated section names such as
    ``.debug_line``.  Production wrapper validation needs a name index, but
    private SDK inputs only need a strict ELF identity and bounds check.
    """
    if len(data) < 52 or data[:4] != b"\x7fELF":
        raise CodeWarriorDriverError(f"{label} is not an ELF file")
    if data[4] != 1 or data[5] != 1 or data[6] != 1:
        raise CodeWarriorDriverError(
            f"{label} must be a 32-bit little-endian ELF v1"
        )
    elf_type, machine = struct.unpack_from("<HH", data, 16)
    if elf_type != 1 or machine != 40:
        raise CodeWarriorDriverError(
            f"{label} ELF type/machine is {elf_type}/{machine}, expected ET_REL/ARM"
        )
    section_offset = struct.unpack_from("<I", data, 32)[0]
    entry_size, count, names_index = struct.unpack_from("<HHH", data, 46)
    if entry_size < 40 or count == 0 or names_index >= count:
        raise CodeWarriorDriverError(f"{label} has an invalid ELF section table")
    if section_offset + entry_size * count > len(data):
        raise CodeWarriorDriverError(f"{label} has a truncated ELF section table")
    for index in range(count):
        offset = section_offset + index * entry_size
        section_type = struct.unpack_from("<I", data, offset + 4)[0]
        file_offset, size = struct.unpack_from("<II", data, offset + 16)
        if section_type != 8 and file_offset + size > len(data):
            raise CodeWarriorDriverError(
                f"{label} has a truncated ELF section payload at index {index}"
            )


def compile_assembly_object(
    source: Path,
    output: Path,
    compiler: str,
    project_root: Path,
    runner: Runner,
) -> list[str]:
    command = [
        compiler,
        "--target=arm-none-eabi",
        "-mcpu=arm946e-s",
        "-marm",
        "-x",
        "assembler",
        "-c",
        str(source),
        "-o",
        str(output),
    ]
    output.parent.mkdir(parents=True, exist_ok=True)
    run_checked(command, project_root, runner, "ARM fallback object compiler")
    if not output.is_file():
        raise CodeWarriorDriverError(
            f"ARM fallback object compiler did not create {output}"
        )
    return command


def quote_spec_path(path: Path) -> str:
    return json.dumps(path.as_posix())


def render_lsf(
    manifest: UnitManifest,
    objects: dict[tuple[str, str], tuple[Path, str]],
    bss_objects: dict[tuple[str, str], tuple[Path, str]],
    replacements: dict[tuple[str, str], Sequence[Path]] | None = None,
) -> str:
    replacements = replacements or {}
    lines = [
        "# Generated from config/linker/units.v1.json.",
        "# This is a real CodeWarrior link specification, not concatenation.",
        "",
    ]
    for target in manifest.targets:
        for region in target.regions:
            if region.linker_mode != "input":
                continue
            assert region.linker_block is not None
            assert region.linker_name is not None
            lines.append(f"{region.linker_block} {region.linker_name}")
            lines.append("{")
            if region.linker_address is not None:
                lines.append(f"    Address 0x{region.linker_address:08x}")
            else:
                lines.append(f"    After {region.linker_after}")
            replacement = replacements.get((region.image, region.region_id))
            if replacement is not None:
                for path in replacement:
                    lines.append(f"    Object {quote_spec_path(path)}")
            else:
                for unit in region.units:
                    path, section = objects[(unit.image, unit.unit_id)]
                    lines.append(
                        f"    Object {quote_spec_path(path)} ({section})"
                    )
                bss = bss_objects.get((region.image, region.region_id))
                if bss is not None:
                    path, section = bss
                    lines.append(
                        f"    Object {quote_spec_path(path)} ({section})"
                    )
            if region.linker_block == "Static":
                lines.append("    StackSize 0 0")
            lines.append("}")
            lines.append("")
    return "\n".join(lines)


def render_response(object_paths: Sequence[Path]) -> str:
    return "\n".join(quote_spec_path(path) for path in object_paths) + "\n"


def selected_artifact(
    record: dict[str, Any],
    build_dir: Path,
    source_dir: Path,
    sdk_dir: Path | None,
    unit: LinkUnit,
) -> tuple[Path, bytes]:
    provider = record["provider"]
    root = {
        "fallback": build_dir,
        "source": source_dir,
        "sdk": sdk_dir,
    }[provider]
    if root is None:
        raise CodeWarriorDriverError(
            f"SDK root is unavailable for {unit.image}/{unit.unit_id}"
        )
    path = safe_join(
        root,
        record["artifact_path"],
        f"{unit.image}/{unit.unit_id} selected linker artifact",
    )
    data = read_regular_file(
        path, f"{unit.image}/{unit.unit_id} selected linker artifact"
    )
    if len(data) != unit.size or sha256_bytes(data) != unit.target_sha256:
        raise CodeWarriorDriverError(
            f"only exact selected units may enter the real link: "
            f"{unit.image}/{unit.unit_id}"
        )
    return path, data


def load_arm9i_sdk_inputs(
    sdk_set: Path,
    arm9i_plan_path: Path,
    project_root: Path,
    build_dir: Path,
) -> dict[str, Any]:
    """Validate and order the private ARM9i objects selected by the public plan."""
    project = project_root.resolve()
    build = build_dir.resolve()
    requested = sdk_set if sdk_set.is_absolute() else project / sdk_set
    try:
        selected_root = requested.resolve(strict=True)
    except OSError:
        raise CodeWarriorDriverError("ARM9i SDK set cannot be resolved") from None
    if not selected_root.is_dir() or not selected_root.is_relative_to(build):
        raise CodeWarriorDriverError(
            "ARM9i SDK set must resolve beneath the active linker build directory"
        )
    if not selected_root.is_relative_to(project):
        raise CodeWarriorDriverError(
            "ARM9i SDK set must resolve beneath the active project"
        )

    plan_path = (
        arm9i_plan_path
        if arm9i_plan_path.is_absolute()
        else project / arm9i_plan_path
    )
    try:
        plan = parse_arm9i_plan(plan_path)
        extraction = validate_arm9i_extraction(selected_root, plan)
    except PreparationError as error:
        raise CodeWarriorDriverError(f"invalid ARM9i SDK set: {error}") from None

    entries = {
        (entry["archive"], entry["member"]): entry
        for entry in extraction["members"]
    }
    link_paths: list[Path] = []
    records: list[dict[str, Any]] = []
    for index, selection in enumerate(plan["link_order"]):
        archive = selection["archive"]
        member = selection["member"]
        entry = entries[(archive, member)]
        try:
            path = (selected_root / entry["path"]).resolve(strict=True)
        except OSError:
            raise CodeWarriorDriverError(
                f"ARM9i SDK member cannot be resolved: {archive}:{member}"
            ) from None
        if not path.is_relative_to(selected_root):
            raise CodeWarriorDriverError("invalid ARM9i SDK set member path")
        try:
            data = path.read_bytes()
            validate_arm_relocatable_elf(
                data, f"ARM9i SDK member {archive}:{member}"
            )
        except (CodeWarriorDriverError, OSError):
            raise CodeWarriorDriverError(
                f"ARM9i SDK member is not an ARM relocatable ELF: "
                f"{archive}:{member}"
            ) from None
        digest = sha256_bytes(data)
        if len(data) != entry["size"] or digest != entry["sha256"]:
            raise CodeWarriorDriverError(
                f"ARM9i SDK member integrity changed: {archive}:{member}"
            )
        link_paths.append(path.relative_to(project))
        records.append(
            {
                "archive": archive,
                "artifact_sha256": digest,
                "compile_command": None,
                "credited_bytes": 0,
                "image": "arm9i",
                "link_order": index,
                "member": member,
                "object": path.relative_to(build).as_posix(),
                "object_sha256": digest,
                "provider": "arm9i-sdk-extracted",
                "size": len(data),
            }
        )
    return {
        "link_paths": link_paths,
        "records": records,
        "selection_sha256": extraction["selection_sha256"],
    }


def arm9i_sdk_region(manifest: UnitManifest) -> tuple[str, str]:
    candidates = [
        region
        for target in manifest.targets
        if target.image == "arm9i"
        for region in target.regions
        if region.linker_mode == "input"
    ]
    if len(candidates) != 1:
        raise CodeWarriorDriverError(
            "extracted ARM9i SDK mode requires exactly one ARM9i input region"
        )
    region = candidates[0]
    if region.linker_block != "Ltdautoload" or region.linker_name != "LTDMAIN":
        raise CodeWarriorDriverError(
            "extracted ARM9i SDK mode requires Ltdautoload LTDMAIN"
        )
    return region.image, region.region_id


def prepare_production(
    manifest_path: Path,
    project_root: Path,
    build_dir: Path,
    source_dir: Path,
    sdk_dir: Path | None,
    object_compiler: str,
    *,
    arm9i_sdk_set: Path | None = None,
    arm9i_plan_path: Path = DEFAULT_ARM9I_PLAN,
    runner: Runner = default_runner,
    probe_path: Path | None = None,
) -> dict[str, Any]:
    manifest = load_manifest(manifest_path)
    validate_fallback_index(manifest, build_dir)
    selections, _ = load_probe(
        probe_path or build_dir / "probe.v1.json",
        manifest,
        build_dir,
        source_dir,
        sdk_dir,
    )
    driver_dir = build_dir / "codewarrior"
    assembly_dir = driver_dir / "assembly"
    object_dir = driver_dir / "objects"
    objects: dict[tuple[str, str], tuple[Path, str]] = {}
    bss_objects: dict[tuple[str, str], tuple[Path, str]] = {}
    object_paths: list[Path] = []
    records: list[dict[str, Any]] = []
    replacements: dict[tuple[str, str], Sequence[Path]] = {}
    sdk_inputs: dict[str, Any] | None = None
    replaced_region: tuple[str, str] | None = None
    if arm9i_sdk_set is not None:
        sdk_inputs = load_arm9i_sdk_inputs(
            arm9i_sdk_set,
            arm9i_plan_path,
            project_root,
            build_dir,
        )
        replaced_region = arm9i_sdk_region(manifest)
        replacements[replaced_region] = sdk_inputs["link_paths"]

    expected_input_units = {
        (unit.image, unit.unit_id)
        for target in manifest.targets
        for region in target.regions
        if region.linker_mode == "input"
        for unit in region.units
        if (unit.image, region.region_id) != replaced_region
    }
    for target in manifest.targets:
        for region in target.regions:
            if region.linker_mode != "input":
                continue
            if (region.image, region.region_id) == replaced_region:
                assert sdk_inputs is not None
                object_paths.extend(sdk_inputs["link_paths"])
                records.extend(
                    {**record, "region_id": region.region_id}
                    for record in sdk_inputs["records"]
                )
                continue
            for unit in region.units:
                record = selections[(unit.image, unit.unit_id)]
                artifact, data = selected_artifact(
                    record, build_dir, source_dir, sdk_dir, unit
                )
                section = unit_section(unit)
                source = (
                    assembly_dir / unit.image / unit.region_id /
                    f"{unit.chunk_id}.S"
                )
                output = (
                    object_dir / unit.image / unit.region_id /
                    f"{unit.chunk_id}.o"
                )
                source.parent.mkdir(parents=True, exist_ok=True)
                source.write_text(
                    data_assembly(section, artifact), encoding="utf-8"
                )
                reusable = False
                if output.is_file():
                    try:
                        validate_data_object(output, section, data)
                        reusable = True
                    except CodeWarriorDriverError:
                        reusable = False
                command: list[str] | None = None
                if not reusable:
                    command = compile_assembly_object(
                        source,
                        output,
                        object_compiler,
                        project_root,
                        runner,
                    )
                validate_data_object(output, section, data)
                objects[(unit.image, unit.unit_id)] = (output, section)
                object_paths.append(output)
                records.append(
                    {
                        "artifact_sha256": unit.target_sha256,
                        "compile_command": command,
                        "credited_bytes": 0,
                        "image": unit.image,
                        "object": str(output.relative_to(build_dir)),
                        "object_sha256": sha256_bytes(output.read_bytes()),
                        "provider": record["provider"],
                        "region_id": unit.region_id,
                        "section": section,
                        "size": unit.size,
                        "unit_id": unit.unit_id,
                    }
                )

            if region.bss_size != 0:
                section = bss_section(region)
                source = assembly_dir / region.image / region.region_id / "bss.S"
                output = object_dir / region.image / region.region_id / "bss.o"
                source.parent.mkdir(parents=True, exist_ok=True)
                source.write_text(
                    bss_assembly(section, region.bss_size), encoding="utf-8"
                )
                reusable = False
                if output.is_file():
                    try:
                        validate_bss_object(output, section, region.bss_size)
                        reusable = True
                    except CodeWarriorDriverError:
                        reusable = False
                command = None
                if not reusable:
                    command = compile_assembly_object(
                        source,
                        output,
                        object_compiler,
                        project_root,
                        runner,
                    )
                validate_bss_object(output, section, region.bss_size)
                bss_objects[(region.image, region.region_id)] = (output, section)
                object_paths.append(output)
                records.append(
                    {
                        "bss_size": region.bss_size,
                        "compile_command": command,
                        "credited_bytes": 0,
                        "image": region.image,
                        "object": str(output.relative_to(build_dir)),
                        "object_sha256": sha256_bytes(output.read_bytes()),
                        "provider": "generated-bss",
                        "region_id": region.region_id,
                        "section": section,
                    }
                )

    if set(objects) != expected_input_units:
        raise CodeWarriorDriverError("not every linker input unit produced an object")

    lsf_path = driver_dir / "production.lsf"
    response_path = driver_dir / "production.response"
    lsf_path.parent.mkdir(parents=True, exist_ok=True)
    lsf_path.write_text(
        render_lsf(manifest, objects, bss_objects, replacements), encoding="utf-8"
    )
    response_path.write_text(render_response(object_paths), encoding="utf-8")
    report = {
        "schema_version": 1,
        "kind": "arm9-codewarrior-link-preparation",
        "arm9i_input_mode": (
            "extracted-sdk" if sdk_inputs is not None else "fallback-wrappers"
        ),
        "arm9i_sdk_object_count": (
            len(sdk_inputs["records"]) if sdk_inputs is not None else 0
        ),
        "arm9i_sdk_selection_sha256": (
            sdk_inputs["selection_sha256"] if sdk_inputs is not None else None
        ),
        "credited_matching_bytes": 0,
        "fallback_credited_bytes": 0,
        "manifest_sha256": manifest.sha256,
        "lsf": str(lsf_path.relative_to(build_dir)),
        "lsf_sha256": sha256_bytes(lsf_path.read_bytes()),
        "object_count": len(object_paths),
        "objects": records,
        "response": str(response_path.relative_to(build_dir)),
        "response_sha256": sha256_bytes(response_path.read_bytes()),
    }
    write_json(driver_dir / "prepare.v1.json", report)
    return report


def load_toolchain_layout(path: Path) -> tuple[str, str]:
    value, _ = read_json_object(path, "link layout")
    toolchain = strict_object(
        value.get("toolchain"),
        "link layout toolchain",
        required={
            "compiler",
            "linker",
            "makelcf_format",
            "specification",
            "production_specification",
            "production_template",
        },
    )
    version = toolchain["makelcf_format"]
    template = toolchain["production_template"]
    if not isinstance(version, str) or not version.startswith("-V"):
        raise CodeWarriorDriverError("makelcf format flag is invalid")
    if not isinstance(template, str) or not template:
        raise CodeWarriorDriverError("production template path is invalid")
    return version, template


def validate_linked_elf(path: Path) -> None:
    read_elf_sections(path, 2)


def invoke_production_link(
    manifest_path: Path,
    layout_path: Path,
    project_root: Path,
    build_dir: Path,
    source_dir: Path,
    sdk_units_dir: Path | None,
    sdk_root: Path,
    object_compiler: str,
    makelcf: str,
    mwldarm: str,
    *,
    arm9i_sdk_set: Path | None = None,
    arm9i_plan_path: Path = DEFAULT_ARM9I_PLAN,
    runner: Runner = default_runner,
) -> dict[str, Any]:
    preparation = prepare_production(
        manifest_path,
        project_root,
        build_dir,
        source_dir,
        sdk_units_dir,
        object_compiler,
        arm9i_sdk_set=arm9i_sdk_set,
        arm9i_plan_path=arm9i_plan_path,
        runner=runner,
    )
    version_flag, template_relative = load_toolchain_layout(layout_path)
    template = safe_join(sdk_root, template_relative, "TwlSDK LCF template")
    if not template.is_file():
        raise CodeWarriorDriverError(f"missing private TwlSDK LCF template: {template}")
    driver_dir = build_dir / "codewarrior"
    lsf = driver_dir / "production.lsf"
    response = driver_dir / "production.response"
    lcf = driver_dir / "production.lcf"
    output = driver_dir / "production.elf"

    makelcf_command = [makelcf, version_flag, str(lsf), str(template)]
    result = run_checked(
        makelcf_command, project_root, runner, "makelcf"
    )
    if not result.stdout or not result.stdout.strip():
        raise CodeWarriorDriverError("makelcf produced an empty LCF")
    lcf.write_text(result.stdout, encoding="utf-8")
    if "MEMORY" not in result.stdout or "SECTIONS" not in result.stdout:
        raise CodeWarriorDriverError("makelcf output is not a production LCF")

    link_command = [
        mwldarm,
        "-proc",
        "arm946e",
        "-nothumb",
        "-nopic",
        "-nopid",
        "-interworking",
        "-stdlib",
        "-map",
        "closure",
        "-main",
        "_start",
        "-lcf",
        str(lcf),
        "-o",
        str(output),
        f"@{response}",
    ]
    run_checked(link_command, project_root, runner, "mwldarm")
    if not output.is_file():
        raise CodeWarriorDriverError(f"mwldarm did not create {output}")
    validate_linked_elf(output)
    report = {
        "schema_version": 1,
        "kind": "arm9-codewarrior-production-link",
        "driver": "real-codewarrior-link",
        "lsf_sha256": preparation["lsf_sha256"],
        "makelcf_command": makelcf_command,
        "mwldarm_command": link_command,
        "output": str(output.relative_to(build_dir)),
        "output_sha256": sha256_bytes(output.read_bytes()),
        "response_sha256": preparation["response_sha256"],
    }
    write_json(driver_dir / "link.v1.json", report)
    return report


def smoke_compile_link(
    project_root: Path,
    build_dir: Path,
    sdk_root: Path,
    mwccarm: str,
    makelcf: str,
    mwldarm: str,
    *,
    runner: Runner = default_runner,
    layout_path: Path = DEFAULT_LAYOUT,
) -> dict[str, Any]:
    version_flag, template_relative = load_toolchain_layout(layout_path)
    template = safe_join(sdk_root, template_relative, "TwlSDK LCF template")
    if not template.is_file():
        raise CodeWarriorDriverError(f"missing private TwlSDK LCF template: {template}")
    smoke_dir = build_dir / "codewarrior-smoke"
    smoke_dir.mkdir(parents=True, exist_ok=True)
    static_source = project_root / "tools/linker/fixtures/smoke_static.c"
    ltd_source = project_root / "tools/linker/fixtures/smoke_ltd.c"
    static_object = smoke_dir / "static.o"
    ltd_object = smoke_dir / "ltd.o"
    compile_commands: list[list[str]] = []
    for source, output in (
        (static_source, static_object),
        (ltd_source, ltd_object),
    ):
        command = [
            mwccarm,
            "-proc",
            "arm946e",
            "-nothumb",
            "-nopic",
            "-nopid",
            "-interworking",
            "-lang",
            "c99",
            "-c",
            str(source),
            "-o",
            str(output),
        ]
        run_checked(command, project_root, runner, "mwccarm smoke compile")
        if not output.is_file():
            raise CodeWarriorDriverError(f"mwccarm did not create {output}")
        read_elf_sections(output, 1)
        compile_commands.append(command)

    specification = (
        project_root / "tools/linker/fixtures/smoke.lsf"
    ).read_text(encoding="utf-8")
    specification = specification.replace(
        "$(STATIC_OBJECT)", quote_spec_path(static_object)
    ).replace("$(LTD_OBJECT)", quote_spec_path(ltd_object))
    lsf = smoke_dir / "smoke.lsf"
    response = smoke_dir / "smoke.response"
    lcf = smoke_dir / "smoke.lcf"
    output = smoke_dir / "smoke.elf"
    lsf.write_text(specification, encoding="utf-8")
    response.write_text(
        render_response((static_object, ltd_object)), encoding="utf-8"
    )

    makelcf_command = [makelcf, version_flag, str(lsf), str(template)]
    result = run_checked(
        makelcf_command, project_root, runner, "makelcf smoke"
    )
    if not result.stdout or "MEMORY" not in result.stdout or (
        "SECTIONS" not in result.stdout
    ):
        raise CodeWarriorDriverError("smoke makelcf output is invalid")
    lcf.write_text(result.stdout, encoding="utf-8")
    link_command = [
        mwldarm,
        "-proc",
        "arm946e",
        "-nothumb",
        "-nopic",
        "-nopid",
        "-interworking",
        "-stdlib",
        "-map",
        "closure",
        "-main",
        "_start",
        "-lcf",
        str(lcf),
        "-o",
        str(output),
        f"@{response}",
    ]
    run_checked(link_command, project_root, runner, "mwldarm smoke link")
    if not output.is_file():
        raise CodeWarriorDriverError(f"smoke mwldarm did not create {output}")
    validate_linked_elf(output)
    report = {
        "schema_version": 1,
        "kind": "arm9-codewarrior-smoke-link",
        "compile_commands": compile_commands,
        "makelcf_command": makelcf_command,
        "mwldarm_command": link_command,
        "output_sha256": sha256_bytes(output.read_bytes()),
    }
    write_json(smoke_dir / "smoke.v1.json", report)
    return report


def add_common(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--project-root", type=Path, default=PROJECT_ROOT)
    parser.add_argument("--build-dir", type=Path, default=DEFAULT_BUILD_DIR)
    parser.add_argument("--source-dir", type=Path)
    parser.add_argument("--sdk-units-dir", type=Path)
    parser.add_argument(
        "--arm9i-sdk-set",
        type=Path,
        help=(
            "opt in to the verified extracted ARM9i SDK set; without this "
            "option the existing fallback wrappers remain active"
        ),
    )
    parser.add_argument(
        "--arm9i-plan", type=Path, default=DEFAULT_ARM9I_PLAN,
    )


def provider_directories(args: argparse.Namespace) -> tuple[Path, Path | None]:
    source = args.source_dir or args.build_dir / "source"
    sdk_setting = args.sdk_units_dir or os.environ.get("FSAE_LINKER_SDK_UNITS")
    return source, Path(sdk_setting) if sdk_setting else None


def required_tool(value: str | None, environment: str, label: str) -> str:
    result = value or os.environ.get(environment)
    if not result:
        raise CodeWarriorDriverError(
            f"{label} is required through --{label} or {environment}"
        )
    return result


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    prepare = commands.add_parser("prepare", help="build real-link ARM objects")
    add_common(prepare)
    prepare.add_argument("--object-compiler")

    link = commands.add_parser("link", help="invoke makelcf and mwldarm")
    add_common(link)
    link.add_argument("--layout", type=Path, default=DEFAULT_LAYOUT)
    link.add_argument("--sdk-root", type=Path)
    link.add_argument("--object-compiler")
    link.add_argument("--makelcf")
    link.add_argument("--mwldarm")

    smoke = commands.add_parser("smoke", help="compile and link public fixtures")
    smoke.add_argument("--project-root", type=Path, default=PROJECT_ROOT)
    smoke.add_argument("--build-dir", type=Path, default=DEFAULT_BUILD_DIR)
    smoke.add_argument("--layout", type=Path, default=DEFAULT_LAYOUT)
    smoke.add_argument("--sdk-root", type=Path)
    smoke.add_argument("--mwccarm")
    smoke.add_argument("--makelcf")
    smoke.add_argument("--mwldarm")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        if args.command == "prepare":
            source, sdk_units = provider_directories(args)
            compiler = args.object_compiler or os.environ.get(
                "FSAE_ARM_OBJECT_COMPILER"
            ) or shutil.which("clang")
            if compiler is None:
                raise CodeWarriorDriverError("an ARM-capable clang is required")
            report = prepare_production(
                args.manifest,
                args.project_root,
                args.build_dir,
                source,
                sdk_units,
                compiler,
                arm9i_sdk_set=args.arm9i_sdk_set,
                arm9i_plan_path=args.arm9i_plan,
            )
        elif args.command == "link":
            source, sdk_units = provider_directories(args)
            compiler = args.object_compiler or os.environ.get(
                "FSAE_ARM_OBJECT_COMPILER"
            ) or shutil.which("clang")
            if compiler is None:
                raise CodeWarriorDriverError("an ARM-capable clang is required")
            sdk_root = args.sdk_root or (
                Path(os.environ["TWLSDK_ROOT"])
                if os.environ.get("TWLSDK_ROOT") else None
            )
            if sdk_root is None:
                raise CodeWarriorDriverError("--sdk-root or TWLSDK_ROOT is required")
            report = invoke_production_link(
                args.manifest,
                args.layout,
                args.project_root,
                args.build_dir,
                source,
                sdk_units,
                sdk_root,
                compiler,
                required_tool(args.makelcf, "MAKELCF", "makelcf"),
                required_tool(args.mwldarm, "MWLDARM", "mwldarm"),
                arm9i_sdk_set=args.arm9i_sdk_set,
                arm9i_plan_path=args.arm9i_plan,
            )
        else:
            sdk_root = args.sdk_root or (
                Path(os.environ["TWLSDK_ROOT"])
                if os.environ.get("TWLSDK_ROOT") else None
            )
            if sdk_root is None:
                raise CodeWarriorDriverError("--sdk-root or TWLSDK_ROOT is required")
            report = smoke_compile_link(
                args.project_root,
                args.build_dir,
                sdk_root,
                required_tool(args.mwccarm, "MWCCARM", "mwccarm"),
                required_tool(args.makelcf, "MAKELCF", "makelcf"),
                required_tool(args.mwldarm, "MWLDARM", "mwldarm"),
                layout_path=args.layout,
            )
    except CodeWarriorLicenseError as exc:
        print(f"CodeWarrior license error: {exc}", file=sys.stderr)
        return 3
    except (CodeWarriorDriverError, IncrementalLinkError, OSError) as exc:
        print(f"CodeWarrior driver error: {exc}", file=sys.stderr)
        return 2
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
