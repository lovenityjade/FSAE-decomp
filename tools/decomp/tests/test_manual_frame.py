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


class ManualFrameRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        self.assertEqual(read_u32(0x020BD00C), 0xE92D4078)
        self.assertEqual(read_u32(0x020BD01C), 0xEB00073C)
        self.assertEqual(read_u32(0x020BD020), 0xE2840E37)
        self.assertEqual(read_u32(0x020BD034), 0xEB00058C)
        self.assertEqual(read_u32(0x020BD04C), 0xEBFFEDA8)
        self.assertEqual(read_u32(0x020BD05C), 0xEBFFEE3E)
        self.assertEqual(read_u32(0x020BD084), 0xEB000745)
        self.assertEqual(read_u32(0x020BD090), 0xEB0005C0)
        self.assertEqual(read_u32(0x020BD09C), 0xEBFD6D05)
        self.assertEqual(read_u32(0x020BD0B4), 0xEBFFEDA9)
        self.assertEqual(read_u32(0x020BD0C0), 0xEBFFEE27)
        self.assertEqual(read_u32(0x020BD0CC), 0xEBFFEE24)
        self.assertEqual(read_u32(0x020BD0D4), 0xEBFFF6F7)
        self.assertEqual(read_u32(0x020BD0F8), 0xE12FFF33)
        self.assertEqual(read_u32(0x020BD118), 0xE12FFF33)
        self.assertEqual(read_u32(0x020BD124), 0x0004038C)
        self.assertEqual(read_u32(0x020BD128), 0xE92D43F8)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_frame_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_frame.c"),
                    str(Path(__file__).with_name("manual_frame_harness.c")),
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
