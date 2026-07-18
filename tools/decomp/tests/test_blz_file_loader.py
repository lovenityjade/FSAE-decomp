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
SOURCE = PROJECT_ROOT / "src/arm9/game/blz_file_loader.c"


class BlzFileLoaderRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        expected = {
            0x020B7B1C: 0xE92D47F8,
            0x020B7B3C: 0xEBFD61E4,
            0x020B7B48: 0xEBFD64E3,
            0x020B7B58: 0x08BD87F8,
            0x020B7B60: 0xEBFD64E1,
            0x020B7B78: 0xEBFD642E,
            0x020B7B90: 0xEBFD643B,
            0x020B7BB0: 0xEBFD6420,
            0x020B7BCC: 0xEB002C18,
            0x020B7BF8: 0xEBFD6421,
            0x020B7C10: 0xEBFD7BB8,
            0x020B7C34: 0xEB002C03,
            0x020B7C40: 0xEBFD63D6,
            0x020B7C4C: 0xE8BD87F8,
        }
        for address, instruction in expected.items():
            self.assertEqual(read_u32(address), instruction)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "blz_file_loader_test"
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
                        "blz_file_loader_harness.c")),
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
            output = Path(temporary) / "blz_file_loader.o"
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
