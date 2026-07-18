from __future__ import annotations

import os
import json
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000


class ScrollControlsRecoveryTests(unittest.TestCase):
    def test_rom_rtti_vtables_constructors_and_sizes(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        inherited_slots = (
            0x020B8138,
            0x020B81DC,
            0x020B7E48,
            0x020B7E5C,
            0x020B7E70,
        )

        self.assertEqual(read_u32(0x02126DD8), 0x020DFAE0)
        self.assertEqual(read_u32(0x02126DDC), 0x02126DE4)
        self.assertEqual(read_u32(0x02126DE0), 0x02126D5C)
        name_offset = 0x02126DE4 - ARM9_BASE
        name = b"N4ntmv3m2d12ScrollButtonE\0"
        self.assertEqual(arm9[name_offset : name_offset + len(name)], name)
        self.assertEqual(read_u32(0x02126E00), 0)
        self.assertEqual(read_u32(0x02126E04), 0x02126DD8)
        self.assertEqual(
            tuple(read_u32(0x02126E08 + index * 4) for index in range(5)),
            inherited_slots,
        )

        # ScrollIndicator's name is placed after its vtable, but typeinfo is e1c.
        self.assertEqual(read_u32(0x02126E1C), 0x020DFAE0)
        self.assertEqual(read_u32(0x02126E20), 0x02126E44)
        self.assertEqual(read_u32(0x02126E24), 0x02126D5C)
        name_offset = 0x02126E44 - ARM9_BASE
        name = b"N4ntmv3m2d15ScrollIndicatorE\0"
        self.assertEqual(arm9[name_offset : name_offset + len(name)], name)
        self.assertEqual(read_u32(0x02126E28), 0)
        self.assertEqual(read_u32(0x02126E2C), 0x02126E1C)
        self.assertEqual(
            tuple(read_u32(0x02126E30 + index * 4) for index in range(5)),
            inherited_slots,
        )

        self.assertEqual(read_u32(0x020B843C), 0x02126E08)
        self.assertEqual(read_u32(0x020B8398), 0xE92D41F8)
        self.assertEqual(read_u32(0x020B83FC), 0xE3E00000)
        self.assertEqual(read_u32(0x020B8404), 0xE8BD81F8)
        self.assertEqual(read_u32(0x020B85BC), 0xEBFFFF75)
        self.assertEqual(read_u32(0x020B87B0), 0x02126E30)
        self.assertEqual(read_u32(0x020BA724), 0xE3A01024)
        self.assertEqual(read_u32(0x020BCD94), 0xE3A01024)

        catalog = json.loads(
            (PROJECT_ROOT / "build/decomp/arm9/catalog.json").read_text()
        )
        by_entry = {
            item["entry"]: item
            for item in catalog["functions"]
            if item.get("entry") in {"0x020b8398", "0x020b8554"}
        }
        self.assertEqual(by_entry["0x020b8398"]["body_bytes"], 112)
        self.assertEqual(by_entry["0x020b8554"]["body_bytes"], 356)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "scroll_controls_test"
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
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/ui_panel.c"),
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/ui_static.c"),
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/obj_button.c"),
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/scroll_controls.c"),
                    str(Path(__file__).with_name("scroll_controls_harness.c")),
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
