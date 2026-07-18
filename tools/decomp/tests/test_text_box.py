from __future__ import annotations

import hashlib
import os
import shutil
import struct
import subprocess
import tempfile
import unittest
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000


class TextBoxRecoveryTests(unittest.TestCase):
    def test_rom_rtti_vtable_and_literals(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        self.assertEqual(read_u32(0x02126B1C), 0)
        self.assertEqual(read_u32(0x02126B20), 0x020DFAE0)
        self.assertEqual(read_u32(0x02126B24), 0x02126B2C)
        self.assertEqual(read_u32(0x02126B28), 0x02126A64)
        name_offset = 0x02126B2C - ARM9_BASE
        expected_name = b"N4ntmv3m2d7TextBoxE\0"
        self.assertEqual(
            arm9[name_offset : name_offset + len(expected_name)], expected_name
        )

        self.assertEqual(read_u32(0x02126B40), 0)
        self.assertEqual(read_u32(0x02126B44), 0x02126B20)
        self.assertEqual(
            tuple(read_u32(0x02126B48 + index * 4) for index in range(4)),
            (0x020B7074, 0x020B5654, 0x020B6CC4, 0x020B6D74),
        )
        self.assertEqual(read_u32(0x020B6CC0), 0x02126B48)
        self.assertEqual(read_u32(0x020B707C), 0x02126B1C)
        # Factory 0x020b547c requests the proven complete-object size, 0x48.
        self.assertEqual(read_u32(0x020B548C), 0xE3A01048)

        # DrawTextUntil is exactly 188 bytes.  Its next neighbour is the
        # already-recovered 200-byte constructor, followed by a separate
        # vtable literal and UpdatePosition body.
        draw_start = 0x020B6B3C - ARM9_BASE
        draw_body = arm9[draw_start : draw_start + 188]
        self.assertEqual(
            hashlib.sha256(draw_body).hexdigest(),
            "2f6582ac659c860724d97f3275f4923432449f4b95e25d0ae6136911bc656a4f",
        )
        constructor_start = 0x020B6BF8 - ARM9_BASE
        constructor_body = arm9[constructor_start : constructor_start + 200]
        self.assertEqual(
            hashlib.sha256(constructor_body).hexdigest(),
            "aa2acd90a7dfe7b0a371af5c711e34e8fd6df953389ccf30d304a1b539783892",
        )
        self.assertEqual(read_u32(0x020B6B38), 0xE8BD8010)
        self.assertEqual(read_u32(0x020B6B3C), 0xE92D43F8)
        self.assertEqual(read_u32(0x020B6BF4), 0xE8BD83F8)
        self.assertEqual(read_u32(0x020B6BF8), 0xE92D47F0)
        self.assertEqual(read_u32(0x020B6CBC), 0xE8BD87F0)
        self.assertEqual(read_u32(0x020B6CC0), 0x02126B48)
        self.assertEqual(read_u32(0x020B6CC4), 0xE92D4018)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")

        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "text_box_test"
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
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/pane.c"),
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/text_box.c"),
                    str(Path(__file__).with_name("text_box_harness.c")),
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
            output = Path(temporary) / "text_box.o"
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
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/text_box.c"),
                    "-o",
                    str(output),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )


if __name__ == "__main__":
    unittest.main()
