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


class ManualPageIndexRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        self.assertEqual(read_u32(0x020B89E4), 0xE3A01000)
        self.assertEqual(read_u32(0x020B89FC), 0xE1C011B4)
        self.assertEqual(read_u32(0x020B8A04), 0xE92D4038)
        self.assertEqual(read_u32(0x020B8A18), 0xEB000009)
        self.assertEqual(read_u32(0x020B8A24), 0xEB000000)
        self.assertEqual(read_u32(0x020B8A5C), 0xE92D40F8)
        self.assertEqual(read_u32(0x020B8A68), 0xE59F10F4)
        self.assertEqual(read_u32(0x020B8ACC), 0xEB000038)
        self.assertEqual(read_u32(0x020B8ADC), 0xEB000021)
        self.assertEqual(read_u32(0x020B8B64), 0x0000FFFF)
        self.assertEqual(read_u32(0x020B8B68), 0xE92D4010)
        self.assertEqual(read_u32(0x020B8BB4), 0xE92D4010)
        self.assertEqual(read_u32(0x020B8C00), 0xE3A01000)
        self.assertEqual(read_u32(0x020B8C08), 0xE1C010B4)
        self.assertEqual(read_u32(0x020B8C0C), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020B8C10), 0xE1A03000)
        self.assertEqual(read_u32(0x020B8C20), 0xE59FC000)
        self.assertEqual(read_u32(0x020B8C24), 0xE12FFF1C)
        self.assertEqual(read_u32(0x020B8C28), 0x020B8C2C)
        self.assertEqual(read_u32(0x020B8C2C), 0xE92D41F0)
        self.assertEqual(read_u32(0x020B8C3C), 0x08BD81F0)
        self.assertEqual(read_u32(0x020B8C58), 0xEBFFFF69)
        self.assertEqual(read_u32(0x020B8C70), 0xEB0027F4)
        self.assertEqual(read_u32(0x020B8C74), 0xE8BD81F0)
        self.assertEqual(read_u32(0x020B8C78), 0xE92D4038)
        self.assertEqual(read_u32(0x020B8C8C), 0xE59CC00C)
        self.assertEqual(read_u32(0x020B8C98), 0xE12FFF3C)
        self.assertEqual(read_u32(0x020B8CB0), 0xEB000001)
        self.assertEqual(read_u32(0x020B8CB8), 0x6E746D63)
        self.assertEqual(read_u32(0x020B8CBC), 0xE92D4FF0)
        self.assertEqual(read_u32(0x020B8CD8), 0xEBFFF9A6)
        self.assertEqual(read_u32(0x020B8D00), 0xE3500002)
        self.assertEqual(read_u32(0x020B8D5C), 0xE1831001)
        self.assertEqual(read_u32(0x020B8D98), 0xEB000022)
        self.assertEqual(read_u32(0x020B8DD4), 0xEBFFFF20)
        self.assertEqual(read_u32(0x020B8E0C), 0xE3A00001)
        self.assertEqual(read_u32(0x020B8E18), 0x4E544D43)
        self.assertEqual(read_u32(0x020B8E1C), 0x6D746331)
        self.assertEqual(read_u32(0x020B8E20), 0x6E617031)
        self.assertEqual(read_u32(0x020B8E24), 0x74787031)
        self.assertEqual(read_u32(0x020B8E28), 0xE92D40F8)
        self.assertEqual(read_u32(0x020B8E38), 0xE3A01018)
        self.assertEqual(read_u32(0x020B8E44), 0xEB00277A)
        self.assertEqual(read_u32(0x020B8E70), 0xEBFFFEDB)
        self.assertEqual(read_u32(0x020B8E84), 0xE8BD80F8)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_page_index_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_page_index.c"),
                    str(Path(__file__).with_name("manual_page_index_harness.c")),
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
            output = Path(temporary) / "manual_page_index.o"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_page_index.c"),
                    "-o",
                    str(output),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )


if __name__ == "__main__":
    unittest.main()
