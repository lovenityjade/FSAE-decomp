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
SOURCE = PROJECT_ROOT / "src/arm9/ntmv/m2d/inline_object.c"
HARNESS = Path(__file__).with_name("inline_object_harness.c")


class InlineObjectRecoveryTests(unittest.TestCase):
    def test_rom_body_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def body(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        recovered_body = body(0x020B771C, 0x020B7868)
        self.assertEqual(len(recovered_body), 332)
        self.assertEqual(
            hashlib.sha256(recovered_body).hexdigest(),
            "df2eac45b8572c5370602c012a55a0780f13e1200130a6da1feef5e09219e45a",
        )

        self.assertEqual(read_u32(0x020B771C), 0xE92D47F8)
        self.assertEqual(read_u32(0x020B7720), 0xE24DD02C)
        self.assertEqual(read_u32(0x020B7768), 0xEBFFFF46)
        self.assertEqual(read_u32(0x020B77B4), 0xEBFFFF5B)
        self.assertEqual(read_u32(0x020B77E8), 0xEBFFFF46)
        self.assertEqual(read_u32(0x020B781C), 0xE3100902)
        self.assertEqual(read_u32(0x020B782C), 0x118900B1)
        self.assertEqual(read_u32(0x020B7864), 0xE8BD87F8)

        # The recovered function stops before the detail::TexelPicker vtable
        # literal and the neighbouring transformed blitter.
        self.assertEqual(read_u32(0x020B7868), 0x02126C18)
        self.assertEqual(read_u32(0x020B786C), 0xE92D4FF0)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "inline_object_test"
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
            output = Path(temporary) / "inline_object.o"
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
