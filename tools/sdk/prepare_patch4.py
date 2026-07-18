#!/usr/bin/env python3
"""Build a validated TwlSDK 5.5 patch4 reconstruction outside the repository."""

from __future__ import annotations

import argparse
import fnmatch
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import shutil
import stat
import tempfile
from typing import Any, Iterable, Sequence
import zipfile


class SdkPreparationError(RuntimeError):
    """A validation or safe-output error."""


def repository_root() -> Path:
    return Path(__file__).resolve().parents[2]


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def load_config(path: Path) -> dict[str, Any]:
    try:
        raw = path.read_bytes()
        config = json.loads(raw)
    except (OSError, json.JSONDecodeError) as error:
        raise SdkPreparationError(f"cannot read config {path}: {error}") from error
    if config.get("schema_version") != 1:
        raise SdkPreparationError("unsupported SDK preparation config schema")
    required = {
        "target",
        "archives",
        "overlay",
        "version_headers",
        "binary_release_marker",
        "expected_tree",
        "evidence",
    }
    missing = sorted(required - config.keys())
    if missing:
        raise SdkPreparationError(f"config is missing fields: {', '.join(missing)}")
    config["_sha256"] = sha256_bytes(raw)
    return config


def _validate_sha256(value: str, label: str) -> None:
    if len(value) != 64 or any(character not in "0123456789abcdef" for character in value):
        raise SdkPreparationError(f"{label} is not a lowercase SHA-256")


def _safe_relative_member(name: str, prefix: str) -> str | None:
    if "\\" in name or name.startswith("/"):
        raise SdkPreparationError(f"unsafe zip member name: {name!r}")
    path = PurePosixPath(name)
    if any(part in {"", ".", ".."} for part in path.parts):
        raise SdkPreparationError(f"unsafe zip member name: {name!r}")
    if not name.startswith(prefix):
        return None
    relative = name[len(prefix) :]
    if not relative:
        return None
    relative_path = PurePosixPath(relative)
    if any(part in {"", ".", ".."} for part in relative_path.parts):
        raise SdkPreparationError(f"unsafe relative zip member: {relative!r}")
    return relative_path.as_posix()


def inspect_archive(path: Path, specification: dict[str, Any], label: str) -> dict[str, Any]:
    if not path.is_file():
        raise SdkPreparationError(f"{label} archive not found: {path}")
    expected_hash = specification.get("sha256", "")
    _validate_sha256(expected_hash, f"{label} archive hash")
    actual_hash = sha256_file(path)
    if actual_hash != expected_hash:
        raise SdkPreparationError(
            f"{label} archive SHA-256 mismatch: got {actual_hash}, expected {expected_hash}"
        )
    prefix = specification.get("root_prefix")
    if not isinstance(prefix, str) or not prefix.endswith("/TwlSDK/"):
        raise SdkPreparationError(f"invalid {label} root prefix")

    members: dict[str, zipfile.ZipInfo] = {}
    outside_files: list[str] = []
    total_size = 0
    try:
        with zipfile.ZipFile(path) as archive:
            for info in archive.infolist():
                relative = _safe_relative_member(info.filename, prefix)
                if info.is_dir():
                    continue
                if relative is None:
                    outside_files.append(info.filename)
                    continue
                if info.flag_bits & 1:
                    raise SdkPreparationError(f"encrypted zip member is not supported: {info.filename}")
                unix_type = (info.external_attr >> 16) & 0o170000
                if unix_type not in {0, stat.S_IFREG}:
                    raise SdkPreparationError(f"non-regular zip member is not supported: {info.filename}")
                if relative in members:
                    raise SdkPreparationError(f"duplicate SDK member: {relative}")
                members[relative] = info
                total_size += info.file_size
    except (OSError, zipfile.BadZipFile) as error:
        raise SdkPreparationError(f"cannot inspect {label} archive: {error}") from error
    if outside_files:
        raise SdkPreparationError(
            f"{label} archive contains files outside its expected TwlSDK root: {outside_files[0]}"
        )
    expected_files = specification.get("file_count")
    expected_size = specification.get("uncompressed_size")
    if len(members) != expected_files or total_size != expected_size:
        raise SdkPreparationError(
            f"{label} structure mismatch: got {len(members)} files/{total_size} bytes, "
            f"expected {expected_files}/{expected_size}"
        )
    return {
        "path": path,
        "sha256": actual_hash,
        "prefix": prefix,
        "members": members,
        "file_count": len(members),
        "uncompressed_size": total_size,
    }


