#!/usr/bin/env python3
"""Synchronize the public tree and drive the private Windows toolchain."""

from __future__ import annotations

import argparse
import base64
from pathlib import Path, PurePosixPath, PureWindowsPath
import re
import shlex
import shutil
import subprocess
import sys
from typing import Sequence


DEFAULT_HOST = "sekailink-windows"
DEFAULT_REMOTE_ROOT = PureWindowsPath(r"D:\FourSwordAnniversary")
DEFAULT_REMOTE_WORKSPACE = DEFAULT_REMOTE_ROOT / "workspace"
DEFAULT_TOOLCHAIN_ROOT = DEFAULT_REMOTE_ROOT / "toolchains"
DEFAULT_ENVIRONMENT_SCRIPT = DEFAULT_REMOTE_ROOT / "private" / "environment.ps1"
REMOTE_RSYNC = "C:/msys64/usr/bin/rsync.exe"

# Keep this list deliberately conservative. The Windows workspace is a public,
# disposable mirror; private dumps and toolchains live beside it, never in it.
SYNC_EXCLUDES = (
    "/.git/",
    "/private/",
    "/toolchains/",
    "/build/",
    "/out/",
    "/extracted/",
    "/.cache/",
    "/.venv/",
    "private/",
    "toolchains/",
    "build/",
    "__pycache__/",
    "*.py[cod]",
    "*.nds",
    "*.NDS",
    "*.srl",
    "*.SRL",
    "*.tad",
    "*.TAD",
    "bios*.bin",
    "Bios*.bin",
    "BIOS*.bin",
    "firmware*.bin",
    "Firmware*.bin",
    "*Nintendo*SDK*.zip",
    "*Nintendo*Sdk*.zip",
    "*TwlSDK*.zip",
    "*TWLSDK*.zip",
    "*SDK*/",
    "*Sdk*/",
    "*SDK*.7z",
    "*SDK*.rar",
    "*.exe",
    "*.dll",
    "*.msi",
)


class WindowsToolchainError(RuntimeError):
    """A local validation error before an SSH/rsync process starts."""


def repository_root() -> Path:
    return Path(__file__).resolve().parents[2]


def _validate_host(host: str) -> str:
    if not re.fullmatch(r"[A-Za-z0-9_.@-]+", host):
        raise WindowsToolchainError(f"unsafe SSH host or alias: {host!r}")
    return host


def _windows_path(value: str | PureWindowsPath, label: str) -> PureWindowsPath:
    path = PureWindowsPath(value)
    if not path.is_absolute() or not path.drive:
        raise WindowsToolchainError(f"{label} must be an absolute drive path: {value!s}")
    if any(part in {".", ".."} for part in path.parts):
        raise WindowsToolchainError(f"{label} may not contain dot segments: {value!s}")
    return path


def windows_to_msys_path(value: str | PureWindowsPath) -> str:
    path = _windows_path(value, "remote path")
    drive = path.drive.rstrip(":").lower()
    tail = PurePosixPath(*path.parts[1:]).as_posix()
    return f"/{drive}/{tail}" if tail != "." else f"/{drive}"


def build_remote_mkdir_command(host: str, workspace: str | PureWindowsPath) -> list[str]:
    host = _validate_host(host)
    workspace_path = _windows_path(workspace, "remote workspace")
    script = (
        "$ErrorActionPreference='Stop';$ProgressPreference='SilentlyContinue';"
        f"$p='{_powershell_quote(str(workspace_path))}';"
        "[void](New-Item -ItemType Directory -Force -Path $p)"
    )
    return build_encoded_powershell_command(host, script)


def build_rsync_command(
    source: Path,
    host: str = DEFAULT_HOST,
    workspace: str | PureWindowsPath = DEFAULT_REMOTE_WORKSPACE,
    *,
    dry_run: bool = False,
    delete: bool = False,
) -> list[str]:
    host = _validate_host(host)
    source = source.resolve()
    if not source.is_dir():
        raise WindowsToolchainError(f"source directory does not exist: {source}")
    workspace_path = windows_to_msys_path(workspace).rstrip("/") + "/"
    command = [
        "rsync",
        "--recursive",
        "--links",
        "--safe-links",
        "--times",
        "--compress",
        "--checksum",
        "--omit-dir-times",
        "--prune-empty-dirs",
        "--itemize-changes",
        "--human-readable",
        f"--rsync-path={REMOTE_RSYNC}",
    ]
    if delete:
        command.append("--delete-delay")
    if dry_run:
        command.append("--dry-run")
    for pattern in SYNC_EXCLUDES:
        command.extend(("--exclude", pattern))
    command.extend((str(source) + "/", f"{host}:{workspace_path}"))
    return command


def _powershell_quote(value: str) -> str:
    return value.replace("'", "''")


def encode_powershell(script: str) -> str:
    return base64.b64encode(script.encode("utf-16-le")).decode("ascii")


def decode_powershell(encoded: str) -> str:
    return base64.b64decode(encoded, validate=True).decode("utf-16-le")


def build_encoded_powershell_command(host: str, script: str) -> list[str]:
    host = _validate_host(host)
    return [
        "ssh",
        "-o",
        "BatchMode=yes",
        "-o",
        "ConnectTimeout=15",
        "--",
        host,
        "powershell.exe",
        "-NoLogo",
        "-NoProfile",
        "-NonInteractive",
        "-OutputFormat",
        "Text",
        "-EncodedCommand",
        encode_powershell(script),
    ]


