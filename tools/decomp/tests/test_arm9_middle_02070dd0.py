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
CONFIG = PROJECT_ROOT / "config/decomp/arm9-middle-02070dd0.json"
SOURCE = PROJECT_ROOT / "src/arm9/game/arm9_middle_02070dd0_raw.c"
HEADER = PROJECT_ROOT / "include/game/arm9_middle_02070dd0_raw.h"
RAW_SHARD = PROJECT_ROOT / "build/decomp/arm9/modules/game_raw_0207.c"
ARM9_BASE = 0x02004000
FUNCTION_BLOCK = re.compile(
    r"(?ms)^/\* ={64}\n"
    r" \* (?P<name>.+?) @ (?P<entry>[0-9a-fA-F]+)\n"
    r" \* ={64} \*/\n"
    r"(?P<body>.*?)"
    r"(?=^/\* ={64}\n \* |\Z)"
)


class Arm9Middle02070dd0RecoveryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.config = json.loads(CONFIG.read_text(encoding="utf-8"))
        cls.catalog = json.loads(
            (PROJECT_ROOT / "build/decomp/arm9/catalog.json").read_text(
                encoding="utf-8"
            )
        )
        cls.arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

    def test_unique_config_closes_at_32_functions(self) -> None:
        selection = self.config["selection"]
        self.assertEqual(selection["entry_min"], "0x02070dd0")
        self.assertEqual(selection["entry_max"], "0x02072fc8")
        self.assertEqual(selection["last_body_end"], "0x02073034")
        self.assertEqual(selection["next_entry"], "0x0207303c")
        self.assertEqual(selection["function_count"], 32)
        self.assertEqual(selection["body_bytes"], 6120)
        self.assertEqual(selection["pool_bytes"], 2692)
        self.assertEqual(selection["span_bytes"], 8812)
        self.assertEqual(selection["closure"], "32-function checkpoint")

        configured = self.config["functions"]
        addresses = [int(item["address"], 16) for item in configured]
        self.assertEqual(len(addresses), 32)
        self.assertEqual(len(set(addresses)), 32)
        self.assertEqual(addresses, sorted(addresses))

        catalog_index = next(
            index
            for index, item in enumerate(self.catalog["functions"])
            if int(item["entry"], 16) == 0x02070DD0
        )
        catalog_selected = self.catalog["functions"][catalog_index : catalog_index + 32]
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

        selected = set(addresses)
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

    def test_rom_body_pool_extent_hashes_and_boundaries(self) -> None:
        inputs = self.config["inputs"]
        catalog_path = PROJECT_ROOT / inputs["catalog"]
        self.assertEqual(
            hashlib.sha256(catalog_path.read_bytes()).hexdigest(),
            inputs["catalog_sha256"],
        )
        self.assertEqual(
            hashlib.sha256(self.arm9).hexdigest(), inputs["target_sha256"]
        )

        total_bodies = 0
        total_pools = 0
        previous_extent_end = 0
        for function in self.config["functions"]:
            address = int(function["address"], 16)
            body_end = int(function["body_end"], 16)
            next_entry = int(function["next_entry"], 16)
            body = self.arm9[address - ARM9_BASE : body_end - ARM9_BASE]
            pool = self.arm9[body_end - ARM9_BASE : next_entry - ARM9_BASE]
            extent = self.arm9[address - ARM9_BASE : next_entry - ARM9_BASE]
            proof = function["pool"]
            with self.subTest(address=function["address"]):
                self.assertEqual(address + function["body_bytes"], body_end)
                self.assertGreaterEqual(address, previous_extent_end)
                self.assertEqual(hashlib.sha256(body).hexdigest(), function["body_sha256"])
                self.assertEqual(hashlib.sha256(extent).hexdigest(), function["extent_sha256"])
                self.assertEqual(f"0x{struct.unpack_from('<I', body)[0]:08x}", function["first_word"])
                self.assertEqual(
                    f"0x{struct.unpack_from('<I', body, len(body) - 4)[0]:08x}",
                    function["last_word"],
                )
                self.assertEqual(int(proof["address"], 16), body_end)
                self.assertEqual(proof["bytes"], len(pool))
                self.assertEqual(hashlib.sha256(pool).hexdigest(), proof["sha256"])
                if pool:
                    self.assertEqual(f"0x{struct.unpack_from('<I', pool)[0]:08x}", proof["first_word"])
                    self.assertEqual(
                        f"0x{struct.unpack_from('<I', pool, len(pool) - 4)[0]:08x}",
                        proof["last_word"],
                    )
                else:
                    self.assertIsNone(proof["first_word"])
                    self.assertIsNone(proof["last_word"])
            total_bodies += len(body)
            total_pools += len(pool)
            previous_extent_end = next_entry

        self.assertEqual(total_bodies, 6120)
        self.assertEqual(total_pools, 2692)
        self.assertEqual(total_bodies + total_pools, 8812)

    def test_raw_source_and_header_own_exactly_32_entries(self) -> None:
        text = SOURCE.read_text(encoding="utf-8")
        definitions = re.findall(
            r"^(?:void|bool|ushort\s*\*|undefined4|uint|int)\s+FUN_(020[0-9a-f]{5})\([^;]*?\)\s*\{",
            text,
            re.MULTILINE | re.DOTALL,
        )
        expected = [item["address"][2:] for item in self.config["functions"]]
        self.assertEqual(definitions, expected)
        self.assertEqual(len(set(definitions)), 32)

        selected = {int(address, 16) for address in expected}
        owners: dict[int, list[str]] = {address: [] for address in selected}
        definition = re.compile(
            r"^(?:void|bool|ushort\s*\*|undefined4|uint|int)\s+"
            r"FUN_(020[0-9a-f]{5})\([^;]*?\)\s*\{",
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

    def test_raw_bodies_match_export_with_only_documented_repairs(self) -> None:
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
                "FUN_0208a954();", "FUN_0208a954(iVar14);"
            )
            expected = expected.replace(
                "FUN_02085980();", "FUN_02085980(param_1);"
            )
            expected = expected.replace(
                "FUN_02061dc4();", "FUN_02061dc4(param_1);"
            )
            expected = expected.replace(
                "FUN_02072638();", "FUN_02072638(param_1);"
            )
            expected = expected.replace(
                "FUN_020859a0();", "FUN_020859a0(param_1);"
            )
            expected = expected.replace(
                "FUN_020506d8();", "FUN_020506d8(param_1);"
            )
            with self.subTest(address=f"0x{address:08x}"):
                self.assertEqual(recovered[address].rstrip(), expected.rstrip())

    def test_host_c_syntax(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        subprocess.run(
            [compiler, "-std=gnu89", "-w", "-I", str(PROJECT_ROOT / "include"),
             "-fsyntax-only", str(SOURCE)],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )

    def test_arm946es_syntax(self) -> None:
        compiler = shutil.which("clang")
        if compiler is None:
            self.skipTest("clang is unavailable")
        subprocess.run(
            [compiler, "--target=arm-none-eabi", "-mcpu=arm946e-s", "-marm",
             "-std=gnu89", "-w", "-ffreestanding", "-I", str(PROJECT_ROOT / "include"),
             "-fsyntax-only", str(SOURCE)],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )


if __name__ == "__main__":
    unittest.main()