def _matches(pattern: str, relative: str) -> bool:
    # fnmatch is intentional: config patterns are always anchored at the SDK
    # root and may span the small, fixed number of directory components.
    return fnmatch.fnmatchcase(relative, pattern)


def classify_overlay(
    patch_members: Iterable[str], overlay: dict[str, Any]
) -> tuple[list[str], list[dict[str, Any]]]:
    rules = overlay.get("skip_patch5_only")
    if not isinstance(rules, list):
        raise SdkPreparationError("overlay skip rules must be a list")
    matches_by_rule: list[list[str]] = [[] for _ in rules]
    applied: list[str] = []
    skipped: list[dict[str, Any]] = []
    for relative in sorted(patch_members):
        matched = [index for index, rule in enumerate(rules) if _matches(rule["pattern"], relative)]
        if len(matched) > 1:
            raise SdkPreparationError(f"overlapping patch5-only rules for {relative}")
        if not matched:
            applied.append(relative)
            continue
        index = matched[0]
        matches_by_rule[index].append(relative)
        skipped.append({"path": relative, "reason": rules[index]["reason"]})
    for rule, matched in zip(rules, matches_by_rule):
        expected = rule.get("expected_matches")
        if len(matched) != expected:
            raise SdkPreparationError(
                f"skip rule {rule.get('pattern')!r} matched {len(matched)} files, expected {expected}"
            )
    if len(applied) != overlay.get("expected_applied_files"):
        raise SdkPreparationError("unexpected number of cumulative overlay files")
    if len(skipped) != overlay.get("expected_skipped_files"):
        raise SdkPreparationError("unexpected number of skipped patch5-only files")
    return applied, skipped


def diagnose(
    base_archive: Path,
    patch5_archive: Path,
    config: dict[str, Any],
) -> dict[str, Any]:
    base = inspect_archive(base_archive, config["archives"]["base"], "base")
    patch5 = inspect_archive(patch5_archive, config["archives"]["patch5"], "patch5")
    applied, skipped = classify_overlay(patch5["members"], config["overlay"])
    missing_base = sorted(
        relative
        for relative in applied
        if relative not in base["members"]
        and relative != "build/buildtools/commondefs.parens"
    )
    if missing_base:
        raise SdkPreparationError(f"unexpected new cumulative patch member: {missing_base[0]}")
    expected_tree = config["expected_tree"]
    if not isinstance(expected_tree.get("file_count"), int):
        raise SdkPreparationError("expected tree file count is missing")
    return {
        "status": "ready",
        "config_sha256": config["_sha256"],
        "target": config["target"],
        "inputs": {
            "base": {
                "sha256": base["sha256"],
                "file_count": base["file_count"],
                "uncompressed_size": base["uncompressed_size"],
            },
            "patch5": {
                "sha256": patch5["sha256"],
                "file_count": patch5["file_count"],
                "uncompressed_size": patch5["uncompressed_size"],
            },
        },
        "overlay": {"applied_files": len(applied), "skipped_files": len(skipped)},
        "_base": base,
        "_patch5": patch5,
        "_applied": applied,
        "_skipped": skipped,
    }


def _write_member(
    archive: zipfile.ZipFile,
    info: zipfile.ZipInfo,
    destination: Path,
) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    data = archive.read(info)
    destination.write_bytes(data)
    mode = (info.external_attr >> 16) & 0o777
    if mode:
        destination.chmod(mode)


def extract_base(base: dict[str, Any], sdk_root: Path) -> None:
    with zipfile.ZipFile(base["path"]) as archive:
        for relative, info in sorted(base["members"].items()):
            _write_member(archive, info, sdk_root.joinpath(*relative.split("/")))


def apply_overlay(patch5: dict[str, Any], applied: Sequence[str], sdk_root: Path) -> None:
    with zipfile.ZipFile(patch5["path"]) as archive:
        for relative in applied:
            _write_member(
                archive,
                patch5["members"][relative],
                sdk_root.joinpath(*relative.split("/")),
            )


