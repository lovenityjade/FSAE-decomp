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


class ScrollableTileBufferRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        # Init clears the complete 0x29-byte state through +0x28.
        self.assertEqual(read_u32(0x020BE470), 0xE3A01000)
        self.assertEqual(read_u32(0x020BE4B0), 0xE5C01028)
        self.assertEqual(read_u32(0x020BE4B4), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020BE4B8), 0xE12FFF1E)
        # Content installation resets the range and refreshes its canvas.
        self.assertEqual(read_u32(0x020BE4CC), 0xE5841004)
        self.assertEqual(read_u32(0x020BE4D8), 0xEB000002)
        self.assertEqual(read_u32(0x020BE4E0), 0xEB00009A)
        # RequestStep moves four rows and anchors its downward target to height.
        self.assertEqual(read_u32(0x020BE55C), 0xE2400004)
        self.assertEqual(read_u32(0x020BE578), 0xE2811004)
        self.assertEqual(read_u32(0x020BE584), 0xE0410002)
        # ApplyDelta clamps one update to +/-0x40.
        self.assertEqual(read_u32(0x020BE5E4), 0xE2441040)
        self.assertEqual(read_u32(0x020BE630), 0xE3550040)
        self.assertEqual(read_u32(0x020BE634), 0xC3A05040)
        # Tick only advances the upload cursor after a 0x20-row distance.
        self.assertEqual(read_u32(0x020BE6D8), 0xE3520020)
        self.assertEqual(read_u32(0x020BE6E4), 0xC3A00020)
        self.assertEqual(read_u32(0x020BE6E8), 0xD3E0001F)
        # Extent uses a minimum content height of 0xae.
        self.assertEqual(read_u32(0x020BE89C), 0xE35100AE)
        self.assertEqual(read_u32(0x020BE8A4), 0xB3A010AE)
        self.assertEqual(read_u32(0x020BE8AC), 0xE24000AE)
        self.assertEqual(read_u32(0x020BE8E0), 0xE5801018)
        # Full/partial content refreshes share the 0x24-byte draw descriptor.
        self.assertEqual(read_u32(0x020BE724), 0xE5950010)
        self.assertEqual(read_u32(0x020BE73C), 0xE5950008)
        self.assertEqual(read_u32(0x020BE744), 0xE5950004)
        self.assertEqual(read_u32(0x020BE748), 0xE584001C)
        self.assertEqual(read_u32(0x020BE784), 0xE5950004)
        self.assertEqual(read_u32(0x020BE83C), 0xEBFFDA7E)
        # The filler repeats the configured x/y tile step below content.
        self.assertEqual(read_u32(0x020BE940), 0xE1D1B0F2)
        self.assertEqual(read_u32(0x020BE944), 0xE1D170F0)
        self.assertEqual(read_u32(0x020BE9A8), 0xEBFFE35B)
        self.assertEqual(read_u32(0x020BE9E4), 0xE5900004)
        self.assertEqual(read_u32(0x020BE9E8), 0xE5900010)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "scrollable_tile_buffer_test"
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
                    str(
                        PROJECT_ROOT
                        / "src/arm9/game/scrollable_tile_buffer.c"
                    ),
                    str(
                        Path(__file__).with_name(
                            "scrollable_tile_buffer_harness.c"
                        )
                    ),
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
