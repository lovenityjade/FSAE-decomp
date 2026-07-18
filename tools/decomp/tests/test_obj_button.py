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


class ObjButtonRecoveryTests(unittest.TestCase):
    def test_rom_rtti_vtable_and_literals(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        self.assertEqual(read_u32(0x02126D98), 0x020DFAE0)
        self.assertEqual(read_u32(0x02126D9C), 0x02126DA4)
        self.assertEqual(read_u32(0x02126DA0), 0x02126CE0)
        name_offset = 0x02126DA4 - ARM9_BASE
        expected_name = b"N4ntmv3m2d9ObjButtonE\0"
        self.assertEqual(
            arm9[name_offset : name_offset + len(expected_name)], expected_name
        )

        self.assertEqual(read_u32(0x02126DBC), 0)
        self.assertEqual(read_u32(0x02126DC0), 0x02126D98)
        self.assertEqual(
            tuple(read_u32(0x02126DC4 + index * 4) for index in range(5)),
            (0x020B7CD0, 0x020B82F4, 0x020B7E48, 0x020B7E5C, 0x020B7E70),
        )

        # Both base/complete constructor variants install the same address point.
        self.assertEqual(read_u32(0x020B8298), 0x02126DC4)
        self.assertEqual(read_u32(0x020B82C8), 0x02126DC4)
        # Factory 0x020b8528 allocates the complete 0x1c-byte object.
        self.assertEqual(read_u32(0x020B8530), 0xE3A0101C)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "obj_button_test"
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
                    str(Path(__file__).with_name("obj_button_harness.c")),
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
