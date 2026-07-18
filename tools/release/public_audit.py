#!/usr/bin/env python3
"""Reject private Nintendo material and credentials before a public push."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
import re
import subprocess
import sys
from typing import Iterable, Sequence


PROJECT_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_MAX_FILE_SIZE = 5 * 1024 * 1024

FORBIDDEN_SUFFIXES = {
    ".3ds",
    ".7z",
    ".bin",
    ".cia",
    ".dll",
    ".dsi",
    ".elf",
    ".exe",
    ".lic",
    ".license",
    ".msi",
    ".nds",
    ".o",
    ".rar",
    ".srl",
    ".tad",
    ".zip",
}
FORBIDDEN_COMPONENTS = {
    "build",
    "extracted",
    "out",
    "private",
    "toolchains",
}
FORBIDDEN_NAMES = {
    "bios7.bin",
    "bios7i.bin",
    "bios9.bin",
    "bios9i.bin",
    "firmware.bin",
    "license.dat",
}
TEXT_SUFFIXES = {
    "",
    ".c",
    ".css",
    ".h",
    ".html",
    ".java",
    ".js",
    ".json",
    ".lsf",
    ".md",
    ".ps1",
    ".py",
    ".s",
    ".sh",
    ".txt",
    ".yml",
    ".yaml",
}
SECRET_PATTERNS = (
    (
        "private key",
        re.compile(rb"-----BEGIN (?:OPENSSH |RSA |EC |DSA )?PRIVATE KEY-----"),
    ),
    ("GitHub token", re.compile(rb"\b(?:gh[opurs]_[A-Za-z0-9_]{20,}|github_pat_[A-Za-z0-9_]{20,})\b")),
    (
        "credential assignment",
        re.compile(
            rb"(?i)\b(?:password|passwd|api[_-]?key|access[_-]?token)\b"
            rb"\s*[:=]\s*['\"][^'\"\r\n]{8,}['\"]"
        ),
    ),
    ("credential URL", re.compile(rb"https?://[^/@:\s]+:[^/@\s]+@")),
)


class PublicAuditError(RuntimeError):
    """The candidate public tree is unsafe or cannot be inspected."""


@dataclass(frozen=True)
class Finding:
    path: str
    reason: str


def git_paths(root: Path, scope: str) -> list[str]:
    command = ["git", "-C", str(root), "ls-files", "-z"]
    if scope == "worktree":
        command.extend(("--cached", "--others", "--exclude-standard"))
    completed = subprocess.run(command, check=False, capture_output=True)
    if completed.returncode != 0:
        detail = completed.stderr.decode("utf-8", errors="replace").strip()
        raise PublicAuditError(f"git file inventory failed: {detail}")
    return sorted(
        item.decode("utf-8", errors="surrogateescape")
        for item in completed.stdout.split(b"\0")
        if item
    )


def path_findings(path_text: str, maximum_size: int, root: Path) -> list[Finding]:
    findings: list[Finding] = []
    relative = PurePosixPath(path_text)
    top_level = relative.parts[0].lower() if relative.parts else ""
    lowered_name = relative.name.lower()
    suffix = relative.suffix.lower()
    path = root / Path(*relative.parts)

    if top_level in FORBIDDEN_COMPONENTS:
        findings.append(Finding(path_text, "private/generated directory"))
    if lowered_name in FORBIDDEN_NAMES or suffix in FORBIDDEN_SUFFIXES:
        findings.append(Finding(path_text, "forbidden binary/archive type"))
    if not path.exists() and not path.is_symlink():
        findings.append(Finding(path_text, "listed path is missing"))
        return findings
    if path.is_symlink():
        try:
            path.resolve(strict=True).relative_to(root.resolve())
        except (OSError, ValueError):
            findings.append(Finding(path_text, "symlink escapes the repository"))
        return findings
    if path.stat().st_size > maximum_size:
        findings.append(Finding(path_text, f"file exceeds {maximum_size} bytes"))
    return findings


def content_findings(path_text: str, root: Path) -> list[Finding]:
    relative = PurePosixPath(path_text)
    if relative.suffix.lower() not in TEXT_SUFFIXES:
        return []
    path = root / Path(*relative.parts)
    if not path.is_file() or path.is_symlink():
        return []
    try:
        data = path.read_bytes()
    except OSError as exc:
        return [Finding(path_text, f"cannot read file: {exc}")]
    return [
        Finding(path_text, f"possible {label}")
        for label, pattern in SECRET_PATTERNS
        if pattern.search(data)
    ]


def audit_paths(paths: Iterable[str], root: Path, maximum_size: int) -> list[Finding]:
    findings: list[Finding] = []
    for path_text in paths:
        findings.extend(path_findings(path_text, maximum_size, root))
        findings.extend(content_findings(path_text, root))
    return findings


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--scope",
        choices=("tracked", "worktree"),
        default="worktree",
        help="audit tracked files only or every non-ignored public candidate",
    )
    parser.add_argument(
        "--root",
        type=Path,
        default=PROJECT_ROOT,
        help="git repository to inspect",
    )
    parser.add_argument(
        "--max-file-size",
        type=int,
        default=DEFAULT_MAX_FILE_SIZE,
        help="largest allowed public file in bytes",
    )
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    root = args.root.resolve()
    if args.max_file_size < 1:
        print("public audit error: --max-file-size must be positive", file=sys.stderr)
        return 2
    try:
        paths = git_paths(root, args.scope)
        findings = audit_paths(paths, root, args.max_file_size)
    except PublicAuditError as exc:
        print(f"public audit error: {exc}", file=sys.stderr)
        return 2
    if findings:
        for finding in findings:
            print(f"public audit rejected {finding.path}: {finding.reason}", file=sys.stderr)
        return 1
    print(f"public audit passed: {len(paths)} {args.scope} files")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
