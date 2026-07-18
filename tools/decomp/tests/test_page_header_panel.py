from __future__ import annotations

import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000


class PageHeaderPanelRecoveryTests(unittest.TestCase):
    def test_rom_rtti_layout_and_instruction_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        # GNU RTTI: direct UIStatic base and five entirely inherited slots.
        self.assertEqual(read_u32(0x02126F4C), 0x020DFAE0)
        self.assertEqual(read_u32(0x02126F50), 0x02126F74)
        self.assertEqual(read_u32(0x02126F54), 0x02126D1C)
        self.assertEqual(read_u32(0x02126F58), 0)
        self.assertEqual(read_u32(0x02126F5C), 0x02126F4C)
        self.assertEqual(
            tuple(read_u32(0x02126F60 + index * 4) for index in range(5)),
            (0x020B7CD0, 0x020B7FA0, 0x020B7E48, 0x020B7E5C, 0x020B7E70),
        )

        # Constructor vtable literal and target +0x20 text initialization.
        self.assertEqual(read_u32(0x020BBB9C), 0x02126F60)
        self.assertEqual(read_u32(0x020BBB90), 0xE5842000)
        self.assertEqual(read_u32(0x020BBB94), 0xE5841020)
        # Configure stores 256x18, center (128,9), and context at +0x1c.
        self.assertEqual(read_u32(0x020BBBC8), 0xE1CD40B0)
        self.assertEqual(read_u32(0x020BBBCC), 0xE1CDE0B2)
        self.assertEqual(read_u32(0x020BBBD0), 0xE1CDC0B4)
        self.assertEqual(read_u32(0x020BBBD4), 0xE1CD30B6)
        self.assertEqual(read_u32(0x020BBBF0), 0xE586501C)

        # SetText writes +0x20 and selects sub OBJ VRAM through helper arg 1.
        self.assertEqual(read_u32(0x020BBC08), 0xE5881020)
        self.assertEqual(read_u32(0x020BBC48), 0xE1A01005)
        self.assertEqual(read_u32(0x020BBC50), 0xEB000060)
        # The sentinel is UINT16_MAX and page text begins at x=0x16 otherwise.
        self.assertEqual(read_u32(0x020BBC54), 0xE2650801)
        self.assertEqual(read_u32(0x020BBD24), 0xE3A06016)
        # Canvas helper chooses 0x06600000/0x06400000 and clears via slot +4.
        self.assertEqual(read_u32(0x020BBDF0), 0xE3A0C666)
        self.assertEqual(read_u32(0x020BBDFC), 0x03A0C519)
        self.assertEqual(read_u32(0x020BBE38), 0xE5902004)
        self.assertEqual(read_u32(0x020BBE40), 0xE12FFF32)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "page_header_panel_test"
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
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/ui_element.c"),
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/ui_static.c"),
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/page_header_panel.c"),
                    str(Path(__file__).with_name("page_header_panel_harness.c")),
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
