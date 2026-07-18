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


class GraphicsStateRecoveryTests(unittest.TestCase):
    def test_rom_register_and_call_order_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        self.assertEqual(read_u32(0x020BC390), 0xE59F0000)
        self.assertEqual(read_u32(0x020BC394), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020BC398), 0x00100010)
        # The saved default DMA is accepted only for encoded channels 0..7.
        self.assertEqual(read_u32(0x020BC528), 0xE59F0040)
        self.assertEqual(read_u32(0x020BC530), 0xE5900000)
        self.assertEqual(read_u32(0x020BC534), 0xE3500003)
        self.assertEqual(read_u32(0x020BC540), 0x93A00001)
        self.assertEqual(read_u32(0x020BC550), 0xE3500007)
        self.assertEqual(read_u32(0x020BC560), 0xE3500003)
        self.assertEqual(read_u32(0x020BC568), 0x83A00000)
        self.assertEqual(read_u32(0x020BC56C), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020BC570), 0x020DFB80)
        # One brightness argument is sent to 0x0400006c then base + 0x1000.
        self.assertEqual(read_u32(0x020BC3AC), 0xE59F401C)
        self.assertEqual(read_u32(0x020BC3C4), 0xE2840A01)
        self.assertEqual(read_u32(0x020BC3D0), 0x0400006C)
        # Fixed blend: main register, plane masks 1/0x3e and coefficients 6/10.
        self.assertEqual(read_u32(0x020BC3E0), 0xE3A01001)
        self.assertEqual(read_u32(0x020BC3E4), 0xE3A0203E)
        self.assertEqual(read_u32(0x020BC3E8), 0xE3A03006)
        self.assertEqual(read_u32(0x020BC3F8), 0x04000050)
        # The same two-byte stack color goes to main and sub at offset zero.
        self.assertEqual(read_u32(0x020BC410), 0xE1CD00B0)
        self.assertEqual(read_u32(0x020BC418), 0xE1A01005)
        self.assertEqual(read_u32(0x020BC420), 0xEBFD5C56)
        self.assertEqual(read_u32(0x020BC430), 0xEBFD5C78)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "graphics_state_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/graphics_state.c"),
                    str(Path(__file__).with_name("graphics_state_harness.c")),
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
