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


class ManualGraphicsRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        self.assertEqual(read_u32(0x020BDABC), 0xE92D40F8)
        self.assertEqual(read_u32(0x020BDAC0), 0xE59F0068)
        self.assertEqual(read_u32(0x020BDAC4), 0xEBFD5D50)
        self.assertEqual(read_u32(0x020BDAD8), 0xEBFD600C)
        self.assertEqual(read_u32(0x020BDADC), 0xEBFD5E59)
        self.assertEqual(read_u32(0x020BDAF4), 0xEBFD6005)
        self.assertEqual(read_u32(0x020BDB08), 0xEBFD6000)
        self.assertEqual(read_u32(0x020BDB18), 0xEBFD5FFC)
        self.assertEqual(read_u32(0x020BDB28), 0xEBFD5FF8)
        self.assertEqual(read_u32(0x020BDB2C), 0xE8BD80F8)
        self.assertEqual(read_u32(0x020BDB30), 0x000001FF)
        self.assertEqual(read_u32(0x020BDB34), 0xE92D41F0)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_graphics_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_graphics.c"),
                    str(Path(__file__).with_name("manual_graphics_harness.c")),
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
            output = Path(temporary) / "manual_graphics.o"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_graphics.c"),
                    "-o",
                    str(output),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )


if __name__ == "__main__":
    unittest.main()
