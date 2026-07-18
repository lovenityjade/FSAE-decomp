from __future__ import annotations

import hashlib
import json
from pathlib import Path
import tempfile
import unittest
import zipfile

from tools.sdk import prepare_patch4 as sdk


P5 = bytes.fromhex("35750505")
P4 = bytes.fromhex("34750505")
BASE = bytes.fromhex("30750505")


def write_zip(path: Path, prefix: str, files: dict[str, bytes]) -> None:
    with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        for relative, data in files.items():
            archive.writestr(prefix + relative, data)


def archive_spec(path: Path, prefix: str, files: dict[str, bytes]) -> dict:
    return {
        "name": path.name,
        "sha256": sdk.sha256_file(path),
        "root_prefix": prefix,
        "file_count": len(files),
        "uncompressed_size": sum(map(len, files.values())),
    }


def fixture_config(base_zip: Path, patch_zip: Path, base_files: dict, patch_files: dict) -> dict:
    return {
        "schema_version": 1,
        "target": {"name": "synthetic", "release_step": 30004},
        "archives": {
            "base": archive_spec(base_zip, "base/TwlSDK/", base_files),
            "patch5": archive_spec(patch_zip, "patch/TwlSDK/", patch_files),
        },
        "overlay": {
            "expected_applied_files": 3,
            "expected_skipped_files": 1,
            "skip_patch5_only": [
                {"pattern": "skip.txt", "expected_matches": 1, "reason": "patch5 only"}
            ],
        },
        "version_headers": [
            {
                "path": "include/twl/version.h",
                "replacements": [
                    {"from": "30005", "to": "30004", "expected_count": 1}
                ],
            }
        ],
        "binary_release_marker": {
            "from_hex": P5.hex(),
            "to_hex": P4.hex(),
            "base_hex": BASE.hex(),
            "expected_files": 1,
            "expected_occurrences": 1,
        },
        "expected_tree": {"file_count": 0, "total_size": 0, "sha256": "0" * 64},
        "evidence": {"proved": [], "inferred": []},
        "_sha256": "1" * 64,
    }


class PreparePatch4Tests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.base_zip = self.root / "base.zip"
        self.patch_zip = self.root / "patch.zip"
        self.base_files = {
            "include/twl/version.h": b"RELSTEP 30000\n",
            "binary.bin": b"prefix" + BASE + b"suffix",
            "keep.txt": b"base keep",
            "skip.txt": b"base skip",
        }
        self.patch_files = {
            "include/twl/version.h": b"RELSTEP 30005\n",
            "binary.bin": b"prefix" + P5 + b"suffix",
            "keep.txt": b"patched cumulative",
            "skip.txt": b"patch5 only",
        }
        write_zip(self.base_zip, "base/TwlSDK/", self.base_files)
        write_zip(self.patch_zip, "patch/TwlSDK/", self.patch_files)
        self.config = fixture_config(
            self.base_zip, self.patch_zip, self.base_files, self.patch_files
        )

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def derive_expected(self, output: Path) -> None:
        diagnostic = sdk.diagnose(self.base_zip, self.patch_zip, self.config)
        output.mkdir()
        built = sdk.construct_tree(output, self.config, diagnostic)
        self.config["expected_tree"] = built["tree"]

    def test_prepare_and_idempotent_second_run(self) -> None:
        derivation = self.root / "derive"
        self.derive_expected(derivation)
        output = self.root / "external-output"
        first = sdk.prepare(self.base_zip, self.patch_zip, output, self.config)
        second = sdk.prepare(self.base_zip, self.patch_zip, output, self.config)
        self.assertEqual(first["status"], "prepared")
        self.assertEqual(second["status"], "already-prepared")
        tree = output / "TwlSDK"
        self.assertEqual((tree / "include/twl/version.h").read_bytes(), b"RELSTEP 30004\n")
        self.assertEqual((tree / "binary.bin").read_bytes(), b"prefix" + P4 + b"suffix")
        self.assertEqual((tree / "keep.txt").read_bytes(), b"patched cumulative")
        self.assertEqual((tree / "skip.txt").read_bytes(), b"base skip")
        manifest = json.loads((output / "preparation-manifest.json").read_text())
        self.assertEqual(manifest["tree"], self.config["expected_tree"])

    def test_rejects_archive_hash_mismatch(self) -> None:
        self.config["archives"]["base"]["sha256"] = "0" * 64
        with self.assertRaisesRegex(sdk.SdkPreparationError, "SHA-256 mismatch"):
            sdk.diagnose(self.base_zip, self.patch_zip, self.config)

    def test_rejects_skip_rule_count_mismatch(self) -> None:
        self.config["overlay"]["skip_patch5_only"][0]["expected_matches"] = 2
        with self.assertRaisesRegex(sdk.SdkPreparationError, "matched 1 files"):
            sdk.diagnose(self.base_zip, self.patch_zip, self.config)

    def test_rejects_final_tree_digest_mismatch(self) -> None:
        self.config["expected_tree"] = {
            "file_count": 4,
            "total_size": 1,
            "sha256": "0" * 64,
        }
        with self.assertRaisesRegex(sdk.SdkPreparationError, "prepared tree total_size mismatch"):
            sdk.prepare(
                self.base_zip,
                self.patch_zip,
                self.root / "wrong-tree-output",
                self.config,
            )
        self.assertFalse((self.root / "wrong-tree-output").exists())

    def test_rejects_unsafe_zip_member(self) -> None:
        unsafe = self.root / "unsafe.zip"
        with zipfile.ZipFile(unsafe, "w") as archive:
            archive.writestr("patch/TwlSDK/../escape", b"bad")
        self.config["archives"]["patch5"] = {
            "name": "unsafe.zip",
            "sha256": sdk.sha256_file(unsafe),
            "root_prefix": "patch/TwlSDK/",
            "file_count": 1,
            "uncompressed_size": 3,
        }
        with self.assertRaisesRegex(sdk.SdkPreparationError, "unsafe"):
            sdk.inspect_archive(unsafe, self.config["archives"]["patch5"], "patch5")

    def test_refuses_output_inside_repository(self) -> None:
        with self.assertRaisesRegex(sdk.SdkPreparationError, "outside the repository"):
            sdk.ensure_external_output(sdk.repository_root() / "build" / "sdk")


if __name__ == "__main__":
    unittest.main()
