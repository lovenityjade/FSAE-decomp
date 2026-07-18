from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_glyph_modifiers.c"

FUNCTIONS = (
    (0x020AF978, 156, "00ae8447a04cb61738c9125ad92230b794107bf617654c23717bcd27d132d9dd"),
    (0x020AFA1C, 40, "fa64d031a32c9223c897316820ddd1f74aba61a01f60d50474ed59a875629662"),
    (0x020AFA4C, 40, "fa64d031a32c9223c897316820ddd1f74aba61a01f60d50474ed59a875629662"),
    (0x020AFA7C, 180, "84cfd34b4a5fe729e99b836ff4ad16795b63cebaf6c4e02d9cac31dff60efbbb"),
    (0x020AFB4C, 64, "68a0ebd1c174933dd15d719ac646d8fc18a7288480b3e17cb644be79c667f3fb"),
    (0x020AFB94, 112, "d6e8308907bf41cc4baa2c3896c23e5a15d8d6f17e7e464651590ef41b3cde08"),
    (0x020AFC0C, 80, "c8de2c83387dbb45f4aa054e2ab035c47d1e2afff64838141e873e76fe27422d"),
)


class DatalinkGlyphModifierRecoveryTests(unittest.TestCase):
    def test_rom_bodies(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()
        for address, size, expected_sha256 in FUNCTIONS:
            start = address - ARM9_BASE
            body = arm9[start : start + size]
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