def apply_version_headers(sdk_root: Path, specifications: Sequence[dict[str, Any]]) -> list[dict[str, Any]]:
    results = []
    for specification in specifications:
        relative = specification["path"]
        path = sdk_root.joinpath(*relative.split("/"))
        if not path.is_file():
            raise SdkPreparationError(f"version header is missing: {relative}")
        data = path.read_bytes()
        replacements = []
        for replacement in specification["replacements"]:
            before = replacement["from"].encode("ascii")
            after = replacement["to"].encode("ascii")
            if len(before) != len(after):
                raise SdkPreparationError(f"non-size-preserving header replacement in {relative}")
            count = data.count(before)
            if count != replacement["expected_count"]:
                raise SdkPreparationError(
                    f"header marker {replacement['from']!r} occurs {count} times in {relative}, "
                    f"expected {replacement['expected_count']}"
                )
            data = data.replace(before, after)
            replacements.append({"from": replacement["from"], "to": replacement["to"], "count": count})
        path.write_bytes(data)
        results.append({"path": relative, "sha256": sha256_bytes(data), "replacements": replacements})
    return results


def _iter_files(root: Path) -> Iterable[Path]:
    return (path for path in sorted(root.rglob("*")) if path.is_file())


def apply_binary_marker(sdk_root: Path, specification: dict[str, Any]) -> dict[str, Any]:
    before = bytes.fromhex(specification["from_hex"])
    after = bytes.fromhex(specification["to_hex"])
    base = bytes.fromhex(specification["base_hex"])
    if len(before) != 4 or len(after) != 4 or len(base) != 4:
        raise SdkPreparationError("release marker values must each contain four bytes")
    changed = []
    occurrences = 0
    preexisting_target = 0
    remaining_base = 0
    for path in _iter_files(sdk_root):
        data = path.read_bytes()
        count = data.count(before)
        preexisting_target += data.count(after)
        remaining_base += data.count(base)
        if not count:
            continue
        data = data.replace(before, after)
        path.write_bytes(data)
        occurrences += count
        changed.append(
            {
                "path": path.relative_to(sdk_root).as_posix(),
                "occurrences": count,
                "sha256": sha256_bytes(data),
            }
        )
    if preexisting_target:
        raise SdkPreparationError(f"tree unexpectedly contained {preexisting_target} patch4 markers")
    if remaining_base:
        raise SdkPreparationError(f"tree still contains {remaining_base} base release markers")
    if len(changed) != specification["expected_files"] or occurrences != specification["expected_occurrences"]:
        raise SdkPreparationError(
            f"binary marker mismatch: transformed {len(changed)} files/{occurrences} occurrences, "
            f"expected {specification['expected_files']}/{specification['expected_occurrences']}"
        )
    return {"files": changed, "occurrences": occurrences}


def tree_digest(root: Path) -> dict[str, Any]:
    digest = hashlib.sha256()
    count = 0
    total_size = 0
    for path in _iter_files(root):
        relative = path.relative_to(root).as_posix().encode("utf-8")
        size = path.stat().st_size
        file_hash = bytes.fromhex(sha256_file(path))
        digest.update(len(relative).to_bytes(4, "big"))
        digest.update(relative)
        digest.update(size.to_bytes(8, "big"))
        digest.update(file_hash)
        count += 1
        total_size += size
    return {"file_count": count, "total_size": total_size, "sha256": digest.hexdigest()}


def validate_expected_tree(actual: dict[str, Any], expected: dict[str, Any]) -> None:
    _validate_sha256(expected.get("sha256", ""), "expected tree hash")
    for key in ("file_count", "total_size", "sha256"):
        if actual[key] != expected.get(key):
            raise SdkPreparationError(
                f"prepared tree {key} mismatch: got {actual[key]}, expected {expected.get(key)}"
            )


def ensure_external_output(output: Path) -> Path:
    resolved = output.expanduser().resolve()
    root = repository_root().resolve()
    try:
        resolved.relative_to(root)
    except ValueError:
        return resolved
    raise SdkPreparationError(f"SDK output must be outside the repository: {resolved}")


