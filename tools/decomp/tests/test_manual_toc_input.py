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


class ManualTocInputRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        self.assertEqual(read_u32(0x020BD2D8), 0xE92D4078)
        self.assertEqual(read_u32(0x020BD2E8), 0xE5960358)
        self.assertEqual(read_u32(0x020BD2F4), 0xEBFFF5AA)
        self.assertEqual(read_u32(0x020BD308), 0x908FF100)
        self.assertEqual(read_u32(0x020BD32C), 0xEB0001CF)
        self.assertEqual(read_u32(0x020BD348), 0xEB000424)
        self.assertEqual(read_u32(0x020BD364), 0xEB0001B4)
        self.assertEqual(read_u32(0x020BD370), 0xEB000377)
        self.assertEqual(read_u32(0x020BD384), 0x118610B0)
        self.assertEqual(read_u32(0x020BD39C), 0x118610B0)
        self.assertEqual(read_u32(0x020BD3A8), 0xE8BD8078)
        self.assertEqual(read_u32(0x020BD3AC), 0x0004061E)
        self.assertEqual(read_u32(0x020BD3B0), 0xE92D4078)
        self.assertEqual(read_u32(0x020BD3BC), 0xE5960360)
        self.assertEqual(read_u32(0x020BD3E0), 0xEBFFEC5B)
        self.assertEqual(read_u32(0x020BD404), 0xE59F1064)
        self.assertEqual(read_u32(0x020BD414), 0x15C04398)
        self.assertEqual(read_u32(0x020BD418), 0x118640B1)
        self.assertEqual(read_u32(0x020BD42C), 0xEB000439)
        self.assertEqual(read_u32(0x020BD44C), 0xEB000653)
        self.assertEqual(read_u32(0x020BD460), 0x118610B0)
        self.assertEqual(read_u32(0x020BD46C), 0xE8BD8078)
        self.assertEqual(read_u32(0x020BD470), 0x0004061E)
        self.assertEqual(read_u32(0x020BD474), 0xE92D40F8)
        # D-pad scrolling has distinct initial/held key words and mode 4.
        self.assertEqual(read_u32(0x020BD830), 0xE92D40F8)
        self.assertEqual(read_u32(0x020BD838), 0xE5970358)
        self.assertEqual(read_u32(0x020BD83C), 0xEBFFF7BB)
        self.assertEqual(read_u32(0x020BD86C), 0xE19700B0)
        self.assertEqual(read_u32(0x020BD870), 0xE3100040)
        self.assertEqual(read_u32(0x020BD87C), 0xE3100080)
        self.assertEqual(read_u32(0x020BD8A4), 0xEB00031B)
        self.assertEqual(read_u32(0x020BD8B8), 0xE18710B0)
        self.assertEqual(read_u32(0x020BD8CC), 0xEBFFEB79)
        self.assertEqual(read_u32(0x020BD8D4), 0xE5C06398)
        self.assertEqual(read_u32(0x020BD8E8), 0xEB00052C)
        self.assertEqual(read_u32(0x020BD8FC), 0xE19700B0)
        self.assertEqual(read_u32(0x020BD934), 0xEB0002F7)
        self.assertEqual(read_u32(0x020BD950), 0xEBFFEB58)
        self.assertEqual(read_u32(0x020BD958), 0xE8BD80F8)
        self.assertEqual(read_u32(0x020BD95C), 0x0004061E)
        self.assertEqual(read_u32(0x020BD960), 0x000403A0)
        self.assertEqual(read_u32(0x020BD964), 0x000403A4)
        self.assertEqual(read_u32(0x020BD968), 0xE92D4038)
        # Button priority is Left, Right, Select, A, then Start.
        self.assertEqual(read_u32(0x020BD970), 0xE5950358)
        self.assertEqual(read_u32(0x020BD974), 0xEBFFF76D)
        self.assertEqual(read_u32(0x020BD990), 0xE3100020)
        self.assertEqual(read_u32(0x020BD99C), 0xEBFFF6AB)
        self.assertEqual(read_u32(0x020BD9AC), 0xE3100010)
        self.assertEqual(read_u32(0x020BD9B8), 0xEBFFF6EB)
        self.assertEqual(read_u32(0x020BD9C8), 0xEB0001E1)
        self.assertEqual(read_u32(0x020BD9D0), 0xE3100004)
        self.assertEqual(read_u32(0x020BD9E0), 0xEBFFF72C)
        self.assertEqual(read_u32(0x020BD9F4), 0xEB000010)
        self.assertEqual(read_u32(0x020BD9FC), 0xE3100001)
        self.assertEqual(read_u32(0x020BDA08), 0xEBFFF72E)
        self.assertEqual(read_u32(0x020BDA14), 0xEB000271)
        self.assertEqual(read_u32(0x020BDA1C), 0xE3100008)
        self.assertEqual(read_u32(0x020BDA28), 0xEB000010)
        self.assertEqual(read_u32(0x020BDA34), 0xE8BD8038)
        self.assertEqual(read_u32(0x020BDA38), 0x000403A0)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_toc_input_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_toc_input.c"),
                    str(Path(__file__).with_name("manual_toc_input_harness.c")),
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
