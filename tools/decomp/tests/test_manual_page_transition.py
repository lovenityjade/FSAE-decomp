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


class ManualPageTransitionRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        self.assertEqual(read_u32(0x020BD128), 0xE92D43F8)
        self.assertEqual(read_u32(0x020BD13C), 0xEBFFFC8A)
        self.assertEqual(read_u32(0x020BD14C), 0xEBFFF768)
        self.assertEqual(read_u32(0x020BD158), 0xEBFD5656)
        self.assertEqual(read_u32(0x020BD184), 0xEBFD57A9)
        self.assertEqual(read_u32(0x020BD1A4), 0xEBFD57A1)
        self.assertEqual(read_u32(0x020BD1B4), 0xE2000A0E)
        self.assertEqual(read_u32(0x020BD1D4), 0x03C00A0E)
        self.assertEqual(read_u32(0x020BD218), 0x03800A02)
        self.assertEqual(read_u32(0x020BD23C), 0xEB00044B)
        self.assertEqual(read_u32(0x020BD260), 0xEBFFFC75)
        self.assertEqual(read_u32(0x020BD280), 0xEBFFFC6D)
        self.assertEqual(read_u32(0x020BD294), 0xE18630B1)
        self.assertEqual(read_u32(0x020BD2A0), 0xE18610B0)
        self.assertEqual(read_u32(0x020BD2A8), 0xE8BD83F8)
        self.assertEqual(read_u32(0x020BD2AC), 0x0004038C)
        self.assertEqual(read_u32(0x020BD2C0), 0xE59FC000)
        self.assertEqual(read_u32(0x020BD2CC), 0xE59FC000)
        self.assertEqual(read_u32(0x020BD2D8), 0xE92D4078)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_page_transition_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_page_transition.c"),
                    str(Path(__file__).with_name(
                        "manual_page_transition_harness.c")),
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
            output = Path(temporary) / "manual_page_transition.o"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_page_transition.c"),
                    "-o",
                    str(output),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )


if __name__ == "__main__":
    unittest.main()