def build_remote_toolchain_command(
    *,
    host: str = DEFAULT_HOST,
    action: str,
    workspace: str | PureWindowsPath = DEFAULT_REMOTE_WORKSPACE,
    toolchain_root: str | PureWindowsPath = DEFAULT_TOOLCHAIN_ROOT,
    environment_script: str | PureWindowsPath = DEFAULT_ENVIRONMENT_SCRIPT,
    mwccarm: str | PureWindowsPath | None = None,
    mwldarm: str | PureWindowsPath | None = None,
    build_command: str | None = None,
) -> list[str]:
    if action not in {"Check", "Build"}:
        raise WindowsToolchainError(f"unsupported remote action: {action}")
    if action == "Build" and not build_command:
        raise WindowsToolchainError("a non-empty build command is required")
    if action == "Check" and build_command is not None:
        raise WindowsToolchainError("a check action may not carry a build command")

    workspace_path = _windows_path(workspace, "remote workspace")
    toolchain_path = _windows_path(toolchain_root, "toolchain root")
    environment_path = _windows_path(environment_script, "environment script")
    remote_script = workspace_path / "tools" / "windows" / "remote_toolchain.ps1"
    parameters = {
        "Action": action,
        "Workspace": str(workspace_path),
        "ToolchainRoot": str(toolchain_path),
        "EnvironmentScript": str(environment_path),
    }
    if mwccarm is not None:
        parameters["Mwccarm"] = str(_windows_path(mwccarm, "mwccarm"))
    if mwldarm is not None:
        parameters["Mwldarm"] = str(_windows_path(mwldarm, "mwldarm"))
    if build_command is not None:
        parameters["BuildCommandBase64"] = base64.b64encode(
            build_command.encode("utf-8")
        ).decode("ascii")

    entries = ";".join(
        f"{name}='{_powershell_quote(value)}'" for name, value in parameters.items()
    )
    launcher = (
        "$ErrorActionPreference='Stop';$ProgressPreference='SilentlyContinue';"
        f"$script='{_powershell_quote(str(remote_script))}';"
        "if(-not(Test-Path -LiteralPath $script -PathType Leaf)){"
        "throw ('Remote helper missing; run sync first: '+$script)};"
        f"$params=@{{{entries}}};"
        "& $script @params;"
        "exit $LASTEXITCODE"
    )
    return build_encoded_powershell_command(host, launcher)


def _require_program(name: str) -> None:
    if shutil.which(name) is None:
        raise WindowsToolchainError(f"required local program is unavailable: {name}")


def _display_command(command: Sequence[str]) -> None:
    visible = list(command)
    if "-EncodedCommand" in visible:
        index = visible.index("-EncodedCommand") + 1
        visible[index] = "<base64-powershell>"
    print("+ " + shlex.join(visible), flush=True)


def _run(command: Sequence[str]) -> int:
    _display_command(command)
    try:
        return subprocess.run(command, check=False).returncode
    except OSError as error:
        raise WindowsToolchainError(f"cannot execute {command[0]}: {error}") from error


def sync(args: argparse.Namespace) -> int:
    _require_program("ssh")
    _require_program("rsync")
    if not args.dry_run:
        mkdir_command = build_remote_mkdir_command(args.host, args.remote_workspace)
        status = _run(mkdir_command)
        if status:
            return status
    command = build_rsync_command(
        args.source,
        args.host,
        args.remote_workspace,
        dry_run=args.dry_run,
        delete=args.delete_stale,
    )
    return _run(command)


def remote(args: argparse.Namespace, action: str) -> int:
    _require_program("ssh")
    command = build_remote_toolchain_command(
        host=args.host,
        action=action,
        workspace=args.remote_workspace,
        toolchain_root=args.toolchain_root,
        environment_script=args.environment_script,
        mwccarm=args.mwccarm,
        mwldarm=args.mwldarm,
        build_command=getattr(args, "build_command", None),
    )
    return _run(command)


def _add_remote_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--host", default=DEFAULT_HOST, help="SSH host or alias")
    parser.add_argument(
        "--remote-workspace",
        default=str(DEFAULT_REMOTE_WORKSPACE),
        help="absolute Windows workspace path",
    )


def _add_toolchain_arguments(parser: argparse.ArgumentParser) -> None:
    _add_remote_arguments(parser)
    parser.add_argument(
        "--toolchain-root", default=str(DEFAULT_TOOLCHAIN_ROOT), help="private toolchain search root"
    )
    parser.add_argument(
        "--environment-script",
        default=str(DEFAULT_ENVIRONMENT_SCRIPT),
        help="optional private PowerShell environment file",
    )
    parser.add_argument("--mwccarm", help="explicit absolute path to mwccarm.exe")
    parser.add_argument("--mwldarm", help="explicit absolute path to mwldarm.exe")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="action", required=True)

    sync_parser = subparsers.add_parser("sync", help="mirror public source files to Windows")
    _add_remote_arguments(sync_parser)
    sync_parser.add_argument("--source", type=Path, default=repository_root())
    sync_parser.add_argument(
        "--dry-run", action="store_true", help="connect and list changes without writing"
    )
    sync_parser.add_argument(
        "--delete-stale",
        action="store_true",
        help="delete stale non-excluded workspace files (off by default)",
    )

    check_parser = subparsers.add_parser("check", help="locate both compilers and report versions")
    _add_toolchain_arguments(check_parser)

    build_parser_ = subparsers.add_parser(
        "build", help="verify compilers, then execute one command in the remote workspace"
    )
    _add_toolchain_arguments(build_parser_)
    build_parser_.add_argument(
        "--command", dest="build_command", required=True, help="command interpreted by remote cmd.exe"
    )
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        if args.action == "sync":
            return sync(args)
        return remote(args, "Check" if args.action == "check" else "Build")
    except WindowsToolchainError as error:
        print(f"Windows toolchain error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
