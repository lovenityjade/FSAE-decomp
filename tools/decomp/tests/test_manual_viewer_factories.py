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
SOURCE = PROJECT_ROOT / "src/arm9/game/manual_viewer_factories.c"


class ManualViewerFactoriesRecoveryTests(unittest.TestCase):
    def test_rom_instruction_and_boundary_proofs(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        def body(start: int, size: int) -> bytes:
            offset = start - ARM9_BASE
            return arm9[offset : offset + size]

        scoped_factories = (
            (
                0x020BA71C,
                44,
                "9571f63c9664b42e0fb35d0b47eb987662e049285e885097b597bb45f50fff6e",
            ),
            (
                0x020BA774,
                108,
                "67e5a83f59b776934ba82ff971778b8c4eb0ef34a1027938797bc0e662217785",
            ),
            (
                0x020BA7E0,
                44,
                "d7870136d0c192ba38dcad9329d600bef0bc806fd8e12a79b6c69e90c3344cfe",
            ),
        )
        for start, size, expected_hash in scoped_factories:
            recovered = body(start, size)
            self.assertEqual(len(recovered), size)
            self.assertEqual(hashlib.sha256(recovered).hexdigest(), expected_hash)

        expected = {
            0x020BA718: 0x020DEF36,
            0x020BA71C: 0xE92D4008,
            0x020BA724: 0xE3A01024,
            0x020BA744: 0xE8BD8008,
            0x020BA748: 0xE92D4008,
            0x020BA754: 0xEB002136,
            0x020BA764: 0xEBFFF638,
            0x020BA770: 0xE8BD8008,
            0x020BA774: 0xE92D4038,
            0x020BA784: 0xE3A01014,
            0x020BA7C8: 0xEBFFF52C,
            0x020BA7DC: 0xE8BD8038,
            0x020BA7E0: 0xE92D4008,
            0x020BA7E8: 0xE3A01040,
            0x020BA808: 0xE8BD8008,
            0x020BA80C: 0xE92D4038,
            0x020BCD34: 0xE92D4008,
            0x020BCD40: 0xEB0017BB,
            0x020BCD50: 0xEBFFF4AB,
            0x020BCD5C: 0xE8BD8008,
            0x020BCD8C: 0xE92D4008,
            0x020BCD98: 0xEB0017A5,
            0x020BCDA8: 0xEBFFEE75,
            0x020BCDB4: 0xE8BD8008,
            0x020BCDB8: 0xE92D4008,
            0x020BCDC4: 0xEB00179A,
            0x020BCDD4: 0xEBFFFB67,
            0x020BCDE0: 0xE8BD8008,
            0x020BCDE4: 0xE92D4008,
            0x020BCDF0: 0xEB00178F,
            0x020BCE00: 0xEBFFFC53,
            0x020BCE0C: 0xE8BD8008,
            0x020BCE10: 0x00003034,
        }
        for address, instruction in expected.items():
            self.assertEqual(read_u32(address), instruction)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_viewer_factories_test"
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
                        "manual_viewer_factories_harness.c")),
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
            output = Path(temporary) / "manual_viewer_factories.o"
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
