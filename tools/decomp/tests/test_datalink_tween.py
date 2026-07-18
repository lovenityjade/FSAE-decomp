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
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_tween.c"
HARNESS = Path(__file__).with_name("datalink_tween_harness.c")


class DatalinkTweenRecoveryTests(unittest.TestCase):
    def test_initializer_rom_body_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        start = 0x020ACBF4
        end = 0x020ACC28
        body = arm9[start - ARM9_BASE : end - ARM9_BASE]
        with_literal = arm9[start - ARM9_BASE : 0x020ACC2C - ARM9_BASE]
        self.assertEqual(len(body), 52)
        self.assertEqual(len(with_literal), 56)
        self.assertEqual(
            hashlib.sha256(body).hexdigest(),
            "df8f118ba350ee9413ffc9561343e021f429ced696db6027939dce7baaf08a25",
        )
        self.assertEqual(
            hashlib.sha256(with_literal).hexdigest(),
            "512c959625e484141f452656889adb66a2af93056848f6c748afa6ac55f2d2b5",
        )
        self.assertEqual(with_literal[:52], body)
        self.assertEqual(read_u32(0x020ACBF0), 0x00000517)
        self.assertEqual(read_u32(0x020ACBF4), 0xE92D4038)
        self.assertEqual(read_u32(0x020ACC00), 0xE3A02058)
        self.assertEqual(read_u32(0x020ACC08), 0xEBFDA3E1)
        self.assertEqual(read_u32(0x020ACC0C), 0xE3A01001)
        self.assertEqual(read_u32(0x020ACC10), 0xE59F0010)
        self.assertEqual(read_u32(0x020ACC14), 0xE5C54000)
        self.assertEqual(read_u32(0x020ACC18), 0xE5C51003)
        self.assertEqual(read_u32(0x020ACC1C), 0xE5850010)
        self.assertEqual(read_u32(0x020ACC20), 0xE5C51001)
        self.assertEqual(read_u32(0x020ACC24), 0xE8BD8038)
        self.assertEqual(read_u32(0x020ACC28), 0x020ACAB4)
        self.assertEqual(read_u32(0x020ACC2C), 0xE92D4010)

    def test_rom_body_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        start = 0x020ACC7C
        end = 0x020ACCA0
        body = arm9[start - ARM9_BASE : end - ARM9_BASE]
        self.assertEqual(len(body), 36)
        self.assertEqual(
            hashlib.sha256(body).hexdigest(),
            "f4c11250950db4a5bd0520e902bd76aba75bd73f61efeb71df3dca2196530661",
        )
        self.assertEqual(read_u32(0x020ACC78), 0xE8BD8010)
        self.assertEqual(read_u32(0x020ACC7C), 0xE590302C)
        self.assertEqual(read_u32(0x020ACC88), 0xE580102C)
        self.assertEqual(read_u32(0x020ACC90), 0xA580202C)
        self.assertEqual(read_u32(0x020ACC9C), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020ACCA0), 0xE92D4038)

        # Catalog body_bytes=112 is exact: the apparent 124-byte span includes
        # a separate three-word callback pool at 0x020acd10..0x020acd1b.
        configure_start = 0x020ACCA0
        configure_end = 0x020ACD10
        configure = arm9[
            configure_start - ARM9_BASE : configure_end - ARM9_BASE
        ]
        pool = arm9[0x020ACD10 - ARM9_BASE : 0x020ACD1C - ARM9_BASE]
        combined = arm9[
            configure_start - ARM9_BASE : 0x020ACD1C - ARM9_BASE
        ]
        self.assertEqual(len(configure), 112)
        self.assertEqual(len(pool), 12)
        self.assertEqual(len(combined), 124)
        self.assertEqual(
            hashlib.sha256(configure).hexdigest(),
            "991b76d8e25a5c8d6799b14d21ea0f1bb96b2631e38d41b17350ba687d405322",
        )
        self.assertEqual(
            hashlib.sha256(pool).hexdigest(),
            "de5d840338a97779117b9f5da364c9f10a8457ebef8f2091922aa2272c9ccd7f",
        )
        self.assertEqual(
            hashlib.sha256(combined).hexdigest(),
            "5d3de87d13cdf8dcd2c984fb97c8a7c607bbc6310d8e47a02abe8dd3b53c543b",
        )
        self.assertEqual(read_u32(0x020ACCA4), 0xE5905030)
        self.assertEqual(read_u32(0x020ACCA8), 0xE1A0E603)
        self.assertEqual(read_u32(0x020ACCAC), 0xE5904034)
        self.assertEqual(read_u32(0x020ACCB0), 0xE59DC010)
        self.assertEqual(read_u32(0x020ACCB8), 0xE5805014)
        self.assertEqual(read_u32(0x020ACCC0), 0xE580101C)
        self.assertEqual(read_u32(0x020ACCC8), 0xE580E028)
        self.assertEqual(read_u32(0x020ACCCC), 0xE580302C)
        self.assertEqual(read_u32(0x020ACCD0), 0xE35C0003)
        self.assertEqual(read_u32(0x020ACCD4), 0x908FF10C)
        self.assertEqual(read_u32(0x020ACCE8), 0xE8BD8038)
        self.assertEqual(read_u32(0x020ACCEC), 0xE59F101C)
        self.assertEqual(read_u32(0x020ACD0C), 0xE8BD8038)
        self.assertEqual(read_u32(0x020ACD10), 0x020ACAB4)
        self.assertEqual(read_u32(0x020ACD14), 0x020ACB04)
        self.assertEqual(read_u32(0x020ACD18), 0x020ACB7C)
        self.assertEqual(read_u32(0x020ACD1C), 0xE92D4038)

        update_start = 0x020ACD1C
        update_end = 0x020ACD74
        update = arm9[
            update_start - ARM9_BASE : update_end - ARM9_BASE
        ]
        self.assertEqual(len(update), 88)
        self.assertEqual(
            hashlib.sha256(update).hexdigest(),
            "f568a5c25242b15d7a52a83f8df0e685f27b2fa2472c5dcc287f79576673f9fe",
        )
        self.assertEqual(read_u32(0x020ACD20), 0xE24DD010)
        self.assertEqual(read_u32(0x020ACD28), 0xEBFFFFD3)
        self.assertEqual(read_u32(0x020ACD34), 0xEB00000E)
        self.assertEqual(read_u32(0x020ACD3C), 0xE595C010)
        self.assertEqual(read_u32(0x020ACD4C), 0xE12FFF3C)
        self.assertEqual(read_u32(0x020ACD64), 0xE5852030)
        self.assertEqual(read_u32(0x020ACD68), 0xE5851034)
        self.assertEqual(read_u32(0x020ACD70), 0xE8BD8038)
        self.assertEqual(read_u32(0x020ACD74), 0xE92D4008)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "datalink_tween_test"
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
            output = Path(temporary) / "datalink_tween.o"
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
