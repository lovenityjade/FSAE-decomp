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
    PROJECT_ROOT / "src/arm9/game/format_buffer.c",
    PROJECT_ROOT / "src/arm9/game/log_format.c",
    PROJECT_ROOT / "src/arm9/game/format_runtime_wrappers.c",
    PROJECT_ROOT / "src/arm9/game/format_runtime_wide_parse.c",
    PROJECT_ROOT / "src/arm9/game/format_runtime_wide_float.c",
    PROJECT_ROOT / "src/arm9/game/format_runtime_wide_core.c",
    PROJECT_ROOT / "src/arm9/game/format_runtime_decimal.c",
)
NEW_SOURCES = SOURCES[2:]

# name, body start/end, extent end, body hash, extent hash, literal words
FUNCTIONS = (
    ("GameFormatBuffer_Write", 0x02007F14, 0x02007F58, 0x02007F58,
     "028a036cf8c52bdd0f2c72ae55d48d1babc2551487f434846deb56b8c9af0feb",
     "028a036cf8c52bdd0f2c72ae55d48d1babc2551487f434846deb56b8c9af0feb", ()),
    ("Game_LogFormat", 0x02007F58, 0x02008068, 0x02008080,
     "9fb9a701aa20f582279169599510342327b54c6e0728528ad3baa0bda9e2b1d5",
     "7085fad1467e809bb422da78640ff7689c337e12392fdade4d7dd29364224fb2",
     (0x020DF628, 0x02127420, 0x02129938, 0x02127390, 0x021273B4, 0x02007EE8)),
    ("Game_VSNPrintf_02008080", 0x02008080, 0x020080E0, 0x020080E4,
     "1a45fe9fabe82e81d6da17ecbdb45dcd8eaed5af10617d6be0279891a2f66d16",
     "350376f918b96cc65ea31cf416ea0eb4d305a1ddb0412a3e308a322b7beebd95", (0x02007F14,)),
    ("Game_SPrintf_020080e4", 0x020080E4, 0x02008110, 0x02008110,
     "72ab82eff3f7688a3fad13d112800d400a934a8951839724edc37b30a0ede3c4",
     "72ab82eff3f7688a3fad13d112800d400a934a8951839724edc37b30a0ede3c4", ()),
    ("Game_RaiseFormatConstraint_02008110", 0x02008110, 0x02008134, 0x02008138,
     "5f4aae6e6881930253f95eaa6e4e621f2857f723c4ec8417a49b0389481b9a23",
     "35d8d56b2ed74c787388a3cddf4db755e30aadfd7adb6546a9678911fb7d96ba", (0x021277B0,)),
    ("Game_DefaultFormatConstraintHandler_02008138", 0x02008138, 0x0200813C, 0x0200813C,
     "379bec29dccd0a93c94826144d7ef6e42fab64ef195a3b8313a16926f66f388f",
     "379bec29dccd0a93c94826144d7ef6e42fab64ef195a3b8313a16926f66f388f", ()),
    ("Game_RaiseRuntimeSignal", 0x0200813C, 0x02008250, 0x02008264,
     "9c389f4030f1ff2961ffc1c0f98c37185787d6b897ff0a6359aa7f58ccc40a75",
     "4d3c792d0dd5f6a1e17fea2f9fe1c5e65b69ff3a5b931cb26351eb1cb6b192f3",
     (0x02127480, 0x02129938, 0x02127390, 0x021273B4, 0x021277B4)),
    ("Game_StringLength_02008264", 0x02008264, 0x02008280, 0x02008280,
     "9d324faa9558d83201c859ca876c5716529e7e2378efbdfb9f4d44bba50213d4",
     "9d324faa9558d83201c859ca876c5716529e7e2378efbdfb9f4d44bba50213d4", ()),
    ("Game_CopyString_02008280", 0x02008280, 0x02008340, 0x02008348,
     "ef590a4a9874d68c5b2ecbbacff624715cf7b61e9e69b23d7dda09e39dd5dadc",
     "c4adcb7b1d9aadbcf7530e4236f7ab5cd1619809aa91433e984aebabf41ab4b6",
     (0xFEFEFEFF, 0x80808080)),
    ("Game_CopyStringBounded_02008348", 0x02008348, 0x02008398, 0x02008398,
     "a2c364beb592b63fab78fec96ec9e91ff7597d036f40cff3ac3ab34dafc3307e",
     "a2c364beb592b63fab78fec96ec9e91ff7597d036f40cff3ac3ab34dafc3307e", ()),
    ("Game_FindCharacter_02008398", 0x02008398, 0x020083D0, 0x020083D0,
     "36b68e4dcffe536303a1522ab919533e7bfd75da603ab68bb2e5c5778cba429d",
     "36b68e4dcffe536303a1522ab919533e7bfd75da603ab68bb2e5c5778cba429d", ()),
    ("Game_LogPrepareStream", 0x020083D0, 0x02008440, 0x02008440,
     "afe70d71cd69c3b8e80118bbdb4c99708a46176e10a2f7c5eae92be35120752e",
     "afe70d71cd69c3b8e80118bbdb4c99708a46176e10a2f7c5eae92be35120752e", ()),
    ("Game_CopyWideUnits_02008440", 0x02008440, 0x0200844C, 0x02008450,
     "11cc7ae6e8850f17d8f6d0088f909eed8dd38edf42efeab8422ca77b141f7f43",
     "3a28dc58a38e7097785d5ecca4fec26c04ea2381e51b9fef187804f4c8550f46", (0x02005DC0,)),
    ("Game_FindWideUnit_02008450", 0x02008450, 0x02008478, 0x02008478,
     "598f79cd7e03b3a63d1f22ad55609789d46d8af02da248fe9a7ebc0f3cfaf2f2",
     "598f79cd7e03b3a63d1f22ad55609789d46d8af02da248fe9a7ebc0f3cfaf2f2", ()),
    ("Game_ParseWideFormatConversion_02008478", 0x02008478, 0x02008984, 0x02008990,
     "38a89a8a61d8c39e36481a4b575844744cda2b5366ac5a6f958b4738f8a94ccd",
     "6f1ed5fcee1e1015c0374db20a3bccc0446e20e7b413265db8d833b1fc4f42fd",
     (0x020C8E7C, 0x000001FD, 0x0000FFFF)),
    ("Game_FormatWideInteger32_02008990", 0x02008990, 0x02008BD8, 0x02008BDC,
     "cebe49fabead7c46558f2ba94a168a2d49aa65b9350ac606991d599cc1fa1680",
     "ff0c71f82ffd815327f4f9403be49cb8e73ead77961a82a723b926dae8515dd2", (0x000001FD,)),
    ("Game_FormatWideInteger64_02008bdc", 0x02008BDC, 0x02008EB0, 0x02008EB4,
     "e7bdabff5ae0a7f22286793b8ee560e4295d1dec31d31ea2069cf93feb09b7bc",
     "7cb063cffc0bb888c682114ae9d57ceb35325395797852447a17200de796553c", (0x000001FD,)),
    ("Game_FormatWideHexFloat_02008eb4", 0x02008EB4, 0x0200923C, 0x02009274,
     "b431a28d33ffbe1fbd2d1d2b42a48e468623155d6d2bebd53f4edc364d33daba",
     "00b1b13636df8520e3fe75fccb46d13dd0f2d754d946f2bd8098523225cd9ee9",
     (0x000001FD, 0x020DF94C, 0x020DF958, 0x020DF964, 0x020DF96C, 0x020DF974,
      0x020DF980, 0x020DF98C, 0x020DF994, 0x020DF99C, 0x020DF9A8, 0x020DF9B4,
      0x020DF9BC, 0x000007FF)),
    ("Game_RoundWideDecimalDigits_02009274", 0x02009274, 0x02009388, 0x02009388,
     "e3210b25c42c98c471121d3a5a240753d366983503000c883da67092d0456c17",
     "e3210b25c42c98c471121d3a5a240753d366983503000c883da67092d0456c17", ()),
    ("Game_FormatWideDecimalFloat_02009388", 0x02009388, 0x02009954, 0x02009980,
     "faa7d9431493b61cc49f27bb3cadb86695992a5dbd310544701857381bb636e9",
     "c7ade065d3bf2a44e804489934e3c1c432a5ceb96cbedd85973dc24cbf47ad38",
     (0x000001FD, 0x020C8E7C, 0x020DF974, 0x020DF980, 0x020DF98C, 0x020DF994,
      0x020DF99C, 0x020DF9A8, 0x020DF9B4, 0x020DF9BC, 0x66666667)),
    ("Game_RunWideFormatter_02009980", 0x02009980, 0x0200A2DC, 0x0200A2E4,
     "22548b5ba922df275cda85353d06d2bf868a89388dafed35672160c1c37cf910",
     "04a423f3223459a59d5b3108c68c851ec3de177bdf914d8aa252478fe5f492aa",
     (0x020DF9C4, 0x020DF9C8)),
    ("GameWideFormatBuffer_Write_0200a2e4", 0x0200A2E4, 0x0200A324, 0x0200A324,
     "fa0ac01d941f270da9f49713884d5222426e9774105e1b74f720adfbaf97d6d0",
     "fa0ac01d941f270da9f49713884d5222426e9774105e1b74f720adfbaf97d6d0", ()),
    ("NtmvM2d_FormatWideText_0200a324", 0x0200A324, 0x0200A34C, 0x0200A34C,
     "035780c4b7939b02ca0e9acf55a874cc18ce54b2c9f5d34517e13db3fe990b77",
     "035780c4b7939b02ca0e9acf55a874cc18ce54b2c9f5d34517e13db3fe990b77", ()),
    ("Game_VSNPrintfWide_0200a34c", 0x0200A34C, 0x0200A3A8, 0x0200A3AC,
     "f46bf988434a99fbbffc671be7ac6f68bd4737bcdba6e1bb0229ee57f68376e4",
     "774fd4b278c1a3b94cdf18015b5ef8f529e6810c6a7754ca7cba54c5503d9c08", (0x0200A2E4,)),
    ("Game_WideStringLength_0200a3ac", 0x0200A3AC, 0x0200A3C8, 0x0200A3C8,
     "773fb87f15eadfe719f3e4858fc170ed701b2725e456bbaa1409b1eaf26e93b9",
     "773fb87f15eadfe719f3e4858fc170ed701b2725e456bbaa1409b1eaf26e93b9", ()),
    ("Game_CopyWideString_0200a3c8", 0x0200A3C8, 0x0200A3E8, 0x0200A3E8,
     "dbf716a63cd6593700a3c64c60519afd515ae4881a76f9916f690297a7fb68ae",
     "dbf716a63cd6593700a3c64c60519afd515ae4881a76f9916f690297a7fb68ae", ()),
    ("Game_FindWideCharacter_0200a3e8", 0x0200A3E8, 0x0200A41C, 0x0200A41C,
     "68e40d951e4d09237d4ce50e4599fed2fef44cd80c2caaaf2b40bfac46867455",
     "68e40d951e4d09237d4ce50e4599fed2fef44cd80c2caaaf2b40bfac46867455", ()),
    ("Game_CompareDecimalRoundingHalf_0200a41c", 0x0200A41C, 0x0200A488, 0x0200A488,
     "72a62c7790764fc9755dbfd66cdf9960b665176632376292a0e4493b3c73bf5f",
     "72a62c7790764fc9755dbfd66cdf9960b665176632376292a0e4493b3c73bf5f", ()),
    ("Game_IncrementRuntimeDecimal_0200a488", 0x0200A488, 0x0200A4D8, 0x0200A4D8,
     "ad924a339b877b9ab75c359a425d9cd407fc08f84f7499644d8f43d589f50dfd",
     "ad924a339b877b9ab75c359a425d9cd407fc08f84f7499644d8f43d589f50dfd", ()),
    ("Game_RoundRuntimeDecimal_0200a4d8", 0x0200A4D8, 0x0200A518, 0x0200A518,
     "477c8f5f3eb530ef995c61cad7f4acb208ea09779386ca100359681096f68fc6",
     "477c8f5f3eb530ef995c61cad7f4acb208ea09779386ca100359681096f68fc6", ()),
    ("Game_RuntimeDecimalFromUint64_0200a518", 0x0200A518, 0x0200A5DC, 0x0200A5DC,
     "fa43562336ffcf43004338b721c4f9e22b39abbaa7adf5bfef44064e2088baa5",
     "fa43562336ffcf43004338b721c4f9e22b39abbaa7adf5bfef44064e2088baa5", ()),
    ("Game_MultiplyRuntimeDecimals_0200a5dc", 0x0200A5DC, 0x0200A758, 0x0200A75C,
     "e4a90f897193fb53ae74bd8b02abd6ef3caa1af08ea8318801c2c00eb7d2bf8d",
     "11863935db32072db90cbf2c1d2702b2e5f289faebb9218f2b2140dee0119e0a", (0xCCCCCCCD,)),
)


