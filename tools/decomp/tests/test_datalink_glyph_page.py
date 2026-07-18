from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_glyph_page.c"
FUNCTIONS = (
    (0x020AF7EC, 212, "984b63c007ba0a86da28d2085112306735cae71d5a65472865543da329e4a797"),
    (0x020AF8C8, 164, "7e6be843eadbb34880f9fafccef9ab0a95f289313addca9eaafeb8b0c4bc7db2"),
)


class DatalinkGlyphPageRecoveryTests(unittest.TestCase):
    def test_rom_bodies(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()
        for address, size, expected_sha256 in FUNCTIONS:
            body = arm9[address - ARM9_BASE : address - ARM9_BASE + size]
            with self.subTest(address=f"0x{address:08X}"):
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
