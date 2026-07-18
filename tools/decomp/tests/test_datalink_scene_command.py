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
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_scene_command.c"
HARNESS = Path(__file__).with_name("datalink_scene_command_harness.c")


class DatalinkSceneCommandRecoveryTests(unittest.TestCase):
    def test_rom_bodies_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def body(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        reset = body(0x020AD954, 0x020AD964)
        apply = body(0x020AD964, 0x020AD9A4)
        self.assertEqual(len(reset), 16)
        self.assertEqual(len(apply), 64)
        self.assertEqual(
            hashlib.sha256(reset).hexdigest(),
            "48442d8acafef557015903fd5ae079bbddac23b126b07954bb9d0e5ce2444204",
        )
        self.assertEqual(
            hashlib.sha256(apply).hexdigest(),
            "ac440ba0ab27b5f8f05ed239b81ec6901427546b37ae04b7eab37095b59aece8",
        )

        self.assertEqual(read_u32(0x020AD950), 0x0217A23C)
        self.assertEqual(read_u32(0x020AD954), 0xE2800C25)
        self.assertEqual(read_u32(0x020AD960), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020AD964), 0xE92D4010)
        self.assertEqual(read_u32(0x020AD968), 0xE1A04000)
        self.assertEqual(read_u32(0x020AD96C), 0xE284003C)
        self.assertEqual(read_u32(0x020AD970), 0xE2802B01)
        self.assertEqual(read_u32(0x020AD974), 0xE3A00058)
        self.assertEqual(read_u32(0x020AD978), 0xE0222091)
        self.assertEqual(read_u32(0x020AD97C), 0xE2840A02)
        self.assertEqual(read_u32(0x020AD980), 0xE5900544)
        self.assertEqual(read_u32(0x020AD984), 0xE3A01040)
        self.assertEqual(read_u32(0x020AD988), 0xEBFFFBC9)
        self.assertEqual(read_u32(0x020AD98C), 0xE2841C25)
        self.assertEqual(read_u32(0x020AD990), 0xE1D12AB8)
        self.assertEqual(read_u32(0x020AD994), 0xE1A00800)
        self.assertEqual(read_u32(0x020AD998), 0xE0820820)
        self.assertEqual(read_u32(0x020AD99C), 0xE1C10AB8)
        self.assertEqual(read_u32(0x020AD9A0), 0xE8BD8010)
        self.assertEqual(read_u32(0x020AD9A4), 0xE92D41F0)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "datalink_scene_command_test"
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
            output = Path(temporary) / "datalink_scene_command.o"
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
