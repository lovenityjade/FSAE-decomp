from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import struct
import subprocess
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_glyph_layout.c"

FUNCTIONS = (
    (0x020AE280, 16, "92963632ebcbda6ce0ff56ce3b0df65aed84a070bfa5240010f40ef48984a0ea"),
    (0x020AE294, 28, "61a40d19bc8ebda0c9a0c1ffcae2360e961910bda39089b20930cfd0c632ad69"),
    (0x020AE2B8, 36, "0ae14197927253d03930daa4b36144310eb7c1869a5c25da7efc0d0bf5520978"),
    (0x020AE318, 28, "04618e933b8a5f713d38d61e92eb32d72b09581d773a2de4de2af7a9d3ae24e8"),
    (0x020AE338, 28, "04618e933b8a5f713d38d61e92eb32d72b09581d773a2de4de2af7a9d3ae24e8"),
    (0x020AE358, 88, "345dbf1f74bf7a35279f54fb5631cf9d670cba7cbab5115ccc69a41b4bfa3150"),
    (0x020AE3B0, 244, "8bdf4eb172722b02525052d9fcd809f2607b00bc9396e9fbde65254bcb139f57"),
    (0x020AE4A4, 152, "9e800fe14fd74af3ee381260497ea25a3e80eafb53bb5b7d4ad2cc28b9cd9fc6"),
    (0x020AE540, 16, "114f9eb2f70cd81e3b468c0c338ca7f12e2a23562a531fed0b0faf2d3f8d8305"),
    (0x020AE558, 96, "b1589a5ceb3c6acbd042967f01948cdddc175489b93b2161190a0613762c4052"),
    (0x020AE5BC, 112, "27839226787e6d30e739aa2b0e39c77726340cf195dfe499c8c76dcb81db89a9"),
    (0x020AE634, 228, "5b0ce09dd9960fe02e7240090205e7dd582c10de10c235fb451b809aa1ba6979"),
    (0x020AE72C, 120, "afe8e16519d08f72aa15bea2f276fb3f0234db69727f417726cde7493d7f55d5"),
    (0x020AE7A8, 120, "fc6aba3cb14b20713f4525a1e401b0a02226d129d739e502ef856a002708b03d"),
    (0x020AE830, 128, "04d38ada93c877609e7ec8d9a2f6df9af6903f5ce4ac90931b03b872c1cd909a"),
    (0x020AE8C0, 68, "da68e78b836ed7094ad91b81d869cdb6187bc65efac343b170786573ed25a296"),
    (0x020AE908, 68, "4ede3fdeea248caabc939d8b54a47eae515022e9891ca151e74d47b4c03230c4"),
    (0x020AE954, 16, "7a404bc37d665e6b0d83aa12dfedd64b335e53a18546396ec7add6d4eeebc8c4"),
    (0x020AE968, 176, "9a036b6d6aa04f77e0dc520317ffe6277a4bacf3440ce64a5d9f207a13e03f34"),
)


class DatalinkGlyphLayoutRecoveryTests(unittest.TestCase):
    def test_rom_bodies_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        for address, size, expected_sha256 in FUNCTIONS:
            with self.subTest(address=f"0x{address:08X}"):
                start = address - ARM9_BASE
                recovered = arm9[start : start + size]
                self.assertEqual(len(recovered), size)
                self.assertEqual(
                    hashlib.sha256(recovered).hexdigest(),
                    expected_sha256,
                )

        self.assertEqual(read_u32(0x020AE280), 0xE59F0008)
        self.assertEqual(read_u32(0x020AE28C), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020AE318), 0xE5912000)
        self.assertEqual(read_u32(0x020AE338), 0xE5912000)
        self.assertEqual(read_u32(0x020AE3B0), 0xE92D41F0)
        self.assertEqual(read_u32(0x020AE4A0), 0xE8BD81F0)
        self.assertEqual(read_u32(0x020AE4A4), 0xE92D4008)
        self.assertEqual(read_u32(0x020AE538), 0xE8BD8008)

    def test_host_c_syntax(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
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
                "-fsyntax-only",
                str(SOURCE),
            ],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )

    def test_arm946es_syntax(self) -> None:
        compiler = shutil.which("clang")
        if compiler is None:
            self.skipTest("clang is unavailable")
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
                "-fsyntax-only",
                str(SOURCE),
            ],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )


if __name__ == "__main__":
    unittest.main()
