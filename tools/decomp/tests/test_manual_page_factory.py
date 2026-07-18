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


class ManualPageFactoryRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        self.assertEqual(read_u32(0x020BCD60), 0xE92D4008)
        self.assertEqual(read_u32(0x020BCD64), 0xE5900000)
        self.assertEqual(read_u32(0x020BCD68), 0xE3A01020)
        self.assertEqual(read_u32(0x020BCD6C), 0xEB0017B0)
        self.assertEqual(read_u32(0x020BCD74), 0x0A000002)
        self.assertEqual(read_u32(0x020BCD7C), 0xEBFFE037)
        self.assertEqual(read_u32(0x020BCD84), 0xE3A00000)
        self.assertEqual(read_u32(0x020BCD88), 0xE8BD8008)
        self.assertEqual(read_u32(0x020BCD8C), 0xE92D4008)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_page_factory_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_page_factory.c"),
                    str(Path(__file__).with_name("manual_page_factory_harness.c")),
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
