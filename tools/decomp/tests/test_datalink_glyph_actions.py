from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_glyph_actions.c"

FUNCTIONS = (
    (0x020AF050, 96, "e67e09b0a5c2bc89d383f8468a0daf7e35eebde8acd5878e10f14b24266d8e19"),
    (0x020AF0B0, 80, "7c2f96981782d16471d1a33090885929d6be58eef1e541259cccf536c0ff704b"),
    (0x020AF104, 120, "fd936f27890c981b56c1508f87df68319370758934e600248a9e09593cfc29c3"),
    (0x020AF17C, 120, "17348aad40c5f5cc071acce319ac4d090b4fad1675f9f76eae6ae80e7aa05956"),
    (0x020AF1F4, 324, "88127d2b2ef1d872270992799e8bda5a748e5c433d2e74141a46183909130077"),
    (0x020AF344, 60, "6e27a4259beae93d3957bbb1dd8d6c7930928642bb66152130c40ce90e8e7d7a"),
    (0x020AF380, 164, "7e47619476067dbf327e09449b13e9d6a26c5d119d6517d677fc504b7a4a13ab"),
    (0x020AF424, 116, "9a81f25fe67a9c1481047dc012e3e314b20aa660701edeef7ebf16b20d682ecf"),
    (0x020AF4A0, 160, "432d8dd44d78fb72c0f42b8a6645c7b15a6d5ad66c63c0d928d3bc760c167cde"),
)


class DatalinkGlyphActionsRecoveryTests(unittest.TestCase):
    def test_rom_bodies(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()
        for address, size, expected_sha256 in FUNCTIONS:
            with self.subTest(address=f"0x{address:08X}"):
                start = address - ARM9_BASE
                body = arm9[start : start + size]
                self.assertEqual(len(body), size)
                self.assertEqual(
                    hashlib.sha256(body).hexdigest(), expected_sha256
                )

    def test_host_c_syntax(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        subprocess.run(
            [compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
             "-pedantic", "-I", str(PROJECT_ROOT / "include"),
             "-fsyntax-only", str(SOURCE)],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )

    def test_arm946es_syntax(self) -> None:
        compiler = shutil.which("clang")
        if compiler is None:
            self.skipTest("clang is unavailable")
        subprocess.run(
            [compiler, "--target=arm-none-eabi", "-mcpu=arm946e-s",
             "-marm", "-std=c11", "-Wall", "-Wextra", "-Werror",
             "-pedantic", "-ffreestanding", "-I",
             str(PROJECT_ROOT / "include"), "-fsyntax-only", str(SOURCE)],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )


if __name__ == "__main__":
    unittest.main()
