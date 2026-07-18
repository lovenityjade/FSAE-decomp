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


class CircularTileUploadRecoveryTests(unittest.TestCase):
    def test_rom_range_wrap_and_callback_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        self.assertEqual(read_u32(0x020BC444), 0xE1D501FC)
        self.assertEqual(read_u32(0x020BC448), 0xE1D532F0)
        self.assertEqual(read_u32(0x020BC450), 0xE1D572F2)
        self.assertEqual(read_u32(0x020BC4B8), 0xE1D511F6)
        # Source wraps by ring_rows-1 and destination by the low eight bits.
        self.assertEqual(read_u32(0x020BC4C4), 0xE2416001)
        self.assertEqual(read_u32(0x020BC4CC), 0xE0027006)
        self.assertEqual(read_u32(0x020BC4DC), 0xE20710FF)
        self.assertEqual(read_u32(0x020BC4E0), 0xE2619C01)
        # Row stride is in halfwords; destination offset/size are rows << 9.
        self.assertEqual(read_u32(0x020BC4E4), 0xE0000097)
        self.assertEqual(read_u32(0x020BC4F4), 0xE0820080)
        self.assertEqual(read_u32(0x020BC4F8), 0xE1A01481)
        self.assertEqual(read_u32(0x020BC4FC), 0xE1A02489)
        self.assertEqual(read_u32(0x020BC500), 0xE12FFF34)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "circular_tile_upload_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/circular_tile_upload.c"),
                    str(Path(__file__).with_name("circular_tile_upload_harness.c")),
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
