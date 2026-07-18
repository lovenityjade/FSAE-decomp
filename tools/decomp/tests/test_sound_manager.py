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


class SoundManagerRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        self.assertEqual(read_u32(0x020BEC70), 0xE5801000)
        self.assertEqual(read_u32(0x020BEC74), 0xE5801004)
        # Init uses archive +0x10 and 16 entries at +0xb4, stride 0x18.
        self.assertEqual(read_u32(0x020BEC94), 0xE2840010)
        self.assertEqual(read_u32(0x020BECA8), 0xE28480B4)
        self.assertEqual(read_u32(0x020BECB4), 0xE3A05018)
        self.assertEqual(read_u32(0x020BECD8), 0xE3590010)
        self.assertEqual(read_u32(0x020BECF4), 0xE5846240)
        # Tick honors one-frame keep-alive before stopping pending entries.
        self.assertEqual(read_u32(0x020BED54), 0xE59100C0)
        self.assertEqual(read_u32(0x020BED60), 0xE59100BC)
        self.assertEqual(read_u32(0x020BED88), 0xE58140C0)
        # Play checks the sequence-arc range, reuses/allocates and starts it.
        self.assertEqual(read_u32(0x020BEDC4), 0xE5993008)
        self.assertEqual(read_u32(0x020BEE20), 0xE1A02008)
        self.assertEqual(read_u32(0x020BEE28), 0xE3570080)
        self.assertEqual(read_u32(0x020BEE50), 0xE2841CFF)
        self.assertEqual(read_u32(0x020BEE54), 0xE2422080)
        # Allocation is round-robin across exactly 16 0x18-byte slots.
        self.assertEqual(read_u32(0x020BEEDC), 0xE5983240)
        self.assertEqual(read_u32(0x020BEF00), 0xE3A04010)
        self.assertEqual(read_u32(0x020BEF04), 0xE3A09018)
        # Helpers implement wrap, inclusive clamp, and half-open range.
        self.assertEqual(read_u32(0x020BEF68), 0xE0530002)
        self.assertEqual(read_u32(0x020BEF98), 0xE1510002)
        self.assertEqual(read_u32(0x020BEFB4), 0xE1510002)
        self.assertEqual(read_u32(0x020BEFC0), 0xB3A00001)
        # Handle init/stop wrappers preserve the SDK handle ABI.
        self.assertEqual(read_u32(0x020BF89C), 0xE5900000)
        self.assertEqual(read_u32(0x020BF8A4), 0xE12FFF1C)
        self.assertEqual(read_u32(0x020BF954), 0xE3A01000)
        # Per-entry setters resolve the SDK player number from handle +0x3c.
        self.assertEqual(read_u32(0x020BF9C0), 0xE5902000)
        self.assertEqual(read_u32(0x020BF9D0), 0xE5D0003C)
        self.assertEqual(read_u32(0x020BFA00), 0xE5902000)
        self.assertEqual(read_u32(0x020BFA10), 0xE5D0003C)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "sound_manager_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/sound_manager.c"),
                    str(Path(__file__).with_name("sound_manager_harness.c")),
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
