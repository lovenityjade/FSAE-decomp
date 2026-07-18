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
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_peer_list.c"
HARNESS = Path(__file__).with_name("datalink_peer_list_harness.c")

FUNCTIONS = (
    (
        "count_ready",
        0x020B07BC,
        0x020B0800,
        0x020B0800,
        "8241ecf25331215f60c019c1b50fe48421d53046e11d5e7583067351024001d2",
        "8241ecf25331215f60c019c1b50fe48421d53046e11d5e7583067351024001d2",
        (),
    ),
    (
        "upsert",
        0x020B0800,
        0x020B09F0,
        0x020B09F4,
        "f09310e493b49692c5bb9999a507c8a9dc9bb4ed6055ac25b4d299c30a439ca0",
        "21b88c196eb6b7d1d7e05842db94277b064015d0608fdcea5792aa7f4851238d",
        (0x020DED24,),
    ),
    (
        "initialize_visuals",
        0x020B09F4,
        0x020B0A68,
        0x020B0A6C,
        "a6e0063b011f00c8ba76a3d91d94dedcf2f3a17fb3de7ceecb54081211ffeca5",
        "e1c40c14daffecc54fa78dfe2e5e32f343ecd03780eb0f3ab9e740e21e4fce25",
        (0x0217EB2C,),
    ),
    (
        "arrange_sprites",
        0x020B0A6C,
        0x020B0B38,
        0x020B0B40,
        "b30f9cc45a30879139b27553f46d3fa1c5e2eb1aca6bd82ba61949440966b37c",
        "b2b0941af367bd81388f9f33aa3c09697864ef6fd3957b558682bca65ab4dc58",
        (0x0217EB2C, 0x021263CC),
    ),
    (
        "remove",
        0x020B0B40,
        0x020B0C34,
        0x020B0C38,
        "42aa963fd4f9ac754833d37f01c68f2a3414363c9591ee70c65b305924635517",
        "45345e67cd56d2a8b614fa25e4efdf2bc5237f5c078705f944b06765dff827cc",
        (0x0217EB2C,),
    ),
    (
        "sort",
        0x020B0C38,
        0x020B0D54,
        0x020B0D58,
        "96a3d4d89acbdc78cda1ae67a4522929d6e8797b2fbbb1d5fd5ff0e65d99a333",
        "1c0c7f7251d59285fa30d4e9c8e620bf2e2c202b1495b689216f508c250b1ed4",
        (0x0217EB2C,),
    ),
    (
        "clear",
        0x020B0D58,
        0x020B0D7C,
        0x020B0D7C,
        "3d75704dbc9dc4909284f997a447a8b8973d40b75083d59732ad9b45aed73588",
        "3d75704dbc9dc4909284f997a447a8b8973d40b75083d59732ad9b45aed73588",
        (),
    ),
)


class DatalinkPeerListRecoveryTests(unittest.TestCase):
    def test_rom_bodies_pools_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def body(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        total_body_bytes = 0
        for (
            name,
            start,
            body_end,
            extent_end,
            body_sha256,
            extent_sha256,
            pool_words,
        ) in FUNCTIONS:
            with self.subTest(function=name):
                recovered = body(start, body_end)
                extent = body(start, extent_end)
                total_body_bytes += len(recovered)
                self.assertEqual(
                    hashlib.sha256(recovered).hexdigest(),
                    body_sha256,
                )
                self.assertEqual(
                    hashlib.sha256(extent).hexdigest(),
                    extent_sha256,
                )
                self.assertEqual(
                    tuple(
                        read_u32(body_end + index * 4)
                        for index in range(len(pool_words))
                    ),
                    pool_words,
                )

        self.assertEqual(total_body_bytes, 1448)
        self.assertEqual(read_u32(0x020B07BC), 0xE59010C8)
        self.assertEqual(read_u32(0x020B07FC), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020B0800), 0xE92D47F0)
        self.assertEqual(read_u32(0x020B09EC), 0xE8BD87F0)
        self.assertEqual(read_u32(0x020B09F4), 0xE92D40F8)
        self.assertEqual(read_u32(0x020B0A64), 0xE8BD80F8)
        self.assertEqual(read_u32(0x020B0A6C), 0xE92D41F8)
        self.assertEqual(read_u32(0x020B0B34), 0xE8BD81F8)
        self.assertEqual(read_u32(0x020B0B40), 0xE92D4FF8)
        self.assertEqual(read_u32(0x020B0C30), 0xE8BD8FF8)
        self.assertEqual(read_u32(0x020B0C38), 0xE92D4FF8)
        self.assertEqual(read_u32(0x020B0D50), 0xE8BD8FF8)
        self.assertEqual(read_u32(0x020B0D58), 0xE3A03000)
        self.assertEqual(read_u32(0x020B0D78), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020B0D7C), 0xE92D47F0)

        tween_indices = struct.unpack(
            "<8I", body(0x020DED24, 0x020DED44)
        )
        self.assertEqual(
            tween_indices,
            (0x23, 0x27, 0x29, 0x2B, 0x2D, 0x2F, 0x31, 0x33),
        )
        sprite_offsets = struct.unpack(
            "<4i", body(0x021263CC, 0x021263DC)
        )
        self.assertEqual(sprite_offsets, (-72, -8, 40, -8))

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
