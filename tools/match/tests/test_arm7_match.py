from __future__ import annotations

import hashlib
import json
import struct
import tempfile
import unittest
from pathlib import Path

from tools.match.arm7_match import MatchError, run_match


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


class Arm7MatchTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.project = self.root / "project"
        self.sdk = self.root / "sdk"
        self.project.mkdir()
        self.sdk.mkdir()

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def write_config(
        self,
        *,
        section: str,
        reference: bytes,
        target: bytes,
        normalized: bytes,
        operations: list[dict[str, object]],
    ) -> Path:
        reference_path = self.sdk / "racoon.bin"
        target_path = self.project / "build/rom/sections/target.bin"
        reference_path.write_bytes(reference)
        target_path.parent.mkdir(parents=True)
        target_path.write_bytes(target)
        config = {
            "schema_version": 1,
            "kind": "arm7-match-config",
            "section": section,
            "reference": {"path": "racoon.bin", "size": len(reference), "sha256": digest(reference)},
            "target": {
                "path": "build/rom/sections/target.bin",
                "size": len(target),
                "sha256": digest(target),
            },
            "normalization": operations,
            "normalized": {"size": len(normalized), "sha256": digest(normalized)},
            "output": {
                "artifact": f"build/match/{section}.bin",
                "proof": f"build/match/{section}.proof.json",
                "progress": f"build/match/{section}.progress.json",
            },
        }
        config_path = self.project / "config.json"
        config_path.write_text(json.dumps(config), encoding="utf-8")
        return config_path

    def test_identity_component_matches_byte_for_byte(self) -> None:
        data = b"synthetic-arm7i-component"
        config = self.write_config(
            section="arm7i", reference=data, target=data, normalized=data, operations=[]
        )
        proof = run_match(config, self.project, self.sdk)
        self.assertTrue(proof["comparison"]["byte_for_byte_equal"])
        self.assertEqual(proof["dashboard_evidence"]["metrics"]["bytes"]["matched"], len(data))
        self.assertEqual((self.project / "build/match/arm7i.bin").read_bytes(), data)

    def test_patch_marker_and_static_footer_match(self) -> None:
        patch5 = b"ABCD" + bytes.fromhex("35750505") + b"payload"
        patch4 = b"ABCD" + bytes.fromhex("34750505") + b"payload"
        footer = struct.pack("<4I", 0xDEC00621, 4, 0, 8)
        operations: list[dict[str, object]] = [
            {
                "type": "sdk_version_patch",
                "offset": 4,
                "before_hex": "35750505",
                "after_hex": "34750505",
            },
            {
                "type": "strip_static_footer",
                "footer_hex": footer.hex(),
                "fields": {
                    "magic": "0xdec00621",
                    "module_params_offset": 4,
                    "digest_offset": 0,
                    "ltd_module_params_offset": 8,
                },
            },
        ]
        config = self.write_config(
            section="arm7", reference=patch5 + footer, target=patch4, normalized=patch4, operations=operations
        )
        original = (self.sdk / "racoon.bin").read_bytes()
        proof = run_match(config, self.project, self.sdk)
        self.assertTrue(proof["comparison"]["byte_for_byte_equal"])
        self.assertEqual((self.sdk / "racoon.bin").read_bytes(), original)
        self.assertEqual([item["type"] for item in proof["normalization"]["operations"]], ["sdk_version_patch", "strip_static_footer"])

    def test_wrong_footer_is_rejected(self) -> None:
        payload = b"payload"
        footer = struct.pack("<4I", 0xDEC00621, 4, 0, 8)
        wrong_footer = footer[:-1] + b"\xff"
        operation = {
            "type": "strip_static_footer",
            "footer_hex": footer.hex(),
            "fields": {
                "magic": "0xdec00621",
                "module_params_offset": 4,
                "digest_offset": 0,
                "ltd_module_params_offset": 8,
            },
        }
        config = self.write_config(
            section="arm7", reference=payload + wrong_footer, target=payload, normalized=payload, operations=[operation]
        )
        with self.assertRaisesRegex(MatchError, "does not end"):
            run_match(config, self.project, self.sdk)

    def test_wrong_sdk_marker_is_rejected(self) -> None:
        reference = b"ABCD" + bytes.fromhex("33750505")
        operation = {
            "type": "sdk_version_patch",
            "offset": 4,
            "before_hex": "35750505",
            "after_hex": "34750505",
        }
        config = self.write_config(
            section="arm7", reference=reference, target=reference, normalized=reference, operations=[operation]
        )
        with self.assertRaisesRegex(MatchError, "SDK version marker"):
            run_match(config, self.project, self.sdk)

    def test_mismatch_counts_zero_dashboard_bytes(self) -> None:
        reference = b"official"
        target = b"target!!"
        config = self.write_config(
            section="arm7i", reference=reference, target=target, normalized=reference, operations=[]
        )
        proof = run_match(config, self.project, self.sdk)
        self.assertFalse(proof["comparison"]["byte_for_byte_equal"])
        self.assertEqual(proof["dashboard_evidence"]["metrics"]["bytes"]["matched"], 0)
        self.assertGreater(proof["comparison"]["mismatch_bytes"], 0)

    def test_reference_hash_mismatch_is_rejected(self) -> None:
        data = b"official"
        config_path = self.write_config(
            section="arm7i", reference=data, target=data, normalized=data, operations=[]
        )
        config = json.loads(config_path.read_text(encoding="utf-8"))
        config["reference"]["sha256"] = "0" * 64
        config_path.write_text(json.dumps(config), encoding="utf-8")
        with self.assertRaisesRegex(MatchError, "reference: SHA-256"):
            run_match(config_path, self.project, self.sdk)


if __name__ == "__main__":
    unittest.main()
