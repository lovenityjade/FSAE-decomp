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
NEW_SOURCES = (
    PROJECT_ROOT / "src/arm9/game/runtime_decimal_conversion.c",
    PROJECT_ROOT / "src/arm9/game/runtime_unwind_decode.c",
    PROJECT_ROOT / "src/arm9/game/runtime_unwind_core.c",
)
OWNERSHIP_SOURCES = NEW_SOURCES + (
    PROJECT_ROOT / "src/arm9/game/termination_dispatch.c",
)

# name, body start/end, extent end, body hash, extent hash, trailing words
FUNCTIONS = (
    ("Game_RuntimeDecimalFromDigitString_0200a75c", 0x0200A75C, 0x0200A7F8, 0x0200A7F8,
     "0595e4e9a89e68de35aa6bee23f693ad032ca0f548c193343ae41cde3958b048",
     "0595e4e9a89e68de35aa6bee23f693ad032ca0f548c193343ae41cde3958b048", ()),
    ("Game_RuntimeDecimalPowerOfTwo_0200a7f8", 0x0200A7F8, 0x0200AB24, 0x0200AB78,
     "00bf04b559958840102d4273bc10144e59cb13243c498712f48a0d5fd1755afa",
     "841174d0abb48aefd211524ba1b4a1980cd52ee14ab5bd4f0a03305c512bfd21",
     (0x020DF9CC, 0x020DF9FC, 0x020DFA24, 0x020DFA3C, 0x020DFA4C,
      0x020DFA54, 0x020DFA5C, 0x020DFA64, 0x020DFA6C, 0x020DFA70,
      0x020DFA74, 0x020DFA78, 0x020DFA7C, 0x020DFA80, 0x020DFA84,
      0x020DFA88, 0x020DFA8C, 0x020DFA90, 0x020DFA94, 0x020DFA98,
      0x020DFA9C)),
    ("Game_RuntimeDecimalFromDouble_0200ab78", 0x0200AB78, 0x0200ACF8, 0x0200ACF8,
     "b42a1b074b0d787de10c5a583a9dd35f2dc1f649572669049ff056ef841a5fb2",
     "b42a1b074b0d787de10c5a583a9dd35f2dc1f649572669049ff056ef841a5fb2", ()),
    ("Game_ConvertDoubleToDecimal_0200acf8", 0x0200ACF8, 0x0200ADA4, 0x0200ADA4,
     "08b24337abb7971975bd3f9ae3778260a947db5fcc4b664dfc1a05beb7d5a9f9",
     "08b24337abb7971975bd3f9ae3778260a947db5fcc4b664dfc1a05beb7d5a9f9", ()),
    ("Game_CopyDoubleSign_0200ada4", 0x0200ADA4, 0x0200ADCC, 0x0200ADCC,
     "4e4687500f32f84e9436058ceb82a3914b9c34c41bdfb1f54684393acb79e688",
     "4e4687500f32f84e9436058ceb82a3914b9c34c41bdfb1f54684393acb79e688", ()),
    ("Game_AbsoluteDouble_0200adcc", 0x0200ADCC, 0x0200ADEC, 0x0200ADEC,
     "0f3183f4f1f26afc214eda956f87c0a2d1a2cc6cafa2b8fb545a03211749457c",
     "0f3183f4f1f26afc214eda956f87c0a2d1a2cc6cafa2b8fb545a03211749457c", ()),
    ("Game_NormalizeDoubleFraction_0200adec", 0x0200ADEC, 0x0200AE9C, 0x0200AEAC,
     "a6d9726c84b1390fdc13f67eb3fb1d51d30943d8797c922e699d18d471705f0d",
     "c3a41bc469c862dd8ab186f1eb69d5ed688182f1926c944d6b92c1d870394274",
     (0x7FF00000, 0x43500000, 0x800FFFFF, 0xFFFFFC02)),
    ("Game_ScaleDoubleByPowerOfTwo_0200aeac", 0x0200AEAC, 0x0200B0E0, 0x0200B10C,
     "ab55d188a712dd4300467d7131062c5c714a63ef2b60b163de4fbf9ec4baf461",
     "6c12f7c240ee4db1a73cc75f368338eb920f95b37c119ed60732c6a7c952e1bc",
     (0x7FF00000, 0x43500000, 0xFFFF3CB0, 0xC2F8F359, 0x01A56E1F,
      0x000007FF, 0x8800759C, 0x7E37E43C, 0x800FFFFF, 0x0000C350,
      0x3C900000)),
    ("Game_RuntimeNoOpThunk_0200b10c", 0x0200B10C, 0x0200B114, 0x0200B118,
     "e8c968b1c0e2eb8c47edf56001d7a29395359d831cfa8528bd272323b413e77c",
     "c65f339bdd2fd72607c38ce8bcc154c1cd625f177c1912ea1b9cbc919168e4f5", (0x0200B128,)),
    ("Game_RuntimeInputBegin_0200b118", 0x0200B118, 0x0200B11C, 0x0200B11C,
     "379bec29dccd0a93c94826144d7ef6e42fab64ef195a3b8313a16926f66f388f",
     "379bec29dccd0a93c94826144d7ef6e42fab64ef195a3b8313a16926f66f388f", ()),
    ("Game_RuntimeInputEnd_0200b11c", 0x0200B11C, 0x0200B120, 0x0200B120,
     "379bec29dccd0a93c94826144d7ef6e42fab64ef195a3b8313a16926f66f388f",
     "379bec29dccd0a93c94826144d7ef6e42fab64ef195a3b8313a16926f66f388f", ()),
    ("Game_RuntimeReadByte_0200b120", 0x0200B120, 0x0200B128, 0x0200B128,
     "6dcd5e75586fe6683156c0d559d4827b73bd48d501fa5781a1c6099aacd7c877",
     "6dcd5e75586fe6683156c0d559d4827b73bd48d501fa5781a1c6099aacd7c877", ()),
    ("Game_RuntimeNoOp_0200b128", 0x0200B128, 0x0200B12C, 0x0200B12C,
     "379bec29dccd0a93c94826144d7ef6e42fab64ef195a3b8313a16926f66f388f",
     "379bec29dccd0a93c94826144d7ef6e42fab64ef195a3b8313a16926f66f388f", ()),
    ("Game_ReadRuntimeLine_0200b12c", 0x0200B12C, 0x0200B17C, 0x0200B17C,
     "0cc5ca81f56e3dc36c707e6ba3cab6ed0323099b6777e785bbae9a2ad64571ef",
     "0cc5ca81f56e3dc36c707e6ba3cab6ed0323099b6777e785bbae9a2ad64571ef", ()),
    ("Game_FinalizeRuntimeLine_0200b17c", 0x0200B17C, 0x0200B1C0, 0x0200B1C8,
     "e7280e563aa572c557fec003b642452acd74443668b049d90fd3f5aeff6873ec",
     "10f46464e9ca6fe132880673e047c19749b06c92e089c99cf732ea778ee94b9b",
     (0xE3A00000, 0xE12FFF1E)),
    ("Game_DecodeSignedUnwindValue_0200b1c8", 0x0200B1C8, 0x0200B24C, 0x0200B24C,
     "5e58524c149baf2c677202a3748f2247f21bce0fa10875ef70eb0110649d41e6",
     "5e58524c149baf2c677202a3748f2247f21bce0fa10875ef70eb0110649d41e6", ()),
    ("Game_DecodeUnsignedUnwindValue_0200b24c", 0x0200B24C, 0x0200B2D0, 0x0200B2D0,
     "b6f9a38109754d78cc61a43754b000e5f44f500df32cd9defc70c1aa8f7f8b31",
     "b6f9a38109754d78cc61a43754b000e5f44f500df32cd9defc70c1aa8f7f8b31", ()),
    ("Game_FindUnwindCodeRange_0200b2d0", 0x0200B2D0, 0x0200B334, 0x0200B334,
     "138849ab40cda3dd8b3b9dc0170fb6cf8ec27321285f4d765202d60b9c12b4b4",
     "138849ab40cda3dd8b3b9dc0170fb6cf8ec27321285f4d765202d60b9c12b4b4", ()),
    ("Game_LoadUnwindDescriptor_0200b334", 0x0200B334, 0x0200B438, 0x0200B43C,
     "f0123a0061f2798c1ecb44f2f24bcd564b02d8a0c0e5171f8a29ec15cac64f30",
     "4f6e7cd490d9260d47c3a28cb406c551c4398ea82af5293ba8743db810cf762b", (0x2AAAAAAB,)),
    ("Game_GetUnwindOpcode_0200b43c", 0x0200B43C, 0x0200B458, 0x0200B458,
     "2efcbf24937db3165ed89bc13b8e477163b51075652df51c3893a038aabbbaf8",
     "2efcbf24937db3165ed89bc13b8e477163b51075652df51c3893a038aabbbaf8", ()),
    ("Game_AdvanceUnwindOpcode_0200b458", 0x0200B458, 0x0200B714, 0x0200B714,
     "6108b651e62d0c0172cdec121f02685677f0ead4c01d54a8107526f2dc7396cb",
     "6108b651e62d0c0172cdec121f02685677f0ead4c01d54a8107526f2dc7396cb", ()),
    ("Game_ExecuteUnwindCleanup_0200b714", 0x0200B714, 0x0200BF40, 0x0200BF48,
     "3c2022b1414dcbd0237a000d3a329d412c245b5dd69a643e9b921032639a8648",
     "e87f70cd12c6f057974bd84913e2f327e4673e0f0a655646675c37fca04ac8c4",
     (0xE28DD0A4, 0xE8BD8FF0)),
    ("Game_FindUnwindCatchTarget_0200bf48", 0x0200BF48, 0x0200C09C, 0x0200C09C,
     "f522f69561f7613ba9c439370f52a75655819119ef7aaf4d3c6c62d88555572c",
     "f522f69561f7613ba9c439370f52a75655819119ef7aaf4d3c6c62d88555572c", ()),
    ("Game_UnwindMatchesTypeList_0200c09c", 0x0200C09C, 0x0200C11C, 0x0200C11C,
     "de53bcef89f92f54801f8746ef4d2e3369c5cbd303ff050e8c64e1bac3c8baf4",
     "de53bcef89f92f54801f8746ef4d2e3369c5cbd303ff050e8c64e1bac3c8baf4", ()),
    ("Game_InstallUnwindCatchContext_0200c11c", 0x0200C11C, 0x0200C17C, 0x0200C17C,
     "44816a0d67954f71e8fa0fe4176d5177f71b7421c337c17ccae614b6fbeb2f68",
     "44816a0d67954f71e8fa0fe4176d5177f71b7421c337c17ccae614b6fbeb2f68", ()),
    ("Game_FindUnwindLandingPad_0200c17c", 0x0200C17C, 0x0200C358, 0x0200C358,
     "b2129a56e286e3e5975bc196347192e02266a5b9061d55fd0de4bd057d87a459",
     "b2129a56e286e3e5975bc196347192e02266a5b9061d55fd0de4bd057d87a459", ()),
    ("Game_WriteUnwindFrameRecord_0200c358", 0x0200C358, 0x0200C3B4, 0x0200C3B4,
     "56e2d44245921cf326b1807686167319138e895d72bea3575be9a4ac962b39ea",
     "56e2d44245921cf326b1807686167319138e895d72bea3575be9a4ac962b39ea", ()),
    ("Game_UnwindOneFrame_0200c3b4", 0x0200C3B4, 0x0200C4A8, 0x0200C4A8,
     "3add0302b9203428dd2c05f3710c2e8292b6d0b269b03c15452996b46fc875a1",
     "3add0302b9203428dd2c05f3710c2e8292b6d0b269b03c15452996b46fc875a1", ()),
    ("Game_InvokeUnwindDestructor_0200c4a8", 0x0200C4A8, 0x0200C4D0, 0x0200C4D0,
     "c57dac5697410b32bb73d9fcd0e94d6f7c78175469323475534dbebad2d44f35",
     "c57dac5697410b32bb73d9fcd0e94d6f7c78175469323475534dbebad2d44f35", ()),
    ("Game_RunStaticInitializerTable_0200c4d0", 0x0200C4D0, 0x0200C4FC, 0x0200C500,
     "eb44594e43fca0d3f9c640d77be5c73c3c8190bd99b37d4f3137bcbad26e737c",
     "622cb72821966187c3aaa204c481a19a3718c05f970097be7031fda68c58175b", (0x020DF56C,)),
    ("Game_RunPendingDestructors", 0x0200C500, 0x0200C540, 0x0200C544,
     "10e6bb0cc808adf2b3ee5d2f3febd24ffe202a5b4144b6fc856cab38037f0777",
     "0eb132147d97fdeef5be739c60cf5296c16a3007982c67467e31bf2fc63d3121", (0x021277D0,)),
    ("Game_HandleFatalAssertionThunk_0200c544", 0x0200C544, 0x0200C54C, 0x0200C55C,
     "e8c968b1c0e2eb8c47edf56001d7a29395359d831cfa8528bd272323b413e77c",
     "51c4fa7053aec04276bc3cc8f00cee475a0e4c7726cb242c719fcac7fd1516b0",
     (0x02005488, 0xE59FC000, 0xE12FFF1C, 0x0200C55C)),
)


