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
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_scene_lifecycle.c"
HARNESS = Path(__file__).with_name("datalink_scene_lifecycle_harness.c")


class DatalinkSceneLifecycleRecoveryTests(unittest.TestCase):
    def test_rom_bodies_hashes_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def body(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        setup = body(0x020AD528, 0x020AD660)
        setup_pool = body(0x020AD660, 0x020AD668)
        serialize = body(0x020AD668, 0x020AD6BC)
        deserialize = body(0x020AD6BC, 0x020AD710)
        release = body(0x020AD710, 0x020AD87C)
        self.assertEqual(len(setup), 312)
        self.assertEqual(len(setup_pool), 8)
        self.assertEqual(len(serialize), 84)
        self.assertEqual(len(deserialize), 84)
        self.assertEqual(len(release), 364)
        self.assertEqual(
            len(setup) + len(serialize) + len(deserialize) + len(release),
            844,
        )
        self.assertEqual(
            hashlib.sha256(setup).hexdigest(),
            "c4fb309831f6953510667597438f54d01fd16f7756e70d22985f5e2352e1c759",
        )
        self.assertEqual(
            hashlib.sha256(setup_pool).hexdigest(),
            "fff16dfc80d0e87cab0edc32de3554e2f15d20cb187b15fda8cf4a9c266d8324",
        )
        self.assertEqual(
            hashlib.sha256(serialize).hexdigest(),
            "0b89613dcf518646514aed854b96bf25fc8b77624bdd5ce71f92a23f558215d5",
        )
        self.assertEqual(
            hashlib.sha256(deserialize).hexdigest(),
            "1f587b0c2aa9982bdef315ada876c76d41e313156ec9f108893c3df703098ce2",
        )
        self.assertEqual(
            hashlib.sha256(release).hexdigest(),
            "81b17224e4b3fd8760feb5feb337325af017f8912f01fb4b73d15768742e42cc",
        )

        self.assertEqual(read_u32(0x020AD524), 0x021348FC)
        self.assertEqual(read_u32(0x020AD528), 0xE92D4FF8)
        self.assertEqual(read_u32(0x020AD534), 0xEBFD9686)
        self.assertEqual(read_u32(0x020AD544), 0xE3A00080)
        self.assertEqual(read_u32(0x020AD54C), 0xE59F810C)
        self.assertEqual(read_u32(0x020AD5B8), 0xEBFDA154)
        self.assertEqual(read_u32(0x020AD65C), 0xE8BD8FF8)
        self.assertEqual(read_u32(0x020AD660), 0x0400100E)
        self.assertEqual(read_u32(0x020AD664), 0x0217D23C)

        self.assertEqual(read_u32(0x020AD668), 0xE92D4070)
        self.assertEqual(read_u32(0x020AD674), 0xEBFFFE08)
        self.assertEqual(read_u32(0x020AD68C), 0xEBFDA1EA)
        self.assertEqual(read_u32(0x020AD6B8), 0xE8BD8070)
        self.assertEqual(read_u32(0x020AD6BC), 0xE92D4070)
        self.assertEqual(read_u32(0x020AD6C8), 0xEBFFFE23)
        self.assertEqual(read_u32(0x020AD6E0), 0xEBFDA1D5)
        self.assertEqual(read_u32(0x020AD70C), 0xE8BD8070)

        self.assertEqual(read_u32(0x020AD710), 0xE92D41F0)
        self.assertEqual(read_u32(0x020AD71C), 0xE5910544)
        self.assertEqual(read_u32(0x020AD73C), 0xE286001C)
        self.assertEqual(read_u32(0x020AD7BC), 0xEBFFBFB3)
        self.assertEqual(read_u32(0x020AD870), 0x3AFFFFD0)
        self.assertEqual(read_u32(0x020AD874), 0xEBFFB99A)
        self.assertEqual(read_u32(0x020AD878), 0xE8BD81F0)
        self.assertEqual(read_u32(0x020AD87C), 0xE92D40F8)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "datalink_scene_lifecycle_test"
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
            output = Path(temporary) / "datalink_scene_lifecycle.o"
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
