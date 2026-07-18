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
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_lobby_scene_initialize.c"
HARNESS = Path(__file__).with_name(
    "datalink_lobby_scene_initialize_harness.c"
)


class DatalinkLobbySceneInitializeRecoveryTests(unittest.TestCase):
    def test_rom_body_pool_hashes_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        start = 0x020B1378
        end = 0x020B18DC
        pool_end = 0x020B1920
        body = arm9[start - ARM9_BASE : end - ARM9_BASE]
        pool = arm9[end - ARM9_BASE : pool_end - ARM9_BASE]
        combined = arm9[start - ARM9_BASE : pool_end - ARM9_BASE]

        self.assertEqual(len(body), 1380)
        self.assertEqual(len(pool), 68)
        self.assertEqual(len(combined), 1448)
        self.assertEqual(
            hashlib.sha256(body).hexdigest(),
            "a334031a70611580bcc2e66ef8d9749a1cf3bb248ad576b2e02ae9e9af13a067",
        )
        self.assertEqual(
            hashlib.sha256(pool).hexdigest(),
            "9cfd4e7dc196d628617df40ddd3a820f57f451d441c4ffc4a720cc127022b065",
        )
        self.assertEqual(
            hashlib.sha256(combined).hexdigest(),
            "4ebae1600993efe4f744f1e1c123a0a8dfd06b20f5afe3c2f5ef5f74af2b38c7",
        )

        self.assertEqual(read_u32(0x020B1374), 0x0217D23C)
        self.assertEqual(read_u32(0x020B1378), 0xE92D4FF8)
        self.assertEqual(read_u32(0x020B137C), 0xE24DDC01)
        self.assertEqual(read_u32(0x020B13B0), 0xEBFD921C)
        self.assertEqual(read_u32(0x020B13D8), 0xEBFF735A)
        self.assertEqual(read_u32(0x020B1438), 0xEBFFC177)
        self.assertEqual(read_u32(0x020B147C), 0xEBFFEDDC)
        self.assertEqual(read_u32(0x020B1494), 0xEBFFFF62)
        self.assertEqual(read_u32(0x020B1514), 0xEBFFAB2D)
        self.assertEqual(read_u32(0x020B1548), 0xEBFFEC3F)
        self.assertEqual(read_u32(0x020B15AC), 0xEBFFEC8A)
        self.assertEqual(read_u32(0x020B1618), 0xEBFFAA19)
        self.assertEqual(read_u32(0x020B1638), 0xEB004A74)
        self.assertEqual(read_u32(0x020B166C), 0xEBFFAE41)
        self.assertEqual(read_u32(0x020B1748), 0xEBFFC4F1)
        self.assertEqual(read_u32(0x020B1750), 0xEBFFC808)
        self.assertEqual(read_u32(0x020B18AC), 0xEBFF7C45)
        self.assertEqual(read_u32(0x020B18D0), 0xEB0009DB)
        self.assertEqual(read_u32(0x020B18D4), 0xE28DDC01)
        self.assertEqual(read_u32(0x020B18D8), 0xE8BD8FF8)

        expected_pool = (
            0x0217D348,
            0x021818A8,
            0x02181860,
            0x0216F020,
            0x02171C84,
            0x0217C23C,
            0x02126A34,
            0x0217A2F8,
            0x0217A2D8,
            0x020DEDD4,
            0x0217A258,
            0x0217A23C,
            0x0211C168,
            0x020DE834,
            0x04001000,
            0x0212C9A8,
            0x020DE840,
        )
        self.assertEqual(
            tuple(read_u32(end + index * 4) for index in range(17)),
            expected_pool,
        )
        self.assertEqual(read_u32(0x020B1920), 0xE92D4010)

    def test_tween_descriptor_table(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()
        offset = 0x020DEDD4 - ARM9_BASE
        words = struct.unpack_from("<64I", arm9, offset)
        expected = (
            0, 0, 1, 0, 2, 0, 3, 0,
            5, 0, 6, 0, 4, 0, 4, 0,
            4, 0, 4, 0, 7, 0, 7, 0,
            7, 0, 7, 0, 13, 0, 14, 0,
            15, 0, 16, 0, 0x80, 0x1000, 0x81, 0x1000,
            0x82, 0x1000, 0x83, 0x1000, 0x84, 0x1000, 17, 0,
            18, 0, 19, 0, 20, 0, 21, 0x1000,
            0xFFFFFFFF, 0xFFFFFFFE, 0xFFFFFFFD, 0xFFFFFFFC,
            0, 0, 0, 0,
        )
        self.assertEqual(words, expected)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "datalink_lobby_scene_initialize_test"
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
            output = Path(temporary) / "datalink_lobby_scene_initialize.o"
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
