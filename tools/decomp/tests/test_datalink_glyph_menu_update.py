from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_glyph_menu_update.c"
HARNESS = Path(__file__).with_name("datalink_glyph_menu_update_harness.c")


class DatalinkGlyphMenuUpdateRecoveryTests(unittest.TestCase):
    def test_rom_body_hash_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        start = 0x020AEA40
        end = 0x020AF020
        body = arm9[start - ARM9_BASE : end - ARM9_BASE]
        pool = arm9[0x020AF020 - ARM9_BASE : 0x020AF050 - ARM9_BASE]
        combined = arm9[start - ARM9_BASE : 0x020AF050 - ARM9_BASE]
        self.assertEqual(len(body), 1504)
        self.assertEqual(len(pool), 48)
        self.assertEqual(len(combined), 1552)
        self.assertEqual(
            hashlib.sha256(body).hexdigest(),
            "1a804a985caadb7dd93ca4aa477f4fc0e5d6c2dfebe2155fc76d7c1907526795",
        )
        self.assertEqual(
            hashlib.sha256(pool).hexdigest(),
            "885550d21a04a9313ff2a1b42eb882cb28e6e4d5d17fa57d5ddb08111056d1d0",
        )
        self.assertEqual(
            hashlib.sha256(combined).hexdigest(),
            "f05e8aeca704f08f03ca09b820bc8e426c1fe7369afdc9b672679f9434145310",
        )

        self.assertEqual(read_u32(0x020AEA3C), 0x0217F314)
        self.assertEqual(read_u32(0x020AEA40), 0xE92D47F8)
        self.assertEqual(read_u32(0x020AEA44), 0xE24DD024)
        self.assertEqual(read_u32(0x020AEA4C), 0xE1D654FA)
        self.assertEqual(read_u32(0x020AEA8C), 0xEBFDEE03)
        self.assertEqual(read_u32(0x020AEB40), 0xEBFFFF88)
        self.assertEqual(read_u32(0x020AEB84), 0xEB00019A)
        self.assertEqual(read_u32(0x020AEBD0), 0xEB00033C)
        self.assertEqual(read_u32(0x020AED60), 0xEB0000BA)
        self.assertEqual(read_u32(0x020AEDF0), 0xEBFFF7AA)
        self.assertEqual(read_u32(0x020AEE18), 0xEBFFFE84)
        self.assertEqual(read_u32(0x020AEEC0), 0xEBFD8F1A)
        self.assertEqual(read_u32(0x020AEF10), 0xEBFFF762)
        self.assertEqual(read_u32(0x020AEFA0), 0xEBFFFA6F)
        self.assertEqual(read_u32(0x020AEFBC), 0xEBFFFE3F)
        self.assertEqual(read_u32(0x020AF018), 0xE28DD024)
        self.assertEqual(read_u32(0x020AF01C), 0xE8BD87F8)
        self.assertEqual(read_u32(0x020AF020), 0x66666667)
        self.assertEqual(read_u32(0x020AF024), 0x0217E6F0)
        self.assertEqual(read_u32(0x020AF028), 0x0212C510)
        self.assertEqual(read_u32(0x020AF02C), 0x021806F0)
        self.assertEqual(read_u32(0x020AF030), 0x0217F314)
        self.assertEqual(read_u32(0x020AF040), 0x0217F3C4)
        self.assertEqual(read_u32(0x020AF044), 0x2AAAAAAB)
        self.assertEqual(read_u32(0x020AF048), 0x0217F83C)
        self.assertEqual(read_u32(0x020AF04C), 0x0217F6F0)
        self.assertEqual(read_u32(0x020AF050), 0xE3510005)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "datalink_glyph_menu_update_test"
            subprocess.run(
                [
                    compiler,
                    "-std=c11",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-pedantic",
                    "-I",
                    str(PROJECT_ROOT / "include"),
                    str(SOURCE),
                    str(HARNESS),
                    "-o",
                    str(executable),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )
            subprocess.run([str(executable)], check=True, cwd=PROJECT_ROOT)

    def test_arm946es_syntax(self) -> None:
        compiler = shutil.which("clang")
        if compiler is None:
            self.skipTest("clang is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "datalink_glyph_menu_update.o"
            subprocess.run(
                [
                    compiler,
                    "--target=arm-none-eabi",
                    "-mcpu=arm946e-s",
                    "-marm",
                    "-std=c11",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-pedantic",
                    "-ffreestanding",
                    "-I",
                    str(PROJECT_ROOT / "include"),
                    "-c",
                    str(SOURCE),
                    "-o",
                    str(output),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )


if __name__ == "__main__":
    unittest.main()
