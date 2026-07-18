from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000
PUBLIC_SOURCES = (
    PROJECT_ROOT / "src/arm9/game/runtime_unwind_support.c",
    PROJECT_ROOT / "src/arm9/game/runtime_double_math.c",
)
DRAFT_ROOT = PROJECT_ROOT / "build/agents/rom-pipeline"
DRAFT_SOURCES = (
    DRAFT_ROOT / "runtime_unwind_support.c",
    DRAFT_ROOT / "runtime_double_math.c",
)
if all(source.exists() for source in PUBLIC_SOURCES):
    NEW_SOURCES = PUBLIC_SOURCES
    INCLUDE_DIRS = (PROJECT_ROOT / "include",)
else:
    NEW_SOURCES = DRAFT_SOURCES
    INCLUDE_DIRS = (PROJECT_ROOT / "include", DRAFT_ROOT / "include")
OWNERSHIP_SOURCES = NEW_SOURCES + (
    PROJECT_ROOT / "src/arm9/game/termination_registration.c",
)

# name, catalogued body start/end, complete extent end, body hash, extent hash
#
# An extent may include a non-contiguous instruction tail or a literal.  It is
# deliberately not labelled a pool without instruction-level evidence.
FUNCTIONS = (
    ("Game_UnwindAbort_0200c55c", 0x0200C55C, 0x0200C570, 0x0200C574,
     "e810f8676efb18f9d900578ea8bd8562605825bb5c438d7857e0c482e89f8fc8",
     "de5424349833e6d9970250f854ad9869d1b192f012bf484af9dae32b9e110c32"),
    ("Game_RegisterTerminationNode", 0x0200C574, 0x0200C590, 0x0200C594,
     "7589b58006832b3844788f2154268b249c41ba9c759161e04a5e6793bd60ed61",
     "b1e3440beb45893217ee5993cb7b5e729824718686776044fa97ec5cfcdc573f"),
    ("Game_UnwindTypeMatches_0200c594", 0x0200C594, 0x0200C748, 0x0200C748,
     "97dbbb8ebd1942eac08117d72cfb9dc41eb5e091693d7c9262107137872c7d33",
     "97dbbb8ebd1942eac08117d72cfb9dc41eb5e091693d7c9262107137872c7d33"),
    ("Game_OperatorDelete", 0x0200C748, 0x0200C75C, 0x0200C75C,
     "1acb023875901522afe2da98f84b267c46e366ae0d315f721f060350ab6f9278",
     "1acb023875901522afe2da98f84b267c46e366ae0d315f721f060350ab6f9278"),
    ("Game_DestroyObjectRangeReverse_0200c75c", 0x0200C75C, 0x0200C7A4, 0x0200C7B8,
     "6fe292416b1b608e1f3fe5d29ba84d9821f9d6096a974ef46ef1151e35900edd",
     "4c224eed16818dbcad6d87e625ec210d8f710f976f68d8786212922c73bee44c"),
    ("Game_ConstructObjectArray_0200c7b8", 0x0200C7B8, 0x0200C840, 0x0200C868,
     "a364373e80ef6fb01426e87a5e4cf2f399bfb09961b85a31a0a08ae26f59f48b",
     "8d57441f22bfbf0485b498805bb420e26e9928a32d5dd3d631e681c163e0d362"),
    ("Game_DestroyObjectArray_0200c868", 0x0200C868, 0x0200C8B0, 0x0200C8C4,
     "97eeb1598d044b4292c6b3ac52997d872969704b6862d9063e92f16245e2bf38",
     "a6c42f06083507585a7f4f41e9e5707e57f706cfff3d81bad6f3064178c6a903"),
    ("Game_RuntimeBaseDestructor_0200c8c4", 0x0200C8C4, 0x0200C8C8, 0x0200C8C8,
     "379bec29dccd0a93c94826144d7ef6e42fab64ef195a3b8313a16926f66f388f",
     "379bec29dccd0a93c94826144d7ef6e42fab64ef195a3b8313a16926f66f388f"),
    ("Game_RuntimeBaseDestructorThunk_0200c8c8", 0x0200C8C8, 0x0200C8DC, 0x0200C8DC,
     "6c054f264ce80f8d367d88e8102081ab081a42b3b13eb8237ca009a675c815a6",
     "6c054f264ce80f8d367d88e8102081ab081a42b3b13eb8237ca009a675c815a6"),
    ("Game_RuntimeDeletingDestructor_0200c8dc", 0x0200C8DC, 0x0200C8F8, 0x0200C8F8,
     "c4ab1b71c607ab8bbaeda9f07699cb3bfa370c50f94b9f30fd8f90822ae7304e",
     "c4ab1b71c607ab8bbaeda9f07699cb3bfa370c50f94b9f30fd8f90822ae7304e"),
    ("Game_RuntimeDerivedDestructor_0200c8f8", 0x0200C8F8, 0x0200C90C, 0x0200C90C,
     "73686a4906bf654cde16c90a4ad142e229adfb4ef36c12b3bd2f1bba2334e28c",
     "73686a4906bf654cde16c90a4ad142e229adfb4ef36c12b3bd2f1bba2334e28c"),
    ("Game_RuntimeDerivedDestructorThunk_0200c90c", 0x0200C90C, 0x0200C920, 0x0200C920,
     "4e32e676f80db7ae90fc4c37ee7bbad5d239e73606146318c01f4049d3e4eef3",
     "4e32e676f80db7ae90fc4c37ee7bbad5d239e73606146318c01f4049d3e4eef3"),
    ("Game_RuntimeDerivedDeletingDestructor_0200c920", 0x0200C920, 0x0200C93C, 0x0200C93C,
     "a982578b870d6397f0730019b30023b0ec768465febed9f2717dd3ef2f10276b",
     "a982578b870d6397f0730019b30023b0ec768465febed9f2717dd3ef2f10276b"),
    ("Game_UnwindResolveContext_0200c93c", 0x0200C93C, 0x0200C998, 0x0200C998,
     "ac648b30063a9e23961549e6b87e96324c92f8d818414f6a2ddabef3bffce69b",
     "ac648b30063a9e23961549e6b87e96324c92f8d818414f6a2ddabef3bffce69b"),
    ("Game_UnwindSynchronizeDescriptor_0200c998", 0x0200C998, 0x0200CA38, 0x0200CA38,
     "0dec1ea95c53d74e5e30afe0c764f973d7808b6886952cfdb06a897d1d8c6804",
     "0dec1ea95c53d74e5e30afe0c764f973d7808b6886952cfdb06a897d1d8c6804"),
    ("Game_UnwindLocateTables_0200ca38", 0x0200CA38, 0x0200CA50, 0x0200CA58,
     "6764cb99371eef1df974911b9ea354af848d3c552e38155a8631c56d00e5167e",
     "fe31f545593dc20d1b0c4dbe0674b8761babd09fe5f988f2e24f1a81dd18778d"),
    ("Game_UnwindGetProgram_0200ca58", 0x0200CA58, 0x0200CA80, 0x0200CA80,
     "550997f7409373a3d8cc852d913327fa37f4b46e4111973acd33aeac15040044",
     "550997f7409373a3d8cc852d913327fa37f4b46e4111973acd33aeac15040044"),
    ("Game_UnwindCommitDescriptor_0200ca80", 0x0200CA80, 0x0200CAB0, 0x0200CAFC,
     "e7ff02e1abff015edc15a122901399abf6a9ccd24e65ba557c6f6d4444cd9f96",
     "3f21857868360e25c7afe42c9f06f5304ff9676c2002cf74f33358819e904f0f"),
    ("Game_AddDoubleBits_0200cafc", 0x0200CAFC, 0x0200CE00, 0x0200CE2C,
     "e2fa1b93d498f4c14a1864c6063c2a32db71458f8a00a636d56871ffc3036677",
     "13250e48e61d966669ca5cc5a8065f33e8e0bf1d098d99fe31c78937292da57d"),
    ("Game_SubtractDoubleBits_0200ce2c", 0x0200CE2C, 0x0200D1DC, 0x0200D1E0,
     "d6a970367f792ecbb229a2ccb5a78219c2a8b06b3029f69439cf255d93d8057b",
     "dbeaf6aee540d5755b85669163b690dfbf2a20d926a79b1bc86e16f15d89d685"),
    ("Game_MultiplyDoubleBits_0200d1e0", 0x0200D1E0, 0x0200D540, 0x0200D544,
     "403065cea9720cb498dafd7ae0cf79e4ee86b6dce85bb96ff785c2f7012d9dfe",
     "16a30a586ebe9ac165d52d79b8d6ece3be666b610328fe3fc4a778744e676c4e"),
    ("Game_DoubleBitsLessThan_0200d544", 0x0200D544, 0x0200D5E0, 0x0200D5E0,
     "c89eab0fdc174faf385a8d90c50be7178a9462fc3664374ac6d2100113b84ebc",
     "c89eab0fdc174faf385a8d90c50be7178a9462fc3664374ac6d2100113b84ebc"),
    ("Game_DoubleBitsEqual_0200d5e0", 0x0200D5E0, 0x0200D66C, 0x0200D66C,
     "d647e60f306d2f4623a7c6c0a1968c53d58225e92554b162c7370f39f90b6f76",
     "d647e60f306d2f4623a7c6c0a1968c53d58225e92554b162c7370f39f90b6f76"),
    ("Game_DoubleBitsNotEqual_0200d66c", 0x0200D66C, 0x0200D6F8, 0x0200D6F8,
     "72c6fbb9b03bd663f0478fd48cd17308e889897d92b307318ec03b004a76b68e",
     "72c6fbb9b03bd663f0478fd48cd17308e889897d92b307318ec03b004a76b68e"),
    ("Game_Int32ToFloatBits_0200d6f8", 0x0200D6F8, 0x0200D740, 0x0200D900,
     "770a6347a450031c316f34fcb22fcf452a2bd1a0f2fcd1abb3b7b70f146dd8ba",
     "eeded8f2be061a6a5e5fff7cf6cac5c39216a72692a3f8b094396ba2fa3ce797"),
    ("Game_DivideUint64_0200d900", 0x0200D900, 0x0200DA40, 0x0200DA40,
     "a4dabeeff141db0df3a3c22934f75df548c7b124996d1d6db661621f51219625",
     "a4dabeeff141db0df3a3c22934f75df548c7b124996d1d6db661621f51219625"),
    ("Game_RemainderUint64_0200d90c", 0x0200D90C, 0x0200D948, 0x0200D948,
     "04551970d10dab0e134e011f8909464d07016db3383f7e51e46003e184be12a1",
     "04551970d10dab0e134e011f8909464d07016db3383f7e51e46003e184be12a1"),
    ("Game_DivideInt32WithRemainder_0200d948", 0x0200D948, 0x0200D9CC, 0x0200DB54,
     "a59d4341b9656fee0a5d6889019aca7d15ab9cf27c935976a35402693de0f376",
     "e8183a62ffe6a54210cd7d025b01431ff792025c1b45a0e8537abd8cec16ccfb"),
    ("Game_DivideUint32Checked_0200db54", 0x0200DB54, 0x0200DB5C, 0x0200DB5C,
     "39ae71278d6ec40136278b02dfc9fa1f120ae28536326e1f9bbe66c51da57c51",
     "39ae71278d6ec40136278b02dfc9fa1f120ae28536326e1f9bbe66c51da57c51"),
    ("Game_DivideUint32_0200db5c", 0x0200DB5C, 0x0200DBAC, 0x0200DD38,
     "188e27deb0974ced93f5a4d323b4e95878dd2620602cb0e22f1194091c74be73",
     "5c91d1640b64a99527e9faff2db6a08378db2a9324a7c1a3ca5dc765b7671f07"),
    ("Game_FloatBitsToUint32_0200dd38", 0x0200DD38, 0x0200DD78, 0x0200DD78,
     "bca10f90b1a31d2b3ff9aab49f848e3c98c2e0a2f6df92a6b62e56ebe8c3c7dd",
     "bca10f90b1a31d2b3ff9aab49f848e3c98c2e0a2f6df92a6b62e56ebe8c3c7dd"),
    ("Game_DoubleBitsToUint64_0200dd78", 0x0200DD78, 0x0200DE00, 0x0200DE04,
     "d3be74b078583e99481e41ee6be12a653648613f66a18517a5ed0c15212b9110",
     "69a48b26acc1871c63b88f7920dc071f4111e0b99aae88821b76a956890bae8a"),
)


