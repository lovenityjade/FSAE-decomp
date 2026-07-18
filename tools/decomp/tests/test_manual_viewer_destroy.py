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
SOURCE = PROJECT_ROOT / "src/arm9/game/manual_viewer_destroy.c"


class ManualViewerDestroyRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        expected = {
            0x020BC6C4: 0xE92D4078,
            0x020BC6D0: 0xEB00063A,
            0x020BC6EC: 0xEB000049,
            0x020BC704: 0xEB000038,
            0x020BC710: 0xEB000028,
            0x020BC71C: 0xEB00001A,
            0x020BC728: 0xEB000017,
            0x020BC73C: 0xEB001941,
            0x020BC750: 0xEB00193C,
            0x020BC75C: 0xEBFFEA66,
            0x020BC768: 0xEBFFEA63,
            0x020BC774: 0xEB00074F,
            0x020BC77C: 0xEB00093E,
            0x020BC788: 0xE8BD8078,
            0x020BC78C: 0xE92D4038,
            0x020BC7A4: 0xEBFFFE94,
            0x020BC7B4: 0xE8BD8038,
            0x020BC7B8: 0xE92D4038,
            0x020BC7D8: 0xE12FFF32,
            0x020BC7E8: 0xE8BD8038,
            0x020BC7EC: 0xE92D4038,
            0x020BC804: 0xEBFFF101,
            0x020BC814: 0xE8BD8038,
            0x020BC818: 0xE92D4038,
            0x020BC838: 0xE12FFF32,
            0x020BC848: 0xE8BD8038,
        }
        for address, instruction in expected.items():
            self.assertEqual(read_u32(address), instruction)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_viewer_destroy_test"
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
                        "manual_viewer_destroy_harness.c")),
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
            output = Path(temporary) / "manual_viewer_destroy.o"
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