class FormatRuntimeWideRecoveryTests(unittest.TestCase):
    def test_rom_bodies_pools_hashes_and_contiguous_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def data(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        body_bytes = 0
        pool_bytes = 0
        previous = FUNCTIONS[0][1]
        for name, start, body_end, extent_end, body_sha, extent_sha, pool in FUNCTIONS:
            with self.subTest(function=name):
                self.assertEqual(start, previous)
                self.assertEqual(hashlib.sha256(data(start, body_end)).hexdigest(), body_sha)
                self.assertEqual(hashlib.sha256(data(start, extent_end)).hexdigest(), extent_sha)
                self.assertEqual(
                    tuple(read_u32(body_end + index * 4) for index in range(len(pool))),
                    pool,
                )
                self.assertEqual(extent_end - body_end, len(pool) * 4)
                body_bytes += body_end - start
                pool_bytes += extent_end - body_end
                previous = extent_end

        self.assertEqual(body_bytes, 10112)
        self.assertEqual(pool_bytes, 200)
        self.assertEqual(previous, 0x0200A75C)

    def test_source_and_config_own_each_entry_exactly_once(self) -> None:
        expected = {entry[1]: entry[0] for entry in FUNCTIONS}
        definitions: dict[str, list[str]] = {}
        definition = re.compile(
            r"^(?:const\s+uint16_t\s*\*|uint16_t\s*\*|char\s*\*|"
            r"uint32_t|int|void)\s*([A-Za-z_][A-Za-z0-9_]*)\s*"
            r"\([^;]*?\)\s*\{",
            re.MULTILINE | re.DOTALL,
        )
        for source in SOURCES:
            for name in definition.findall(source.read_text()):
                definitions.setdefault(name, []).append(source.name)
        for name in expected.values():
            self.assertEqual(len(definitions.get(name, [])), 1, name)

        config = json.loads((PROJECT_ROOT / "config/decomp/arm9-symbols.json").read_text())
        entries: dict[int, list[str]] = {}
        for symbol in config["symbols"]:
            address = int(symbol["address"], 16)
            if FUNCTIONS[0][1] <= address < FUNCTIONS[-1][3]:
                entries.setdefault(address, []).append(symbol["name"])
        self.assertEqual(set(entries), set(expected))
        for address, name in expected.items():
            self.assertEqual(entries[address], [name], hex(address))

    def test_host_c_syntax(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        subprocess.run(
            [compiler, "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
             "-I", str(PROJECT_ROOT / "include"), "-fsyntax-only",
             *(str(source) for source in NEW_SOURCES)],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )

    def test_arm946es_c_syntax(self) -> None:
        compiler = shutil.which("clang")
        if compiler is None:
            self.skipTest("clang is unavailable")
        subprocess.run(
            [compiler, "--target=arm-none-eabi", "-mcpu=arm946e-s", "-marm",
             "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
             "-ffreestanding", "-I", str(PROJECT_ROOT / "include"),
             "-fsyntax-only", *(str(source) for source in NEW_SOURCES)],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )


if __name__ == "__main__":
    unittest.main()
