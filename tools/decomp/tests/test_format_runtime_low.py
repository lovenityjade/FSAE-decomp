from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000
SOURCES = (
    PROJECT_ROOT / "src/arm9/game/format_runtime_parse.c",
    PROJECT_ROOT / "src/arm9/game/format_runtime_integer.c",
    PROJECT_ROOT / "src/arm9/game/format_runtime_float.c",
    PROJECT_ROOT / "src/arm9/game/format_runtime_core.c",
)

# name, body start/end, extent end, body hash, extent hash, literal pool
FUNCTIONS = (
    (
        "Game_ParseFormatConversion_02005f38",
        0x02005F38,
        0x0200643C,
        0x02006444,
        "081fef53b59ba51cd30655f6a1e8a1d1bebe59a42d33923cdd82b9d50d909393",
        "1fdf0f628e964f15a29347474adf12a2ad35ae22ac851b784ea5e0a08c82c150",
        (0x020C8D7C, 0x000001FD),
    ),
    (
        "Game_FormatInteger32_02006444",
        0x02006444,
        0x02006690,
        0x02006694,
        "e9fbd7553dc40e2c4dfc5b8f679eeb627157e6bdd9619e2f920b3c2473e62fd9",
        "15231293983bd62fd3758cecbc99e42ab04b8fc1c83f8a933dd566eee3453369",
        (0x000001FD,),
    ),
    (
        "Game_FormatInteger64_02006694",
        0x02006694,
        0x02006964,
        0x02006968,
        "69ee9c37c1b14c146875f2e40b01e1d542dbfd31e80d92edd6ed0d7f098a207c",
        "e2cb5749627cc49b3bb8c9f67e1cdd9335fb160696bf4a6f39fde7ccb5ee35a3",
        (0x000001FD,),
    ),
    (
        "Game_FormatHexFloat_02006968",
        0x02006968,
        0x02006DEC,
        0x02006E14,
        "203b2ae4e40ea3118cb3d3216942352e486ac4b80f89fe5013c0c6f657cb0eae",
        "562476e74d06ff246664b569a55938d23067e87d3e711139a890b4936f0e9f7b",
        (
            0x000001FD,
            0x020DF914,
            0x020DF91C,
            0x020DF924,
            0x020DF928,
            0x020DF92C,
            0x020DF934,
            0x020DF93C,
            0x020DF940,
            0x000007FF,
        ),
    ),
    (
        "Game_RoundDecimalDigits_02006e14",
        0x02006E14,
        0x02006F28,
        0x02006F28,
        "e3210b25c42c98c471121d3a5a240753d366983503000c883da67092d0456c17",
        "e3210b25c42c98c471121d3a5a240753d366983503000c883da67092d0456c17",
        (),
    ),
    (
        "Game_FormatDecimalFloat_02006f28",
        0x02006F28,
        0x0200760C,
        0x02007638,
        "fd78a73f11bb956b7612d1d82be4370bfc573dd119dc42d7ac6e886a0bb3a68c",
        "96c89b34ec8c59dcdcd66025f82b0c5986bf41c5516286315c7e3ee304565143",
        (
            0x000001FD,
            0x020C8D7C,
            0x020DF914,
            0x020DF91C,
            0x020DF924,
            0x020DF928,
            0x020DF92C,
            0x020DF934,
            0x020DF93C,
            0x020DF940,
            0x66666667,
        ),
    ),
    (
        "Game_RunFormatter",
        0x02007638,
        0x02007EE0,
        0x02007EE8,
        "b73431e110f933b1d3af0bbdc576816dd9721784c8b9801f9fd1153976c04136",
        "7450bf55beed543eed642a4edab4242419a8342efec3eef2d2da713df0eb4c4b",
        (0x020DF944, 0x020DF948),
    ),
    (
        "Game_LogStreamWrite",
        0x02007EE8,
        0x02007F14,
        0x02007F14,
        "9c898ef70fbfc6e4488c0a7a29bc9e2c902f0b95c7bf4b979ac5a6afb40736e2",
        "9c898ef70fbfc6e4488c0a7a29bc9e2c902f0b95c7bf4b979ac5a6afb40736e2",
        (),
    ),
)


