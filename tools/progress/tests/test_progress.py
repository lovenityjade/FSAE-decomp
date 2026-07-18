from __future__ import annotations

import json
import hashlib
import sys
import tempfile
import threading
import unittest
import urllib.error
import urllib.request
import re
from pathlib import Path


sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import progress  # noqa: E402


def matching_metrics(units=(0, 0), functions=(0, 0), byte_count=(0, 0)):
    return {
        "units": {"matched": units[0], "total": units[1]},
        "functions": {"matched": functions[0], "total": functions[1]},
        "bytes": {"matched": byte_count[0], "total": byte_count[1]},
    }


def analysis_metrics(units=(0, 0), functions=(0, 0), byte_count=(0, 0)):
    return {
        "units": {"covered": units[0], "total": units[1]},
        "functions": {"covered": functions[0], "total": functions[1]},
        "bytes": {"covered": byte_count[0], "total": byte_count[1]},
    }


class ProgressTestCase(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        for relative in (
            "tools/progress/evidence",
            "tools/progress/workers",
            "tools/progress/changes",
            "dashboard",
        ):
            (self.root / relative).mkdir(parents=True, exist_ok=True)
        (self.root / "dashboard/index.html").write_text(
            '<link rel="stylesheet" href="/styles.css?v=__DASHBOARD_ASSET_VERSION__">\n'
            '<script src="/app.js?v=__DASHBOARD_ASSET_VERSION__"></script>\n',
            encoding="utf-8",
        )
        for name in ("app.js", "styles.css"):
            (self.root / "dashboard" / name).write_text(f"test {name}\n", encoding="utf-8")
        (self.root / "tools/progress/schema-v2.json").write_text(
            json.dumps(
                {
                    "$schema": "https://json-schema.org/draft/2020-12/schema",
                    "$defs": {name: {} for name in ("project", "evidence", "worker", "change")},
                }
            ),
            encoding="utf-8",
        )
        self.config = {
            "schema_version": 2,
            "kind": "project",
            "project": {"name": "Test", "short_name": "T"},
            "refresh_seconds": 1,
            "worker_stale_seconds": 180,
            "recent_change_limit": 5,
            "sections": [
                {
                    "id": "arm9",
                    "label": "ARM9",
                    "dimension": "section",
                    "include_in_total": True,
                    "optional": False,
                    "targets": {"units": 0, "functions": 0, "bytes": 100},
                },
                {
                    "id": "sdk",
                    "label": "SDK",
                    "dimension": "category",
                    "include_in_total": False,
                    "optional": True,
                    "targets": {"units": 0, "functions": 0, "bytes": 0},
                },
            ],
            "inputs": {
                "evidence_globs": ["tools/progress/evidence/*.json"],
                "worker_globs": ["tools/progress/workers/*.json"],
                "change_globs": ["tools/progress/changes/*.json"],
            },
            "expected_workers": [{"id": "agent", "label": "Agent"}],
        }
        self.config_path = self.root / "tools/progress/project.v2.json"
        self.write_json(self.config_path, self.config)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    @staticmethod
    def write_json(path: Path, value: dict) -> None:
        path.write_text(json.dumps(value), encoding="utf-8")

    def add_evidence(
        self,
        track: str = "matching",
        filename: str = "unit.progress.json",
        evidence_id: str = "unit",
    ) -> Path:
        path = self.root / "tools/progress/evidence" / filename
        value = {
            "schema_version": 2,
            "kind": "evidence",
            "id": evidence_id,
            "track": track,
            "section": "arm9",
            "category": "sdk",
            "metrics": (
                matching_metrics((1, 2), (2, 4), (50, 100))
                if track == "matching"
                else analysis_metrics((1, 2), (2, 4), (50, 100))
            ),
            "updated_at": progress.iso_now(),
            "worker": "agent",
            "summary": "Progress evidence",
            "provenance": "source" if track == "matching" else "fallback",
        }
        if track == "matching":
            value.update(
                {
                    "linked": True,
                    "build_id": "test-build",
                    "commit": "abcdef1234567890",
                }
            )
        self.write_json(path, value)
        return path

    def add_legacy_analysis(self) -> None:
        value = {
            "schema_version": 1,
            "kind": "evidence",
            "id": "legacy-map",
            "section": "arm9",
            "category": "sdk",
            "metrics": matching_metrics((1, 2), (2, 4), (50, 100)),
            "updated_at": progress.iso_now(),
            "worker": "agent",
            "summary": "Legacy mapping",
        }
        self.write_json(self.root / "tools/progress/evidence/legacy.json", value)

    def test_canonical_baseline_starts_at_zero_matching(self) -> None:
        value = progress.collect_progress(self.root, self.config_path)
        self.assertEqual(value["overall"]["percent"], 0.0)
        self.assertEqual(value["overall"]["metrics"]["bytes"], {"covered": 0, "total": 100})
        self.assertEqual(value["overall"]["matching"]["metrics"]["bytes"], {"matched": 0, "total": 100})
        self.assertEqual(value["overall"]["analysis"]["metrics"]["bytes"], {"covered": 0, "total": 100})
        self.assertFalse(value["sections"][1]["available"])
        self.assertEqual(value["workers"][0]["status"], "offline")
        self.assertEqual(value["issues"], [])

    def test_analysis_coverage_never_increments_matching(self) -> None:
        self.add_evidence("analysis")
        value = progress.collect_progress(self.root, self.config_path)
        by_id = {item["id"]: item for item in value["sections"]}
        self.assertEqual(value["overall"]["matching"]["percent"], 0.0)
        self.assertEqual(value["overall"]["matching"]["metrics"]["bytes"]["matched"], 0)
        self.assertEqual(value["overall"]["analysis"]["percent"], 50.0)
        self.assertEqual(value["overall"]["percent"], 50.0)
        self.assertEqual(value["overall"]["metrics"]["functions"], {"covered": 2, "total": 4})
        self.assertEqual(by_id["arm9"]["metrics"]["functions"], {"covered": 2, "total": 4})
        self.assertEqual(by_id["sdk"]["analysis"]["percent"], 50.0)
        self.assertEqual(by_id["sdk"]["matching"]["percent"], 0.0)
        self.assertTrue(by_id["sdk"]["available"])

    def test_matching_evidence_stays_on_secondary_track(self) -> None:
        self.add_evidence("matching")
        value = progress.collect_progress(self.root, self.config_path)
        self.assertEqual(value["overall"]["percent"], 0.0)
        self.assertEqual(value["overall"]["metrics"]["bytes"], {"covered": 0, "total": 100})
        self.assertEqual(value["overall"]["matching"]["percent"], 50.0)
        self.assertEqual(value["overall"]["matching"]["metrics"]["bytes"], {"matched": 50, "total": 100})
        self.assertEqual(value["overall"]["analysis"]["percent"], 0.0)
        self.assertEqual(value["sources"]["matching_linked"], 1)
        self.assertEqual(value["sources"]["matching_rejected"], 0)

    def test_unlinked_and_fallback_matching_never_receive_credit(self) -> None:
        unlinked = self.add_evidence("matching", "unlinked.json", "unlinked")
        unlinked_value = json.loads(unlinked.read_text(encoding="utf-8"))
        unlinked_value.pop("linked")
        unlinked_value.pop("build_id")
        unlinked_value.pop("commit")
        self.write_json(unlinked, unlinked_value)

        fallback = self.add_evidence("matching", "fallback.json", "fallback")
        fallback_value = json.loads(fallback.read_text(encoding="utf-8"))
        fallback_value["provenance"] = "fallback"
        self.write_json(fallback, fallback_value)

        value = progress.collect_progress(self.root, self.config_path)
        self.assertEqual(value["overall"]["matching"]["percent"], 0.0)
        self.assertEqual(
            value["overall"]["matching"]["metrics"]["bytes"]["matched"],
            0,
        )
        self.assertEqual(value["sources"]["matching_linked"], 0)
        self.assertEqual(value["sources"]["matching_rejected"], 2)
        self.assertEqual(value["overall"]["matching_rejected"], 2)

    def test_unlinked_function_only_matching_claim_is_rejected(self) -> None:
        path = self.add_evidence("matching")
        value = json.loads(path.read_text(encoding="utf-8"))
        value.pop("linked")
        value.pop("build_id")
        value.pop("commit")
        value["metrics"] = matching_metrics((0, 0), (1, 4), (0, 0))
        self.write_json(path, value)

        result = progress.collect_progress(self.root, self.config_path)
        self.assertEqual(result["overall"]["matching"]["metrics"]["functions"]["matched"], 0)
        self.assertEqual(result["sources"]["matching_rejected"], 1)
        self.assertEqual(result["overall"]["matching_rejected"], 1)

    def test_exact_proof_link_promotes_existing_unannotated_evidence(self) -> None:
        evidence = self.add_evidence("matching")
        record = json.loads(evidence.read_text(encoding="utf-8"))
        for field in ("linked", "build_id", "commit", "provenance"):
            record.pop(field)
        self.write_json(evidence, record)

        proof_path = self.root / "build/match/unit.proof.json"
        proof_path.parent.mkdir(parents=True)
        proof = {
            "dashboard_evidence": record,
            "comparison": {
                "byte_for_byte_equal": True,
                "mismatch_bytes": 0,
            },
        }
        self.write_json(proof_path, proof)
        proof_hash = hashlib.sha256(proof_path.read_bytes()).hexdigest()

        links = self.root / "tools/progress/proof-links"
        links.mkdir(parents=True)
        self.write_json(
            links / "unit.json",
            {
                "schema_version": 2,
                "kind": "proof-link",
                "id": "unit-proof-link",
                "evidence_id": "unit",
                "proof": "build/match/unit.proof.json",
                "proof_sha256": proof_hash,
                "provenance": "sdk",
                "build_id": "verified-unit",
                "commit": "unknown",
                "units": {"probe": 1, "promoted": 1, "rejected": 0},
            },
        )
        self.config["inputs"]["proof_link_globs"] = [
            "tools/progress/proof-links/*.json"
        ]
        self.write_json(self.config_path, self.config)

        result = progress.collect_progress(self.root, self.config_path)
        self.assertEqual(result["overall"]["matching"]["percent"], 50.0)
        self.assertEqual(result["sources"]["matching_linked"], 1)
        self.assertEqual(result["sources"]["proof_links"], 1)
        self.assertEqual(result["overall"]["provenance"]["sdk"], 1)
        self.assertEqual(result["overall"]["pipeline_units"]["promoted"], 1)

        proof["comparison"]["mismatch_bytes"] = 1
        self.write_json(proof_path, proof)
        rejected = progress.collect_progress(self.root, self.config_path)
        self.assertEqual(rejected["overall"]["matching"]["percent"], 0.0)
        self.assertEqual(rejected["sources"]["matching_linked"], 0)
        self.assertEqual(rejected["sources"]["matching_rejected"], 1)
        self.assertTrue(any("proof_sha256" in issue for issue in rejected["issues"]))

    def test_provenance_and_pipeline_units_are_aggregated(self) -> None:
        path = self.add_evidence("analysis")
        value = json.loads(path.read_text(encoding="utf-8"))
        value["provenance"] = "sdk"
        value["units"] = {"probe": 5, "promoted": 3, "rejected": 2}
        self.write_json(path, value)

        result = progress.collect_progress(self.root, self.config_path)
        self.assertEqual(
            result["overall"]["provenance"],
            {"source": 0, "sdk": 5, "fallback": 0, "unmatched": 0},
        )
        self.assertEqual(
            result["overall"]["pipeline_units"],
            {"probe": 5, "promoted": 3, "rejected": 2},
        )

    def test_global_rollup_is_weighted_by_all_executable_bytes(self) -> None:
        self.config["sections"].insert(
            1,
            {
                "id": "arm7",
                "label": "ARM7",
                "dimension": "section",
                "include_in_total": True,
                "optional": False,
                "targets": {"units": 0, "functions": 0, "bytes": 300},
            },
        )
        self.write_json(self.config_path, self.config)
        self.add_evidence("analysis")
        value = progress.collect_progress(self.root, self.config_path)
        self.assertEqual(value["overall"]["metrics"]["bytes"], {"covered": 50, "total": 400})
        self.assertEqual(value["overall"]["percent"], 12.5)
        self.assertEqual(value["overall"]["basis"], "bytes")

    def test_v1_evidence_is_migrated_to_analysis_only(self) -> None:
        self.add_legacy_analysis()
        value = progress.collect_progress(self.root, self.config_path)
        self.assertEqual(value["overall"]["percent"], 50.0)
        self.assertEqual(value["overall"]["analysis"]["percent"], 50.0)
        self.assertEqual(value["sources"]["legacy_inputs"], 1)
        self.assertEqual(value["recent_changes"][0]["track"], "analysis")

    def test_real_config_has_verified_executable_sizes(self) -> None:
        config = progress.read_json(progress.DEFAULT_ROOT / "tools/progress/project.v2.json")
        sizes = {
            item["id"]: item["targets"]["bytes"]
            for item in config["sections"]
            if item["dimension"] == "section"
        }
        self.assertEqual(
            sizes,
            {"arm9": 0x123600, "arm7": 0x25860, "arm9i": 0x4794, "arm7i": 0x49710},
        )
        self.assertEqual(sum(sizes.values()), 1_666_308)

    def test_invalid_and_duplicate_inputs_are_reported_without_breaking_collection(self) -> None:
        self.add_evidence("matching", "first.json", "same")
        self.add_evidence("matching", "second.json", "same")
        invalid = self.add_evidence("matching", "invalid.json", "invalid")
        value = json.loads(invalid.read_text(encoding="utf-8"))
        value["metrics"]["bytes"] = {"matched": 2, "total": 1}
        self.write_json(invalid, value)
        result = progress.collect_progress(self.root, self.config_path)
        self.assertEqual(result["sources"]["evidence"], 1)
        self.assertEqual(len(result["issues"]), 2)
        self.assertTrue(any("duplicate id" in item for item in result["issues"]))
        self.assertTrue(any("cannot exceed" in item for item in result["issues"]))

    def test_matching_track_rejects_analysis_counter(self) -> None:
        path = self.add_evidence("analysis")
        value = json.loads(path.read_text(encoding="utf-8"))
        value["track"] = "matching"
        self.write_json(path, value)
        result = progress.collect_progress(self.root, self.config_path)
        self.assertEqual(result["sources"]["evidence"], 0)
        self.assertIn("matched and total", result["issues"][0])

    def test_fresh_v1_worker_heartbeat_is_visible(self) -> None:
        self.write_json(
            self.root / "tools/progress/workers/agent.json",
            {
                "schema_version": 1,
                "kind": "worker",
                "id": "agent",
                "label": "Agent",
                "status": "working",
                "section": "arm9",
                "task": "Mapping",
                "updated_at": progress.iso_now(),
            },
        )
        value = progress.collect_progress(self.root, self.config_path)
        self.assertEqual(value["workers"][0]["status"], "working")
        self.assertFalse(value["workers"][0]["stale"])
        self.assertEqual(value["sources"]["legacy_inputs"], 1)

    def test_heartbeat_worker_refreshes_only_timestamp(self) -> None:
        path = self.root / "tools/progress/workers/agent.json"
        original = {
            "$schema": "../schema-v2.json#/$defs/worker",
            "schema_version": 2,
            "kind": "worker",
            "id": "agent",
            "label": "Agent",
            "status": "working",
            "section": "arm9",
            "task": "Long-running analysis",
            "updated_at": "2000-01-01T00:00:00Z",
        }
        self.write_json(path, original)

        result = progress.main(
            [
                "--root",
                str(self.root),
                "--config",
                "tools/progress/project.v2.json",
                "heartbeat-worker",
                "--id",
                "agent",
            ]
        )
        self.assertEqual(result, 0)
        refreshed = json.loads(path.read_text(encoding="utf-8"))
        self.assertNotEqual(refreshed["updated_at"], original["updated_at"])
        self.assertEqual(
            {key: value for key, value in refreshed.items() if key != "updated_at"},
            {key: value for key, value in original.items() if key != "updated_at"},
        )
        worker = progress.collect_progress(self.root, self.config_path)["workers"][0]
        self.assertEqual(worker["status"], "working")
        self.assertFalse(worker["stale"])

    def test_atomic_publication_records_build_and_clears_last_error(self) -> None:
        self.add_evidence("analysis")
        output = self.root / "dashboard/progress.json"
        result = progress.main(
            [
                "--root",
                str(self.root),
                "--config",
                "tools/progress/project.v2.json",
                "publish",
                "--build-id",
                "validated-block-7",
                "--commit",
                "abcdef1234567890",
            ]
        )
        self.assertEqual(result, 0)
        published = json.loads(output.read_text(encoding="utf-8"))
        self.assertEqual(published["build"]["id"], "validated-block-7")
        self.assertEqual(published["build"]["commit"], "abcdef1234567890")
        self.assertIsNotNone(published["build"]["published_at"])
        self.assertIsNone(published["build"]["last_error"])
        self.assertEqual(published["overall"]["analysis"]["percent"], 50.0)
        self.assertTrue(output.read_bytes().endswith(b"\n"))
        self.assertEqual(list(output.parent.glob(f".{output.name}.*")), [])

    def test_failed_publication_preserves_metrics_and_records_last_error(self) -> None:
        self.add_evidence("analysis")
        output = self.root / "dashboard/progress.json"
        progress.publish_dashboard_snapshot(
            self.root,
            self.config_path,
            output,
            "good-build",
            "abcdef1234567890",
        )
        before = json.loads(output.read_text(encoding="utf-8"))

        duplicate = self.add_evidence("analysis", "duplicate.json", "unit")
        self.assertTrue(duplicate.exists())
        result = progress.main(
            [
                "--root",
                str(self.root),
                "--config",
                "tools/progress/project.v2.json",
                "publish",
                "--build-id",
                "bad-build",
                "--commit",
                "1234567890abcdef",
            ]
        )
        self.assertEqual(result, 2)
        after = json.loads(output.read_text(encoding="utf-8"))
        self.assertEqual(after["overall"], before["overall"])
        self.assertEqual(after["build"]["id"], "good-build")
        self.assertIn("duplicate id", after["build"]["last_error"]["message"])
        self.assertEqual(after["build"]["last_error"]["build_id"], "bad-build")

    def test_dashboard_value_serves_last_validated_snapshot(self) -> None:
        evidence = self.add_evidence("analysis")
        output = self.root / "dashboard/progress.json"
        progress.publish_dashboard_snapshot(
            self.root,
            self.config_path,
            output,
            "validated-build",
            "abcdef1234567890",
        )

        changed = json.loads(evidence.read_text(encoding="utf-8"))
        changed["metrics"] = analysis_metrics((2, 2), (4, 4), (100, 100))
        self.write_json(evidence, changed)

        served = progress.dashboard_progress_value(
            self.root,
            self.config_path,
            self.root / "dashboard",
        )
        self.assertEqual(served["build"]["id"], "validated-build")
        self.assertEqual(served["overall"]["analysis"]["percent"], 50.0)
        self.assertEqual(
            progress.collect_progress(self.root, self.config_path)["overall"]["analysis"]["percent"],
            100.0,
        )

    def test_dashboard_value_ignores_legacy_unpublished_snapshot(self) -> None:
        self.add_evidence("analysis")
        self.write_json(
            self.root / "dashboard/progress.json",
            {"schema_version": 2, "overall": {"percent": 99}, "sections": []},
        )
        served = progress.dashboard_progress_value(
            self.root,
            self.config_path,
            self.root / "dashboard",
        )
        self.assertEqual(served["build"]["id"], "live")
        self.assertEqual(served["overall"]["analysis"]["percent"], 50.0)

    def test_invalid_config_publication_records_last_error(self) -> None:
        self.add_evidence("analysis")
        output = self.root / "dashboard/progress.json"
        progress.publish_dashboard_snapshot(
            self.root,
            self.config_path,
            output,
            "good-build",
            "abcdef1234567890",
        )
        self.config["refresh_seconds"] = 0
        self.write_json(self.config_path, self.config)

        result = progress.main(
            [
                "--root",
                str(self.root),
                "--config",
                "tools/progress/project.v2.json",
                "publish",
                "--build-id",
                "bad-config-build",
                "--commit",
                "1234567890abcdef",
            ]
        )
        self.assertEqual(result, 2)
        after = json.loads(output.read_text(encoding="utf-8"))
        self.assertEqual(after["build"]["id"], "good-build")
        self.assertIn("refresh_seconds", after["build"]["last_error"]["message"])
        self.assertEqual(after["build"]["last_error"]["build_id"], "bad-config-build")

    def test_set_evidence_refuses_unlinked_matching_claim(self) -> None:
        result = progress.main(
            [
                "--root",
                str(self.root),
                "--config",
                "tools/progress/project.v2.json",
                "set-evidence",
                "--id",
                "unsafe-match",
                "--track",
                "matching",
                "--section",
                "arm9",
                "--bytes-matched",
                "10",
                "--bytes-total",
                "100",
            ]
        )
        self.assertEqual(result, 2)
        self.assertFalse(
            (self.root / "tools/progress/evidence/unsafe-match.json").exists()
        )

    def test_server_exposes_only_dashboard_and_recalculated_api(self) -> None:
        self.add_evidence("analysis")
        matching = self.add_evidence("matching", "matching.json", "matching")
        matching_value = json.loads(matching.read_text(encoding="utf-8"))
        matching_value["metrics"] = matching_metrics((0, 2), (1, 4), (25, 100))
        self.write_json(matching, matching_value)
        server = progress.make_server(self.root, self.config_path, "127.0.0.1", 0)
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        base = f"http://127.0.0.1:{server.server_address[1]}"
        try:
            with urllib.request.urlopen(f"{base}/api/progress?refresh=1", timeout=2) as response:
                value = json.load(response)
            self.assertEqual(value["overall"]["percent"], 50.0)
            self.assertEqual(value["overall"]["analysis"]["percent"], 50.0)
            self.assertEqual(value["overall"]["matching"]["percent"], 25.0)
            self.assertEqual(
                value["dashboard_version"],
                progress.dashboard_asset_version(self.root / "dashboard"),
            )
            with urllib.request.urlopen(f"{base}/", timeout=2) as response:
                self.assertEqual(response.status, 200)
            with self.assertRaises(urllib.error.HTTPError) as context:
                urllib.request.urlopen(f"{base}/private.bin", timeout=2)
            self.assertEqual(context.exception.code, 404)
            context.exception.close()
        finally:
            server.shutdown()
            server.server_close()
            thread.join(timeout=2)

    def test_dashboard_responses_cannot_be_reused_from_browser_cache(self) -> None:
        server = progress.make_server(self.root, self.config_path, "127.0.0.1", 0)
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        base = f"http://127.0.0.1:{server.server_address[1]}"

        def assert_no_store(response) -> None:
            self.assertEqual(response.headers["Cache-Control"], progress.NO_STORE_POLICY)
            self.assertEqual(response.headers["Pragma"], "no-cache")
            self.assertEqual(response.headers["Expires"], "0")
            self.assertEqual(response.headers["Surrogate-Control"], "no-store")

        try:
            version = progress.dashboard_asset_version(self.root / "dashboard")
            with urllib.request.urlopen(f"{base}/", timeout=2) as response:
                assert_no_store(response)
                self.assertEqual(response.headers["Clear-Site-Data"], '"cache", "storage"')
                self.assertEqual(response.headers["X-Dashboard-Version"], version)
                html = response.read().decode("utf-8")
            self.assertNotIn(progress.DASHBOARD_ASSET_TOKEN, html)
            self.assertIn(f'/styles.css?v={version}', html)
            self.assertIn(f'/app.js?v={version}', html)

            for path in (
                f"/app.js?v={version}",
                f"/styles.css?v={version}",
                "/api/progress?refresh=2",
            ):
                request = urllib.request.Request(f"{base}{path}", method="HEAD")
                with urllib.request.urlopen(request, timeout=2) as response:
                    self.assertEqual(response.status, 200)
                    assert_no_store(response)
                    self.assertEqual(response.headers["X-Dashboard-Version"], version)
                    self.assertEqual(response.read(), b"")

            (self.root / "dashboard/app.js").write_text("changed app.js\n", encoding="utf-8")
            changed_version = progress.dashboard_asset_version(self.root / "dashboard")
            self.assertNotEqual(changed_version, version)
            with urllib.request.urlopen(f"{base}/", timeout=2) as response:
                self.assertEqual(response.headers["X-Dashboard-Version"], changed_version)
                changed_html = response.read().decode("utf-8")
            self.assertIn(f'/app.js?v={changed_version}', changed_html)
            with urllib.request.urlopen(f"{base}/api/progress?client_version={version}", timeout=2) as response:
                changed_value = json.load(response)
                self.assertEqual(response.headers["X-Dashboard-Version"], changed_version)
            self.assertEqual(changed_value["dashboard_version"], changed_version)
        finally:
            server.shutdown()
            server.server_close()
            thread.join(timeout=2)

    def test_dashboard_renders_linked_matching_as_primary(self) -> None:
        dashboard = progress.DEFAULT_ROOT / "dashboard"
        source = (dashboard / "app.js").read_text(encoding="utf-8")
        markup = (dashboard / "index.html").read_text(encoding="utf-8")

        self.assertRegex(
            source,
            re.compile(r'#overall-percent"\)\.textContent\s*=\s*percentFormat\.format\(data\.overall\.matching\.percent\)'),
        )
        self.assertIn(
            'renderOverallMetrics(data.overall.matching.metrics, matchingLabels, "matched")',
            source,
        )
        self.assertIn("section.matching.percent", source)
        self.assertIn("data.overall.matching.percent", source)
        self.assertIn("data.dashboard_version !== dashboardClientVersion", source)
        self.assertIn("getRegistrations", source)
        self.assertIn("caches.keys()", source)
        self.assertIn("window.location.replace", source)
        self.assertIn('id="analysis-percent"', markup)
        self.assertIn("Reconstruction byte-matching vérifiée", markup)
        self.assertIn("preuve de linkage exacte", markup)
        self.assertIn("renderPublication(data)", source)
        self.assertIn('id="build-id"', markup)
        self.assertIn('id="hero-build-id"', markup)
        self.assertIn('id="hero-build-meta"', markup)
        self.assertIn("Dernier gros bloc publié", markup)
        self.assertIn('document.querySelector("#hero-build-id")', source)
        self.assertIn('id="last-publication-error"', markup)

    def test_full_installation_validation(self) -> None:
        self.assertEqual(progress.validate_installation(self.root, str(self.config_path)), [])


if __name__ == "__main__":
    unittest.main()
