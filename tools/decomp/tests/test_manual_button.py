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


class ManualButtonRecoveryTests(unittest.TestCase):
    def test_rom_rtti_layout_render_and_state_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        # GNU RTTI record and direct ObjButton base.
        self.assertEqual(read_u32(0x02126F08), 0x020DFAE0)
        self.assertEqual(read_u32(0x02126F0C), 0x02126F14)
        self.assertEqual(read_u32(0x02126F10), 0x02126D98)
        name_offset = 0x02126F14 - ARM9_BASE
        expected_name = b"N4ntmv3m2d12ManualButtonE\0"
        self.assertEqual(
            arm9[name_offset : name_offset + len(expected_name)], expected_name
        )

        # Header/typeinfo and the one overridden plus four inherited slots.
        self.assertEqual(read_u32(0x02126F30), 0)
        self.assertEqual(read_u32(0x02126F34), 0x02126F08)
        self.assertEqual(
            tuple(read_u32(0x02126F38 + index * 4) for index in range(5)),
            (0x020B7CD0, 0x020BBA88, 0x020B7E48, 0x020B7E5C, 0x020B7E70),
        )

        # Constructor installs the address point and initializes +20/+24/+28/+29.
        self.assertEqual(read_u32(0x020BBA34), 0x02126F38)
        self.assertEqual(read_u32(0x020BBA20), 0xE5842020)
        self.assertEqual(read_u32(0x020BBA24), 0xE5842024)
        self.assertEqual(read_u32(0x020BBA28), 0xE5C41028)
        self.assertEqual(read_u32(0x020BBA2C), 0xE5C41029)

        # Factory allocation is the independent proof of the 0x2c-byte layout.
        self.assertEqual(read_u32(0x020B9260), 0xE3A0102C)
        # Configure stores its two stack arguments at +20 and +24.
        self.assertEqual(read_u32(0x020BBA78), 0xE5841020)
        self.assertEqual(read_u32(0x020BBA7C), 0xE5840024)

        # Render reads timer/selection, chooses +14 or +20, and tests +24 for -1.
        self.assertEqual(read_u32(0x020BBAB4), 0xE1D701B8)
        self.assertEqual(read_u32(0x020BBAE8), 0xE5D70029)
        self.assertEqual(read_u32(0x020BBAFC), 0xE5D70028)
        self.assertEqual(read_u32(0x020BBB00), 0xE5971024)
        self.assertEqual(read_u32(0x020BBB08), 0x15975020)
        self.assertEqual(read_u32(0x020BBB0C), 0x05975014)
        self.assertEqual(read_u32(0x020BBB10), 0xE3710001)
        # The trailing exact setter is a single byte store to +28.
        self.assertEqual(read_u32(0x020BBB70), 0xE5C01028)
        self.assertEqual(read_u32(0x020BBB74), 0xE12FFF1E)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")

        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_button_test"
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
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/obj_button.c"),
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/manual_button.c"),
                    str(Path(__file__).with_name("manual_button_harness.c")),
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
