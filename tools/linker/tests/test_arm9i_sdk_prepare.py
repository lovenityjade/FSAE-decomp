from __future__ import annotations

import hashlib
import importlib.util
import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).parents[1] / "arm9i_sdk_prepare.py"
REPOSITORY_ROOT = Path(__file__).resolve().parents[3]
SPEC = importlib.util.spec_from_file_location("arm9i_sdk_prepare_draft", MODULE_PATH)
assert SPEC and SPEC.loader
sdk_prepare = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(sdk_prepare)


def ar_header(name: str, size: int) -> bytes:
    fields = (
        name.encode("ascii").ljust(16)
        + b"0".ljust(12)
        + b"0".ljust(6)
        + b"0".ljust(6)
        + b"100644".ljust(8)
        + str(size).encode("ascii").ljust(10)
        + b"`\n"
    )
    assert len(fields) == 60
    return fields


def ar_archive(members: list[tuple[str, bytes]], style: str = "short") -> bytes:
    result = bytearray(b"!<arch>\n")
    if style == "gnu":
        table = b"".join(name.encode() + b"/\n" for name, _ in members)
        result += ar_header("//", len(table)) + table
        if len(table) & 1:
            result += b"\n"
        offset = 0
        encoded: list[tuple[str, bytes]] = []
        for name, payload in members:
            encoded.append((f"/{offset}", payload))
            offset += len(name.encode()) + 2
    elif style == "bsd":
        encoded = []
        for name, payload in members:
            name_bytes = name.encode()
            encoded.append((f"#1/{len(name_bytes)}", name_bytes + payload))
    else:
        encoded = [(name + "/", payload) for name, payload in members]
    for name, payload in encoded:
        result += ar_header(name, len(payload)) + payload
        if len(payload) & 1:
            result += b"\n"
    return bytes(result)


