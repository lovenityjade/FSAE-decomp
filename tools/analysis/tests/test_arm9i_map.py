from __future__ import annotations

import json
import unittest
from pathlib import Path

from tools.analysis.arm9i_map import (
    Arm9iError,
    classify_function,
    compute_uncovered,
    matching_evidence,
    validate_partition,
)


class Arm9iMapTests(unittest.TestCase):
    def test_payload_offset_preserves_1232_byte_gap_total(self) -> None:
        project_root = Path(__file__).resolve().parents[3]
        with (project_root / "config/sdk-signatures-arm9i.json").open(
            encoding="utf-8"
        ) as stream:
            signatures = json.load(stream)
        with (project_root / "config/build/arm9i.json").open(encoding="utf-8") as stream:
            build_plan = json.load(stream)

        coverage = signatures["coverage_ranges"]
        gaps = compute_uncovered(coverage, 4, 18304)
        configured = [
            (entry["offset"], entry["end"])
            for entry in build_plan["previously_uncovered_ranges"]
        ]
        self.assertEqual(gaps, configured)
        self.assertEqual(sum(end - start for start, end in gaps), 1232)
        validate_partition(build_plan["object_ranges"], build_plan["target"]["size"])

    def test_partition_must_cover_without_holes(self) -> None:
        validate_partition([{"offset": 0, "end": 4}, {"offset": 4, "end": 20}], 20)
        with self.assertRaises(Arm9iError):
            validate_partition([{"offset": 0, "end": 4}, {"offset": 8, "end": 20}], 20)

    def test_exact_and_relocation_masked_matches_are_distinct(self) -> None:
        exact = classify_function(
            b"ABCD", {"code": b"ABCD", "mask": b"\x01\x01\x01\x01"}
        )
        masked = classify_function(
            b"ABxy", {"code": b"AB00", "mask": b"\x01\x01\x00\x00"}
        )
        self.assertEqual(exact["match_kind"], "byte_exact_reference")
        self.assertEqual(masked["match_kind"], "relocation_masked_reference")
        self.assertEqual(masked["reference_different_bytes"], 2)

    def test_nonmatching_clean_byte_is_rejected(self) -> None:
        with self.assertRaises(Arm9iError):
            classify_function(b"XBCD", {"code": b"ABCD", "mask": b"\x01\x01\x01\x01"})

    def test_partial_rebuild_does_not_claim_functions_or_unit(self) -> None:
        evidence = matching_evidence(72, 18324, "2026-01-01T00:00:00Z", False)
        self.assertEqual(evidence["metrics"]["bytes"], {"matched": 72, "total": 18324})
        self.assertEqual(evidence["metrics"]["functions"], {"matched": 0, "total": 0})
        self.assertEqual(evidence["metrics"]["units"], {"matched": 0, "total": 1})


if __name__ == "__main__":
    unittest.main()