def interval_union_size(intervals: list[tuple[int, int]]) -> int:
    merged_start, merged_end = sorted(intervals)[0]
    total = 0
    for start, end in sorted(intervals)[1:]:
        if start > merged_end:
            total += merged_end - merged_start
            merged_start, merged_end = start, end
        else:
            merged_end = max(merged_end, end)
    return total + merged_end - merged_start


class RuntimeSupportRecoveryTests(unittest.TestCase):
    def test_rom_catalog_hashes_extents_and_overlap_topology(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def data(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        body_bytes = 0
        trailing_bytes = 0
        for index, entry in enumerate(FUNCTIONS):
            name, start, body_end, extent_end, body_sha, extent_sha = entry
            next_start = (
                FUNCTIONS[index + 1][1]
                if index + 1 < len(FUNCTIONS)
                else 0x0200DE04
            )
            with self.subTest(function=name):
                self.assertLess(start, body_end)
                self.assertEqual(extent_end, max(body_end, next_start))
                self.assertEqual(hashlib.sha256(data(start, body_end)).hexdigest(), body_sha)
                self.assertEqual(hashlib.sha256(data(start, extent_end)).hexdigest(), extent_sha)
                body_bytes += body_end - start
                trailing_bytes += extent_end - body_end

        body_intervals = [(entry[1], entry[2]) for entry in FUNCTIONS]
        extent_intervals = [(entry[1], entry[3]) for entry in FUNCTIONS]
        self.assertEqual(len(FUNCTIONS), 32)
        self.assertEqual(FUNCTIONS[0][1], 0x0200C55C)
        self.assertEqual(FUNCTIONS[-1][3], 0x0200DE04)
        self.assertEqual(0x0200DE04 - 0x0200C55C, 6312)
        self.assertEqual(body_bytes, 5156)
        self.assertEqual(trailing_bytes, 1464)
        self.assertEqual(interval_union_size(body_intervals), 4964)
        self.assertEqual(interval_union_size(extent_intervals), 6312)

        # 0x0200D90C and 0x0200D948 are true alternate/internal entries in
        # the 0x0200D900 helper, not boundaries that truncate its body.
        divide64 = FUNCTIONS[25]
        remainder64 = FUNCTIONS[26]
        divide32_signed = FUNCTIONS[27]
        self.assertEqual(divide64[2] - remainder64[1], 308)
        self.assertGreaterEqual(remainder64[1], divide64[1])
        self.assertLessEqual(remainder64[2], divide64[2])
        self.assertGreaterEqual(divide32_signed[1], divide64[1])
        self.assertLessEqual(divide32_signed[2], divide64[2])

    def test_source_and_config_own_each_entry_exactly_once(self) -> None:
        expected = {entry[1]: entry[0] for entry in FUNCTIONS}
        definition = re.compile(
            r"^(?:const\s+[A-Za-z_][A-Za-z0-9_]*\s*\*|uint64_t|uint32_t|"
            r"uint8_t|bool|int|void\s*\*|void)\s*"
            r"([A-Za-z_][A-Za-z0-9_]*)\s*\([^;]*?\)\s*\{",
            re.MULTILINE | re.DOTALL,
        )
        definitions: dict[str, list[str]] = {}
        for source in OWNERSHIP_SOURCES:
            for name in definition.findall(source.read_text()):
                definitions.setdefault(name, []).append(source.name)
        for name in expected.values():
            self.assertEqual(len(definitions.get(name, [])), 1, name)

        config = json.loads(
            (PROJECT_ROOT / "config/decomp/arm9-symbols.json").read_text()
        )
        draft = json.loads(
            (DRAFT_ROOT / "runtime-support-symbols.json").read_text()
        )
        symbols = list(config["symbols"])
        public_addresses = {int(symbol["address"], 16) for symbol in symbols}
        symbols.extend(
            symbol for symbol in draft["symbols"]
            if int(symbol["address"], 16) not in public_addresses
        )

        entries: dict[int, list[str]] = {}
        for symbol in symbols:
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
        include_args = [arg for path in INCLUDE_DIRS for arg in ("-I", str(path))]
        subprocess.run(
            [compiler, "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
             *include_args, "-fsyntax-only", *(str(source) for source in NEW_SOURCES)],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )

    def test_arm946es_c_syntax(self) -> None:
        compiler = shutil.which("clang")
        if compiler is None:
            self.skipTest("clang is unavailable")
        include_args = [arg for path in INCLUDE_DIRS for arg in ("-I", str(path))]
        subprocess.run(
            [compiler, "--target=arm-none-eabi", "-mcpu=arm946e-s", "-marm",
             "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
             "-ffreestanding", *include_args, "-fsyntax-only",
             *(str(source) for source in NEW_SOURCES)],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )


if __name__ == "__main__":
    unittest.main()
