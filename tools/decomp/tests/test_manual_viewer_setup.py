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
SOURCE = PROJECT_ROOT / "src/arm9/game/manual_viewer_setup.c"


class ManualViewerSetupRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        expected = {
            0x020BC99C: 0xE92D41F0,
            0x020BC9B4: 0xEB00045E,
            0x020BC9C4: 0xEB0017FF,
            0x020BC9E4: 0xEBFFEA17,
            0x020BCA04: 0xEB0017EF,
            0x020BCA20: 0xEB00041A,
            0x020BCA34: 0xEB000891,
            0x020BCA48: 0xEB00069B,
            0x020BCA60: 0xEB00079E,
            0x020BCA9C: 0xEB001A8B,
            0x020BCAA8: 0xEB0000CD,
            0x020BCAB4: 0xEBFFFD8A,
            0x020BCABC: 0xEB0000C8,
            0x020BCAC8: 0xEBFFFDA8,
            0x020BCACC: 0xEBFFFD3F,
            0x020BCAD4: 0xEB0000B7,
            0x020BCAE4: 0xEBFFFC2D,
            0x020BCB28: 0xEBFFEF21,
            0x020BCB80: 0xEBFFED3C,
            0x020BCB8C: 0xEB00051E,
            0x020BCBBC: 0xEB000067,
            0x020BCBD4: 0xEBFFE0C9,
            0x020BCC04: 0xEB00004A,
            0x020BCC30: 0xEBFFF526,
            0x020BCC48: 0xEB00004F,
            0x020BCC88: 0xEBFFEEC9,
            0x020BCCA0: 0xEBFFF6A8,
            0x020BCCE4: 0xEBFFECE3,
            0x020BCCEC: 0xEBFFFDC2,
            0x020BCCF4: 0xEB0004F2,
            0x020BCCF8: 0xEBFFFDA7,
            0x020BCD00: 0xE8BD81F0,
            0x020BCD04: 0x02127110,
            0x020BCD30: 0x00007FFF,
            0x020BCD34: 0xE92D4008,
        }
        for address, instruction in expected.items():
            self.assertEqual(read_u32(address), instruction)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_viewer_setup_test"
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
                    str(Path(__file__).with_name(
                        "manual_viewer_setup_harness.c")),
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
            output = Path(temporary) / "manual_viewer_setup.o"
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
