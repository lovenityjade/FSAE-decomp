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


class ManualGraphicsSetupRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        self.assertEqual(read_u32(0x020BDB34), 0xE92D41F0)
        self.assertEqual(read_u32(0x020BDB54), 0xE1C350B0)
        self.assertEqual(read_u32(0x020BDB58), 0xEBFD54E3)
        self.assertEqual(read_u32(0x020BDB68), 0xEBFD5B60)
        self.assertEqual(read_u32(0x020BDB7C), 0xE3800201)
        self.assertEqual(read_u32(0x020BDBB4), 0xE3800903)
        self.assertEqual(read_u32(0x020BDBE0), 0xE3800B31)
        self.assertEqual(read_u32(0x020BDC0C), 0xE3800B22)
        self.assertEqual(read_u32(0x020BDC48), 0xEBFD5D0D)
        self.assertEqual(read_u32(0x020BDC84), 0xEBFD5B9A)
        self.assertEqual(read_u32(0x020BDC88), 0xEBFFF9C0)
        self.assertEqual(read_u32(0x020BDCA4), 0xEBFD5C1C)
        self.assertEqual(read_u32(0x020BDCC8), 0xE3A00C01)
        self.assertEqual(read_u32(0x020BDCCC), 0xEBFD5D4F)
        self.assertEqual(read_u32(0x020BDCF8), 0xEBFD549C)
        self.assertEqual(read_u32(0x020BDD04), 0xEBFD5499)
        self.assertEqual(read_u32(0x020BDD08), 0xEBFFF9B1)
        self.assertEqual(read_u32(0x020BDD48), 0xE1C603BC)
        self.assertEqual(read_u32(0x020BDD4C), 0xE8BD81F0)
        self.assertEqual(read_u32(0x020BDD50), 0x04000304)
        self.assertEqual(read_u32(0x020BDD60), 0xE92D4FF8)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_graphics_setup_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_graphics_setup.c"),
                    str(Path(__file__).with_name(
                        "manual_graphics_setup_harness.c")),
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
            output = Path(temporary) / "manual_graphics_setup.o"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_graphics_setup.c"),
                    "-o",
                    str(output),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )


if __name__ == "__main__":
    unittest.main()