class FormatRuntimeLowRecoveryTests(unittest.TestCase):
    def test_rom_bodies_pools_hashes_and_contiguous_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def body(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        total_body_bytes = 0
        total_pool_bytes = 0
        previous_extent_end = FUNCTIONS[0][1]
        for entry in FUNCTIONS:
            name, start, body_end, extent_end, body_sha, extent_sha, pool = entry
            with self.subTest(function=name):
                self.assertEqual(start, previous_extent_end)
                recovered = body(start, body_end)
                extent = body(start, extent_end)
                self.assertEqual(hashlib.sha256(recovered).hexdigest(), body_sha)
                self.assertEqual(hashlib.sha256(extent).hexdigest(), extent_sha)
                self.assertEqual(
                    tuple(
                        read_u32(body_end + index * 4)
                        for index in range(len(pool))
                    ),
                    pool,
                )
                self.assertEqual(extent_end - body_end, len(pool) * 4)
                total_body_bytes += len(recovered)
                total_pool_bytes += extent_end - body_end
                previous_extent_end = extent_end

        self.assertEqual(total_body_bytes, 8048)
        self.assertEqual(total_pool_bytes, 108)
        self.assertEqual(previous_extent_end, 0x02007F14)

        anchors = {
            0x02005F38: 0xE92D40F8,
            0x02006438: 0xE8BD80F8,
            0x02006444: 0xE92D000F,
            0x0200668C: 0xE12FFF1E,
            0x02006694: 0xE92D000F,
            0x02006960: 0xE12FFF1E,
            0x02006968: 0xE92D000F,
            0x02006DE8: 0xE12FFF1E,
            0x02006E14: 0xE92D4038,
            0x02006F24: 0xE8BD8038,
            0x02006F28: 0xE92D000F,
            0x02007608: 0xE12FFF1E,
            0x02007638: 0xE92D000F,
            0x02007EDC: 0xE12FFF1E,
            0x02007EE8: 0xE92D4038,
            0x02007F10: 0xE8BD8038,
            0x02007F14: 0xE92D4038,
        }
        for address, expected in anchors.items():
            self.assertEqual(read_u32(address), expected, hex(address))

    def test_source_and_config_own_each_entry_exactly_once(self) -> None:
        expected = {entry[1]: entry[0] for entry in FUNCTIONS}
        definition = re.compile(
            r"^(?:char\s*\*|const\s+char\s*\*|int|void)\s*"
            r"(Game_[A-Za-z0-9_]+|Game_RunFormatter|Game_LogStreamWrite)\s*"
            r"\([^;]*?\)\s*\{",
            re.MULTILINE | re.DOTALL,
        )
        definitions: dict[str, list[str]] = {}
        for source in SOURCES:
            for name in definition.findall(source.read_text()):
                definitions.setdefault(name, []).append(source.name)

        for name in expected.values():
            with self.subTest(source=name):
                self.assertEqual(len(definitions.get(name, [])), 1)

        config = json.loads(
            (PROJECT_ROOT / "config/decomp/arm9-symbols.json").read_text()
        )
        entries: dict[int, list[str]] = {}
        for symbol in config["symbols"]:
            address = int(symbol["address"], 16)
            if 0x02005F38 <= address <= 0x02007EE8:
                entries.setdefault(address, []).append(symbol["name"])
        self.assertEqual(set(entries), set(expected))
        for address, name in expected.items():
            with self.subTest(config=f"0x{address:08X}"):
                self.assertEqual(entries[address], [name])

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
            ],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )

    def test_arm946es_c_syntax(self) -> None:
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
