from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000
SOURCE = (
    PROJECT_ROOT / "src/arm9/game/datalink_multiplayer_session_frame.c"
)
HARNESS = Path(__file__).with_name(
    "datalink_multiplayer_session_frame_harness.c"
)


class DatalinkMultiplayerSessionFrameRecoveryTests(unittest.TestCase):
    def test_rom_body_hash_calls_switch_pool_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        def branch_target(address: int) -> int:
            instruction = read_u32(address)
            self.assertEqual(instruction >> 28, 0xE, hex(address))
            self.assertIn((instruction >> 24) & 0xF, (0xA, 0xB))
            displacement = (instruction & 0x00FFFFFF) << 2
            if displacement & 0x02000000:
                displacement -= 0x04000000
            return address + 8 + displacement

        start = 0x020B19D8
        end = 0x020B2170
        body = arm9[start - ARM9_BASE : end - ARM9_BASE]
        self.assertEqual(len(body), 1944)
        self.assertEqual(
            hashlib.sha256(body).hexdigest(),
            "8d0863a8cf589303a6aadc0da6446322f4f5a9eacac595782323b0ab93c6bad1",
        )

        anchors = {
            0x020B19D4: 0x0212C9A8,
            0x020B19D8: 0xE92D4FF8,
            0x020B1D04: 0x908FF100,
            0x020B216C: 0xE8BD8FF8,
            0x020B2170: 0x0217D348,
            0x020B2174: 0x02181860,
            0x020B2178: 0x0217A23C,
            0x020B217C: 0x0212C9A8,
            0x020B2180: 0x0217D23C,
            0x020B2184: 0x2AAAAAAB,
            0x020B2188: 0x0217C23C,
            0x020B218C: 0x0212C9B8,
            0x020B2190: 0xE92D41F0,
        }
        for address, expected in anchors.items():
            self.assertEqual(read_u32(address), expected, hex(address))

        calls = {
            0x020B1A30: 0x020A31AC,
            0x020B1AD0: 0x0209CF68,
            0x020B1B14: 0x0209A8B8,
            0x020B1B18: 0x020B2AAC,
            0x020B1B1C: 0x020B2898,
            0x020B1B20: 0x0208DA4C,
            0x020B1B24: 0x0208DE54,
            0x020B1BD4: 0x020B2560,
            0x020B1BF0: 0x020B235C,
            0x020B1C04: 0x020B235C,
            0x020B1C1C: 0x020B235C,
            0x020B1CDC: 0x020B23FC,
            0x020B1D94: 0x020ACC2C,
            0x020B1E60: 0x02015C28,
            0x020B1EC8: 0x020B11DC,
            0x020B1FA4: 0x020B2580,
            0x020B2084: 0x020B4150,
            0x020B20C4: 0x020B26D0,
            0x020B20C8: 0x020B292C,
            0x020B20CC: 0x020B4D68,
            0x020B20DC: 0x020B2614,
            0x020B20FC: 0x020B2190,
            0x020B2124: 0x0209DA7C,
            0x020B2138: 0x0209DA7C,
            0x020B2154: 0x020827A8,
            0x020B2164: 0x02082EE0,
        }
        for address, expected in calls.items():
            self.assertEqual(branch_target(address), expected, hex(address))

        phase_targets = (
            0x020B2B54,
            0x020B32CC,
            0x020B3998,
            0x020B3A84,
            0x020B3B28,
            0x020B3D70,
            0x020B3EA0,
            0x020B3EA8,
            0x020B3FCC,
        )
        for index, expected in enumerate(phase_targets):
            table_branch = 0x020B1D0C + index * 4
            self.assertEqual(
                branch_target(table_branch),
                0x020B1D30 + index * 8,
                hex(table_branch),
            )
            self.assertEqual(
                branch_target(0x020B1D30 + index * 8),
                expected,
            )

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "datalink_multiplayer_frame_test"
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
                    str(SOURCE),
                    str(HARNESS),
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
            output = Path(temporary) / "datalink_multiplayer_frame.o"
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
                    str(SOURCE),
                    "-o",
                    str(output),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )


if __name__ == "__main__":
    unittest.main()
