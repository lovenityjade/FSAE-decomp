from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

PROJECT_ROOT = Path(__file__).resolve().parents[3]
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

from tools.linker.incremental import bootstrap
from tools.linker.source_provider import (
    SourceProviderError,
    SourceProviderLicenseError,
    build_plan,
    compile_sources,
    stage_candidates,
)
from tools.linker.tests.test_codewarrior_driver import write_minimal_arm_elf
from tools.linker.tests.test_incremental import IncrementalLinkFixture


class FakeCompiler:
    def __init__(self, *, license_failure: bool = False) -> None:
        self.calls: list[list[str]] = []
        self.license_failure = license_failure

    def __call__(
        self, command: tuple[str, ...] | list[str], cwd: Path
    ) -> subprocess.CompletedProcess[str]:
        call = list(command)
        self.calls.append(call)
        if self.license_failure:
            return subprocess.CompletedProcess(
                call, 1, "", "FLEXlm checkout failed (-10,32)"
            )
        output = Path(call[call.index("-o") + 1])
        if not output.is_absolute():
            output = cwd / output
        write_minimal_arm_elf(output, 1)
        return subprocess.CompletedProcess(call, 0, "compiled", "")


class SourceProviderTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.fixture = IncrementalLinkFixture(self.root)
        bootstrap(self.fixture.manifest, self.root, self.fixture.build)
        source = self.root / "src/unit.c"
        header = self.root / "include/unit.h"
        proof = self.root / "config/decomp/unit.json"
        source.parent.mkdir(parents=True)
        header.parent.mkdir(parents=True)
        proof.parent.mkdir(parents=True)
        source.write_text('#include "unit.h"\nint Unit(void) { return 1; }\n')
        header.write_text("int Unit(void);\n")
        proof.write_text(json.dumps({
            "schema_version": 1,
            "kind": "arm9-raw-recovery-block",
            "inputs": {"target_sha256": self.fixture.value["targets"][0]["sha256"]},
            "outputs": {"source": "src/unit.c", "header": "include/unit.h"},
        }))
        self.promotions = self.root / "config/linker/source-promotions.v1.json"
        self.promotions.write_text(json.dumps({
            "schema_version": 1,
            "kind": "arm9-source-promotions",
            "units": [{
                "image": "arm9",
                "unit_id": "static-u000",
                "include_dirs": ["include"],
                "defines": ["UNIT_BUILD=1"],
                "sources": [{"path": "src/unit.c", "proof": "config/decomp/unit.json"}],
            }],
        }))

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def plan(self):
        return build_plan(
            self.fixture.manifest,
            self.promotions,
            self.root,
            self.fixture.build,
        )

    def compile(
        self,
        runner: FakeCompiler | None = None,
        promotions: Path | None = None,
    ):
        return compile_sources(
            self.fixture.manifest,
            promotions or self.promotions,
            self.root,
            self.fixture.build,
            "/private/codewarrior/mwccarm",
            runner=runner or FakeCompiler(),
        )

    def candidate(self, body: bytes, region: str = "static") -> Path:
        path = (
            self.fixture.build /
            f"source-provider/candidates/arm9/{region}/u000.bin"
        )
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(body)
        return path

    def test_plan_is_deterministic_per_unit_and_redacts_private_compiler(self) -> None:
        first = self.plan()
        second = self.plan()
        self.assertEqual(first, second)
        self.assertEqual(first["unit_count"], 1)
        self.assertEqual(first["total_bytes"], 4)
        self.assertEqual(first["credited_matching_bytes"], 0)
        command = first["units"][0]["commands"][0]
        self.assertEqual(command[0], "mwccarm")
        self.assertIn("-proc", command)
        self.assertIn("arm946e", command)
        self.assertIn("src/unit.c", command)
        self.assertNotIn("/private", json.dumps(first))

    def test_compile_uses_planned_command_and_keeps_zero_credit(self) -> None:
        runner = FakeCompiler()
        report = self.compile(runner)
        self.assertEqual(report["object_count"], 1)
        self.assertEqual(report["credited_matching_bytes"], 0)
        self.assertEqual(report["status"], "compiled-unlinked-uncredited")
        self.assertEqual(runner.calls[0][0], "/private/codewarrior/mwccarm")
        self.assertNotIn("/private", json.dumps(report))

    def test_stage_exact_candidate_runs_probe_but_awards_no_credit(self) -> None:
        self.compile()
        self.candidate(b"ABCD")
        report = stage_candidates(
            self.fixture.manifest,
            self.promotions,
            self.root,
            self.fixture.build,
            self.fixture.build / "source-provider/candidates",
            self.fixture.source,
        )
        self.assertEqual(report["exact_candidate_units"], 1)
        self.assertEqual(report["credited_matching_bytes"], 0)
        self.assertEqual(report["fallback_credited_bytes"], 0)
        self.assertEqual(report["units"][0]["selected_provider"], "source")
        self.assertEqual(
            (self.fixture.source / "arm9/static/u000.bin").read_bytes(), b"ABCD"
        )
        probe_report = json.loads(
            (self.fixture.build / "probe.v1.json").read_text()
        )
        source_candidate = next(
            item for item in probe_report["candidates"]
            if item["provider"] == "source"
        )
        self.assertTrue(source_candidate["exact"])
        self.assertNotIn("credited_matching_bytes", probe_report)

    def test_stage_non_exact_candidate_remains_fallback_without_credit(self) -> None:
        self.compile()
        self.candidate(b"ABXD")
        report = stage_candidates(
            self.fixture.manifest,
            self.promotions,
            self.root,
            self.fixture.build,
            self.fixture.build / "source-provider/candidates",
            self.fixture.source,
        )
        self.assertEqual(report["exact_candidate_units"], 0)
        self.assertFalse(report["units"][0]["exact"])
        self.assertEqual(report["units"][0]["selected_provider"], "fallback")
        self.assertEqual(report["credited_matching_bytes"], 0)

    def test_license_failure_is_distinct_and_has_no_bypass(self) -> None:
        runner = FakeCompiler(license_failure=True)
        with self.assertRaisesRegex(SourceProviderLicenseError, "-10,32"):
            self.compile(runner)
        command = runner.calls[0]
        self.assertNotIn("faketime", " ".join(command).lower())
        self.assertNotIn("license", " ".join(command).lower())

    def test_stage_rejects_wrong_size_and_unmanaged_source_artifacts(self) -> None:
        self.compile()
        self.candidate(b"ABC")
        with self.assertRaisesRegex(SourceProviderError, "wrong size"):
            stage_candidates(
                self.fixture.manifest,
                self.promotions,
                self.root,
                self.fixture.build,
                self.fixture.build / "source-provider/candidates",
                self.fixture.source,
            )
        extra = self.fixture.source / "arm9/autoload/u000.bin"
        extra.parent.mkdir(parents=True, exist_ok=True)
        extra.write_bytes(b"EFGH")
        self.candidate(b"ABCD")
        with self.assertRaisesRegex(SourceProviderError, "not present in its registry"):
            stage_candidates(
                self.fixture.manifest,
                self.promotions,
                self.root,
                self.fixture.build,
                self.fixture.build / "source-provider/candidates",
                self.fixture.source,
            )

    def test_stages_cumulative_batches_and_rejects_mutation_or_injection(self) -> None:
        self.compile()
        self.candidate(b"ABCD")
        stage_candidates(
            self.fixture.manifest,
            self.promotions,
            self.root,
            self.fixture.build,
            self.fixture.build / "source-provider/candidates",
            self.fixture.source,
        )

        source = self.root / "src/autoload.c"
        header = self.root / "include/autoload.h"
        proof = self.root / "config/decomp/autoload.json"
        source.write_text('#include "autoload.h"\nint Autoload(void) { return 2; }\n')
        header.write_text("int Autoload(void);\n")
        proof.write_text(json.dumps({
            "schema_version": 1,
            "kind": "arm9-raw-recovery-block",
            "inputs": {"target_sha256": self.fixture.value["targets"][0]["sha256"]},
            "outputs": {
                "source": "src/autoload.c",
                "header": "include/autoload.h",
            },
        }))
        second_promotions = self.root / "config/linker/source-promotions-b.v1.json"
        second_promotions.write_text(json.dumps({
            "schema_version": 1,
            "kind": "arm9-source-promotions",
            "units": [{
                "image": "arm9",
                "unit_id": "autoload-u000",
                "include_dirs": ["include"],
                "sources": [{
                    "path": "src/autoload.c",
                    "proof": "config/decomp/autoload.json",
                }],
            }],
        }))
        self.compile(promotions=second_promotions)
        self.candidate(b"EFGH", "autoload")
        report = stage_candidates(
            self.fixture.manifest,
            second_promotions,
            self.root,
            self.fixture.build,
            self.fixture.build / "source-provider/candidates",
            self.fixture.source,
        )
        self.assertEqual(report["managed_artifact_count"], 2)
        self.assertEqual(
            (self.fixture.source / "arm9/static/u000.bin").read_bytes(), b"ABCD"
        )
        self.assertEqual(
            (self.fixture.source / "arm9/autoload/u000.bin").read_bytes(), b"EFGH"
        )
        probe_report = json.loads((self.fixture.build / "probe.v1.json").read_text())
        exact_sources = {
            (item["image"], item["unit_id"])
            for item in probe_report["candidates"]
            if item["provider"] == "source" and item["exact"]
        }
        self.assertEqual(
            exact_sources,
            {("arm9", "static-u000"), ("arm9", "autoload-u000")},
        )
        registry = json.loads(
            (self.fixture.build / "source-provider/staged-artifacts.v1.json").read_text()
        )
        self.assertEqual(registry["artifact_count"], 2)

        static_artifact = self.fixture.source / "arm9/static/u000.bin"
        static_artifact.write_bytes(b"ABXD")
        with self.assertRaisesRegex(SourceProviderError, "integrity changed"):
            stage_candidates(
                self.fixture.manifest,
                second_promotions,
                self.root,
                self.fixture.build,
                self.fixture.build / "source-provider/candidates",
                self.fixture.source,
            )
        static_artifact.write_bytes(b"ABCD")
        extra = self.fixture.source / "arm9/unmanaged/u000.bin"
        extra.parent.mkdir(parents=True)
        extra.write_bytes(b"IJKL")
        with self.assertRaisesRegex(SourceProviderError, "not present in its registry"):
            stage_candidates(
                self.fixture.manifest,
                second_promotions,
                self.root,
                self.fixture.build,
                self.fixture.build / "source-provider/candidates",
                self.fixture.source,
            )


if __name__ == "__main__":
    unittest.main()
