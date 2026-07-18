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
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_tween_math.c"
HARNESS = Path(__file__).with_name("datalink_tween_math_harness.c")


class DatalinkTweenMathRecoveryTests(unittest.TestCase):
    def test_rom_body_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def body(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        linear = body(0x020ACAB4, 0x020ACB04)
        self.assertEqual(len(linear), 80)
        self.assertEqual(
            hashlib.sha256(linear).hexdigest(),
            "5e1091b25b1f3fe0372bafa85e10bbeda8ab1a8db18af273b13db7354060fb1a",
        )

        self.assertEqual(read_u32(0x020ACAB0), 0x0217A23C)
        self.assertEqual(read_u32(0x020ACAB4), 0xE92D4010)
        self.assertEqual(read_u32(0x020ACB00), 0xE8BD8010)
        self.assertEqual(read_u32(0x020ACB04), 0xE92D4010)

        ease_in = body(0x020ACB04, 0x020ACB74)
        self.assertEqual(len(ease_in), 112)
        self.assertEqual(
            hashlib.sha256(ease_in).hexdigest(),
            "b83fa61a87daa62cbbdcc41c30b88064738c49345276aa1bf182cbf66522f683",
        )
        self.assertEqual(read_u32(0x020ACB70), 0xE8BD8010)
        self.assertEqual(read_u32(0x020ACB74), 0x020C91FC)
        self.assertEqual(read_u32(0x020ACB78), 0xFFFFFAE9)
        self.assertEqual(read_u32(0x020ACB7C), 0xE92D4010)
        self.assertEqual(
            hashlib.sha256(body(0x020ACB04, 0x020ACB7C)).hexdigest(),
            "c0ca6b65707879ec721ba53c6e9d7426d497cd1b3f9d5b43757f315f186cd9c6",
        )

        ease_out = body(0x020ACB7C, 0x020ACBEC)
        self.assertEqual(len(ease_out), 112)
        self.assertEqual(
            hashlib.sha256(ease_out).hexdigest(),
            "b83fa61a87daa62cbbdcc41c30b88064738c49345276aa1bf182cbf66522f683",
        )
        self.assertEqual(read_u32(0x020ACBE8), 0xE8BD8010)
        self.assertEqual(read_u32(0x020ACBEC), 0x020C91FC)
        self.assertEqual(read_u32(0x020ACBF0), 0x00000517)
        self.assertEqual(read_u32(0x020ACBF4), 0xE92D4038)
        self.assertEqual(
            hashlib.sha256(body(0x020ACB7C, 0x020ACBF4)).hexdigest(),
            "dcff3af326c467d6da0820c3c57b76b15c5895060e0cb10206734c001bae38e5",
        )

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "datalink_tween_math_test"
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
            output = Path(temporary) / "datalink_tween_math.o"
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
