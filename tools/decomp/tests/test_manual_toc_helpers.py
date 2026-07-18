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
SOURCE = PROJECT_ROOT / "src/arm9/ntmv/m2d/manual_toc_helpers.c"
HARNESS = Path(__file__).with_name("manual_toc_helpers_harness.c")


class ManualTocHelpersRecoveryTests(unittest.TestCase):
    def test_rom_bodies_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def body(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        hit_scan = body(0x020B8E88, 0x020B8EF8)
        initial_child = body(0x020B8EF8, 0x020B8F0C)
        self.assertEqual(len(hit_scan) + len(initial_child), 132)
        self.assertEqual(
            hashlib.sha256(hit_scan).hexdigest(),
            "3b1b6d47e8c48c136b8ef5d80af2150dd0284e0d490604d150d26ef0de8f55bc",
        )
        self.assertEqual(
            hashlib.sha256(initial_child).hexdigest(),
            "63d4436dfb0a49558ae60924571dd122bc41bff050aedc7610db2cb8092be9d1",
        )

        # The 0x020b8398 sibling differs only at the PC-relative BL word;
        # both calls resolve to UIElement_ContainsPoint at 0x020b7db4.
        sibling = body(0x020B8398, 0x020B8408)
        self.assertEqual(sibling[:0x3C], hit_scan[:0x3C])
        self.assertEqual(sibling[0x40:], hit_scan[0x40:])

        def branch_target(address: int, instruction: int) -> int:
            displacement = (instruction & 0x00FFFFFF) << 2
            if displacement & (1 << 25):
                displacement -= 1 << 26
            return address + 8 + displacement

        self.assertEqual(
            branch_target(0x020B83D4, read_u32(0x020B83D4)), 0x020B7DB4
        )
        self.assertEqual(
            branch_target(0x020B8EC4, read_u32(0x020B8EC4)), 0x020B7DB4
        )

        # Previous function, both recovered entries, and the already-known
        # ItemsPanel constructor are four independent boundaries.
        self.assertEqual(read_u32(0x020B8E84), 0xE8BD80F8)
        self.assertEqual(read_u32(0x020B8E88), 0xE92D41F8)
        self.assertEqual(read_u32(0x020B8EF4), 0xE8BD81F8)
        self.assertEqual(read_u32(0x020B8EF8), 0xE5901000)
        self.assertEqual(read_u32(0x020B8F08), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020B8F0C), 0xE92D4010)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_toc_helpers_test"
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
            output = Path(temporary) / "manual_toc_helpers.o"
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
