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
SOURCE = PROJECT_ROOT / "src/arm9/ntmv/m2d/text_renderer.c"
HARNESS = Path(__file__).with_name("text_renderer_harness.c")


class TextRendererRecoveryTests(unittest.TestCase):
    def test_rom_body_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def body(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        # Five independent bodies total 872 + 40 + 44 + 280 + 328 = 1564 bytes.
        bodies = {
            (0x020B64FC, 0x020B6864):
                "d23e6c23a0f31490682cef6de257b19f630973c3e16fef0da69b0ea4cababe46",
            (0x020B6870, 0x020B6898):
                "8dcefe2553ae0dcc5f92711ce03eb6908366404e2a431031d8a02bd5707abb05",
            (0x020B6898, 0x020B68C4):
                "286116f6c1e3ec84d7b323c4ccfe6619e219b4db8b8bfa21b68ff6946e3bcc2e",
            (0x020B68C8, 0x020B69E0):
                "e4cadb82c3994dcdbfa994a35dc11f7bdd21d072a5f880489e5004d53ee8b226",
            (0x020B69F4, 0x020B6B3C):
                "884a7333b199904998d4b64bddf2ef9846579da8caf06f79beefc2cebbada879",
        }
        self.assertEqual(sum(end - start for start, end in bodies), 1564)
        for (start, end), expected_hash in bodies.items():
            recovered_body = body(start, end)
            self.assertEqual(
                hashlib.sha256(recovered_body).hexdigest(), expected_hash
            )

        # Prove the precise neighbours excluded from the recovered bodies:
        # vtable pointer before the bitmap draw, the 1/3 multiply literal,
        # no-op operation, renderer-operations literal, invalid-index literal,
        # a distinct SDK accessor, and the next text-drawing function.
        self.assertEqual(read_u32(0x020B64F8), 0x02126AD4)
        self.assertEqual(read_u32(0x020B6864), 0x55555556)
        self.assertEqual(read_u32(0x020B6868), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020B686C), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020B68C4), 0x020DEED4)
        self.assertEqual(read_u32(0x020B69E0), 0x0000FFFF)
        self.assertEqual(read_u32(0x020B69E4), 0xE880000E)
        self.assertEqual(read_u32(0x020B69F0), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020B6B3C), 0xE92D43F8)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "text_renderer_test"
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
            output = Path(temporary) / "text_renderer.o"
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