class RuntimeUnwindRecoveryTests(unittest.TestCase):
    def test_rom_bodies_trailing_words_hashes_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def data(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        body_bytes = 0
        trailing_bytes = 0
        previous = FUNCTIONS[0][1]
        for name, start, body_end, extent_end, body_sha, extent_sha, words in FUNCTIONS:
            with self.subTest(function=name):
                self.assertEqual(start, previous)
                self.assertEqual(hashlib.sha256(data(start, body_end)).hexdigest(), body_sha)
                self.assertEqual(hashlib.sha256(data(start, extent_end)).hexdigest(), extent_sha)
                self.assertEqual(
                    tuple(read_u32(body_end + index * 4) for index in range(len(words))),
                    words,
                )
                self.assertEqual(extent_end - body_end, len(words) * 4)
                body_bytes += body_end - start
                trailing_bytes += extent_end - body_end
                previous = extent_end

        self.assertEqual(body_bytes, 7488)
        self.assertEqual(trailing_bytes, 192)
        self.assertEqual(previous, 0x0200C55C)

    def test_source_and_config_own_each_entry_exactly_once(self) -> None:
        expected = {entry[1]: entry[0] for entry in FUNCTIONS}
        definition = re.compile(
            r"^(?:const\s+[A-Za-z_][A-Za-z0-9_]*\s*\*|uint64_t|uint32_t|"
            r"uint8_t|int|void)\s*([A-Za-z_][A-Za-z0-9_]*)\s*"
            r"\([^;]*?\)\s*\{",
            re.MULTILINE | re.DOTALL,
        )
        definitions: dict[str, list[str]] = {}
        for source in OWNERSHIP_SOURCES:
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
