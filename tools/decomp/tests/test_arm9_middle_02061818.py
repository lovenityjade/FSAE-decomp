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
CONFIG = PROJECT_ROOT / "config/decomp/arm9-middle-02061818.json"
SOURCE = PROJECT_ROOT / "src/arm9/game/arm9_middle_02061818_raw.c"
HEADER = PROJECT_ROOT / "include/game/arm9_middle_02061818_raw.h"
ARM9_BASE = 0x02004000
RAW_SHARD = PROJECT_ROOT / "build/decomp/arm9/modules/game_raw_0206.c"
FUNCTION_BLOCK = re.compile(
    r"(?ms)^/\* ={64}\n"
    r" \* (?P<name>.+?) @ (?P<entry>[0-9a-fA-F]+)\n"
    r" \* ={64} \*/\n"
    r"(?P<body>.*?)"
    r"(?=^/\* ={64}\n \* |\Z)"
)


class Arm9Middle02061818RecoveryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.config = json.loads(CONFIG.read_text(encoding="utf-8"))
        cls.catalog = json.loads(
            (PROJECT_ROOT / "build/decomp/arm9/catalog.json").read_text(
                encoding="utf-8"
            )
        )
        cls.arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

    def test_unique_config_selects_exact_catalog_block(self) -> None:
        selection = self.config["selection"]
        self.assertEqual(selection["entry_min"], "0x02061818")
        self.assertEqual(selection["entry_max"], "0x02063fff")
        self.assertEqual(selection["last_body_end"], "0x0206402c")
        self.assertEqual(selection["function_count"], 31)
        self.assertEqual(selection["body_bytes"], 8848)

        configured = self.config["functions"]
        configured_addresses = [int(item["address"], 16) for item in configured]
        self.assertEqual(len(configured_addresses), 31)
        self.assertEqual(len(set(configured_addresses)), 31)
        self.assertEqual(configured_addresses, sorted(configured_addresses))

        catalog_selected = [
            item
            for item in self.catalog["functions"]
            if 0x02061818 <= int(item["entry"], 16) <= 0x02063FFF
        ]
        self.assertEqual(len(catalog_selected), 31)
        self.assertEqual(
            [
                (item["address"], item["name"], item["body_bytes"])
                for item in configured
            ],
            [
                (item["entry"], item["name"], item["body_bytes"])
                for item in catalog_selected
            ],
        )

        selected = set(configured_addresses)
        foreign_owners: dict[int, list[str]] = {}
        for path in (PROJECT_ROOT / "config/decomp").glob("*.json"):
            if path == CONFIG:
                continue
            value = json.loads(path.read_text(encoding="utf-8"))
            records = []
            for field in ("functions", "symbols"):
                if isinstance(value.get(field), list):
                    records.extend(value[field])
            for record in records:
                if not isinstance(record, dict):
                    continue
                address_text = record.get("address", record.get("entry"))
                if not isinstance(address_text, str):
                    continue
                address = int(address_text, 16)
                if address in selected:
                    foreign_owners.setdefault(address, []).append(path.name)
        self.assertEqual(foreign_owners, {})

    def test_rom_hashes_and_function_boundaries(self) -> None:
        inputs = self.config["inputs"]
        catalog_path = PROJECT_ROOT / inputs["catalog"]
        self.assertEqual(
            hashlib.sha256(catalog_path.read_bytes()).hexdigest(),
            inputs["catalog_sha256"],
        )
        self.assertEqual(
            hashlib.sha256(self.arm9).hexdigest(), inputs["target_sha256"]
        )

        catalog_functions = self.catalog["functions"]
        catalog_index = {
            int(item["entry"], 16): index
            for index, item in enumerate(catalog_functions)
        }
        total = 0
        previous_end = 0
        for function in self.config["functions"]:
            address = int(function["address"], 16)
            size = function["body_bytes"]
            body = self.arm9[address - ARM9_BASE : address - ARM9_BASE + size]
            with self.subTest(address=function["address"]):
                self.assertEqual(len(body), size)
                self.assertGreaterEqual(address, previous_end)
                self.assertEqual(
                    hashlib.sha256(body).hexdigest(), function["body_sha256"]
                )
                self.assertEqual(
                    f"0x{struct.unpack_from('<I', body, 0)[0]:08x}",
                    function["first_word"],
                )
                self.assertEqual(
                    f"0x{struct.unpack_from('<I', body, len(body) - 4)[0]:08x}",
                    function["last_word"],
                )
                next_catalog = catalog_functions[catalog_index[address] + 1]
                self.assertEqual(
                    function["next_entry"], next_catalog["entry"]
                )
                self.assertLessEqual(
                    address + size, int(function["next_entry"], 16)
                )
            previous_end = address + size
            total += size

        self.assertEqual(total, 8848)
        last = self.config["functions"][-1]
        self.assertEqual(
            int(last["address"], 16) + last["body_bytes"], 0x0206402C
        )

    def test_raw_source_has_one_body_per_selected_entry(self) -> None:
        text = SOURCE.read_text(encoding="utf-8")
        definitions = re.findall(
            r"^(?:void|bool)\s+FUN_(020[0-9a-f]{5})\([^;]*?\)\s*\{",
            text,
            re.MULTILINE | re.DOTALL,
        )
        expected = [item["address"][2:] for item in self.config["functions"]]
        self.assertEqual(definitions, expected)
        self.assertEqual(len(set(definitions)), 31)
        self.assertIn("not linked byte-matching evidence", text)

        selected = {int(address, 16) for address in expected}
        owners: dict[int, list[str]] = {address: [] for address in selected}
        definition = re.compile(
            r"^(?:void|bool)\s+FUN_(020[0-9a-f]{5})\([^;]*?\)\s*\{",
            re.MULTILINE | re.DOTALL,
        )
        for path in (PROJECT_ROOT / "src/arm9/game").glob("*.c"):
            for address_text in definition.findall(
                path.read_text(encoding="utf-8")
            ):
                address = int(address_text, 16)
                if address in owners:
                    owners[address].append(path.name)
        self.assertTrue(
            all(names == [SOURCE.name] for names in owners.values()), owners
        )

        header = HEADER.read_text(encoding="utf-8")
        for address in expected:
            self.assertEqual(header.count(f"FUN_{address}("), 1)

    def test_raw_bodies_match_export_with_only_documented_syntax_repairs(
        self,
    ) -> None:
        def blocks(text: str) -> dict[int, str]:
            return {
                int(match.group("entry"), 16): match.group(0)
                for match in FUNCTION_BLOCK.finditer(text)
            }

        exported = blocks(RAW_SHARD.read_text(encoding="utf-8"))
        recovered = blocks(SOURCE.read_text(encoding="utf-8"))
        selected = [int(item["address"], 16) for item in self.config["functions"]]
        self.assertEqual(set(recovered), set(selected))

        for address in selected:
            expected = "\n".join(
                line.rstrip() for line in exported[address].splitlines()
            )
            expected = expected.replace(
                "  ushort uVar2;\n  ushort uVar2;", "  ushort uVar2;"
            )
            for callee in (
                "FUN_02061d0c",
                "FUN_02061e44",
                "FUN_020623c8",
                "FUN_02061dc4",
                "FUN_020630b0",
                "FUN_020633bc",
            ):
                expected = expected.replace(
                    f"{callee}();", f"{callee}(param_1);"
                )
            with self.subTest(address=f"0x{address:08x}"):
                self.assertEqual(recovered[address].rstrip(), expected.rstrip())

    def test_host_c_syntax(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        subprocess.run(
            [
                compiler,
                "-std=gnu89",
                "-w",
                "-I",
                str(PROJECT_ROOT / "include"),
                "-fsyntax-only",
                str(SOURCE),
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
                "-std=gnu89",
                "-w",
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
