#!/usr/bin/env python3
"""Select ARM9i SDK archive members without redistributing the SDK."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import shutil
import sys
import tempfile
from collections import OrderedDict
from pathlib import Path, PurePosixPath
from typing import Any, Iterable


SCHEMA_VERSION = 1
PLAN_KIND = "arm9i-build-plan"
INVENTORY_KIND = "arm9i-sdk-member-inventory"
EXTRACTION_KIND = "arm9i-sdk-member-extraction"
SHA256_RE = re.compile(r"[0-9a-f]{64}\Z")


class PreparationError(Exception):
    """Expected, user-facing preparation failure."""


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")
    return sha256(payload)


def default_project_root() -> Path:
    here = Path(__file__).resolve()
    for candidate in (here.parent, *here.parents):
        if (candidate / "config/build/arm9i.json").is_file():
            return candidate
    return Path.cwd().resolve()


def require_component(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value:
        raise PreparationError(f"{label} must be a non-empty string")
    if value in (".", "..") or "/" in value or "\\" in value or "\0" in value:
        raise PreparationError(f"{label} is not a safe file name: {value!r}")
    return value


def require_relative_path(value: Any, label: str) -> PurePosixPath:
    if not isinstance(value, str) or not value:
        raise PreparationError(f"{label} must be a non-empty relative path")
    if "\\" in value or "\0" in value:
        raise PreparationError(f"{label} is not a safe POSIX path: {value!r}")
    path = PurePosixPath(value)
    if path.is_absolute() or any(part in ("", ".", "..") for part in path.parts):
        raise PreparationError(f"{label} is not a safe relative path: {value!r}")
    return path


def read_json(path: Path, label: str) -> tuple[dict[str, Any], bytes]:
    try:
        raw = path.read_bytes()
    except OSError as error:
        raise PreparationError(f"cannot read {label}: {error.strerror or error}") from None
    try:
        value = json.loads(raw)
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise PreparationError(f"invalid JSON in {label}: {error}") from None
    if not isinstance(value, dict):
        raise PreparationError(f"{label} must contain a JSON object")
    return value, raw


def parse_plan(path: Path) -> dict[str, Any]:
    plan, raw = read_json(path, "ARM9i build plan")
    if plan.get("schema_version") != SCHEMA_VERSION or plan.get("kind") != PLAN_KIND:
        raise PreparationError(
            f"ARM9i build plan must have schema_version {SCHEMA_VERSION} "
            f"and kind {PLAN_KIND!r}"
        )

    raw_archives = plan.get("archives")
    if not isinstance(raw_archives, list):
        raise PreparationError("ARM9i build plan archives must be a list")
    archives: OrderedDict[str, dict[str, str]] = OrderedDict()
    paths: set[str] = set()
    for index, entry in enumerate(raw_archives):
        label = f"archives[{index}]"
        if not isinstance(entry, dict):
            raise PreparationError(f"{label} must be an object")
        name = require_component(entry.get("name"), f"{label}.name")
        relative = require_relative_path(entry.get("path"), f"{label}.path").as_posix()
        digest = entry.get("sha256")
        if not isinstance(digest, str) or SHA256_RE.fullmatch(digest) is None:
            raise PreparationError(f"{label}.sha256 must be 64 lowercase hex digits")
        if name in archives:
            raise PreparationError(f"duplicate archive name in build plan: {name}")
        if relative in paths:
            raise PreparationError(f"duplicate archive path in build plan: {relative}")
        archives[name] = {"name": name, "path": relative, "sha256": digest}
        paths.add(relative)

    raw_ranges = plan.get("object_ranges")
    if not isinstance(raw_ranges, list):
        raise PreparationError("ARM9i build plan object_ranges must be a list")
    selections: list[dict[str, str]] = []
    seen: set[tuple[str, str]] = set()
    for index, entry in enumerate(raw_ranges):
        if not isinstance(entry, dict):
            raise PreparationError(f"object_ranges[{index}] must be an object")
        if entry.get("kind") != "sdk_object":
            continue
        archive = require_component(
            entry.get("archive"), f"object_ranges[{index}].archive"
        )
        member = require_component(
            entry.get("object"), f"object_ranges[{index}].object"
        )
        if archive not in archives:
            raise PreparationError(
                f"object_ranges[{index}] references unknown archive {archive!r}"
            )
        pair = (archive, member)
        if pair not in seen:
            selections.append({"archive": archive, "member": member})
            seen.add(pair)
    if not selections:
        raise PreparationError("ARM9i build plan selects no sdk_object members")

    required: OrderedDict[str, list[str]] = OrderedDict()
    selected_archives = {entry["archive"] for entry in selections}
    for archive in archives:
        if archive in selected_archives:
            required[archive] = []
    for selection in selections:
        required[selection["archive"]].append(selection["member"])
    return {
        "plan_sha256": sha256(raw),
        "archives": archives,
        "required": required,
        "link_order": selections,
    }


def _decode_ascii(value: bytes, label: str) -> str:
    try:
        return value.decode("ascii")
    except UnicodeDecodeError:
        raise PreparationError(f"invalid non-ASCII {label} in ar archive") from None


def _gnu_long_name(table: bytes, offset: int) -> str:
    if offset < 0 or offset >= len(table):
        raise PreparationError(f"GNU ar long-name offset is out of range: {offset}")
    slash_newline = table.find(b"/\n", offset)
    nul = table.find(b"\0", offset)
    ends = [position for position in (slash_newline, nul) if position >= 0]
    end = min(ends) if ends else len(table)
    if end <= offset:
        raise PreparationError(f"empty GNU ar long name at offset {offset}")
    try:
        return table[offset:end].decode("utf-8")
    except UnicodeDecodeError:
        raise PreparationError(f"invalid UTF-8 GNU ar long name at offset {offset}") from None


def ar_members(data: bytes) -> Iterable[tuple[str, bytes]]:
    """Yield ar members, including GNU and BSD extended names, strictly."""
    if not data.startswith(b"!<arch>\n"):
        raise PreparationError("configured SDK file is not an ar archive")
    position = 8
    raw_members: list[tuple[str, bytes]] = []
    long_names = b""
    while position < len(data):
        if position + 60 > len(data):
            raise PreparationError(f"truncated ar member header at offset {position}")
        header = data[position : position + 60]
        if header[58:60] != b"`\n":
            raise PreparationError(f"invalid ar member header at offset {position}")
        raw_name = _decode_ascii(header[:16], "member name").strip()
        size_text = _decode_ascii(header[48:58], "member size").strip()
        try:
            size = int(size_text, 10)
        except ValueError:
            raise PreparationError(f"invalid ar member size at offset {position}") from None
        if size < 0 or position + 60 + size > len(data):
            raise PreparationError(f"truncated ar member payload at offset {position}")
        payload = data[position + 60 : position + 60 + size]
        if raw_name == "//":
            long_names = payload
        else:
            raw_members.append((raw_name, payload))
        position += 60 + size
        if position & 1:
            if position >= len(data):
                raise PreparationError("missing ar alignment byte")
            position += 1

    for raw_name, payload in raw_members:
        if raw_name in ("/", "/SYM64/"):
            continue
        if raw_name.startswith("#1/"):
            try:
                name_size = int(raw_name[3:], 10)
            except ValueError:
                raise PreparationError(f"invalid BSD ar member name size: {raw_name!r}") from None
            if name_size <= 0 or name_size > len(payload):
                raise PreparationError(f"invalid BSD ar member name size: {name_size}")
            try:
                name = payload[:name_size].rstrip(b"\0").decode("utf-8")
            except UnicodeDecodeError:
                raise PreparationError("invalid UTF-8 BSD ar member name") from None
            payload = payload[name_size:]
        elif raw_name.startswith("/") and raw_name[1:].isdigit():
            name = _gnu_long_name(long_names, int(raw_name[1:], 10))
        else:
            name = raw_name[:-1] if raw_name.endswith("/") else raw_name
        require_component(name, "ar member name")
        yield name, payload


def output_root(project_root: Path, requested: Path) -> Path:
    project = project_root.resolve()
    build = (project / "build").resolve()
    if not build.is_relative_to(project):
        raise PreparationError("project build directory resolves outside the project")
    candidate = requested if requested.is_absolute() else project / requested
    candidate = candidate.resolve()
    if not candidate.is_relative_to(build):
        raise PreparationError("output directory must resolve beneath project build/")
    return candidate


def resolve_archive(sdk_root: Path, configured_path: str, archive_name: str) -> Path:
    try:
        candidate = (sdk_root / PurePosixPath(configured_path)).resolve(strict=True)
    except OSError as error:
        raise PreparationError(
            f"archive {archive_name!r} cannot be resolved at configured path "
            f"{configured_path!r}: {error.strerror or 'not found'}"
        ) from None
    if not candidate.is_relative_to(sdk_root) or not candidate.is_file():
        raise PreparationError(
            f"archive {archive_name!r} configured path escapes the SDK root or is not a file"
        )
    return candidate


def build_inventory(plan: dict[str, Any], sdk_root_arg: Path) -> tuple[dict[str, Any], dict[tuple[str, str], bytes]]:
    try:
        sdk_root = sdk_root_arg.resolve(strict=True)
    except OSError as error:
        raise PreparationError(f"SDK root cannot be resolved: {error.strerror or 'not found'}") from None
    if not sdk_root.is_dir():
        raise PreparationError("SDK root is not a directory")

    selected_payloads: dict[tuple[str, str], bytes] = {}
    report_archives: list[dict[str, Any]] = []
    for archive_index, (archive_name, required_members) in enumerate(plan["required"].items()):
        configured = plan["archives"][archive_name]
        archive_path = resolve_archive(sdk_root, configured["path"], archive_name)
        try:
            archive_data = archive_path.read_bytes()
        except OSError as error:
            raise PreparationError(
                f"archive {archive_name!r} cannot be read: {error.strerror or error}"
            ) from None
        actual_archive_sha = sha256(archive_data)
        if actual_archive_sha != configured["sha256"]:
            raise PreparationError(
                f"archive {archive_name!r} SHA-256 mismatch: expected "
                f"{configured['sha256']}, got {actual_archive_sha}"
            )

        required_set = set(required_members)
        found: dict[str, list[bytes]] = {name: [] for name in required_members}
        try:
            for member_name, payload in ar_members(archive_data):
                if member_name in required_set:
                    found[member_name].append(payload)
        except PreparationError as error:
            raise PreparationError(f"archive {archive_name!r}: {error}") from None

        report_members: list[dict[str, Any]] = []
        for member_name in required_members:
            matches = found[member_name]
            if not matches:
                raise PreparationError(
                    f"archive {archive_name!r} is missing required member {member_name!r}"
                )
            if len(matches) != 1:
                raise PreparationError(
                    f"archive {archive_name!r} contains required member "
                    f"{member_name!r} more than once"
                )
            payload = matches[0]
            selected_payloads[(archive_name, member_name)] = payload
            report_members.append(
                {"name": member_name, "size": len(payload), "sha256": sha256(payload)}
            )
        report_archives.append(
            {
                "index": archive_index,
                "name": archive_name,
                "configured_path": configured["path"],
                "sha256": configured["sha256"],
                "members": report_members,
            }
        )

    selection_basis = {
        "plan_sha256": plan["plan_sha256"],
        "archives": report_archives,
        "link_order": plan["link_order"],
    }
    inventory = {
        "schema_version": SCHEMA_VERSION,
        "kind": INVENTORY_KIND,
        "plan_sha256": plan["plan_sha256"],
        "selection_sha256": canonical_sha256(selection_basis),
        "archive_count": len(report_archives),
        "selected_member_count": len(plan["link_order"]),
        "archives": report_archives,
        "link_order": plan["link_order"],
    }
    return inventory, selected_payloads


def atomic_json(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = (json.dumps(value, indent=2, ensure_ascii=False) + "\n").encode("utf-8")
    descriptor, temporary_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    temporary = Path(temporary_name)
    try:
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)


def _archive_directory(index: int, archive_name: str) -> str:
    return f"{index:02d}-{archive_name}"


def extraction_entries(inventory: dict[str, Any]) -> list[dict[str, Any]]:
    prefix = f"sets/{inventory['selection_sha256']}"
    entries: list[dict[str, Any]] = []
    for archive in inventory["archives"]:
        directory = _archive_directory(archive["index"], archive["name"])
        for member in archive["members"]:
            entries.append(
                {
                    "archive": archive["name"],
                    "member": member["name"],
                    "size": member["size"],
                    "sha256": member["sha256"],
                    "path": f"{prefix}/{directory}/{member['name']}",
                }
            )
    return entries


def validate_set(root: Path, entries: list[dict[str, Any]]) -> None:
    expected_paths: set[Path] = set()
    set_roots: set[Path] = set()
    for entry in entries:
        lexical_path = root / PurePosixPath(entry["path"])
        cursor = lexical_path
        while cursor != root:
            if cursor.is_symlink():
                raise PreparationError(
                    f"extracted member path contains a symbolic link: {entry['path']}"
                )
            cursor = cursor.parent
        try:
            path = lexical_path.resolve(strict=True)
        except OSError:
            raise PreparationError(
                f"extracted member is missing or not a regular file: {entry['path']}"
            ) from None
        if not path.is_relative_to(root):
            raise PreparationError("extraction report contains a path outside its output root")
        expected_paths.add(path)
        set_roots.add(path.parents[1])
        if not path.is_file():
            raise PreparationError(f"extracted member is missing or not a regular file: {entry['path']}")
        try:
            payload = path.read_bytes()
        except OSError as error:
            raise PreparationError(
                f"cannot read extracted member {entry['path']}: {error.strerror or error}"
            ) from None
        if len(payload) != entry["size"] or sha256(payload) != entry["sha256"]:
            raise PreparationError(f"extracted member integrity mismatch: {entry['path']}")
    if len(set_roots) != 1:
        raise PreparationError("extraction entries do not share one selected set")
    set_root = next(iter(set_roots))
    actual_paths = {
        path.resolve()
        for path in set_root.rglob("*")
        if path.is_file() or path.is_symlink()
    }
    if actual_paths != expected_paths:
        raise PreparationError("selected extraction set contains unexpected or missing files")


def extract_members(
    root: Path,
    inventory: dict[str, Any],
    selected_payloads: dict[tuple[str, str], bytes],
) -> dict[str, Any]:
    entries = extraction_entries(inventory)
    final_set = root / "sets" / inventory["selection_sha256"]
    if final_set.exists():
        validate_set(root, entries)
    else:
        final_set.parent.mkdir(parents=True, exist_ok=True)
        staging = Path(tempfile.mkdtemp(prefix=".arm9i-sdk-", dir=root))
        try:
            for archive in inventory["archives"]:
                directory = staging / _archive_directory(archive["index"], archive["name"])
                directory.mkdir(parents=True)
                for member in archive["members"]:
                    payload = selected_payloads[(archive["name"], member["name"])]
                    (directory / member["name"]).write_bytes(payload)
            try:
                staging.rename(final_set)
            except FileExistsError:
                pass
            if staging.exists():
                shutil.rmtree(staging)
        finally:
            if staging.exists():
                shutil.rmtree(staging)
        validate_set(root, entries)

    return {
        "schema_version": SCHEMA_VERSION,
        "kind": EXTRACTION_KIND,
        "plan_sha256": inventory["plan_sha256"],
        "selection_sha256": inventory["selection_sha256"],
        "archive_count": inventory["archive_count"],
        "selected_member_count": inventory["selected_member_count"],
        "members": entries,
        "link_order": inventory["link_order"],
    }


def validate_inventory(root: Path, plan: dict[str, Any]) -> dict[str, Any]:
    report, _ = read_json(root / "inventory.v1.json", "ARM9i SDK inventory report")
    if report.get("schema_version") != SCHEMA_VERSION or report.get("kind") != INVENTORY_KIND:
        raise PreparationError("invalid ARM9i SDK inventory report identity")
    if report.get("plan_sha256") != plan["plan_sha256"]:
        raise PreparationError("inventory report was produced from a different ARM9i build plan")
    if report.get("archive_count") != len(plan["required"]):
        raise PreparationError("inventory report has the wrong archive count")
    if report.get("selected_member_count") != len(plan["link_order"]):
        raise PreparationError("inventory report has the wrong selected member count")
    if report.get("link_order") != plan["link_order"]:
        raise PreparationError("inventory report link order differs from the ARM9i build plan")

    archives = report.get("archives")
    if not isinstance(archives, list) or len(archives) != len(plan["required"]):
        raise PreparationError("inventory report has the wrong archives")
    for index, ((archive_name, required_members), archive) in enumerate(
        zip(plan["required"].items(), archives, strict=True)
    ):
        if not isinstance(archive, dict):
            raise PreparationError(f"inventory archives[{index}] must be an object")
        configured = plan["archives"][archive_name]
        if (
            archive.get("index") != index
            or archive.get("name") != archive_name
            or archive.get("configured_path") != configured["path"]
            or archive.get("sha256") != configured["sha256"]
        ):
            raise PreparationError(f"inventory archives[{index}] differs from the build plan")
        members = archive.get("members")
        if not isinstance(members, list) or len(members) != len(required_members):
            raise PreparationError(f"inventory archives[{index}] has the wrong members")
        for member_index, (required_name, member) in enumerate(
            zip(required_members, members, strict=True)
        ):
            if not isinstance(member, dict) or member.get("name") != required_name:
                raise PreparationError(
                    f"inventory archives[{index}].members[{member_index}] differs from the plan"
                )
            size = member.get("size")
            digest = member.get("sha256")
            if not isinstance(size, int) or isinstance(size, bool) or size < 0:
                raise PreparationError(
                    f"inventory archives[{index}].members[{member_index}].size is invalid"
                )
            if not isinstance(digest, str) or SHA256_RE.fullmatch(digest) is None:
                raise PreparationError(
                    f"inventory archives[{index}].members[{member_index}].sha256 is invalid"
                )

    selection_basis = {
        "plan_sha256": plan["plan_sha256"],
        "archives": archives,
        "link_order": plan["link_order"],
    }
    expected_selection = canonical_sha256(selection_basis)
    if report.get("selection_sha256") != expected_selection:
        raise PreparationError("inventory report selection SHA-256 is invalid")
    return report


def validate_extraction(root: Path, plan: dict[str, Any]) -> dict[str, Any]:
    inventory = validate_inventory(root, plan)
    report, _ = read_json(root / "extraction.v1.json", "ARM9i SDK extraction report")
    if report.get("schema_version") != SCHEMA_VERSION or report.get("kind") != EXTRACTION_KIND:
        raise PreparationError("invalid ARM9i SDK extraction report identity")
    if report.get("plan_sha256") != plan["plan_sha256"]:
        raise PreparationError("extraction report was produced from a different ARM9i build plan")
    entries = report.get("members")
    if not isinstance(entries, list) or len(entries) != len(plan["link_order"]):
        raise PreparationError("extraction report has the wrong selected member count")
    selection = report.get("selection_sha256")
    if not isinstance(selection, str) or SHA256_RE.fullmatch(selection) is None:
        raise PreparationError("extraction report has an invalid selection SHA-256")
    if selection != inventory["selection_sha256"]:
        raise PreparationError("extraction report selection differs from the inventory")
    if report.get("archive_count") != len(plan["required"]):
        raise PreparationError("extraction report has the wrong archive count")
    if report.get("selected_member_count") != len(plan["link_order"]):
        raise PreparationError("extraction report has the wrong selected member count")

    archive_indices = {
        archive_name: index
        for index, archive_name in enumerate(plan["required"])
    }
    checked_entries: list[dict[str, Any]] = []
    inventory_members = {
        (archive["name"], member["name"]): member
        for archive in inventory["archives"]
        for member in archive["members"]
    }
    for index, item in enumerate(entries):
        if not isinstance(item, dict):
            raise PreparationError(f"extraction report members[{index}] must be an object")
        archive = require_component(item.get("archive"), f"members[{index}].archive")
        member = require_component(item.get("member"), f"members[{index}].member")
        size = item.get("size")
        digest = item.get("sha256")
        relative = item.get("path")
        if not isinstance(size, int) or isinstance(size, bool) or size < 0:
            raise PreparationError(f"members[{index}].size must be a non-negative integer")
        if not isinstance(digest, str) or SHA256_RE.fullmatch(digest) is None:
            raise PreparationError(f"members[{index}].sha256 must be 64 lowercase hex digits")
        if archive not in archive_indices:
            raise PreparationError(f"members[{index}] references an unknown archive")
        expected_member = inventory_members.get((archive, member))
        if expected_member is None or (
            size != expected_member["size"] or digest != expected_member["sha256"]
        ):
            raise PreparationError(f"members[{index}] metadata differs from the inventory")
        expected_path = (
            f"sets/{selection}/"
            f"{_archive_directory(archive_indices[archive], archive)}/{member}"
        )
        if relative != expected_path:
            raise PreparationError(f"members[{index}].path is not the expected generated path")
        checked_entries.append(item)
    expected_order = [(item["archive"], item["member"]) for item in plan["link_order"]]
    report_pairs = [(item["archive"], item["member"]) for item in checked_entries]
    expected_pairs = set(expected_order)
    if set(report_pairs) != expected_pairs or len(report_pairs) != len(set(report_pairs)):
        raise PreparationError("extraction report does not describe the exact selected members")
    if report.get("link_order") != plan["link_order"]:
        raise PreparationError("extraction report link order differs from the ARM9i build plan")
    validate_set(root, checked_entries)
    return report


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(
        description="Inventory or extract only the SDK archive members selected by an ARM9i build plan."
    )
    result.add_argument("command", choices=("inventory", "extract", "validate"))
    result.add_argument(
        "--sdk-root",
        "--twlsdk-root",
        dest="sdk_root",
        type=Path,
        help="external TwlSDK root (or set TWLSDK_ROOT); never recorded in reports",
    )
    result.add_argument("--config", type=Path, help="ARM9i build plan JSON")
    result.add_argument(
        "--output", type=Path, default=Path("build/linker/arm9i-sdk"),
        help="generated output beneath project build/",
    )
    result.add_argument(
        "--project-root", type=Path, help=argparse.SUPPRESS,
    )
    result.add_argument("--quiet", action="store_true")
    return result


def run(argv: list[str] | None = None) -> int:
    args = parser().parse_args(argv)
    project = args.project_root.resolve() if args.project_root else default_project_root()
    config = args.config or project / "config/build/arm9i.json"
    if not config.is_absolute():
        config = project / config
    plan = parse_plan(config)
    root = output_root(project, args.output)
    root.mkdir(parents=True, exist_ok=True)

    if args.command == "validate":
        report = validate_extraction(root, plan)
    else:
        sdk_root = args.sdk_root
        if sdk_root is None and os.environ.get("TWLSDK_ROOT"):
            sdk_root = Path(os.environ["TWLSDK_ROOT"])
        if sdk_root is None:
            raise PreparationError("--sdk-root or TWLSDK_ROOT is required")
        inventory, selected_payloads = build_inventory(plan, sdk_root)
        atomic_json(root / "inventory.v1.json", inventory)
        if args.command == "inventory":
            report = inventory
        else:
            report = extract_members(root, inventory, selected_payloads)
            atomic_json(root / "extraction.v1.json", report)
    if not args.quiet:
        print(json.dumps(report, indent=2, ensure_ascii=False))
    return 0


def main() -> int:
    try:
        return run()
    except PreparationError as error:
        print(f"error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
