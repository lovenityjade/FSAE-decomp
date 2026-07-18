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
SOURCE = PROJECT_ROOT / "src/arm9/game/scrollable_tile_content.c"
HARNESS = Path(__file__).with_name("scrollable_tile_content_harness.c")


class ScrollableTileContentRecoveryTests(unittest.TestCase):
    def test_rom_body_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def body(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        bodies = {
            (0x020B5214, 0x020B523C):
                "42181c95ee7d5eb98068b55e657b404e1856bc7c96da8a4cb76c44121f6b7a27",
            (0x020B523C, 0x020B5264):
                "c60ed7f3e62f7ef367f738a4eaf32bc9bfd24c1c110afde599371cc4cd0c773f",
            (0x020B5264, 0x020B52F8):
                "8a3344dd750402a8f95f3c888b381c427ba23dab24643418cef8fca4a128ef19",
            (0x020B52F8, 0x020B5318):
                "bdd524c14070dfc257bb535fea2a907c71fbec4355ce3af3fdb00979e43fcf99",
        }
        self.assertEqual(sum(end - start for start, end in bodies), 260)
        for (start, end), expected_hash in bodies.items():
            self.assertEqual(hashlib.sha256(body(start, end)).hexdigest(), expected_hash)

        self.assertEqual(read_u32(0x020B5214), 0xE92D4008)
        self.assertEqual(read_u32(0x020B5238), 0xE8BD8008)
        self.assertEqual(read_u32(0x020B523C), 0xE92D4038)
        self.assertEqual(read_u32(0x020B5260), 0xE8BD8038)
        self.assertEqual(read_u32(0x020B5264), 0xE92D40F8)
        self.assertEqual(read_u32(0x020B52AC), 0xEB000895)
        self.assertEqual(read_u32(0x020B52D0), 0xE18050B3)
        self.assertEqual(read_u32(0x020B52F4), 0xE8BD80F8)
        self.assertEqual(read_u32(0x020B52F8), 0xE1D130FA)
        self.assertEqual(read_u32(0x020B5314), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020B5318), 0xE92D4018)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "scrollable_tile_content_test"
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
            output = Path(temporary) / "scrollable_tile_content.o"
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
