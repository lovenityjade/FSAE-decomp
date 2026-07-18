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
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_tween_record_update.c"
HARNESS = Path(__file__).with_name("datalink_tween_record_update_harness.c")


class DatalinkTweenRecordUpdateRecoveryTests(unittest.TestCase):
    def test_rom_body_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        start = 0x020ACC2C
        end = 0x020ACC7C
        body = arm9[start - ARM9_BASE : end - ARM9_BASE]
        self.assertEqual(len(body), 80)
        self.assertEqual(
            hashlib.sha256(body).hexdigest(),
            "2f646d5868a939c077da2134e012146e426e7d539144befef82f0d576f6e2ad9",
        )
        self.assertEqual(read_u32(0x020ACC28), 0x020ACAB4)
        self.assertEqual(read_u32(0x020ACC2C), 0xE92D4010)
        self.assertEqual(read_u32(0x020ACC34), 0xE5941028)
        self.assertEqual(read_u32(0x020ACC3C), 0x0A000001)
        self.assertEqual(read_u32(0x020ACC40), 0xE3A01A01)
        self.assertEqual(read_u32(0x020ACC44), 0xEB000034)
        self.assertEqual(read_u32(0x020ACC48), 0xE5D40005)
        self.assertEqual(read_u32(0x020ACC54), 0xE3500077)
        self.assertEqual(read_u32(0x020ACC58), 0xE5C41005)
        self.assertEqual(read_u32(0x020ACC60), 0x85C40005)
        self.assertEqual(read_u32(0x020ACC64), 0xE5940054)
        self.assertEqual(read_u32(0x020ACC70), 0xE5941050)
        self.assertEqual(read_u32(0x020ACC74), 0xEB005CFA)
        self.assertEqual(read_u32(0x020ACC78), 0xE8BD8010)
        self.assertEqual(read_u32(0x020ACC7C), 0xE590302C)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "datalink_tween_record_update_test"
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
            output = Path(temporary) / "datalink_tween_record_update.o"
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
