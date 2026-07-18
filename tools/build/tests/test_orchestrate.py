from __future__ import annotations

import hashlib
import io
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from types import SimpleNamespace
from unittest import mock


sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import orchestrate  # noqa: E402


PROJECT_ROOT = Path(__file__).resolve().parents[3]


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


class FakeRunner:
    def __init__(self, statuses=(0,)) -> None:
        self.statuses = list(statuses)
        self.commands: list[list[str]] = []

    def __call__(self, command, **kwargs):
        self.commands.append(list(command))
        status = self.statuses.pop(0) if self.statuses else 0
        return SimpleNamespace(returncode=status)


class OrchestrationTestCase(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    @staticmethod
    def write_json(path: Path, value: dict) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(value), encoding="utf-8")

    def write_progress_config(self) -> None:
        sections = []
        for section, size in orchestrate.CANONICAL_SIZES.items():
            sections.append(
                {
                    "id": section,
                    "dimension": "section",
                    "targets": {"bytes": size},
                }
            )
        self.write_json(self.root / "tools/progress/project.v2.json", {"sections": sections})

    def write_map_fixture(self) -> tuple[Path, Path, Path]:
        self.write_progress_config()
        decrypted_data = b"synthetic decrypted ROM"
        source_sha = digest(decrypted_data)
        decrypted_path = self.root / "build/rom/decrypted.nds"
        decrypted_path.parent.mkdir(parents=True, exist_ok=True)
        decrypted_path.write_bytes(decrypted_data)
        executables = []
        manifest_sections = {}
        for index, (section, size) in enumerate(orchestrate.CANONICAL_SIZES.items()):
            data = bytes([index + 1]) * size
            section_sha = digest(data)
            section_path = self.root / "build/rom/sections" / f"{section}.bin"
            section_path.parent.mkdir(parents=True, exist_ok=True)
            section_path.write_bytes(data)
            executables.append({"id": section, "rom": {"size": size, "sha256": section_sha}})
            manifest_sections[section] = {
                "path": f"sections/{section}.bin",
                "size": size,
                "sha256": section_sha,
            }
        nitro_data = b"synthetic nitro data"
        nitro_sha = digest(nitro_data)
        nitro_path = self.root / "build/rom/nitrofs/example.bin"
        nitro_path.parent.mkdir(parents=True, exist_ok=True)
        nitro_path.write_bytes(nitro_data)
        binary_map = {
            "source": {"sha256": source_sha, "size": len(decrypted_data)},
            "executables": executables,
            "filesystem": {"file_count": 1},
        }
        nitrofs_map = {
            "file_count": 1,
            "files": [
                {
                    "id": 0,
                    "path": "example.bin",
                    "rom": {"size": len(nitro_data)},
                    "sha256": nitro_sha,
                }
            ],
        }
        manifest = {
            "derived": {
                "decrypted_rom": {
                    "path": "decrypted.nds",
                    "sha256": source_sha,
                    "size": len(decrypted_data),
                },
                "sections": manifest_sections,
                "nitrofs": [
                    {
                        "path": "example.bin",
                        "output": "nitrofs/example.bin",
                        "size": len(nitro_data),
                        "sha256": nitro_sha,
                    }
                ],
            }
        }
        binary_path = self.root / "config/binary-map.json"
        nitro_map_path = self.root / "config/nitrofs.json"
        manifest_path = self.root / "build/rom/manifest.json"
        self.write_json(binary_path, binary_map)
        self.write_json(nitro_map_path, nitrofs_map)
        self.write_json(manifest_path, manifest)
        return binary_path, nitro_map_path, manifest_path

    def write_match_fixture(self, *, valid: bool = True) -> tuple[Path, Path]:
        section = "arm7"
        size = orchestrate.CANONICAL_SIZES[section]
        data = b"A" * size
        sha = digest(data)
        sdk_root = self.root / "external-sdk"
        reference_path = sdk_root / "components/reference.bin"
        target_path = self.root / "build/rom/sections/arm7.bin"
        artifact_path = self.root / "build/match/arm7.bin"
        for path in (reference_path, target_path, artifact_path):
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
        evidence = {
            "schema_version": 2,
            "kind": "evidence",
            "id": "arm7-match-test",
            "track": "matching",
            "section": section,
            "category": "sdk",
            "metrics": {
                "units": {"matched": 1, "total": 1},
                "functions": {"matched": 0, "total": 0},
                "bytes": {"matched": size if valid else 0, "total": size},
            },
        }
        proof = {
            "kind": "binary-match-proof",
            "section": section,
            "reference": {"verified": True, "size": size, "sha256": sha},
            "target": {"verified": True, "size": size, "sha256": sha},
            "normalization": {"verified": True, "size": size, "sha256": sha},
            "comparison": {
                "byte_for_byte_equal": valid,
                "mismatch_bytes": 0 if valid else 1,
                "first_mismatch_offset": None if valid else 0,
            },
            "dashboard_evidence": evidence,
        }
        config = {
            "section": section,
            "reference": {
                "path": "components/reference.bin",
                "size": size,
                "sha256": sha,
            },
            "target": {
                "path": "build/rom/sections/arm7.bin",
                "size": size,
                "sha256": sha,
            },
            "normalized": {"size": size, "sha256": sha},
            "output": {
                "artifact": "build/match/arm7.bin",
                "proof": "build/match/arm7.proof.json",
                "progress": "build/match/arm7.progress.json",
            },
        }
        config_path = self.root / "config/build/arm7.json"
        self.write_json(config_path, config)
        self.write_json(self.root / "build/match/arm7.proof.json", proof)
        self.write_json(self.root / "build/match/arm7.progress.json", evidence)
        return sdk_root, config_path

    def test_missing_private_inputs_are_reported_before_delegation(self) -> None:
        runner = FakeRunner()
        stderr = io.StringIO()
        with mock.patch.dict(os.environ, {}, clear=True), mock.patch("sys.stderr", stderr):
            status = orchestrate.main(["--root", str(self.root), "diagnose"], runner=runner)
        self.assertEqual(status, 2)
        self.assertEqual(runner.commands, [])
        output = stderr.getvalue()
        self.assertIn("FSAE_ROM", output)
        self.assertIn("FSAE_BIOS7", output)
        self.assertIn("TWLTOOL_ZIP", output)

    def test_diagnose_delegates_only_explicit_inputs(self) -> None:
        inputs = []
        for name in ("target.nds", "bios7.bin", "twltool.zip"):
            path = self.root / name
            path.write_bytes(b"fixture")
            inputs.append(path)
        runner = FakeRunner()
        status = orchestrate.main(
            [
                "--root",
                str(self.root),
                "diagnose",
                "--rom",
                str(inputs[0]),
                "--bios7",
                str(inputs[1]),
                "--twltool-zip",
                str(inputs[2]),
            ],
            runner=runner,
        )
        self.assertEqual(status, 0)
        command = runner.commands[0]
        self.assertIn("tools/rom/pipeline.py", command[1])
        self.assertEqual(command[2], "diagnose")
        self.assertIn(str(inputs[0]), command)

    def test_map_validation_checks_every_prepared_byte_hash(self) -> None:
        binary_map, nitrofs_map, manifest = self.write_map_fixture()
        result = orchestrate.validate_map_outputs(self.root, binary_map, nitrofs_map, manifest)
        self.assertEqual(result["status"], "valid")
        self.assertEqual(result["nitrofs_file_count"], 1)
        section = self.root / "build/rom/sections/arm9.bin"
        section.write_bytes(b"tampered")
        with self.assertRaisesRegex(orchestrate.OrchestrationError, "prepared arm9 bytes"):
            orchestrate.validate_map_outputs(self.root, binary_map, nitrofs_map, manifest)

    def test_exact_match_proof_is_independently_revalidated(self) -> None:
        sdk_root, config = self.write_match_fixture(valid=True)
        result = orchestrate.validate_match_outputs(self.root, sdk_root, [config])
        self.assertEqual(result["status"], "matched")
        self.assertEqual(result["sections"]["arm7"]["matched"], 0x25860)

    def test_mismatch_proof_can_never_report_success(self) -> None:
        sdk_root, config = self.write_match_fixture(valid=False)
        with self.assertRaisesRegex(orchestrate.OrchestrationError, "exact byte equality"):
            orchestrate.validate_match_outputs(self.root, sdk_root, [config])

    def test_zero_delegate_status_without_proof_is_not_matching_success(self) -> None:
        sdk_root = self.root / "sdk"
        sdk_root.mkdir()
        config = self.root / "config/build/arm7.json"
        self.write_json(config, {})
        runner = FakeRunner((0,))
        stderr = io.StringIO()
        with mock.patch("sys.stderr", stderr):
            status = orchestrate.main(
                [
                    "--root",
                    str(self.root),
                    "match",
                    "--twlsdk-root",
                    str(sdk_root),
                    "--config",
                    str(config),
                ],
                runner=runner,
            )
        self.assertEqual(status, 2)
        self.assertIn("invalid or duplicate match section", stderr.getvalue())

    def test_nonzero_match_delegate_status_is_preserved(self) -> None:
        sdk_root = self.root / "sdk"
        sdk_root.mkdir()
        runner = FakeRunner((1,))
        status = orchestrate.main(
            [
                "--root",
                str(self.root),
                "match",
                "--twlsdk-root",
                str(sdk_root),
                "--config",
                "anything.json",
            ],
            runner=runner,
        )
        self.assertEqual(status, 1)

    def test_windows_actions_require_publicly_supplied_locations(self) -> None:
        runner = FakeRunner()
        stderr = io.StringIO()
        with mock.patch.dict(os.environ, {}, clear=True), mock.patch("sys.stderr", stderr):
            status = orchestrate.main(["--root", str(self.root), "windows-build"], runner=runner)
        self.assertEqual(status, 2)
        self.assertEqual(runner.commands, [])
        self.assertIn("--command", stderr.getvalue())

    def test_makefile_dispatches_every_public_target(self) -> None:
        targets = (
            "diagnose",
            "prepare",
            "map",
            "validate",
            "match",
            "progress-validate",
            "serve",
            "test",
            "windows-check",
            "windows-sync",
            "windows-build",
        )
        for target in targets:
            completed = subprocess.run(
                ["make", "--no-print-directory", "-n", target],
                cwd=PROJECT_ROOT,
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(completed.returncode, 0, msg=f"{target}: {completed.stderr}")
            self.assertIn(f"orchestrate.py {target}", completed.stdout)

    def test_makefile_contains_no_private_host_or_windows_path_default(self) -> None:
        makefile = (PROJECT_ROOT / "Makefile").read_text(encoding="utf-8")
        self.assertNotIn("sekailink-windows", makefile)
        self.assertNotRegex(makefile, r"[A-Za-z]:\\")

    def test_public_test_dispatch_includes_every_test_directory(self) -> None:
        discovered = {
            str(path.parent.relative_to(PROJECT_ROOT))
            for path in PROJECT_ROOT.glob("**/tests/test*.py")
        }
        self.assertEqual(set(orchestrate.TEST_SUITES), discovered)


if __name__ == "__main__":
    unittest.main()
