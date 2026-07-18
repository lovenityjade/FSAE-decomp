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
SOURCE = PROJECT_ROOT / "src/arm9/game/manual_page_pointer.c"
HARNESS = Path(__file__).with_name("manual_page_pointer_harness.c")


class ManualPagePointerRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        # The complete 916-byte CFG starts independently at 0x020bd474.  A
        # page must exist at +0x40374 and its scroll extent must be usable.
        self.assertEqual(read_u32(0x020BD474), 0xE92D40F8)
        self.assertEqual(read_u32(0x020BD47C), 0xE2870701)
        self.assertEqual(read_u32(0x020BD480), 0xE5900374)
        self.assertEqual(read_u32(0x020BD494), 0x0A000004)
        self.assertEqual(read_u32(0x020BD498), 0xE2870E37)
        self.assertEqual(read_u32(0x020BD4A0), 0xEB000506)
        self.assertEqual(read_u32(0x020BD4AC), 0xE3A00000)
        self.assertEqual(read_u32(0x020BD4B0), 0xE8BD80F8)

        # A press above the TOC content boundary captures the page gesture:
        # pointer/scroll origins, direction zero, history reset, mode three
        # and force-upload at +0x40398.
        self.assertEqual(read_u32(0x020BD4B4), 0xE5D60005)
        self.assertEqual(read_u32(0x020BD4C0), 0xE5970358)
        self.assertEqual(read_u32(0x020BD4C4), 0xEBFFFC2A)
        self.assertEqual(read_u32(0x020BD4C8), 0xE1D610F2)
        self.assertEqual(read_u32(0x020BD4DC), 0xE5C01620)
        self.assertEqual(read_u32(0x020BD4EC), 0xE59F4314)
        self.assertEqual(read_u32(0x020BD4F0), 0xE18700B4)
        self.assertEqual(read_u32(0x020BD4F8), 0xE19720F0)
        self.assertEqual(read_u32(0x020BD504), 0xE18720B0)
        self.assertEqual(read_u32(0x020BD518), 0xE18750B2)
        self.assertEqual(read_u32(0x020BD524), 0xEB000373)
        self.assertEqual(read_u32(0x020BD530), 0xE18710B0)
        self.assertEqual(read_u32(0x020BD53C), 0xE5C10398)
        self.assertEqual(read_u32(0x020BD540), 0xE8BD80F8)

        # Only captured mode three advances the page gesture.  Held input
        # shifts three signed deltas and derives the current direction.
        self.assertEqual(read_u32(0x020BD544), 0xE59F02C0)
        self.assertEqual(read_u32(0x020BD548), 0xE19710F0)
        self.assertEqual(read_u32(0x020BD54C), 0xE3510003)
        self.assertEqual(read_u32(0x020BD558), 0xE2871701)
        self.assertEqual(read_u32(0x020BD568), 0xE5D61004)
        self.assertEqual(read_u32(0x020BD584), 0xE18730B1)
        self.assertEqual(read_u32(0x020BD598), 0xE19730F2)
        self.assertEqual(read_u32(0x020BD5A8), 0xE0413003)
        self.assertEqual(read_u32(0x020BD5AC), 0xE18730B2)
        self.assertEqual(read_u32(0x020BD5BC), 0xE59F224C)
        self.assertEqual(read_u32(0x020BD5C0), 0xB3E00000)
        self.assertEqual(read_u32(0x020BD5C4), 0xA3A00001)

        # Reversals within three pixels freeze effective Y.  Larger reversals
        # rebase the drag origin by +/-3, take the current scroll origin and
        # force the next upload (whose frame path emits sound 11).
        self.assertEqual(read_u32(0x020BD5E8), 0xE1530005)
        self.assertEqual(read_u32(0x020BD5F8), 0xE0510000)
        self.assertEqual(read_u32(0x020BD600), 0xE3500003)
        self.assertEqual(read_u32(0x020BD608), 0xD19710F0)
        self.assertEqual(read_u32(0x020BD618), 0xE18750B0)
        self.assertEqual(read_u32(0x020BD628), 0xE3E0C002)
        self.assertEqual(read_u32(0x020BD630), 0xA3A0C003)
        self.assertEqual(read_u32(0x020BD638), 0xE085500C)
        self.assertEqual(read_u32(0x020BD650), 0xE18750B0)
        self.assertEqual(read_u32(0x020BD658), 0xE5C24398)

        # Held drag computes scrollOrigin + dragOrigin - pointerY and clamps
        # it to [0, extent-322], then stores target_start at +0x40394.
        self.assertEqual(read_u32(0x020BD664), 0xE59F01B0)
        self.assertEqual(read_u32(0x020BD674), 0xE19720F2)
        self.assertEqual(read_u32(0x020BD680), 0xE0424001)
        self.assertEqual(read_u32(0x020BD684), 0xEB000475)
        self.assertEqual(read_u32(0x020BD688), 0xE59F1190)
        self.assertEqual(read_u32(0x020BD694), 0xB3A04000)
        self.assertEqual(read_u32(0x020BD69C), 0xE59F0180)
        self.assertEqual(read_u32(0x020BD6A4), 0xE18730B0)
        self.assertEqual(read_u32(0x020BD6B8), 0xE8BD80F8)

        # Release averages three deltas with signed /3, clamps the result to
        # +/-10 and stores a 1/16-pixel velocity.  A zero newest delta stops.
        self.assertEqual(read_u32(0x020BD6BC), 0xE5D61006)
        self.assertEqual(read_u32(0x020BD6C8), 0xE2401008)
        self.assertEqual(read_u32(0x020BD6E4), 0xE0871082)
        self.assertEqual(read_u32(0x020BD6F8), 0xBAFFFFF9)
        self.assertEqual(read_u32(0x020BD70C), 0xE59F1114)
        self.assertEqual(read_u32(0x020BD714), 0xE0C40391)
        self.assertEqual(read_u32(0x020BD71C), 0xE354000A)
        self.assertEqual(read_u32(0x020BD72C), 0xE59F00F8)
        self.assertEqual(read_u32(0x020BD730), 0xE1A01204)
        self.assertEqual(read_u32(0x020BD740), 0xE18750B0)

        # Inertia applies signed velocity/16, clamps the target, decays its
        # magnitude by seven and releases interaction mode on zero/arrival.
        self.assertEqual(read_u32(0x020BD744), 0xE59F40E0)
        self.assertEqual(read_u32(0x020BD750), 0x0A000025)
        self.assertEqual(read_u32(0x020BD75C), 0xEB00043F)
        self.assertEqual(read_u32(0x020BD770), 0xE1A031C6)
        self.assertEqual(read_u32(0x020BD77C), 0xE0540242)
        self.assertEqual(read_u32(0x020BD780), 0x43A00000)
        self.assertEqual(read_u32(0x020BD78C), 0xE59F0090)
        self.assertEqual(read_u32(0x020BD790), 0xE18710B0)
        self.assertEqual(read_u32(0x020BD7AC), 0xB2600000)
        self.assertEqual(read_u32(0x020BD7B8), 0xE2402007)
        self.assertEqual(read_u32(0x020BD7CC), 0x12622000)
        self.assertEqual(read_u32(0x020BD7E4), 0xE1530001)
        self.assertEqual(read_u32(0x020BD7E8), 0x018750B0)
        self.assertEqual(read_u32(0x020BD7F8), 0x02800002)
        self.assertEqual(read_u32(0x020BD7FC), 0x018750B0)
        self.assertEqual(read_u32(0x020BD800), 0xE3A00001)
        self.assertEqual(read_u32(0x020BD804), 0xE8BD80F8)

        # The literal pool proves every recovered offset and constant.  The
        # next independent function begins at 0x020bd830.
        self.assertEqual(read_u32(0x020BD808), 0x0004060C)
        self.assertEqual(read_u32(0x020BD80C), 0x0004061E)
        self.assertEqual(read_u32(0x020BD810), 0x00040610)
        self.assertEqual(read_u32(0x020BD814), 0x0004060E)
        self.assertEqual(read_u32(0x020BD818), 0x00040614)
        self.assertEqual(read_u32(0x020BD81C), 0x00040612)
        self.assertEqual(read_u32(0x020BD820), 0xFFFFFEBE)
        self.assertEqual(read_u32(0x020BD824), 0x00040394)
        self.assertEqual(read_u32(0x020BD828), 0x55555556)
        self.assertEqual(read_u32(0x020BD82C), 0x0004061C)
        self.assertEqual(read_u32(0x020BD830), 0xE92D40F8)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_page_pointer_test"
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
            output = Path(temporary) / "manual_page_pointer.o"
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
