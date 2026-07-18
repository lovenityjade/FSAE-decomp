from __future__ import annotations

import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000
SOURCE = PROJECT_ROOT / "src/arm9/game/data_manager.c"
HARNESS = Path(__file__).with_name("data_manager_harness.c")


class DataManagerRecoveryTests(unittest.TestCase):
    def test_rom_sequence_handle_helpers(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        expected = {
            0x0208C4D4: 0xE2800018,
            0x0208C4D8: 0xE12FFF1E,
            0x0208C63C: 0xE59F0004,
            0x0208C640: 0xE59FC004,
            0x0208C644: 0xE12FFF1C,
            0x0208C648: 0x02171F74,
            0x0208C64C: 0x0208C250,
            0x0208C650: 0xE1A01000,
            0x0208C654: 0xE59F0004,
            0x0208C658: 0xE59FC004,
            0x0208C65C: 0xE12FFF1C,
            0x0208C660: 0x02171F74,
            0x0208C664: 0x0208C2A4,
        }
        for address, instruction in expected.items():
            self.assertEqual(read_u32(address), instruction)

        catalog = json.loads(
            (PROJECT_ROOT / "build/decomp/arm9/catalog.json").read_text()
        )
        by_entry = {
            item["entry"]: item
            for item in catalog["functions"]
            if item.get("entry")
            in {"0x0208c4d4", "0x0208c63c", "0x0208c650"}
        }
        self.assertEqual(by_entry["0x0208c4d4"]["body_bytes"], 8)
        self.assertEqual(by_entry["0x0208c63c"]["body_bytes"], 12)
        self.assertEqual(by_entry["0x0208c650"]["body_bytes"], 16)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")

        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "data_manager_test"
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

    def test_arm946e_s_syntax(self) -> None:
        compiler = shutil.which("clang")
        if compiler is None:
            self.skipTest("clang is unavailable")

        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "data_manager.o"
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
