from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import struct
import subprocess
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_scene_oam.c"
HARNESS = Path(__file__).with_name("datalink_scene_oam_harness.c")


class DatalinkSceneOamRecoveryTests(unittest.TestCase):
    def test_rom_body_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def body(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        recovered = body(0x020AD9A4, 0x020ADABC)
        with_pool = body(0x020AD9A4, 0x020ADAC0)
        self.assertEqual(len(recovered), 280)
        self.assertEqual(len(with_pool), 284)
        self.assertEqual(
            hashlib.sha256(recovered).hexdigest(),
            "ef2c3709822bde786760fbd11f1ab02afc0d4c89df050b9ce67cb6b65c155ccf",
        )
        self.assertEqual(
            hashlib.sha256(with_pool).hexdigest(),
            "fa053d356f7e967a73a9f819d7cc7654492a7e18cc3c112d9b87a5a7c8a428a8",
        )

        self.assertEqual(read_u32(0x020AD9A0), 0xE8BD8010)
        self.assertEqual(read_u32(0x020AD9A4), 0xE92D41F0)
        self.assertEqual(read_u32(0x020AD9A8), 0xE24DD060)
        self.assertEqual(read_u32(0x020AD9AC), 0xE59F7108)
        self.assertEqual(read_u32(0x020AD9D4), 0xE59E2000)
        self.assertEqual(read_u32(0x020ADA18), 0xDA000014)
        self.assertEqual(read_u32(0x020ADA70), 0xE3530000)
        self.assertEqual(read_u32(0x020ADA7C), 0xE3510C01)
        self.assertEqual(read_u32(0x020ADA80), 0xB35200C0)
        self.assertEqual(read_u32(0x020ADAA4), 0xE5900540)
        self.assertEqual(read_u32(0x020ADAB0), 0xEB005711)
        self.assertEqual(read_u32(0x020ADAB8), 0xE8BD81F0)
        self.assertEqual(read_u32(0x020ADABC), 0x020DE998)
        self.assertEqual(read_u32(0x020ADAC0), 0xE59F0004)

        dimensions = struct.unpack(
            "<24i", body(0x020DE998, 0x020DE9F8)
        )
        self.assertEqual(
            list(zip(dimensions[::2], dimensions[1::2])),
            [
                (8, 8), (16, 16), (32, 32), (64, 64),
                (16, 8), (32, 8), (32, 16), (64, 32),
                (8, 16), (8, 32), (16, 32), (32, 64),
            ],
        )

    def test_host_c_syntax(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
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
                "-fsyntax-only",
                str(SOURCE),
                str(HARNESS),
            ],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )

    def test_arm946es_syntax(self) -> None:
        compiler = shutil.which("clang")
        if compiler is None:
            self.skipTest("clang is unavailable")
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
                "-fsyntax-only",
                str(SOURCE),
            ],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )


if __name__ == "__main__":
    unittest.main()
