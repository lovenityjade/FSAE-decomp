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


class ManualDisplayRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_register_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        # Touch content begins at y=148 unless the TOC uses the full sub screen.
        self.assertEqual(read_u32(0x020BC574), 0xE5D010AD)
        self.assertEqual(read_u32(0x020BC578), 0xE3A00094)
        self.assertEqual(read_u32(0x020BC580), 0x13A00000)
        self.assertEqual(read_u32(0x020BC584), 0xE12FFF1E)
        # Input history clears three halfwords at +0x40616 and one at +0x4061c.
        self.assertEqual(read_u32(0x020BE2FC), 0xE59F1020)
        self.assertEqual(read_u32(0x020BE310), 0xE35C0003)
        self.assertEqual(read_u32(0x020BE324), 0x00040616)
        self.assertEqual(read_u32(0x020BE328), 0x0004061C)
        # Blend brightness always reaches sub and conditionally reaches main.
        self.assertEqual(read_u32(0x020BE338), 0xE5D000AD)
        self.assertEqual(read_u32(0x020BE368), 0x04000050)
        self.assertEqual(read_u32(0x020BE36C), 0x04001050)
        # Reveal windows write H/V pairs to 0x04000040 and 0x04001040.
        self.assertEqual(read_u32(0x020BE3A8), 0xE1C100B0)
        self.assertEqual(read_u32(0x020BE3B0), 0xE1C100B4)
        self.assertEqual(read_u32(0x020BE3C8), 0xE1C100B0)
        self.assertEqual(read_u32(0x020BE3D0), 0xE1C100B4)
        self.assertEqual(read_u32(0x020BE3D8), 0x04000040)
        self.assertEqual(read_u32(0x020BE3DC), 0x04001040)
        # Scroll indicator switches between heights 0xae and 0x142.
        self.assertEqual(read_u32(0x020BE3EC), 0xE3A040AE)
        self.assertEqual(read_u32(0x020BE3F4), 0x059F4034)
        self.assertEqual(read_u32(0x020BE430), 0x00000142)
        # Shutdown stops touch/sound/display and restores captured subsystems.
        self.assertEqual(read_u32(0x020BE24C), 0xEBFD7863)
        self.assertEqual(read_u32(0x020BE250), 0xE3A05004)
        self.assertEqual(read_u32(0x020BE26C), 0xE59F2080)
        self.assertEqual(read_u32(0x020BE278), 0xE3C11801)
        self.assertEqual(read_u32(0x020BE280), 0xE5D00622)
        self.assertEqual(read_u32(0x020BE29C), 0xE5D00623)
        self.assertEqual(read_u32(0x020BE2B4), 0xE5D00624)
        self.assertEqual(read_u32(0x020BE2F4), 0x04001000)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_display_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_display.c"),
                    str(Path(__file__).with_name("manual_display_harness.c")),
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
