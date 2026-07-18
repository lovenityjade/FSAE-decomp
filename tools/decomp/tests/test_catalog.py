from __future__ import annotations

import sys
from pathlib import Path
import unittest


sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import catalog  # noqa: E402


def block(name: str, entry: str) -> str:
    return (
        "/* " + "=" * 64 + "\n"
        f" * {name} @ {entry}\n"
        " * " + "=" * 64 + " */\n\n"
        f"void {name}(void) {{}}\n\n"
    )


class CatalogTestCase(unittest.TestCase):
    def fixture(self):
        text = block("SDK_Function", "02001000")
        text += block("FUN_02001020", "02001020")
        text += block("caseD_2", "02001040")
        export = {
            "schema_version": 1,
            "program": "arm9.analysis.elf",
            "minimum_address": "02000000",
            "maximum_address": "0200ffff",
            "selected_functions": 3,
            "functions": [
                {"name": "SDK_Function", "entry": "02001000", "body_bytes": 16, "decompiled": True},
                {"name": "FUN_02001020", "entry": "02001020", "body_bytes": 24, "decompiled": True},
                {"name": "caseD_2", "entry": "02001040", "body_bytes": 8, "decompiled": True},
            ],
        }
        signatures = {
            "schema_version": 1,
            "image": "arm9",
            "load_address": 0x02000000,
            "matches": [
                {"address": 0x02001000, "size": 16, "name": "SDK_Function"},
                {"address": 0x01FF8000, "size": 32, "name": "SDK_Autoload"},
            ],
        }
        return text, export, signatures

    def test_classifies_sdk_game_and_switch_artifact(self) -> None:
        text, export, signatures = self.fixture()
        blocks = catalog.parse_c_blocks(text)
        report, shards = catalog.classify_export(export, signatures, blocks)
        self.assertEqual(
            report["summary"],
            {
                "exported_functions": 3,
                "decompiled_functions": 3,
                "sdk_functions": 1,
                "game_candidates": 1,
                "switch_artifacts": 1,
                "sdk_boundaries_outside_export": 1,
                "game_candidate_body_bytes": 24,
                "raw_module_shards": 1,
            },
        )
        self.assertEqual(list(shards), [0x0200])
        self.assertEqual(report["functions"][1]["file_offset"], 0x1020)

    def test_rejects_export_without_matching_c_block(self) -> None:
        text, export, signatures = self.fixture()
        blocks = catalog.parse_c_blocks(text)
        blocks.pop(0x02001020)
        with self.assertRaisesRegex(catalog.CatalogError, "missing recovered C block"):
            catalog.classify_export(export, signatures, blocks)


if __name__ == "__main__":
    unittest.main()
