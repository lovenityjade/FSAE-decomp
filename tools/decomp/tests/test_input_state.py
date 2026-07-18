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


class InputStateRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        # Button input reads 0x04000130 and 0x02ffffa8, active-low.
        self.assertEqual(read_u32(0x020BE9F4), 0xE59F105C)
        self.assertEqual(read_u32(0x020BE9F8), 0xE59F305C)
        self.assertEqual(read_u32(0x020BEA0C), 0xE0211623)
        self.assertEqual(read_u32(0x020BEA10), 0xE0011623)
        # Opposing left/right and up/down inputs are suppressed.
        self.assertEqual(read_u32(0x020BEA1C), 0xE20C3040)
        self.assertEqual(read_u32(0x020BEA20), 0xE20CC020)
        self.assertEqual(read_u32(0x020BEA24), 0xE1E03083)
        self.assertEqual(read_u32(0x020BEA28), 0xE1E0C0CC)
        # Touch init clears 9 eight-byte raw samples and debounce state.
        self.assertEqual(read_u32(0x020BEA64), 0xE5C0C048)
        self.assertEqual(read_u32(0x020BEA98), 0xE35C0009)
        # Sampling starts at frequency 4 with a 9-entry ring.
        self.assertEqual(read_u32(0x020BEAD4), 0xE3A00000)
        self.assertEqual(read_u32(0x020BEAD8), 0xE3A01004)
        self.assertEqual(read_u32(0x020BEADC), 0xE3A03009)
        # Update consumes the latest four entries and debounces at three.
        self.assertEqual(read_u32(0x020BEB20), 0xEBFD7656)
        self.assertEqual(read_u32(0x020BEB50), 0xE3530003)
        self.assertEqual(read_u32(0x020BEB84), 0xE35C0003)
        self.assertEqual(read_u32(0x020BEBBC), 0xE3520004)
        # Calibrated x/y are written while pressed is returned.
        self.assertEqual(read_u32(0x020BEC48), 0xEBFD768B)
        self.assertEqual(read_u32(0x020BEC50), 0xE5D40005)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "input_state_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/input_state.c"),
                    str(Path(__file__).with_name("input_state_harness.c")),
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
