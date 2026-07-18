from __future__ import annotations

import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000


class LanguageTableRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_format_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        # Shared binary validation checks the big-endian tag and FEFF BOM.
        self.assertEqual(read_u32(0x020B7378), 0xE1D020D1)
        self.assertEqual(read_u32(0x020B7398), 0xE1510002)
        self.assertEqual(read_u32(0x020B739C), 0x01D010B4)
        self.assertEqual(read_u32(0x020B73B4), 0x0000FEFF)
        # Loader calls accessor slot 2 using type 0x6e746c69.
        self.assertEqual(read_u32(0x020BF0B0), 0xE5933008)
        self.assertEqual(read_u32(0x020BF0B8), 0xE12FFF33)
        self.assertEqual(read_u32(0x020BF178), 0x6E746C69)
        # Version is exactly 2.0 and blocks begin at header_size.
        self.assertEqual(read_u32(0x020BF0DC), 0xE1D410B6)
        self.assertEqual(read_u32(0x020BF0EC), 0xE3500002)
        self.assertEqual(read_u32(0x020BF0FC), 0xE1D400BC)
        self.assertEqual(read_u32(0x020BF100), 0xE1D420BE)
        # The mtl1 block publishes its count and payload at +0x0c.
        self.assertEqual(read_u32(0x020BF140), 0xE1D020B8)
        self.assertEqual(read_u32(0x020BF144), 0xE280100C)
        self.assertEqual(read_u32(0x020BF150), 0xE5851004)
        self.assertEqual(read_u32(0x020BF180), 0x6D746C31)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "language_table_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/language_table.c"),
                    str(Path(__file__).with_name("language_table_harness.c")),
                    "-o",
                    str(executable),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )
            subprocess.run([str(executable)], check=True, cwd=PROJECT_ROOT)


if __name__ == "__main__":
    unittest.main()
