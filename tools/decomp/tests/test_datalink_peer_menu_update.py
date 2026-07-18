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
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_peer_menu_update.c"
HARNESS = Path(__file__).with_name("datalink_peer_menu_update_harness.c")


class DatalinkPeerMenuUpdateRecoveryTests(unittest.TestCase):
    def test_rom_body_hash_calls_pool_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        start = 0x020B0014
        end = 0x020B0778
        body = arm9[start - ARM9_BASE : end - ARM9_BASE]
        self.assertEqual(len(body), 1892)
        self.assertEqual(
            hashlib.sha256(body).hexdigest(),
            "00b40616a0b1dae1eba2aa7dd982548b0b13c5a98a593656bba60fc20c6743a2",
        )

        anchors = {
            0x020B0010: 0x0217EB2C,
            0x020B0014: 0xE92D4FF8,
            0x020B0028: 0xEBFF8266,
            0x020B0030: 0xEBFF839B,
            0x020B0074: 0xEBFFFF10,
            0x020B0090: 0xEBFFF337,
            0x020B00A0: 0xEB0003B9,
            0x020B00C8: 0xEBFF1BC4,
            0x020B00D0: 0xEBFFFF59,
            0x020B00E8: 0xEB000323,
            0x020B00FC: 0xEB0003B9,
            0x020B0110: 0xEB0001A3,
            0x020B0158: 0xEB000197,
            0x020B0170: 0xEBFFF1C8,
            0x020B0188: 0xEBFF1B94,
            0x020B01C4: 0xEBFFF5E6,
            0x020B01E0: 0xEB000327,
            0x020B0234: 0xEBFF86BF,
            0x020B0264: 0xEBFF86B5,
            0x020B02F8: 0xEBFF855C,
            0x020B0310: 0xEBFF1B32,
            0x020B0330: 0xEBFFFE93,
            0x020B0348: 0xEBFF8662,
            0x020B0380: 0xEBFFEF2C,
            0x020B03C8: 0xEB000262,
            0x020B03CC: 0xEBFDDF47,
            0x020B0400: 0xEB000199,
            0x020B0438: 0xEB000246,
            0x020B0440: 0xEBFFF27E,
            0x020B0494: 0xEBFF860F,
            0x020B04E4: 0xEBFFEED3,
            0x020B0538: 0xEBFFF509,
            0x020B054C: 0xEBFFEF6D,
            0x020B056C: 0xEB0001F9,
            0x020B05A4: 0xEBFFEF57,
            0x020B05C4: 0xEB0001E3,
            0x020B05E4: 0xEB000074,
            0x020B0620: 0xEBFE8B0C,
            0x020B0650: 0xEBFFCA29,
            0x020B066C: 0xEBFFF4CC,
            0x020B0680: 0xEBFFCA1D,
            0x020B069C: 0xEBFFF4C0,
            0x020B073C: 0xEBFFF488,
            0x020B0748: 0xEB00001B,
            0x020B0760: 0xEBFFF47F,
            0x020B076C: 0xEBFFF47C,
            0x020B0774: 0xE8BD8FF8,
            0x020B0778: 0x0212C510,
            0x020B077C: 0x0217E6F0,
            0x020B0780: 0x0217F6DC,
            0x020B0784: 0x020DE944,
            0x020B0788: 0x020DE948,
            0x020B078C: 0x021806F0,
            0x020B0790: 0x020DE854,
            0x020B0794: 0x0217F6F0,
            0x020B0798: 0x0000083D,
            0x020B079C: 0x0000083E,
            0x020B07A0: 0x0217EB2C,
            0x020B07A4: 0xE92D4008,
        }
        for address, expected in anchors.items():
            self.assertEqual(read_u32(address), expected, hex(address))

        point_table = arm9[
            0x020DE944 - ARM9_BASE : 0x020DE964 - ARM9_BASE
        ]
        self.assertEqual(
            struct.unpack("<8i", point_table),
            (40, 48, 40, 88, 40, 128, 40, 168),
        )

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "datalink_peer_menu_update_test"
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
            output = Path(temporary) / "datalink_peer_menu_update.o"
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
