from __future__ import annotations

import json
import os
import shutil
import struct
import subprocess
import tempfile
import unittest
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000


class WindowRecoveryTests(unittest.TestCase):
    def test_rom_rtti_vtable_and_literals(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        # GetRuntimeType's returned prefix/tag and the following ABI typeinfo.
        self.assertEqual(read_u32(0x02126AD4), 0)
        self.assertEqual(read_u32(0x02126AD8), 0x020DFAE0)
        self.assertEqual(read_u32(0x02126ADC), 0x02126AE4)
        self.assertEqual(read_u32(0x02126AE0), 0x02126A64)
        name_offset = 0x02126AE4 - ARM9_BASE
        self.assertEqual(
            arm9[name_offset : name_offset + len(b"N4ntmv3m2d6WindowE\0")],
            b"N4ntmv3m2d6WindowE\0",
        )

        # Header, typeinfo word and seven callable slots.
        self.assertEqual(read_u32(0x02126AF8), 0)
        self.assertEqual(read_u32(0x02126AFC), 0x02126AD8)
        expected_slots = (
            0x020B64F0,
            0x020B5CD4,
            0x020B57C8,
            0x020B5D48,
            0x020B5E48,
            0x020B5EC4,
            0x020B6158,
        )
        self.assertEqual(
            tuple(read_u32(0x02126B00 + index * 4) for index in range(7)),
            expected_slots,
        )

        # Constructor vtable literal and GetRuntimeType tag literal.
        self.assertEqual(read_u32(0x020B5C54), 0x02126B00)
        self.assertEqual(read_u32(0x020B64F8), 0x02126AD4)

    def test_rom_border_helper_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        catalog = json.loads(
            (PROJECT_ROOT / "build/decomp/arm9/catalog.json").read_text()
        )
        sizes = {
            int(function["entry"], 16): function["body_bytes"]
            for function in catalog["functions"]
        }
        expected_sizes = {
            0x020B5950: 52,
            0x020B5984: 16,
            0x020B5998: 68,
            0x020B59DC: 88,
            0x020B5A34: 60,
            0x020B5AE4: 76,
            0x020B5BA4: 4,
        }
        for address, size in expected_sizes.items():
            self.assertEqual(sizes[address], size)

        expected_words = {
            # Top rect, top-origin wrapper and its literal target.
            0x020B5950: 0xE92D4008,
            0x020B5980: 0xE8BD8008,
            0x020B5984: 0xE59FC008,
            0x020B5990: 0xE12FFF1C,
            0x020B5994: 0x020B76D0,
            # Right rect and right source-origin helper.
            0x020B5998: 0xE92D4070,
            0x020B59D8: 0xE8BD8070,
            0x020B59DC: 0xE92D4070,
            0x020B5A30: 0xE8BD8070,
            # Left/bottom rects and the per-border no-op destructor.
            0x020B5A34: 0xE92D4038,
            0x020B5A6C: 0xE8BD8038,
            0x020B5AE4: 0xE92D4070,
            0x020B5B2C: 0xE8BD8070,
            0x020B5BA4: 0xE12FFF1E,
        }
        for address, instruction in expected_words.items():
            self.assertEqual(read_u32(address), instruction)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")

        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "window_test"
            subprocess.run(
                [
                    compiler,
                    "-std=c99",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-I",
                    str(PROJECT_ROOT / "include"),
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/pane.c"),
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/window.c"),
                    str(Path(__file__).with_name("window_harness.c")),
                    "-o",
                    str(executable),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )
            subprocess.run([str(executable)], check=True, cwd=PROJECT_ROOT)


if __name__ == "__main__":
    unittest.main()
