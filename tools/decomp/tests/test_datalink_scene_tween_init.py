from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_scene_tween_init.c"
HARNESS = Path(__file__).with_name("datalink_scene_tween_init_harness.c")


class DatalinkSceneTweenInitRecoveryTests(unittest.TestCase):
    def test_rom_body_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        start = 0x020AD020
        end = 0x020AD070
        body = arm9[start - ARM9_BASE : end - ARM9_BASE]
        self.assertEqual(len(body), 80)
        self.assertEqual(
            hashlib.sha256(body).hexdigest(),
            "3aca264312928e319a39ff1939d7f73f1160867975ab9aec0a603b3626d28801",
        )
        self.assertEqual(read_u32(0x020AD01C), 0xE8BD81F0)
        self.assertEqual(read_u32(0x020AD020), 0xE92D4070)
        self.assertEqual(read_u32(0x020AD028), 0xE284003C)
        self.assertEqual(read_u32(0x020AD030), 0xE2843A02)
        self.assertEqual(read_u32(0x020AD038), 0xE2800B01)
        self.assertEqual(read_u32(0x020AD03C), 0xE3A02C21)
        self.assertEqual(read_u32(0x020AD040), 0xE5C3653C)
        self.assertEqual(read_u32(0x020AD044), 0xEBFDA2D2)
        self.assertEqual(read_u32(0x020AD050), 0xE3A04058)
        self.assertEqual(read_u32(0x020AD054), 0xE0205496)
        self.assertEqual(read_u32(0x020AD058), 0xE20610FF)
        self.assertEqual(read_u32(0x020AD05C), 0xEBFFFEE4)
        self.assertEqual(read_u32(0x020AD064), 0xE3560060)
        self.assertEqual(read_u32(0x020AD068), 0x3AFFFFF9)
        self.assertEqual(read_u32(0x020AD06C), 0xE8BD8070)
        self.assertEqual(read_u32(0x020AD070), 0xE92D4FF0)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "datalink_scene_tween_init_test"
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
                    str(SOURCE),
                    str(HARNESS),
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
            output = Path(temporary) / "datalink_scene_tween_init.o"
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
                    str(SOURCE),
                    "-o",
                    str(output),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )


if __name__ == "__main__":
    unittest.main()
