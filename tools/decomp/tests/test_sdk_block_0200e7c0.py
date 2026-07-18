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
DRAFT_ROOT = PROJECT_ROOT / "build/agents/rom-pipeline"
PUBLIC_MANIFEST = PROJECT_ROOT / "config/decomp/arm9-sdk-0200e7c0.json"
MANIFEST = (
    PUBLIC_MANIFEST
    if PUBLIC_MANIFEST.exists()
    else DRAFT_ROOT / "arm9-sdk-0200e7c0.json"
)
ARM9_BASE = 0x02004000


class SdkBlock0200e7c0RecoveryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.manifest = json.loads(MANIFEST.read_text())
        cls.arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()
        cls.catalog = json.loads(
            (PROJECT_ROOT / "build/decomp/arm9/catalog.json").read_text()
        )
        cls.signatures = json.loads(
            (PROJECT_ROOT / "config/sdk-signatures-arm9.json").read_text()
        )

    def test_catalog_hashes_extents_and_checkpoint_metrics(self) -> None:
        selection = self.manifest["selection"]
        self.assertEqual(selection["entry_min"], "0x0200e7c0")
        self.assertEqual(selection["entry_max"], "0x0200f99c")
        self.assertEqual(selection["next_entry"], "0x0200f9fc")
        self.assertEqual(selection["function_count"], 32)
        self.assertEqual(selection["sdk_function_count"], 32)
        self.assertEqual(selection["game_function_count"], 0)

        inputs = self.manifest["inputs"]
        for hash_field, path_field in (
            ("catalog_sha256", "catalog"),
            ("target_sha256", "target"),
            ("sdk_signatures_sha256", "sdk_signatures"),
        ):
            self.assertEqual(
                hashlib.sha256(
                    (PROJECT_ROOT / inputs[path_field]).read_bytes()
                ).hexdigest(),
                inputs[hash_field],
            )

        catalog_functions = self.catalog["functions"]
        start = next(
            index for index, item in enumerate(catalog_functions)
            if item["entry"] == "0x0200e7c0"
        )
        catalog_selected = catalog_functions[start : start + 32]
        functions = self.manifest["functions"]
        self.assertEqual(catalog_functions[start + 32]["entry"], "0x0200f9fc")
        self.assertEqual(
            [(item["entry"], item["name"]) for item in catalog_selected],
            [(item["address"], item["name"]) for item in functions],
        )

        body_total = 0
        gap_total = 0
        for function, catalog_function in zip(
            functions, catalog_selected, strict=True
        ):
            address = int(function["address"], 16)
            body_end = int(function["body_end"], 16)
            next_entry = int(function["next_entry"], 16)
            body = self.arm9[address - ARM9_BASE : body_end - ARM9_BASE]
            gap = self.arm9[body_end - ARM9_BASE : next_entry - ARM9_BASE]
            extent = self.arm9[address - ARM9_BASE : next_entry - ARM9_BASE]
            with self.subTest(function=function["name"]):
                self.assertEqual(catalog_function["classification"], "sdk")
                self.assertEqual(catalog_function["body_bytes"], len(body))
                self.assertEqual(hashlib.sha256(body).hexdigest(), function["body_sha256"])
                self.assertEqual(hashlib.sha256(extent).hexdigest(), function["extent_sha256"])
                self.assertEqual(len(gap), function["gap"]["bytes"])
                self.assertEqual(hashlib.sha256(gap).hexdigest(), function["gap"]["sha256"])
            body_total += len(body)
            gap_total += len(gap)

        self.assertEqual(body_total, selection["body_bytes"])
        self.assertEqual(body_total, 4616)
        self.assertEqual(gap_total, selection["relocation_literal_bytes"])
        self.assertEqual(gap_total, 52)
        self.assertEqual(body_total + gap_total, selection["span_bytes"])
        self.assertEqual(selection["span_bytes"], 4668)

    def test_existing_sdk_signatures_own_every_byte_without_semantic_duplicates(self) -> None:
        signatures = {item["address"]: item for item in self.signatures["matches"]}
        semantic = json.loads(
            (PROJECT_ROOT / "config/decomp/arm9-symbols.json").read_text()
        )["symbols"]
        semantic_addresses = {int(item["address"], 16) for item in semantic}
        matched_total = 0
        masked_total = 0
        for function in self.manifest["functions"]:
            address = int(function["address"], 16)
            next_entry = int(function["next_entry"], 16)
            signature = signatures[address]
            provenance = function["provenance"]
            with self.subTest(function=function["name"]):
                self.assertNotIn(address, semantic_addresses)
                self.assertEqual(signature["name"], function["name"])
                self.assertEqual(signature["size"], next_entry - address)
                self.assertEqual(signature["size"], provenance["signature_size"])
                self.assertEqual(signature["matched_bytes"], provenance["matched_bytes"])
                self.assertEqual(
                    signature["relocation_masked_bytes"],
                    provenance["relocation_masked_bytes"],
                )
                self.assertEqual(signature["sources"], provenance["sources"])
                self.assertEqual(
                    signature["matched_bytes"] + signature["relocation_masked_bytes"],
                    signature["size"],
                )
            matched_total += signature["matched_bytes"]
            masked_total += signature["relocation_masked_bytes"]

        self.assertEqual(matched_total, 4068)
        self.assertEqual(masked_total, 600)
        self.assertEqual(matched_total + masked_total, 4668)

    def test_patch4_archives_export_all_32_contracts(self) -> None:
        sdk_root_text = os.environ.get("TWLSDK_ROOT")
        if not sdk_root_text:
            self.skipTest("TWLSDK_ROOT is not set")
        readelf = shutil.which("readelf")
        if readelf is None:
            self.skipTest("readelf is unavailable")
        sdk_root = Path(sdk_root_text).resolve()
        library_names = {
            source["library"]
            for function in self.manifest["functions"]
            for source in function["provenance"]["sources"]
        }
        symbol = re.compile(
            r"^\s*\d+:\s+[0-9a-fA-F]+\s+(\d+)\s+FUNC\s+.*\s(\S+)$"
        )
        exports: dict[tuple[str, str], set[int]] = {}
        for library in library_names:
            archive = sdk_root / "lib/ARM9-TS/Rom" / library
            result = subprocess.run(
                [readelf, "-Ws", str(archive)],
                check=True,
                text=True,
                stdout=subprocess.PIPE,
            )
            for line in result.stdout.splitlines():
                match = symbol.match(line)
                if match:
                    exports.setdefault((library, match.group(2)), set()).add(
                        int(match.group(1))
                    )

        for function in self.manifest["functions"]:
            size = function["provenance"]["signature_size"]
            with self.subTest(function=function["name"]):
                self.assertTrue(
                    any(
                        size in exports.get((source["library"], function["name"]), set())
                        for source in function["provenance"]["sources"]
                    )
                )


if __name__ == "__main__":
    unittest.main()
