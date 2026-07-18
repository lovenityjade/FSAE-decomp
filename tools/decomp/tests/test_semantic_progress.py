from __future__ import annotations

import sys
from pathlib import Path
import unittest


sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import semantic_progress  # noqa: E402


class SemanticProgressTests(unittest.TestCase):
    def test_projects_semantic_recovery_over_remaining_budget(self) -> None:
        catalog = {
            "summary": {
                "game_candidate_body_bytes": 100,
                "game_candidates": 2,
                "raw_module_shards": 1,
            },
            "functions": [
                {"entry": "0x02000000", "body_bytes": 25, "classification": "game_candidate"},
                {"entry": "0x02000020", "body_bytes": 75, "classification": "game_candidate"},
                {"entry": "0x02000080", "body_bytes": 40, "classification": "sdk"},
            ],
        }
        half = semantic_progress.measure(catalog, {0x02000020, 0x02000080}, 200)
        self.assertEqual(half["semantic_functions"], 1)
        self.assertEqual(half["semantic_bytes"], 75)
        self.assertEqual(half["covered_bytes"], 175)

        complete = semantic_progress.measure(catalog, {0x02000000, 0x02000020}, 200)
        self.assertEqual(complete["covered_bytes"], 200)

    def test_extracts_only_arm_runtime_style_addresses(self) -> None:
        import tempfile

        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "unit.c"
            path.write_text(
                "/* 0x02012340 */\n"
                " * 0x02123456, recovered function\n"
                "/* dependency at 0x02099999 */\n"
                "call(); /* 0x02088888 */\n"
                "/* 0x00000000 */\n"
            )
            self.assertEqual(
                semantic_progress.source_addresses([path]),
                {0x02012340, 0x02123456},
            )


if __name__ == "__main__":
    unittest.main()
