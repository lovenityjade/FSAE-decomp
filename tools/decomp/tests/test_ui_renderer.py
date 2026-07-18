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


class UiRendererRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_layout_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        # Init clears fields, 0x2c00 animation bytes, manager, and 0x400 OAMs.
        self.assertEqual(read_u32(0x020BBF6C), 0xE3A02B0B)
        self.assertEqual(read_u32(0x020BBF84), 0xE5834C14)
        self.assertEqual(read_u32(0x020BBF94), 0xE3A00007)
        self.assertEqual(read_u32(0x020BBFC0), 0xE3A02B01)
        # Background loader formats one palette path, loads it to both engines,
        # and publishes three screen layers numbered 0, 1 and 2.
        self.assertEqual(read_u32(0x020BBFF4), 0xE3A01040)
        self.assertEqual(read_u32(0x020BC020), 0xE1A02005)
        self.assertEqual(read_u32(0x020BC03C), 0xE1A02004)
        self.assertEqual(read_u32(0x020BC070), 0xE1A00006)
        self.assertEqual(read_u32(0x020BC090), 0xE1A00005)
        self.assertEqual(read_u32(0x020BC0B8), 0xE1A00004)
        # Main/sub setups select engines 0/1 and mapping targets 1/2.
        self.assertEqual(read_u32(0x020BC100), 0xE1A03005)
        self.assertEqual(read_u32(0x020BC108), 0xE3A02080)
        self.assertEqual(read_u32(0x020BC154), 0xE3A02001)
        self.assertEqual(read_u32(0x020BC194), 0xE3A03001)
        self.assertEqual(read_u32(0x020BC1E0), 0xE3A02002)
        self.assertEqual(read_u32(0x020BC1FC), 0xE12FFF1E)

        # Create reads count +0x2c14, uses 0x58-byte records, and reserves 127.
        self.assertEqual(read_u32(0x020BC20C), 0xE5902C14)
        self.assertEqual(read_u32(0x020BC218), 0xE3500080)
        self.assertEqual(read_u32(0x020BC220), 0xE3A00058)
        self.assertEqual(read_u32(0x020BC228), 0xE5950010)
        self.assertEqual(read_u32(0x020BC23C), 0xE595200C)
        self.assertEqual(read_u32(0x020BC254), 0xE5801C14)

        # The handle resolver is a signed count comparison and MLA stride.
        self.assertEqual(read_u32(0x020BC264), 0xE5922C14)
        self.assertEqual(read_u32(0x020BC268), 0xE1510002)
        self.assertEqual(read_u32(0x020BC274), 0xB0202091)

        # Tick and frame wrappers both return zero for invalid handles.
        self.assertEqual(read_u32(0x020BC288), 0xEBFFFFF4)
        self.assertEqual(read_u32(0x020BC2B0), 0xEBFFFFEA)
        self.assertEqual(read_u32(0x020BC2A0), 0xE3A00001)
        self.assertEqual(read_u32(0x020BC2C8), 0xE3A00001)

        # Submit preserves x/y, reads current cell +0x30, and rejects >= capacity.
        self.assertEqual(read_u32(0x020BC2DC), 0xE1A06002)
        self.assertEqual(read_u32(0x020BC2E0), 0xE1A05003)
        self.assertEqual(read_u32(0x020BC2EC), 0x15902030)
        self.assertEqual(read_u32(0x020BC318), 0xE3A01080)
        self.assertEqual(read_u32(0x020BC334), 0xE1550000)
        self.assertEqual(read_u32(0x020BC338), 0x2A000008)

        # Flush applies the OAM manager and then resets the same +0x2c18 object.
        self.assertEqual(read_u32(0x020BC374), 0xE2840018)
        self.assertEqual(read_u32(0x020BC378), 0xE2800B0B)
        self.assertEqual(read_u32(0x020BC380), 0xE2840018)
        self.assertEqual(read_u32(0x020BC384), 0xE2800B0B)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "ui_renderer_test"
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
                    str(PROJECT_ROOT / "src/arm9/ntmv/owner_info.c"),
                    str(PROJECT_ROOT / "src/arm9/ntmv/ui_renderer.c"),
                    str(Path(__file__).with_name("ui_renderer_harness.c")),
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
