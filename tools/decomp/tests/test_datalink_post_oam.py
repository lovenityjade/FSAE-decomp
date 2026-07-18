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
SOURCES = (
    PROJECT_ROOT / "src/arm9/game/datalink_post_oam_scene.c",
    PROJECT_ROOT / "src/arm9/game/datalink_post_oam_palette.c",
)
HARNESS = Path(__file__).with_name("datalink_post_oam_harness.c")

FUNCTIONS = (
    (
        "initialize_scene_graphics",
        0x020ADB04,
        0x020ADB64,
        0x020ADB68,
        "6c35b87ad98dfbfde0a75906c455bf7b45f96f01ff32c79f8879769d6d082cf3",
        "b009073280c352f8247839f9db810c600b6b0747d0f5bb01fb3da11b0b49c847",
        (0x04001000,),
    ),
    (
        "release_scene_graphics",
        0x020ADB68,
        0x020ADBB0,
        0x020ADBB8,
        "ac67116b539b6dddac8f235171c29edb41f8582258bfaf03d7cbafbc823eb30e",
        "cb61e3e415d446fd1066987f2d0c0cc304222f1afbf165992db8aa5534faa28c",
        (0x0217A258, 0x0217D23C),
    ),
    (
        "render_remote_participants",
        0x020ADBB8,
        0x020ADCE8,
        0x020ADCFC,
        "f4a6cd9c19ec50bcb3b50de99479a773eb5cd71ba389e0b30d8da661b02c2c85",
        "041a98227d7cfef24577dac7d379127f5e7fe23a42a88c8ce00584598c93ca99",
        (0x0212C7C8, 0x0212C5BC, 0x0217D348, 0x0212CA20, 0x0217A23C),
    ),
    (
        "commit_pending_resource",
        0x020ADF84,
        0x020ADFDC,
        0x020ADFE4,
        "972b92b3915d459696943170f80b0eca4a293f3ce3528aa2cb360f756d1aa705",
        "ce8ce8c400920d4068cf502af8588b58e5cb4cc3923df69bfea1166fa29330de",
        (0x02180DA8, 0x02180E24),
    ),
    (
        "upload_palette_and_objects",
        0x020ADFF0,
        0x020AE118,
        0x020AE130,
        "52ee0720542ee45437c876a8bfaa5bf68be16a14a1f56020472b9090ad0676d1",
        "eb5adcc271c236b5e68ca3f04fe55d47b6ab8e0095dfc2b8ddaedd9fe238aace",
        (0x02180EE0, 0x02180DA8, 0x0217D23C, 0x02180FE0, 0x02180E24, 0x021250D4),
    ),
    (
        "update_palette_wave",
        0x020AE130,
        0x020AE268,
        0x020AE280,
        "76de1dec4e5076152bb12678541da54daf3a0e6022ac00964bf1d2fad747dc84",
        "18af13aeca9bc5f28a275832667ac33b93983261378b1fb0645e134e41042251",
        (0x02180DA8, 0x0217D23C, 0x0212C584, 0x00000FFF, 0x02180EE0, 0x020C91FC),
    ),
)


class DatalinkPostOamRecoveryTests(unittest.TestCase):
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
                self.assertEqual(read_u32(start), {
                    0x020ADB04: 0xE92D4010,
                    0x020ADB68: 0xE92D4010,
                    0x020ADBB8: 0xE92D4FF8,
                    0x020ADF84: 0xE92D4070,
                    0x020ADFF0: 0xE92D41F0,
                    0x020AE130: 0xE92D40F8,
                }[start])
                self.assertEqual(
                    tuple(
                        read_u32(body_end + index * 4)
                        for index in range(len(pool_words))
                    ),
                    pool_words,
                )

        self.assertEqual(total_body_bytes, 1168)
        self.assertEqual(read_u32(0x020ADB60), 0xE8BD8010)
        self.assertEqual(read_u32(0x020ADBAC), 0xE8BD8010)
        self.assertEqual(read_u32(0x020ADCE4), 0xE8BD8FF8)
        self.assertEqual(read_u32(0x020ADFD8), 0xE8BD8070)
        self.assertEqual(read_u32(0x020AE114), 0xE8BD81F0)
        self.assertEqual(read_u32(0x020AE264), 0xE8BD80F8)

        self.assertEqual(read_u32(0x020ADCFC), 0xE92D4FF0)
        self.assertEqual(read_u32(0x020ADFE4), 0xE59FC000)
        self.assertEqual(read_u32(0x020ADFEC), 0x020ADF84)
        self.assertEqual(read_u32(0x020ADFF0), 0xE92D41F0)
        self.assertEqual(read_u32(0x020AE280), 0xE59F0008)

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
                *(str(source) for source in SOURCES),
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
                *(str(source) for source in SOURCES),
            ],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )


if __name__ == "__main__":
    unittest.main()
