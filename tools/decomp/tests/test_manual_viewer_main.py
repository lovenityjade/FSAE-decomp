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
SOURCE = PROJECT_ROOT / "src/arm9/game/manual_viewer_main.c"
HARNESS = Path(__file__).with_name("manual_viewer_main_harness.c")


class ManualViewerMainRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        # The complete instruction body is 0x1ec bytes, from the independent
        # prologue at 0x020bce14 through the pop at 0x020bcffc.
        self.assertEqual(read_u32(0x020BCE14), 0xE92D4FF8)
        self.assertEqual(read_u32(0x020BCE18), 0xE24DD008)
        self.assertEqual(read_u32(0x020BCE24), 0xEBFD7263)
        self.assertEqual(read_u32(0x020BCE28), 0xEBFD581E)
        self.assertEqual(read_u32(0x020BCE2C), 0xE59F11CC)
        self.assertEqual(read_u32(0x020BCE38), 0xE3800801)
        self.assertEqual(read_u32(0x020BCE3C), 0xE5810000)

        # Fade-in decrements 17 before rendering, producing brightness 16..0
        # with one render/VBlank/transition-zero transaction per value.
        self.assertEqual(read_u32(0x020BCE30), 0xE3A04011)
        self.assertEqual(read_u32(0x020BCE44), 0xE2444001)
        self.assertEqual(read_u32(0x020BCE48), 0xEB00006F)
        self.assertEqual(read_u32(0x020BCE4C), 0xEBFD7259)
        self.assertEqual(read_u32(0x020BCE50), 0xE1A01204)
        self.assertEqual(read_u32(0x020BCE60), 0xEBFFFD50)
        self.assertEqual(read_u32(0x020BCE6C), 0xEB0000AD)
        self.assertEqual(read_u32(0x020BCE74), 0xCAFFFFF1)

        # Each input frame checks +0x40621, updates buttons at +0x403a0,
        # sleeps for lid bit 15, samples touch at +0x403a6, then dispatches
        # the signed mode stored at +0x4061e.
        self.assertEqual(read_u32(0x020BCE78), 0xE2896701)
        self.assertEqual(read_u32(0x020BCE7C), 0xE5D60621)
        self.assertEqual(read_u32(0x020BCE84), 0x1A000049)
        self.assertEqual(read_u32(0x020BCE88), 0xE59FB174)
        self.assertEqual(read_u32(0x020BCE8C), 0xE59FA174)
        self.assertEqual(read_u32(0x020BCE90), 0xE2895E3A)
        self.assertEqual(read_u32(0x020BCE94), 0xE28B4F9E)
        self.assertEqual(read_u32(0x020BCEA0), 0xEB0006D2)
        self.assertEqual(read_u32(0x020BCEA4), 0xE1DA00B0)
        self.assertEqual(read_u32(0x020BCEB4), 0xE3A00004)
        self.assertEqual(read_u32(0x020BCEC0), 0xEBFD805C)
        self.assertEqual(read_u32(0x020BCEC8), 0xE1CD80B0)
        self.assertEqual(read_u32(0x020BCECC), 0xE1CD80B2)
        self.assertEqual(read_u32(0x020BCED4), 0xEB000709)
        self.assertEqual(read_u32(0x020BCED8), 0xE19900F4)
        self.assertEqual(read_u32(0x020BCEE0), 0x908FF100)

        # Mode zero short-circuits scroll, TOC, page drag, D-pad and buttons;
        # the jump-table entries 1..4 call only their captured handler.
        self.assertEqual(read_u32(0x020BCEFC), 0xE1A00009)
        self.assertEqual(read_u32(0x020BCF04), 0xEB000129)
        self.assertEqual(read_u32(0x020BCF18), 0xEB0000EE)
        self.assertEqual(read_u32(0x020BCF2C), 0xEB000150)
        self.assertEqual(read_u32(0x020BCF3C), 0xEB00023B)
        self.assertEqual(read_u32(0x020BCF4C), 0xEB000285)
        self.assertEqual(read_u32(0x020BCF5C), 0xEB000113)
        self.assertEqual(read_u32(0x020BCF6C), 0xEB0000D9)
        self.assertEqual(read_u32(0x020BCF7C), 0xEB00013C)
        self.assertEqual(read_u32(0x020BCF88), 0xEB000228)

        # Every input pass still renders, waits for VBlank, clears the page
        # transition and rechecks exit before the next button sample.
        self.assertEqual(read_u32(0x020BCF90), 0xEB00001D)
        self.assertEqual(read_u32(0x020BCF94), 0xEBFD7207)
        self.assertEqual(read_u32(0x020BCFA0), 0xEB000060)
        self.assertEqual(read_u32(0x020BCFA4), 0xE5D60621)
        self.assertEqual(read_u32(0x020BCFAC), 0x0AFFFFBA)

        # Fade-out produces brightness 0..16, then shutdown and the exact
        # epilogue.  The three following literals prove the fixed MMIO/input
        # dependencies; RenderFrame begins independently at 0x020bd00c.
        self.assertEqual(read_u32(0x020BCFB0), 0xE3A04000)
        self.assertEqual(read_u32(0x020BCFB8), 0xE1A00009)
        self.assertEqual(read_u32(0x020BCFC0), 0xEBFD71FC)
        self.assertEqual(read_u32(0x020BCFD4), 0xEBFFFCF3)
        self.assertEqual(read_u32(0x020BCFE0), 0xEB000050)
        self.assertEqual(read_u32(0x020BCFE4), 0xE2855001)
        self.assertEqual(read_u32(0x020BCFE8), 0xE3550011)
        self.assertEqual(read_u32(0x020BCFEC), 0xBAFFFFF1)
        self.assertEqual(read_u32(0x020BCFF4), 0xEB000492)
        self.assertEqual(read_u32(0x020BCFFC), 0xE8BD8FF8)
        self.assertEqual(read_u32(0x020BD000), 0x04001000)
        self.assertEqual(read_u32(0x020BD004), 0x000403A6)
        self.assertEqual(read_u32(0x020BD008), 0x02FFFFA8)
        self.assertEqual(read_u32(0x020BD00C), 0xE92D4078)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_viewer_main_test"
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
            output = Path(temporary) / "manual_viewer_main.o"
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
