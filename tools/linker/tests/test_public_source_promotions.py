from __future__ import annotations

import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest

PROJECT_ROOT = Path(__file__).resolve().parents[3]
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

from tools.linker.incremental import load_manifest
from tools.linker.source_provider import (
    MAX_PROMOTION_BYTES,
    SourceProviderError,
    build_plan,
)


UNIT_MANIFEST = PROJECT_ROOT / "config/linker/units.v1.json"
PROMOTIONS = PROJECT_ROOT / "config/linker/arm9-source-promotions.v1.json"
SOURCE = "src/arm9/game/arm9_middle_02061818_raw.c"
PROOF = "config/decomp/arm9-middle-02061818.json"
HEADER = "include/game/arm9_middle_02061818_raw.h"
ARM9_BASE = 0x02004000
UNIT_OFFSET = 0x0005C000
UNIT_SIZE = 0x4000
ARM9_TARGET_SHA256 = "8bee49eedfb268c79ab371d3d2152c4e25f433d46daecd5dab2c0378a27300aa"
UNIT_TARGET_SHA256 = "d2ce5008eeb8650ebb378c2fa9232967c954ca5d963b7427117df488ad308443"
SOURCE_SHA256 = "a900f8a479b3733c5b4bf25c33d13e829af2be5fe89aae100b3a97f14c8ee750"
PROOF_SHA256 = "3cc804b3fcb92fb6649e4ea9665c9b9a075c0159701f5ccc3a9973e501ee8962"
HEADER_SHA256 = "8752db63ae6c42636adf1e8e6b3796f0a8205a6bf1e9ed1fc2786662ca179396"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


class PublicSourcePromotionsTests(unittest.TestCase):
    def setUp(self) -> None:
        build_parent = PROJECT_ROOT / "build"
        build_parent.mkdir(exist_ok=True)
        self.temporary = tempfile.TemporaryDirectory(
            prefix="source-promotions-test-", dir=build_parent
        )
        self.build = Path(self.temporary.name) / "linker"

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def test_manifest_pins_the_bounded_real_unit_and_public_evidence(self) -> None:
        value = json.loads(PROMOTIONS.read_text(encoding="utf-8"))
        self.assertEqual(value["schema_version"], 1)
        self.assertEqual(value["kind"], "arm9-source-promotions")
        self.assertEqual(len(value["units"]), 1)
        promotion = value["units"][0]
        self.assertEqual(promotion["image"], "arm9")
        self.assertEqual(promotion["unit_id"], "static-main-u023")
        self.assertEqual(promotion["include_dirs"], ["include"])
        self.assertEqual(promotion["defines"], [])
        self.assertEqual(
            promotion["sources"], [{"path": SOURCE, "proof": PROOF}]
        )

        manifest = load_manifest(UNIT_MANIFEST)
        arm9 = next(target for target in manifest.targets if target.image == "arm9")
        unit = next(unit for unit in arm9.units if unit.unit_id == "static-main-u023")
        self.assertEqual(arm9.sha256, ARM9_TARGET_SHA256)
        self.assertEqual(unit.region_id, "static-main")
        self.assertEqual(unit.target_offset, UNIT_OFFSET)
        self.assertEqual(unit.size, UNIT_SIZE)
        self.assertEqual(unit.size, MAX_PROMOTION_BYTES)
        self.assertEqual(unit.target_sha256, UNIT_TARGET_SHA256)
        self.assertEqual(ARM9_BASE + unit.target_offset, 0x02060000)
        self.assertEqual(ARM9_BASE + unit.target_offset + unit.size, 0x02064000)

        proof = json.loads((PROJECT_ROOT / PROOF).read_text(encoding="utf-8"))
        self.assertEqual(proof["kind"], "arm9-raw-recovery-block")
        self.assertEqual(proof["inputs"]["target_sha256"], arm9.sha256)
        self.assertEqual(proof["outputs"]["source"], SOURCE)
        self.assertEqual(proof["outputs"]["header"], HEADER)
        self.assertEqual(sha256(PROJECT_ROOT / SOURCE), SOURCE_SHA256)
        self.assertEqual(sha256(PROJECT_ROOT / PROOF), PROOF_SHA256)
        self.assertEqual(sha256(PROJECT_ROOT / HEADER), HEADER_SHA256)

    def test_real_plan_is_deterministic_and_pins_all_evidence_hashes(self) -> None:
        first = build_plan(UNIT_MANIFEST, PROMOTIONS, PROJECT_ROOT, self.build)
        second = build_plan(UNIT_MANIFEST, PROMOTIONS, PROJECT_ROOT, self.build)
        self.assertEqual(first, second)
        self.assertEqual(first["plan_sha256"], second["plan_sha256"])
        self.assertEqual(first["unit_count"], 1)
        self.assertEqual(first["command_count"], 1)
        self.assertEqual(first["total_bytes"], MAX_PROMOTION_BYTES)
        self.assertEqual(first["credited_matching_bytes"], 0)
        unit = first["units"][0]
        self.assertEqual(unit["unit_id"], "static-main-u023")
        self.assertEqual(unit["target_sha256"], UNIT_TARGET_SHA256)
        self.assertEqual(unit["provider_path"], "arm9/static-main/u023.bin")
        self.assertEqual(
            unit["sources"],
            [{
                "path": SOURCE,
                "sha256": SOURCE_SHA256,
                "proof": PROOF,
                "proof_sha256": PROOF_SHA256,
                "dependencies": [{"path": HEADER, "sha256": HEADER_SHA256}],
            }],
        )
        self.assertEqual(unit["commands"][0][0], "mwccarm")
        self.assertIn(SOURCE, unit["commands"][0])
        on_disk = json.loads(
            (self.build / "source-provider/plan.v1.json").read_text(
                encoding="utf-8"
            )
        )
        self.assertEqual(on_disk, first)

    def test_manifest_cannot_grow_past_the_16_kib_batch_cap(self) -> None:
        value = json.loads(PROMOTIONS.read_text(encoding="utf-8"))
        second = json.loads(json.dumps(value["units"][0]))
        second["unit_id"] = "static-main-u024"
        value["units"].append(second)
        oversized = Path(self.temporary.name) / "oversized-promotions.json"
        oversized.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(SourceProviderError, "exceeds the 16 KiB cap"):
            build_plan(UNIT_MANIFEST, oversized, PROJECT_ROOT, self.build)


if __name__ == "__main__":
    unittest.main()
