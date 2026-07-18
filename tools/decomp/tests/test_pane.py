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
SOURCE = PROJECT_ROOT / "src/arm9/ntmv/m2d/pane.c"
HARNESS = Path(__file__).with_name("pane_harness.c")


class PaneRecoveryTests(unittest.TestCase):
    def test_rom_tree_helper_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        # Five independent bodies: 24 + 36 + 76 + 28 + 132 = 296 bytes.
        expected = {
            0x020B56A0: 0xE92D4008,
            0x020B56A4: 0xE3510000,
            0x020B56A8: 0x08BD8008,
            0x020B56AC: 0xE5900000,
            0x020B56B0: 0xEB003564,
            0x020B56B4: 0xE8BD8008,
            0x020B56B8: 0xE92D4038,
            0x020B56BC: 0xE1A04002,
            0x020B56C4: 0xE1A00001,
            0x020B56CC: 0xEB000002,
            0x020B56D0: 0xE5850008,
            0x020B56D4: 0xE1C541BA,
            0x020B56D8: 0xE8BD8038,
            0x020B56DC: 0xE92D4010,
            0x020B56E0: 0xE1B04001,
            0x020B56E4: 0x03A01001,
            0x020B56E8: 0xE5900000,
            0x020B56EC: 0xE1A01101,
            0x020B56F0: 0xEB00354F,
            0x020B56F4: 0xE3500000,
            0x020B56FC: 0x08BD8010,
            0x020B5700: 0xE3540000,
            0x020B5708: 0x98BD8010,
            0x020B5710: 0xE0902103,
            0x020B5718: 0x15821000,
            0x020B5720: 0x3AFFFFFA,
            0x020B5724: 0xE8BD8010,
            0x020B5728: 0xE1D0C1B8,
            0x020B572C: 0xE5902008,
            0x020B5730: 0xE28C3001,
            0x020B5734: 0xE1C031B8,
            0x020B5738: 0xE782110C,
            0x020B573C: 0xE5810004,
            0x020B5740: 0xE12FFF1E,
            0x020B5744: 0xE92D40F8,
            0x020B5748: 0xE24DD008,
            0x020B574C: 0xE1D230F0,
            0x020B5758: 0xE1CD30B4,
            0x020B5760: 0xE28D2004,
            0x020B5768: 0xE5903000,
            0x020B576C: 0xE5933008,
            0x020B5770: 0xE12FFF33,
            0x020B5774: 0xE1D601B8,
            0x020B5780: 0xD28DD008,
            0x020B5784: 0xD8BD80F8,
            0x020B5788: 0xE28D7000,
            0x020B578C: 0xE1D601F0,
            0x020B5798: 0xE1CD00B0,
            0x020B57A4: 0xE5960008,
            0x020B57A8: 0xE7900104,
            0x020B57AC: 0xEBFFFFE4,
            0x020B57B0: 0xE1D601B8,
            0x020B57BC: 0xBAFFFFF2,
            0x020B57C0: 0xE28DD008,
            0x020B57C4: 0xE8BD80F8,
            0x020B57C8: 0xE1D0C0FC,
            0x020B57CC: 0xE1D030FE,
            0x020B57D0: 0xE1C0C1B0,
            0x020B57D4: 0xE1C031B2,
            0x020B57D8: 0xE1D210F2,
            0x020B57DC: 0xE1D220F0,
            0x020B57E0: 0xE0831001,
            0x020B57E4: 0xE08C2002,
            0x020B57E8: 0xE1C021B0,
            0x020B57EC: 0xE1C011B2,
            0x020B57F0: 0xE12FFF1E,
            0x020B57F4: 0xE92D4070,
            0x020B57F8: 0xE5902000,
            0x020B57FC: 0xE1A06000,
            0x020B5800: 0xE592200C,
            0x020B5804: 0xE1A05001,
            0x020B5808: 0xE12FFF32,
            0x020B580C: 0xE1D601B8,
            0x020B5810: 0xE3A04000,
            0x020B5814: 0xE3500000,
            0x020B5818: 0xD8BD8070,
            0x020B581C: 0xE5960008,
            0x020B5820: 0xE1A01005,
            0x020B5824: 0xE7900104,
            0x020B5828: 0xEBFFFFF1,
            0x020B582C: 0xE1D601B8,
            0x020B5830: 0xE2844001,
            0x020B5834: 0xE1540000,
            0x020B5838: 0xBAFFFFF7,
            0x020B583C: 0xE8BD8070,
            0x020B5840: 0xE12FFF1E,
            0x020B5844: 0xE59F0000,
            0x020B5848: 0xE12FFF1E,
            0x020B584C: 0x02126A60,
            0x020B5850: 0xE92D4070,
        }
        for address, instruction in expected.items():
            self.assertEqual(read_u32(address), instruction)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")

        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "pane_test"
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
            output = Path(temporary) / "pane.o"
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
