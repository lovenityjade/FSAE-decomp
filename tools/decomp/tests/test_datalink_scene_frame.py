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
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_scene_frame.c"
HARNESS = Path(__file__).with_name("datalink_scene_frame_harness.c")


class DatalinkSceneFrameRecoveryTests(unittest.TestCase):
    def test_rom_body_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        body = arm9[0x020AD87C - ARM9_BASE : 0x020AD950 - ARM9_BASE]
        self.assertEqual(len(body), 212)
        self.assertEqual(
            hashlib.sha256(body).hexdigest(),
            "8d27610ee24534607931536e50a5cfdf4c7af9c4d9b8443567f36d00b70df374",
        )
        self.assertEqual(read_u32(0x020AD878), 0xE8BD81F0)
        self.assertEqual(read_u32(0x020AD87C), 0xE92D40F8)
        self.assertEqual(read_u32(0x020AD884), 0xEB000032)
        self.assertEqual(read_u32(0x020AD88C), 0xE5D0053E)
        self.assertEqual(read_u32(0x020AD898), 0xE200007F)
        self.assertEqual(read_u32(0x020AD89C), 0xEBFF53C1)
        self.assertEqual(read_u32(0x020AD8A8), 0xE5C0153E)
        self.assertEqual(read_u32(0x020AD8AC), 0xE8BD80F8)
        self.assertEqual(read_u32(0x020AD8B0), 0xE1D502DC)
        self.assertEqual(read_u32(0x020AD8C0), 0xE5911004)
        self.assertEqual(read_u32(0x020AD8C4), 0xE12FFF31)
        self.assertEqual(read_u32(0x020AD8D0), 0xE59F4078)
        self.assertEqual(read_u32(0x020AD8E4), 0xE0276192)
        self.assertEqual(read_u32(0x020AD8F8), 0xE58230D8)
        self.assertEqual(read_u32(0x020AD908), 0xE58230DC)
        self.assertEqual(read_u32(0x020AD918), 0xE5D6053C)
        self.assertEqual(read_u32(0x020AD938), 0xEBFFFCBB)
        self.assertEqual(read_u32(0x020AD94C), 0xE8BD80F8)
        self.assertEqual(read_u32(0x020AD950), 0x0217A23C)
        self.assertEqual(read_u32(0x020AD954), 0xE2800C25)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "datalink_scene_frame_test"
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
            output = Path(temporary) / "datalink_scene_frame.o"
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
