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


class ManualPageLoaderRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        self.assertEqual(read_u32(0x020BDD60), 0xE92D4FF8)
        self.assertEqual(read_u32(0x020BDD88), 0xE1A01207)
        self.assertEqual(read_u32(0x020BDD9C), 0xEBFFFC9A)
        self.assertEqual(read_u32(0x020BDDA0), 0xEBFD6E84)
        self.assertEqual(read_u32(0x020BDDAC), 0xEB00015E)
        self.assertEqual(read_u32(0x020BDDE0), 0xEBFFFCD0)
        self.assertEqual(read_u32(0x020BDDF8), 0xEB00013E)
        self.assertEqual(read_u32(0x020BDE40), 0xEB00005E)
        self.assertEqual(read_u32(0x020BDE4C), 0xE59F1164)
        self.assertEqual(read_u32(0x020BDE58), 0xE12FFF33)
        self.assertEqual(read_u32(0x020BDE70), 0xEBFFFBBA)
        self.assertEqual(read_u32(0x020BDE84), 0xE588039C)
        self.assertEqual(read_u32(0x020BDE88), 0xEBFFDC1C)
        self.assertEqual(read_u32(0x020BDE9C), 0xE1C231BC)
        self.assertEqual(read_u32(0x020BDEA8), 0xEB000185)
        self.assertEqual(read_u32(0x020BDEBC), 0xE59F70F8)
        self.assertEqual(read_u32(0x020BDECC), 0xEBFFEA08)
        self.assertEqual(read_u32(0x020BDEF0), 0xEBFFEA99)
        self.assertEqual(read_u32(0x020BDF00), 0xEBFD6E2C)
        self.assertEqual(read_u32(0x020BDF10), 0xEBFFF939)
        self.assertEqual(read_u32(0x020BDF20), 0xE2477001)
        self.assertEqual(read_u32(0x020BDF48), 0xEB0000F7)
        self.assertEqual(read_u32(0x020BDF84), 0xEBFFFC67)
        self.assertEqual(read_u32(0x020BDF90), 0xE59A0358)
        self.assertEqual(read_u32(0x020BDFA4), 0xEBFFF90A)
        self.assertEqual(read_u32(0x020BDFAC), 0xE8BD8FF8)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_page_loader_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_page_loader.c"),
                    str(Path(__file__).with_name("manual_page_loader_harness.c")),
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