class Arm9iSdkPrepareTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.project = self.root / "project"
        self.sdk = self.root / "private-sdk"
        (self.project / "build").mkdir(parents=True)
        (self.project / "config/build").mkdir(parents=True)
        (self.sdk / "lib/one").mkdir(parents=True)
        (self.sdk / "lib/two").mkdir(parents=True)

        self.archive_one = ar_archive(
            [("one.o", b"ONE"), ("not-selected.o", b"SECRET"), ("long_member_name.o", b"LONG")],
            style="gnu",
        )
        self.archive_two = ar_archive([("bare_member", b"BARE")], style="bsd")
        (self.sdk / "lib/one/libone.a").write_bytes(self.archive_one)
        (self.sdk / "lib/two/libtwo.a").write_bytes(self.archive_two)
        self.config = self.project / "config/build/arm9i.json"
        self.plan = {
            "schema_version": 1,
            "kind": "arm9i-build-plan",
            "archives": [
                {
                    "name": "libone.a",
                    "path": "lib/one/libone.a",
                    "sha256": hashlib.sha256(self.archive_one).hexdigest(),
                },
                {
                    "name": "libtwo.a",
                    "path": "lib/two/libtwo.a",
                    "sha256": hashlib.sha256(self.archive_two).hexdigest(),
                },
            ],
            "object_ranges": [
                {"kind": "zero_fill"},
                {"kind": "sdk_object", "archive": "libone.a", "object": "one.o"},
                {"kind": "sdk_object", "archive": "libtwo.a", "object": "bare_member"},
                {"kind": "sdk_object", "archive": "libone.a", "object": "long_member_name.o"},
                {"kind": "sdk_object", "archive": "libone.a", "object": "one.o"},
            ],
        }
        self.config.write_text(json.dumps(self.plan), encoding="utf-8")

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def parse(self):
        return sdk_prepare.parse_plan(self.config)

    def output(self) -> Path:
        return sdk_prepare.output_root(self.project, Path("build/sdk"))

    def test_inventory_selects_only_required_members_and_redacts_sdk_root(self) -> None:
        inventory, payloads = sdk_prepare.build_inventory(self.parse(), self.sdk)
        self.assertEqual(inventory["archive_count"], 2)
        self.assertEqual(inventory["selected_member_count"], 3)
        self.assertEqual(set(payloads), {
            ("libone.a", "one.o"),
            ("libone.a", "long_member_name.o"),
            ("libtwo.a", "bare_member"),
        })
        encoded = json.dumps(inventory)
        self.assertNotIn(str(self.sdk), encoded)
        self.assertNotIn("not-selected.o", encoded)
        self.assertNotIn("SECRET", encoded)
        self.assertEqual(inventory["link_order"], [
            {"archive": "libone.a", "member": "one.o"},
            {"archive": "libtwo.a", "member": "bare_member"},
            {"archive": "libone.a", "member": "long_member_name.o"},
        ])

    def test_checked_in_plan_selects_seven_archives_and_fifteen_members(self) -> None:
        plan = sdk_prepare.parse_plan(REPOSITORY_ROOT / "config/build/arm9i.json")
        self.assertEqual(len(plan["required"]), 7)
        self.assertEqual(len(plan["link_order"]), 15)
        self.assertEqual(plan["link_order"][0], {
            "archive": "libna.TWL.LTD.thumb.a",
            "member": "na_archive.o",
        })
        self.assertEqual(plan["link_order"][-1], {
            "archive": "libcard.TWL.LTD.a",
            "member": "card_hash.o",
        })

    def test_extract_writes_exact_selected_set_and_validates(self) -> None:
        plan = self.parse()
        inventory, payloads = sdk_prepare.build_inventory(plan, self.sdk)
        root = self.output()
        root.mkdir(parents=True)
        report = sdk_prepare.extract_members(root, inventory, payloads)
        sdk_prepare.atomic_json(root / "extraction.v1.json", report)
        validated = sdk_prepare.validate_extraction(root, plan)
        self.assertEqual(validated["selection_sha256"], inventory["selection_sha256"])
        files = {
            path.name: path.read_bytes()
            for path in (root / "sets" / inventory["selection_sha256"]).rglob("*")
            if path.is_file()
        }
        self.assertEqual(files, {
            "one.o": b"ONE",
            "long_member_name.o": b"LONG",
            "bare_member": b"BARE",
        })

    def test_validate_rejects_mutated_or_extra_member(self) -> None:
        plan = self.parse()
        inventory, payloads = sdk_prepare.build_inventory(plan, self.sdk)
        root = self.output()
        root.mkdir(parents=True)
        report = sdk_prepare.extract_members(root, inventory, payloads)
        sdk_prepare.atomic_json(root / "extraction.v1.json", report)
        first = root / report["members"][0]["path"]
        first.write_bytes(b"changed")
        with self.assertRaisesRegex(sdk_prepare.PreparationError, "integrity mismatch"):
            sdk_prepare.validate_extraction(root, plan)
        first.write_bytes(b"ONE")
        (first.parent / "extra.o").write_bytes(b"extra")
        with self.assertRaisesRegex(sdk_prepare.PreparationError, "unexpected"):
            sdk_prepare.validate_extraction(root, plan)

    def test_rejects_archive_hash_mismatch(self) -> None:
        self.plan["archives"][0]["sha256"] = "0" * 64
        self.config.write_text(json.dumps(self.plan), encoding="utf-8")
        with self.assertRaisesRegex(sdk_prepare.PreparationError, "SHA-256 mismatch"):
            sdk_prepare.build_inventory(self.parse(), self.sdk)

    def test_rejects_missing_and_duplicate_required_member(self) -> None:
        missing = ar_archive([("other.o", b"OTHER")])
        (self.sdk / "lib/one/libone.a").write_bytes(missing)
        self.plan["archives"][0]["sha256"] = hashlib.sha256(missing).hexdigest()
        self.config.write_text(json.dumps(self.plan), encoding="utf-8")
        with self.assertRaisesRegex(sdk_prepare.PreparationError, "missing required member"):
            sdk_prepare.build_inventory(self.parse(), self.sdk)

        duplicate = ar_archive(
            [("one.o", b"A"), ("one.o", b"B"), ("long_member_name.o", b"C")],
            style="gnu",
        )
        (self.sdk / "lib/one/libone.a").write_bytes(duplicate)
        self.plan["archives"][0]["sha256"] = hashlib.sha256(duplicate).hexdigest()
        self.config.write_text(json.dumps(self.plan), encoding="utf-8")
        with self.assertRaisesRegex(sdk_prepare.PreparationError, "more than once"):
            sdk_prepare.build_inventory(self.parse(), self.sdk)

    def test_rejects_unsafe_plan_and_output_outside_build(self) -> None:
        with self.assertRaisesRegex(sdk_prepare.PreparationError, "beneath project build"):
            sdk_prepare.output_root(self.project, self.root / "outside")
        self.plan["object_ranges"][1]["object"] = "../one.o"
        self.config.write_text(json.dumps(self.plan), encoding="utf-8")
        with self.assertRaisesRegex(sdk_prepare.PreparationError, "safe file name"):
            self.parse()

    def test_rejects_sdk_symlink_escape(self) -> None:
        outside = self.root / "elsewhere.a"
        outside.write_bytes(self.archive_one)
        archive_path = self.sdk / "lib/one/libone.a"
        archive_path.unlink()
        archive_path.symlink_to(outside)
        with self.assertRaisesRegex(sdk_prepare.PreparationError, "escapes the SDK root"):
            sdk_prepare.build_inventory(self.parse(), self.sdk)

    def test_cli_requires_sdk_and_keeps_diagnostics_private(self) -> None:
        environment = os.environ.copy()
        environment.pop("TWLSDK_ROOT", None)
        result = subprocess.run(
            [sys.executable, str(MODULE_PATH), "inventory", "--project-root", str(self.project),
             "--config", str(self.config),
             "--output", str(self.project / "build/sdk")],
            cwd=self.project,
            env=environment,
            capture_output=True,
            text=True,
            check=False,
        )
        self.assertEqual(result.returncode, 2)
        self.assertIn("--sdk-root or TWLSDK_ROOT is required", result.stderr)

    def test_cli_extracts_synthetic_sdk_without_leaking_its_root(self) -> None:
        output = self.project / "build/sdk-cli"
        result = subprocess.run(
            [sys.executable, str(MODULE_PATH), "extract", "--project-root", str(self.project),
             "--config", str(self.config), "--sdk-root", str(self.sdk),
             "--output", str(output)],
            cwd=self.project,
            capture_output=True,
            text=True,
            check=False,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertNotIn(str(self.sdk), result.stdout)
        self.assertNotIn("not-selected.o", result.stdout)
        self.assertTrue((output / "inventory.v1.json").is_file())
        self.assertTrue((output / "extraction.v1.json").is_file())
        validation = subprocess.run(
            [sys.executable, str(MODULE_PATH), "validate", "--project-root", str(self.project),
             "--config", str(self.config), "--output", str(output), "--quiet"],
            cwd=self.project,
            capture_output=True,
            text=True,
            check=False,
        )
        self.assertEqual(validation.returncode, 0, validation.stderr)

    def test_validate_rejects_malformed_report_instead_of_crashing(self) -> None:
        root = self.output()
        root.mkdir(parents=True)
        malformed = {
            "schema_version": 1,
            "kind": "arm9i-sdk-member-extraction",
            "plan_sha256": self.parse()["plan_sha256"],
            "selection_sha256": "0" * 64,
            "archive_count": 2,
            "selected_member_count": 3,
            "members": [None, None, None],
            "link_order": self.parse()["link_order"],
        }
        (root / "extraction.v1.json").write_text(json.dumps(malformed), encoding="utf-8")
        with self.assertRaisesRegex(sdk_prepare.PreparationError, "must be an object"):
            sdk_prepare.validate_extraction(root, self.parse())


if __name__ == "__main__":
    unittest.main()
