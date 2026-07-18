#!/usr/bin/env python3
"""Compile a public C fixture with the private mwccarm selected by the host."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
from typing import Sequence


def compile_command(compiler: str, source: Path, output: Path) -> list[str]:
    return [compiler, "-c", str(source), "-o", str(output)]


def build_parser() -> argparse.ArgumentParser:
    script = Path(__file__).resolve()
    root = script.parents[2]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--compiler", default=os.environ.get("MWCCARM"), help="mwccarm path (default: MWCCARM)"
    )
    parser.add_argument(
        "--source",
        type=Path,
        default=script.parent / "tests" / "fixtures" / "smoke.c",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=root / "build" / "windows-toolchain-smoke" / "smoke.o",
    )
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    if not args.compiler:
        print("smoke build error: MWCCARM is not set", file=sys.stderr)
        return 2
    source = args.source.resolve()
    output = args.output.resolve()
    if not source.is_file():
        print(f"smoke build error: source not found: {source}", file=sys.stderr)
        return 2
    output.parent.mkdir(parents=True, exist_ok=True)
    command = compile_command(args.compiler, source, output)
    try:
        completed = subprocess.run(command, check=False)
    except OSError as error:
        print(f"smoke build error: cannot execute compiler: {error}", file=sys.stderr)
        return 2
    if completed.returncode != 0:
        return completed.returncode
    if not output.is_file() or output.stat().st_size == 0:
        print("smoke build error: compiler produced no object", file=sys.stderr)
        return 2
    digest = hashlib.sha256(output.read_bytes()).hexdigest()
    print(
        json.dumps(
            {
                "status": "compiled",
                "source": str(source),
                "output": str(output),
                "size": output.stat().st_size,
                "sha256": digest,
            },
            sort_keys=True,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
