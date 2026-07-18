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
SOURCE = PROJECT_ROOT / "src/arm9/ntmv/m2d/blit.c"
HARNESS = Path(__file__).with_name("blit_harness.c")


class BlitRecoveryTests(unittest.TestCase):
    def test_rom_body_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        start = 0x020B786C - ARM9_BASE
        body = arm9[start : start + 656]
        self.assertEqual(len(body), 656)
        self.assertEqual(
            hashlib.sha256(body).hexdigest(),
            "08484e75c3a87458df0046f5a5a93396952c65c66e8edccbd678a6351eb7d484",
        )

        # The preceding TexelPicker literal belongs to DrawInlineObject.
        self.assertEqual(read_u32(0x020B7864), 0xE8BD87F8)
        self.assertEqual(read_u32(0x020B7868), 0x02126C18)
        self.assertEqual(read_u32(0x020B786C), 0xE92D4FF0)
        self.assertEqual(read_u32(0x020B7870), 0xE24DD054)
        self.assertEqual(read_u32(0x020B78B8), 0xEBFFFEF2)
        self.assertEqual(read_u32(0x020B79C0), 0xEBFFFED8)
        self.assertEqual(read_u32(0x020B79FC), 0xEBFFFEC1)
        self.assertEqual(read_u32(0x020B7A88), 0xE12FFF32)
        self.assertEqual(read_u32(0x020B7AF8), 0xE8BD8FF0)

        # Its own TexelPicker-vtable literal and the next independent owner
        # information accessor are both excluded from the 656 body bytes.
        self.assertEqual(read_u32(0x020B7AFC), 0x02126C18)
        self.assertEqual(read_u32(0x020B7B00), 0xE92D4000)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "blit_test"
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
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/transform.c"),
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/texel_picker.c"),
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/render_surface.c"),
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
            output = Path(temporary) / "blit.o"
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
