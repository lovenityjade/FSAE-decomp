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


class ManualViewerLifecycleRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        self.assertEqual(read_u32(0x020BC588), 0xE92D4070)
        self.assertEqual(read_u32(0x020BC5A8), 0xEB0009AF)
        self.assertEqual(read_u32(0x020BC5D0), 0xEB0007A6)
        self.assertEqual(read_u32(0x020BC5FC), 0xEB000917)
        self.assertEqual(read_u32(0x020BC608), 0xEBFFEB02)
        self.assertEqual(read_u32(0x020BC614), 0xEBFFEAFF)
        self.assertEqual(read_u32(0x020BC620), 0xE5824608)
        self.assertEqual(read_u32(0x020BC658), 0xE5C24620)
        self.assertEqual(read_u32(0x020BC680), 0xBAFFFFFA)
        self.assertEqual(read_u32(0x020BC698), 0xBAFFFFFA)
        self.assertEqual(read_u32(0x020BC6B0), 0xBAFFFFFA)
        self.assertEqual(read_u32(0x020BC6B8), 0xE8BD8070)
        self.assertEqual(read_u32(0x020BC6BC), 0x000403A0)
        self.assertEqual(read_u32(0x020BC6C0), 0x0004060E)
        self.assertEqual(read_u32(0x020BC6C4), 0xE92D4078)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_viewer_lifecycle_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_viewer_lifecycle.c"),
                    str(Path(__file__).with_name(
                        "manual_viewer_lifecycle_harness.c")),
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
            output = Path(temporary) / "manual_viewer_lifecycle.o"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_viewer_lifecycle.c"),
                    "-o",
                    str(output),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )


if __name__ == "__main__":
    unittest.main()
