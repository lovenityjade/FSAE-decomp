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
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_scene_initialize.c"
HARNESS = Path(__file__).with_name("datalink_scene_initialize_harness.c")


class DatalinkSceneInitializeRecoveryTests(unittest.TestCase):
    def test_rom_body_boundaries_hash_and_sequence_table(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        start = 0x020AD070
        end = 0x020AD4EC
        body = arm9[start - ARM9_BASE : end - ARM9_BASE]
        self.assertEqual(len(body), 1148)
        self.assertEqual(
            hashlib.sha256(body).hexdigest(),
            "bd020e8ba3f9024ad878d0fb3d1aaa900e3a24ccb809bca31a9411ef5aae9fb9",
        )

        anchors = {
            0x020AD06C: 0xE8BD8070,
            0x020AD070: 0xE92D4FF0,
            0x020AD074: 0xE24DD03C,
            0x020AD084: 0xE3A02019,
            0x020AD09C: 0x1AFFFFF9,
            0x020AD0AC: 0xE3110001,
            0x020AD0BC: 0x05802018,
            0x020AD0CC: 0xE3120001,
            0x020AD0E8: 0xE5803030,
            0x020AD0F4: 0xEBFFFF27,
            0x020AD110: 0xEBFF8413,
            0x020AD124: 0xEBFF840E,
            0x020AD134: 0xEBFFFFB9,
            0x020AD148: 0xEBFDA291,
            0x020AD178: 0xEB0000EA,
            0x020AD1AC: 0xEBFFFC9D,
            0x020AD1CC: 0xEBFFFCF0,
            0x020AD20C: 0xEBFFFD0E,
            0x020AD224: 0xEBFFFD08,
            0x020AD234: 0xEBFF83CA,
            0x020AD24C: 0xEBFF83C4,
            0x020AD2AC: 0xEBFFFCFF,
            0x020AD2BC: 0xEBFFFD46,
            0x020AD2D8: 0xEBFFFCF4,
            0x020AD2EC: 0xEBFFFD3A,
            0x020AD304: 0xE5841000,
            0x020AD308: 0xEB005A0D,
            0x020AD314: 0xEB005A13,
            0x020AD318: 0xEBFFBAF1,
            0x020AD354: 0xEBFFBACA,
            0x020AD390: 0xBAFFFFE8,
            0x020AD3C8: 0xEBFFBAAD,
            0x020AD40C: 0xBAFFFFE6,
            0x020AD438: 0xE5C1343E,
            0x020AD450: 0x3AFFFFF3,
            0x020AD460: 0xE5812808,
            0x020AD464: 0xEBFFD8C3,
            0x020AD468: 0xEBFF8177,
            0x020AD480: 0xE58A2434,
            0x020AD484: 0xE58A1438,
            0x020AD4A4: 0x0A000007,
            0x020AD4B4: 0xE12FFF31,
            0x020AD4C4: 0xE12FFF31,
            0x020AD4D0: 0xEBFFFE36,
            0x020AD4DC: 0xE58A1430,
            0x020AD4E8: 0xE8BD8FF0,
            0x020AD4EC: 0x020DE964,
            0x020AD524: 0x021348FC,
            0x020AD528: 0xE92D4FF8,
        }
        for address, expected in anchors.items():
            self.assertEqual(read_u32(address), expected, hex(address))

        expected_sequences = bytes(
            list(range(35))
            + [36]
            + [32, 33] * 7
            + [37]
        )
        table_start = 0x020DE964 - ARM9_BASE
        self.assertEqual(len(expected_sequences), 51)
        self.assertEqual(
            arm9[table_start : table_start + len(expected_sequences)],
            expected_sequences,
        )

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "datalink_scene_initialize_test"
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
            output = Path(temporary) / "datalink_scene_initialize.o"
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
