from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_peer_roster.c"
FUNCTIONS = (
    (0x020B0D7C, 264, "fa2a462036d38a17302c821f9d26fc1d75971d078ec865534da10d2339adc021"),
    (0x020B0E84, 264, "364cd91d8045da5725daa194bb4c00d3ec02f956bf4866ed80e8fb7d4f6d5ec0"),
    (0x020B0F8C, 92, "2c298c6c447dddf7a6974413c1194edcab663771b388ddb769e7409617715712"),
    (0x020B0FE8, 16, "6e1183aa65e0056f60bcd4c81f864c3a65d50eb073b5bd0819a95b6fa6ecdfd6"),
    (0x020B0FF8, 136, "66ba29152a071ad22748b544e311381dffd9adefa7404fe2b26b701716626167"),
    (0x020B108C, 64, "7771745198d7155040f68b0a7dc416ca713f8557d2eb5b88869a0659727db01a"),
    (0x020B10D4, 184, "24e2b8a8b9713744a1dc076dfaec9b2cfa45ff62809562bf600f900a97e8115f"),
    (0x020B11A0, 40, "de3ee14bb37933780adbcb6b395331646d82b74d3fdf8b115192f19b619f4a9f"),
    (0x020B11DC, 68, "4f69fbb769178d8d78c87cff0b7f43da96cbe8d62b68ec6672488879c790b40b"),
    (0x020B1224, 308, "15cc04f787b87d01ca1c615793855cad7aa0ab16380fcb1fac8b1125a8213f96"),
    (0x020B1920, 172, "16c4f1cbfe546db7e3b0a880cdb5a90e6c69faac74acbe589b7e615c8e0a506d"),
)


class DatalinkPeerRosterRecoveryTests(unittest.TestCase):
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
