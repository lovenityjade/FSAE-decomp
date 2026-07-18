from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path
import tempfile
import unittest

from tools.decomp import apply_semantic_symbols as semantic


class Result:
    def __init__(self, returncode: int = 0):
        self.returncode = returncode


class SemanticSymbolTests(unittest.TestCase):
    def fixture(self, root: Path) -> tuple[Path, Path, Path, dict]:
        program_bytes = b"synthetic ARM9 analysis ELF fixture"
        target_hash = hashlib.sha256(b"synthetic ARM9 target fixture").hexdigest()
        program_hash = hashlib.sha256(program_bytes).hexdigest()
        program = root / "build/decomp/arm9.analysis.elf"
        program.parent.mkdir(parents=True, exist_ok=True)
        program.write_bytes(program_bytes)

        binary_map = root / "config/binary-map.json"
        binary_map.parent.mkdir(parents=True, exist_ok=True)
        binary_map.write_text(
            json.dumps(
                {
                    "executables": [
                        {"id": "arm9", "rom": {"sha256": target_hash}},
                        {"id": "arm7", "rom": {"sha256": "0" * 64}},
                    ]
                }
            ),
            encoding="utf-8",
        )
        manifest = root / "build/decomp/arm9.analysis.json"
        manifest.write_text(
            json.dumps(
                {
                    "image": "arm9",
                    "input_sha256": target_hash,
                    "output": "build/decomp/arm9.analysis.elf",
                    "output_sha256": program_hash,
                }
            ),
            encoding="utf-8",
        )
        config_data = {
            "schema_version": 1,
            "image": "arm9",
            "program": "arm9.analysis.elf",
            "program_sha256": program_hash,
            "target_image_sha256": target_hash,
            "purpose": semantic.PURPOSE,
            "symbols": [
                {
                    "address": "0x02004800",
                    "name": "ARM9_EntryPoint",
                    "expected_names": ["_start", "ARM9_EntryPoint"],
                    "confidence": "verified",
                    "rationale": "Synthetic entry-point fixture.",
                }
            ],
        }
        config = root / "config/decomp/arm9-symbols.json"
        config.parent.mkdir(parents=True, exist_ok=True)
        config.write_text(json.dumps(config_data), encoding="utf-8")
        return config, binary_map, manifest, config_data

    def test_validates_config_and_external_identity(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            config, binary_map, manifest, _ = self.fixture(root)
            value, symbols = semantic.validate_config(config, binary_map, manifest, root)
            self.assertEqual(value["image"], "arm9")
            self.assertEqual(symbols[0]["address"], 0x02004800)

    def test_rejects_unknown_fields_and_non_idempotent_expected_names(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            config, binary_map, manifest, data = self.fixture(root)
            invalid = copy.deepcopy(data)
            invalid["matching"] = True
            config.write_text(json.dumps(invalid), encoding="utf-8")
            with self.assertRaisesRegex(semantic.SemanticSymbolError, "unknown config"):
                semantic.validate_config(config, binary_map, manifest, root)

            invalid = copy.deepcopy(data)
            invalid["symbols"][0]["expected_names"] = ["_start"]
            config.write_text(json.dumps(invalid), encoding="utf-8")
            with self.assertRaisesRegex(semantic.SemanticSymbolError, "idempotence"):
                semantic.validate_config(config, binary_map, manifest, root)

    def test_allows_only_explicit_true_function_creation_guard(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            config, binary_map, manifest, data = self.fixture(root)
            data["symbols"][0]["create_if_missing"] = True
            data["symbols"][0]["expected_names"].insert(0, "NO_SYMBOL")
            config.write_text(json.dumps(data), encoding="utf-8")
            _, symbols = semantic.validate_config(config, binary_map, manifest, root)
            self.assertTrue(symbols[0]["create_if_missing"])

            data["symbols"][0]["create_if_missing"] = False
            config.write_text(json.dumps(data), encoding="utf-8")
            with self.assertRaisesRegex(semantic.SemanticSymbolError, "may only be true"):
                semantic.validate_config(config, binary_map, manifest, root)

    def test_rejects_duplicate_addresses_and_stale_program_bytes(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            config, binary_map, manifest, data = self.fixture(root)
            duplicate = copy.deepcopy(data["symbols"][0])
            duplicate["name"] = "SecondEntryPointName"
            duplicate["expected_names"] = ["SecondEntryPointName"]
            data["symbols"].append(duplicate)
            config.write_text(json.dumps(data), encoding="utf-8")
            with self.assertRaisesRegex(semantic.SemanticSymbolError, "duplicate function address"):
                semantic.validate_config(config, binary_map, manifest, root)

            config, binary_map, manifest, _ = self.fixture(root)
            (root / "build/decomp/arm9.analysis.elf").write_bytes(b"stale")
            with self.assertRaisesRegex(semantic.SemanticSymbolError, "ELF bytes"):
                semantic.validate_config(config, binary_map, manifest, root)

    def test_builds_bounded_noanalysis_headless_command(self) -> None:
        command = semantic.build_ghidra_command(
            "ghidra-headless",
            Path("project"),
            "fsae_decomp",
            "arm9.analysis.elf",
            Path("scripts"),
            Path("symbols.json"),
            Path("report.json"),
            True,
        )
        self.assertIn("-noanalysis", command)
        self.assertEqual(command[command.index("-postScript") + 1], "ApplySemanticSymbols.java")
        self.assertEqual(command[-1], "--dry-run")
        self.assertNotIn("ExportRecoveredC.java", command)

    def test_rejects_report_outside_artifact_directory(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            status = semantic.main(
                [
                    "--root",
                    str(root),
                    "apply",
                    "--report",
                    "config/decomp/arm9-symbols.json",
                ]
            )
            self.assertEqual(status, 2)
            self.assertTrue((root / "config/decomp/arm9-symbols.json").is_file())

    def test_apply_requires_fresh_consistent_report(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            (root / "build/ghidra").mkdir(parents=True)
            fake_headless = root / "ghidra-headless"
            fake_headless.write_text("fixture", encoding="utf-8")
            called = []

            def runner(command, **kwargs):
                called.append((command, kwargs))
                return Result()

            status = semantic.main(
                [
                    "--root",
                    str(root),
                    "apply",
                    "--ghidra-headless",
                    str(fake_headless),
                    "--dry-run",
                ],
                runner=runner,
            )
            self.assertEqual(status, 2)
            self.assertEqual(len(called), 1)
            self.assertEqual(called[0][1]["cwd"], root)

    def test_validates_synthetic_ghidra_report(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            config_path, _, _, config = self.fixture(root)
            report_path = root / "report.json"
            report = {
                "schema_version": 1,
                "status": "dry-run",
                "purpose": semantic.PURPOSE,
                "program": config["program"],
                "program_sha256": config["program_sha256"],
                "config_sha256": semantic.sha256_file(config_path),
                "requested": 1,
                "renamed": 1,
                "unchanged": 0,
                "entries": [{"action": "rename"}],
            }
            report_path.write_text(json.dumps(report), encoding="utf-8")
            semantic.validate_report(report_path, config_path, config, 1, True)
            report["entries"][0]["action"] = "unchanged"
            report_path.write_text(json.dumps(report), encoding="utf-8")
            with self.assertRaisesRegex(semantic.SemanticSymbolError, "actions"):
                semantic.validate_report(report_path, config_path, config, 1, True)


if __name__ == "__main__":
    unittest.main()
