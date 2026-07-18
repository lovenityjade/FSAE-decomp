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
SOURCE = PROJECT_ROOT / "src/arm9/ntmv/m2d/page_factories.c"
HARNESS = Path(__file__).with_name("page_factories_harness.c")


class PageFactoriesRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        # 0x020b5318..0x020b5413: four-tag dispatcher.  Its separate literal
        # pool is asserted below but excluded from recovered body bytes.
        dispatcher = {
            0x020B5318: 0xE92D4018,
            0x020B531C: 0xE24DD00C,
            0x020B5320: 0xE59F00EC,
            0x020B532C: 0xE1520000,
            0x020B5330: 0xE59D1018,
            0x020B5340: 0xE59F00D0,
            0x020B5348: 0x0A000009,
            0x020B5350: 0xE59F00C4,
            0x020B535C: 0x0A000016,
            0x020B5364: 0xE59F00B4,
            0x020B536C: 0x0A00001B,
            0x020B5374: 0xE24D4008,
            0x020B5378: 0xE891000F,
            0x020B537C: 0xE884000F,
            0x020B5388: 0xE894000C,
            0x020B538C: 0xEB000064,
            0x020B5394: 0xE8BD8018,
            0x020B5398: 0xE24D4008,
            0x020B53AC: 0xE894000C,
            0x020B53B0: 0xEB000046,
            0x020B53B8: 0xE8BD8018,
            0x020B53BC: 0xE24D4008,
            0x020B53D0: 0xE894000C,
            0x020B53D4: 0xEB000028,
            0x020B53DC: 0xE8BD8018,
            0x020B53E0: 0xE24D4004,
            0x020B53EC: 0xE5943000,
            0x020B53F0: 0xE59E1000,
            0x020B53F8: 0xE1A0200C,
            0x020B53FC: 0xEB000008,
            0x020B5404: 0xE8BD8018,
            0x020B5408: 0xE3A00000,
            0x020B5410: 0xE8BD8018,
        }
        for address, instruction in dispatcher.items():
            self.assertEqual(read_u32(address), instruction)

        self.assertEqual(read_u32(0x020B5414), 0x70696331)
        self.assertEqual(read_u32(0x020B5418), 0x70616E31)
        self.assertEqual(read_u32(0x020B541C), 0x74787431)
        self.assertEqual(read_u32(0x020B5420), 0x776E6431)

        # Four independent allocation/constructor bodies.  Their exact next
        # starts prove 88 + 84 + 84 + 84 bytes without absorbing neighbours.
        factories = {
            0x020B5424: 0xE92D000F,
            0x020B5428: 0xE92D4010,
            0x020B542C: 0xE5900000,
            0x020B5430: 0xE3A01028,
            0x020B5438: 0xEB0035FD,
            0x020B543C: 0xE3500000,
            0x020B5450: 0xE28D100C,
            0x020B5454: 0xE28D3014,
            0x020B545C: 0xEB0001D1,
            0x020B5468: 0xE12FFF1E,
            0x020B546C: 0xE3A00000,
            0x020B5478: 0xE12FFF1E,
            0x020B547C: 0xE92D000F,
            0x020B5484: 0xE5900000,
            0x020B548C: 0xE3A01048,
            0x020B5490: 0xEB0035E7,
            0x020B5494: 0xE3500000,
            0x020B54A8: 0xE28D2010,
            0x020B54B0: 0xEB0005D0,
            0x020B54BC: 0xE12FFF1E,
            0x020B54C0: 0xE3A00000,
            0x020B54CC: 0xE12FFF1E,
            0x020B54D0: 0xE92D000F,
            0x020B54D8: 0xE5900000,
            0x020B54E0: 0xE3A01028,
            0x020B54E4: 0xEB0035D2,
            0x020B54E8: 0xE3500000,
            0x020B54FC: 0xE28D2010,
            0x020B5504: 0xEB0000D1,
            0x020B5510: 0xE12FFF1E,
            0x020B5514: 0xE3A00000,
            0x020B5520: 0xE12FFF1E,
            0x020B5524: 0xE92D000F,
            0x020B552C: 0xE5900000,
            0x020B5534: 0xE3A01020,
            0x020B5538: 0xEB0035BD,
            0x020B553C: 0xE3500000,
            0x020B5550: 0xE28D2010,
            0x020B5558: 0xEB00001E,
            0x020B5564: 0xE12FFF1E,
            0x020B5568: 0xE3A00000,
            0x020B5574: 0xE12FFF1E,
            0x020B5578: 0xE92D4038,
        }
        for address, instruction in factories.items():
            self.assertEqual(read_u32(address), instruction)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "page_factories_test"
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
            output = Path(temporary) / "page_factories.o"
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
