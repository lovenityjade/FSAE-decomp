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
SOURCE = PROJECT_ROOT / "src/arm9/game/manual_page_resource.c"
HARNESS = Path(__file__).with_name("manual_page_resource_harness.c")


class ManualPageResourceRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        # Complete 688-byte body: validation, local four-word build context,
        # Nintendo block walk and success return.
        expected_body = {
            0x020B4F00: 0xE92D4FF8,
            0x020B4F04: 0xE24DD020,
            0x020B4F10: 0xE59F1298,
            0x020B4F20: 0xEB000914,
            0x020B4F28: 0x028DD020,
            0x020B4F34: 0xE59D0008,
            0x020B4F48: 0xE3500002,
            0x020B4F50: 0x128DD020,
            0x020B4F5C: 0xE3A04000,
            0x020B4F78: 0xE58D9010,
            0x020B4F7C: 0xE1D010BC,
            0x020B4F80: 0xE1D000BE,
            0x020B4F94: 0xE0805001,
            0x020B4F98: 0xDA000081,
            0x020B4F9C: 0xE1D500D1,
            0x020B4FB8: 0xE59F31F4,
            0x020B4FC0: 0xE1520003,
            0x020B4FDC: 0xE59F01D4,
            0x020B4FE4: 0x058D5014,
            0x020B5004: 0xE1520000,
            0x020B5020: 0xE2830A7F,
            0x020B5034: 0xE2410B01,
            0x020B5044: 0xE2810B01,
            0x020B5058: 0xE59F0160,
            0x020B5068: 0xE58D5018,
            0x020B5070: 0xE1D510B8,
            0x020B5078: 0xE1CA11B4,
            0x020B507C: 0xEB000051,
            0x020B5084: 0xE58A0018,
            0x020B5098: 0xE59FB124,
            0x020B50AC: 0xE196C0B1,
            0x020B50B8: 0xE792C10C,
            0x020B50C4: 0xE12FFF33,
            0x020B50CC: 0xE7810107,
            0x020B50E0: 0xE28A0014,
            0x020B50EC: 0xE1D500F8,
            0x020B50F4: 0xE1D500FA,
            0x020B5100: 0xE1CA00BC,
            0x020B5108: 0xE1D500BE,
            0x020B5110: 0xE7910100,
            0x020B5118: 0xE58A0010,
            0x020B5120: 0xE28D6010,
            0x020B5130: 0xE58D6000,
            0x020B5134: 0xEB000077,
            0x020B5144: 0x058A8004,
            0x020B5150: 0xE1A00004,
            0x020B5158: 0xEB000172,
            0x020B5160: 0xE1D520B8,
            0x020B5168: 0xE1A00008,
            0x020B5170: 0xEB000150,
            0x020B5178: 0xE1A08004,
            0x020B517C: 0xE5944004,
            0x020B5184: 0xE5952004,
            0x020B5190: 0xE0855002,
            0x020B51A0: 0xBAFFFF7D,
            0x020B51A4: 0xE3A00001,
            0x020B51AC: 0xE8BD8FF8,
        }
        for address, instruction in expected_body.items():
            self.assertEqual(read_u32(address), instruction)

        # Literal pool proves every accepted block/resource signature.  The
        # independent zero-initializing resource-array helper follows it.
        expected_literals_and_allocator = {
            0x020B51B0: 0x4E545047,
            0x020B51B4: 0x70617331,
            0x020B51B8: 0x6E617031,
            0x020B51BC: 0x74787031,
            0x020B51C0: 0x776E6431,
            0x020B51C4: 0x6E747466,
            0x020B51C8: 0xE92D4010,
            0x020B51CC: 0xE1B04001,
            0x020B51D0: 0x03A01001,
            0x020B51D8: 0xE1A01101,
            0x020B51DC: 0xEB003694,
            0x020B51E0: 0xE3500000,
            0x020B51EC: 0xE3540000,
            0x020B51FC: 0xE0902103,
            0x020B5204: 0x15821000,
            0x020B5210: 0xE8BD8010,
            0x020B5214: 0xE92D4008,
        }
        for address, value in expected_literals_and_allocator.items():
            self.assertEqual(read_u32(address), value)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_page_resource_test"
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
            output = Path(temporary) / "manual_page_resource.o"
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
