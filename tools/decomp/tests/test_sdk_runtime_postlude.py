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
DRAFT_ROOT = PROJECT_ROOT / "build/agents/rom-pipeline"
PUBLIC_MANIFEST = PROJECT_ROOT / "config/decomp/arm9-sdk-runtime-0200de04.json"
MANIFEST = (
    PUBLIC_MANIFEST
    if PUBLIC_MANIFEST.exists()
    else DRAFT_ROOT / "arm9-sdk-runtime-0200de04.json"
)
PUBLIC_SOURCE = PROJECT_ROOT / "src/arm9/game/runtime_postlude.c"
SOURCE = PUBLIC_SOURCE if PUBLIC_SOURCE.exists() else DRAFT_ROOT / "runtime_postlude.c"
PUBLIC_HEADER = PROJECT_ROOT / "include/game/runtime_support.h"
DRAFT_HEADER = DRAFT_ROOT / "include/game/runtime_support.h"
HEADER = PUBLIC_HEADER if PUBLIC_SOURCE.exists() else DRAFT_HEADER
INCLUDE_DIRS = (
    (PROJECT_ROOT / "include",)
    if PUBLIC_SOURCE.exists()
    else (DRAFT_ROOT / "include", PROJECT_ROOT / "include")
)
ARM9_BASE = 0x02004000


class SdkRuntimePostludeRecoveryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.manifest = json.loads(MANIFEST.read_text())
        cls.arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()
        cls.catalog = json.loads(
            (PROJECT_ROOT / "build/decomp/arm9/catalog.json").read_text()
        )
        cls.sdk_signatures = json.loads(
            (PROJECT_ROOT / "config/sdk-signatures-arm9.json").read_text()
        )

    def test_catalog_bodies_extents_hashes_and_complete_topology(self) -> None:
        selection = self.manifest["selection"]
        functions = self.manifest["functions"]
        embedded = self.manifest["embedded_functions"]
        self.assertEqual(selection["entry_min"], "0x0200de04")
        self.assertEqual(selection["entry_max"], "0x0200e76c")
        self.assertEqual(selection["next_entry"], "0x0200e7c0")
        self.assertEqual(selection["catalog_function_count"], 32)
        self.assertEqual(selection["actual_function_count"], 36)
        self.assertEqual(selection["runtime_function_count"], 1)
        self.assertEqual(selection["sdk_function_count"], 35)

        inputs = self.manifest["inputs"]
        for field, path_text in (
            ("catalog_sha256", inputs["catalog"]),
            ("target_sha256", inputs["target"]),
            ("sdk_signatures_sha256", inputs["sdk_signatures"]),
        ):
            self.assertEqual(
                hashlib.sha256((PROJECT_ROOT / path_text).read_bytes()).hexdigest(),
                inputs[field],
            )

        catalog_functions = self.catalog["functions"]
        start = next(
            index for index, item in enumerate(catalog_functions)
            if item["entry"] == "0x0200de04"
        )
        selected = catalog_functions[start : start + 32]
        self.assertEqual(
            [item["address"] for item in functions],
            [item["entry"] for item in selected],
        )
        self.assertEqual(catalog_functions[start + 32]["entry"], "0x0200e7c0")

        body_total = 0
        gap_total = 0
        gaps: list[tuple[int, int]] = []
        for function, catalog_function in zip(functions, selected, strict=True):
            address = int(function["address"], 16)
            body_end = int(function["body_end"], 16)
            next_entry = int(function["next_entry"], 16)
            body = self.arm9[address - ARM9_BASE : body_end - ARM9_BASE]
            extent = self.arm9[address - ARM9_BASE : next_entry - ARM9_BASE]
            gap = self.arm9[body_end - ARM9_BASE : next_entry - ARM9_BASE]
            with self.subTest(function=function["name"]):
                self.assertEqual(function["body_bytes"], catalog_function["body_bytes"])
                self.assertEqual(body_end - address, function["body_bytes"])
                self.assertEqual(hashlib.sha256(body).hexdigest(), function["body_sha256"])
                self.assertEqual(hashlib.sha256(extent).hexdigest(), function["extent_sha256"])
                self.assertEqual(len(gap), function["gap"]["bytes"])
                self.assertEqual(hashlib.sha256(gap).hexdigest(), function["gap"]["sha256"])
            body_total += len(body)
            gap_total += len(gap)
            if gap:
                gaps.append((body_end, next_entry))

        embedded_total = 0
        previous_end = 0
        for function in embedded:
            address = int(function["address"], 16)
            end = int(function["end"], 16)
            body = self.arm9[address - ARM9_BASE : end - ARM9_BASE]
            with self.subTest(embedded=function["name"]):
                self.assertGreaterEqual(address, previous_end)
                self.assertTrue(any(begin <= address and end <= limit for begin, limit in gaps))
                self.assertEqual(len(body), function["bytes"])
                self.assertEqual(hashlib.sha256(body).hexdigest(), function["sha256"])
            embedded_total += len(body)
            previous_end = end

        self.assertEqual(body_total, selection["catalog_body_bytes"])
        self.assertEqual(body_total, 2276)
        self.assertEqual(gap_total, 216)
        self.assertEqual(embedded_total, selection["embedded_function_bytes"])
        self.assertEqual(embedded_total, 100)
        self.assertEqual(gap_total - embedded_total, selection["non_function_gap_bytes"])
        self.assertEqual(gap_total - embedded_total, 116)
        self.assertEqual(body_total + gap_total, selection["span_bytes"])
        self.assertEqual(selection["span_bytes"], 2492)

    def test_sdk_signature_ownership_veneers_and_arm9i_targets(self) -> None:
        signatures = {
            item["address"]: item for item in self.sdk_signatures["matches"]
        }
        signed = [
            item for item in self.manifest["functions"]
            if item["provenance"]["kind"] == "existing_sdk_signature"
        ]
        self.assertEqual(len(signed), 24)
        for function in signed:
            address = int(function["address"], 16)
            signature = signatures[address]
            with self.subTest(signature=function["name"]):
                self.assertEqual(signature["name"], function["provenance"]["signature_name"])
                self.assertEqual(signature["size"], function["provenance"]["signature_size"])
                self.assertEqual(signature["sources"], function["provenance"]["sources"])
                self.assertLessEqual(
                    signature["size"], int(function["next_entry"], 16) - address
                )

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", self.arm9, address - ARM9_BASE)[0]

        self.assertEqual(read_u32(0x0200DF78), 0x0201E498)
        self.assertEqual(read_u32(0x0200E21C), 0x0200E000)

        arm9i = json.loads((PROJECT_ROOT / "build/arm9i/analysis.json").read_text())
        core_names = {
            item["address"]: set(item["names"])
            for item in arm9i["functions"]
        }
        expected_cores = {
            0x021876F4: "SVCi_InitSignHeapCore",
            0x021876FC: "SVCi_SHA1InitCore",
            0x02187708: "SVCi_SHA1UpdateCore",
            0x0218770C: "SVCi_SHA1GetHashCore",
            0x02187718: "SVCi_CalcSHA1Core",
            0x0218771C: "SVCi_CompareSHA1Core",
            0x02187720: "SVCi_DecryptSignCore",
        }
        for address, name in expected_cores.items():
            self.assertIn(name, core_names[address])

    def test_patch4_archives_export_the_recovered_sdk_contracts(self) -> None:
        sdk_root_text = os.environ.get("TWLSDK_ROOT")
        if not sdk_root_text:
            self.skipTest("TWLSDK_ROOT is not set")
        readelf = shutil.which("readelf")
        if readelf is None:
            self.skipTest("readelf is unavailable")
        sdk_root = Path(sdk_root_text).resolve()
        archives = {
            "libsyscall.twl.a": sdk_root / "lib/ARM9-TS/etc/libsyscall.twl.a",
            "libfatfs.TWL.LTD.a": sdk_root / "lib/ARM9-TS/Rom/libfatfs.TWL.LTD.a",
        }
        exported: dict[tuple[str, str], set[int]] = {}
        symbol = re.compile(
            r"^\s*\d+:\s+[0-9a-fA-F]+\s+(\d+)\s+FUNC\s+.*\s(\S+)$"
        )
        for archive_name, archive in archives.items():
            result = subprocess.run(
                [readelf, "-Ws", str(archive)],
                check=True,
                text=True,
                stdout=subprocess.PIPE,
            )
            for line in result.stdout.splitlines():
                match = symbol.match(line)
                if match:
                    exported.setdefault((archive_name, match.group(2)), set()).add(
                        int(match.group(1))
                    )

        expected = {
            ("libsyscall.twl.a", "SVC_InitSignHeap"): 32,
            ("libsyscall.twl.a", "SVC_DecryptSign"): 36,
            ("libsyscall.twl.a", "SVC_SHA1Init"): 24,
            ("libsyscall.twl.a", "SVC_SHA1Update"): 32,
            ("libsyscall.twl.a", "SVC_SHA1GetHash"): 28,
            ("libsyscall.twl.a", "SVC_CalcSHA1"): 32,
            ("libsyscall.twl.a", "SVC_CompareSHA1"): 28,
            ("libfatfs.TWL.LTD.a", "FATFSi_IsInitialized"): 16,
            ("libfatfs.TWL.LTD.a", "FATFSi_GetArcnameList"): 16,
        }
        for key, size in expected.items():
            self.assertIn(size, exported[key], key)

    def test_runtime_source_header_and_symbol_registry_own_only_the_postlude(self) -> None:
        source = SOURCE.read_text()
        header = HEADER.read_text()
        definition = re.compile(
            r"^void\s+Game_RuntimeNoOp_0200de04\s*\(void\)\s*\{",
            re.MULTILINE,
        )
        self.assertEqual(len(definition.findall(source)), 1)
        self.assertEqual(header.count("Game_RuntimeNoOp_0200de04(void);"), 1)

        public = json.loads(
            (PROJECT_ROOT / "config/decomp/arm9-symbols.json").read_text()
        )["symbols"]
        draft = json.loads(
            (DRAFT_ROOT / "runtime-postlude-symbols.json").read_text()
        )["symbols"]
        entries = [item for item in public if item["address"] == "0x0200de04"]
        if not entries:
            entries = draft
        self.assertEqual(len(entries), 1)
        self.assertEqual(entries[0]["name"], "Game_RuntimeNoOp_0200de04")

    def test_host_and_arm946es_c_syntax(self) -> None:
        include_args = [
            argument
            for directory in INCLUDE_DIRS
            for argument in ("-I", str(directory))
        ]
        compilers = (
            (shutil.which("cc"), []),
            (
                shutil.which("clang"),
                ["--target=arm-none-eabi", "-mcpu=arm946e-s", "-marm", "-ffreestanding"],
            ),
        )
        for compiler, target_args in compilers:
            if compiler is None:
                continue
            with self.subTest(compiler=compiler):
                subprocess.run(
                    [compiler, *target_args, "-std=c11", "-Wall", "-Wextra",
                     "-Werror", "-pedantic", *include_args, "-fsyntax-only", str(SOURCE)],
                    check=True,
                    cwd=PROJECT_ROOT,
                    env={**os.environ, "CCACHE_DISABLE": "1"},
                )


if __name__ == "__main__":
    unittest.main()
