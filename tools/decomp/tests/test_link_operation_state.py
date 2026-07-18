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
SOURCE = PROJECT_ROOT / "src/arm9/game/link_operation_state.c"
HARNESS = Path(__file__).with_name("link_operation_state_harness.c")


class LinkOperationStateRecoveryTests(unittest.TestCase):
    def test_rom_bodies_pools_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def body(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        has_succeeded = body(0x020B2A90, 0x020B2AA8)
        no_op = body(0x020B2AAC, 0x020B2AB0)
        self.assertEqual(len(has_succeeded), 24)
        self.assertEqual(len(no_op), 4)
        self.assertEqual(
            hashlib.sha256(has_succeeded).hexdigest(),
            "e578a33b9c14a0d1fee554a2dd77513b2b71be95371ac6d377b9f4c8208bbaf5",
        )
        self.assertEqual(
            hashlib.sha256(no_op).hexdigest(),
            "379bec29dccd0a93c94826144d7ef6e42fab64ef195a3b8313a16926f66f388f",
        )

        # The preceding function's two literals, this predicate's global
        # literal and the following function are all outside recovered bodies.
        self.assertEqual(read_u32(0x020B2A84), 0xE8BD8008)
        self.assertEqual(read_u32(0x020B2A88), 0x0212C7F0)
        self.assertEqual(read_u32(0x020B2A8C), 0x0212C9A8)
        self.assertEqual(read_u32(0x020B2A90), 0xE59F0010)
        self.assertEqual(read_u32(0x020B2A94), 0xE5900008)
        self.assertEqual(read_u32(0x020B2A98), 0xE3500001)
        self.assertEqual(read_u32(0x020B2A9C), 0x03A00001)
        self.assertEqual(read_u32(0x020B2AA0), 0x13A00000)
        self.assertEqual(read_u32(0x020B2AA4), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020B2AA8), 0x0212C9A8)
        self.assertEqual(read_u32(0x020B2AAC), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020B2AB0), 0xE92D4008)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "link_operation_state_test"
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
            output = Path(temporary) / "link_operation_state.o"
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
