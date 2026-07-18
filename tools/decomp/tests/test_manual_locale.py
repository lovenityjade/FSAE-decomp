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


class ManualLocaleRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_data_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        # Locale zero reads SDK owner language and indexes eight u16 codes.
        self.assertEqual(read_u32(0x020BE024), 0xEBFD6E2E)
        self.assertEqual(read_u32(0x020BE02C), 0xE59F0080)
        self.assertEqual(read_u32(0x020BE034), 0xE19040B1)
        self.assertEqual(read_u32(0x020BE0B4), 0x020DF004)
        expected = b"ajnerfedtisehzok"
        offset = 0x020DF004 - ARM9_BASE
        self.assertEqual(arm9[offset : offset + len(expected)], expected)
        # Localized accessor has_file is vtable slot 5 at target +0x40500.
        self.assertEqual(read_u32(0x020BE040), 0xE590C000)
        self.assertEqual(read_u32(0x020BE048), 0xE59CC014)
        self.assertEqual(read_u32(0x020BE054), 0xE12FFF3C)
        self.assertEqual(read_u32(0x020BE0B8), 0x6E746D63)
        # Missing locale falls back to the first entry in the langs table.
        self.assertEqual(read_u32(0x020BE070), 0xEB000409)
        self.assertEqual(read_u32(0x020BE074), 0xE59D0004)
        self.assertEqual(read_u32(0x020BE078), 0xE1D040B0)
        # Both localized and shared accessors receive the final locale.
        self.assertEqual(read_u32(0x020BE08C), 0xE5922010)
        self.assertEqual(read_u32(0x020BE0A4), 0xE5922010)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_locale_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_locale.c"),
                    str(Path(__file__).with_name("manual_locale_harness.c")),
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
