from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_roster.c"
FUNCTIONS = (
    (0x020AFC70, 76, "197f806e0454ec48522ce6e3ffa5d944d2998fdf237811d886af88b75a44a07e"),
    (0x020AFCBC, 164, "8e69d6841c8f690a0e71694b74c50240cdf774bdf13afd9149112152a4733b8c"),
    (0x020AFD84, 152, "ed184cb4ffad230e0ff78023bd70b5e3a6c27f356c48a16e3dc10dfb54490f1b"),
    (0x020AFE3C, 152, "9e991ee3f987c73dccc171ed54870119c7a09c5c0109107beca3128d39872180"),
    (0x020AFEF4, 136, "63f1020539a1bcd404a91aaf34ada8ba676d8c2ac3ac61f5be9fec8ef1c1bc6e"),
    (0x020AFF84, 136, "017e0ffc36a1918d753640e8bb03ac0fb05afcc4c81e53b3532648482fe07962"),
)


class DatalinkRosterRecoveryTests(unittest.TestCase):
    def test_rom_bodies(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()
        for address, size, expected_sha256 in FUNCTIONS:
            body = arm9[address - ARM9_BASE : address - ARM9_BASE + size]
            with self.subTest(address=f"0x{address:08X}"):
                self.assertEqual(len(body), size)
                self.assertEqual(
                    hashlib.sha256(body).hexdigest(), expected_sha256
                )

    def test_host_c_syntax(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        subprocess.run(
            [compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
             "-pedantic", "-I", str(PROJECT_ROOT / "include"),
             "-fsyntax-only", str(SOURCE)],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )

    def test_arm946es_syntax(self) -> None:
        compiler = shutil.which("clang")
        if compiler is None:
            self.skipTest("clang is unavailable")
        subprocess.run(
            [compiler, "--target=arm-none-eabi", "-mcpu=arm946e-s",
             "-marm", "-std=c11", "-Wall", "-Wextra", "-Werror",
             "-pedantic", "-ffreestanding", "-I",
             str(PROJECT_ROOT / "include"), "-fsyntax-only", str(SOURCE)],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )


if __name__ == "__main__":
    unittest.main()
