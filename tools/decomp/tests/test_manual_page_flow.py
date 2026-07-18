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


class ManualPageFlowRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_literal_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        self.assertEqual(read_u32(0x020BE200), 0xE3A04001)
        self.assertEqual(read_u32(0x020BE204), 0xE59F1034)
        self.assertEqual(read_u32(0x020BE20C), 0xE2822701)
        self.assertEqual(read_u32(0x020BE210), 0xEBFFFED2)
        self.assertEqual(read_u32(0x020BE214), 0xE2850E37)
        self.assertEqual(read_u32(0x020BE21C), 0xEB0001F0)
        self.assertEqual(read_u32(0x020BE228), 0xE2642801)
        self.assertEqual(read_u32(0x020BE238), 0xE5801608)
        page_name = read_u32(0x020BE240)
        self.assertEqual(page_name, 0x02127188)
        offset = page_name - ARM9_BASE
        self.assertEqual(arm9[offset : offset + 9], b"000guide\0")
        # Selected-page flow reads root/child, chooses record stride 0x18,
        # publishes a packed (root+1, child+1) state, and updates the header.
        self.assertEqual(read_u32(0x020BE15C), 0xE5970358)
        self.assertEqual(read_u32(0x020BE160), 0xEBFFF2AE)
        self.assertEqual(read_u32(0x020BE16C), 0xEBFFF2B9)
        self.assertEqual(read_u32(0x020BE174), 0xE3A01018)
        self.assertEqual(read_u32(0x020BE180), 0xE1062184)
        self.assertEqual(read_u32(0x020BE1B0), 0xE1825803)
        self.assertEqual(read_u32(0x020BE1D0), 0xEBFFFEE2)
        self.assertEqual(read_u32(0x020BE1E4), 0xEBFFF684)
        self.assertEqual(read_u32(0x020BE1EC), 0xE5805608)
        # Page-index initialization allocates exactly eight bytes, loads the
        # "manual" resource, installs the TOC and chooses page/fallback.
        self.assertEqual(read_u32(0x020BE0D8), 0xEB000012)
        self.assertEqual(read_u32(0x020BE0E8), 0xE5850348)
        self.assertEqual(read_u32(0x020BE0F0), 0xEBFFEAE0)
        self.assertEqual(read_u32(0x020BE0FC), 0xEBFFF211)
        self.assertEqual(read_u32(0x020BE104), 0xEBFFF2C5)
        self.assertEqual(read_u32(0x020BE114), 0xEB00000E)
        self.assertEqual(read_u32(0x020BE11C), 0xEB000034)
        resource_name = read_u32(0x020BE124)
        self.assertEqual(resource_name, 0x02127178)
        offset = resource_name - ARM9_BASE
        self.assertEqual(arm9[offset : offset + 7], b"manual\0")
        self.assertEqual(read_u32(0x020BE12C), 0xE5900000)
        self.assertEqual(read_u32(0x020BE130), 0xE3A01008)
        self.assertEqual(read_u32(0x020BE134), 0xEB0012BE)
        self.assertEqual(read_u32(0x020BE144), 0xEBFFEAAD)
        # Current-page cleanup is guarded, destroys through the viewer's
        # allocator, clears +0x4039c and detaches the scroll-buffer content.
        self.assertEqual(read_u32(0x020BDFD0), 0xE590139C)
        self.assertEqual(read_u32(0x020BDFD4), 0xE3510000)
        self.assertEqual(read_u32(0x020BDFE8), 0xEBFFF9F2)
        self.assertEqual(read_u32(0x020BDFFC), 0xE582139C)
        self.assertEqual(read_u32(0x020BE000), 0xEB00012F)
        # Page selection wrapper and exit request retain their exact branches.
        self.assertEqual(read_u32(0x020BDA3C), 0xE92D4010)
        self.assertEqual(read_u32(0x020BDA48), 0x0A000001)
        self.assertEqual(read_u32(0x020BDA4C), 0xEB0001E8)
        self.assertEqual(read_u32(0x020BDA58), 0xEBFFF470)
        self.assertEqual(read_u32(0x020BDA68), 0xEB0001B9)
        self.assertEqual(read_u32(0x020BDA70), 0xE92D4010)
        self.assertEqual(read_u32(0x020BDA7C), 0xEBFFF722)
        self.assertEqual(read_u32(0x020BDA88), 0xE5C01621)
        # Three context words are copied and published at buffer +0x08.
        self.assertEqual(read_u32(0x020BDA90), 0xE3A0C000)
        self.assertEqual(read_u32(0x020BDA94), 0xE791310C)
        self.assertEqual(read_u32(0x020BDAA0), 0xE58230F0)
        self.assertEqual(read_u32(0x020BDAA8), 0xBAFFFFF9)
        self.assertEqual(read_u32(0x020BDAB4), 0xE5801378)
        self.assertEqual(read_u32(0x020BDAB8), 0xE12FFF1E)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_page_flow_test"
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
                    str(PROJECT_ROOT / "src/arm9/game/manual_page_flow.c"),
                    str(Path(__file__).with_name("manual_page_flow_harness.c")),
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