def _existing_is_current(output: Path, config: dict[str, Any], diagnostic: dict[str, Any]) -> bool:
    manifest_path = output / "preparation-manifest.json"
    sdk_root = output / "TwlSDK"
    if not manifest_path.is_file() or not sdk_root.is_dir():
        return False
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return False
    if manifest.get("config_sha256") != config["_sha256"]:
        return False
    if manifest.get("inputs") != diagnostic["inputs"]:
        return False
    actual = tree_digest(sdk_root)
    return actual == config["expected_tree"] and manifest.get("tree") == actual


def construct_tree(
    temporary: Path,
    config: dict[str, Any],
    diagnostic: dict[str, Any],
) -> dict[str, Any]:
    sdk_root = temporary / "TwlSDK"
    sdk_root.mkdir()
    extract_base(diagnostic["_base"], sdk_root)
    apply_overlay(diagnostic["_patch5"], diagnostic["_applied"], sdk_root)
    header_results = apply_version_headers(sdk_root, config["version_headers"])
    marker_results = apply_binary_marker(sdk_root, config["binary_release_marker"])
    tree = tree_digest(sdk_root)
    return {
        "tree": tree,
        "version_headers": header_results,
        "binary_release_marker": marker_results,
    }


def prepare(
    base_archive: Path,
    patch5_archive: Path,
    output: Path,
    config: dict[str, Any],
    *,
    force: bool = False,
) -> dict[str, Any]:
    output = ensure_external_output(output)
    diagnostic = diagnose(base_archive, patch5_archive, config)
    if output.exists() and not force:
        if _existing_is_current(output, config, diagnostic):
            return {"status": "already-prepared", "output": str(output), "tree": config["expected_tree"]}
        raise SdkPreparationError(f"output exists but is not the expected tree (use --force): {output}")

    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = Path(tempfile.mkdtemp(prefix=".twlsdk-patch4-", dir=output.parent))
    try:
        constructed = construct_tree(temporary, config, diagnostic)
        validate_expected_tree(constructed["tree"], config["expected_tree"])
        manifest = {
            "schema_version": 1,
            "status": "prepared",
            "config_sha256": config["_sha256"],
            "target": config["target"],
            "inputs": diagnostic["inputs"],
            "overlay": {
                "applied_files": len(diagnostic["_applied"]),
                "skipped_patch5_only": diagnostic["_skipped"],
            },
            "transforms": {
                "version_headers": constructed["version_headers"],
                "binary_release_marker": constructed["binary_release_marker"],
            },
            "tree": constructed["tree"],
            "evidence": config["evidence"],
        }
        (temporary / "preparation-manifest.json").write_text(
            json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8"
        )
        if output.exists():
            if output.is_dir() and not output.is_symlink():
                shutil.rmtree(output)
            else:
                output.unlink()
        temporary.rename(output)
        return {"status": "prepared", "output": str(output), "tree": constructed["tree"]}
    except Exception:
        shutil.rmtree(temporary, ignore_errors=True)
        raise


def _public_diagnostic(diagnostic: dict[str, Any]) -> dict[str, Any]:
    return {key: value for key, value in diagnostic.items() if not key.startswith("_")}


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--config",
        type=Path,
        default=repository_root() / "config" / "sdk-patch4.json",
    )
    subparsers = parser.add_subparsers(dest="command", required=True)
    for name in ("diagnose", "prepare"):
        command = subparsers.add_parser(name)
        command.add_argument("--base-archive", type=Path, required=True)
        command.add_argument("--patch5-archive", type=Path, required=True)
        if name == "prepare":
            command.add_argument("--output", type=Path, required=True)
            command.add_argument("--force", action="store_true")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        config = load_config(args.config.resolve())
        if args.command == "diagnose":
            result = _public_diagnostic(
                diagnose(args.base_archive.resolve(), args.patch5_archive.resolve(), config)
            )
        else:
            result = prepare(
                args.base_archive.resolve(),
                args.patch5_archive.resolve(),
                args.output,
                config,
                force=args.force,
            )
        print(json.dumps(result, indent=2, sort_keys=True))
        return 0
    except SdkPreparationError as error:
        print(f"SDK preparation error: {error}", file=os.sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
