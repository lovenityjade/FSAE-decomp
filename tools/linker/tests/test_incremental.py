from __future__ import annotations

import json
from pathlib import Path
import tempfile
import unittest

from tools.linker.incremental import (
    IncrementalLinkError,
    bootstrap,
    compare,
    load_manifest,
    main,
    probe,
    sha256_bytes,
    target_bytes,
)


PROJECT_ROOT = Path(__file__).resolve().parents[3]


class IncrementalLinkFixture:
    def __init__(self, root: Path) -> None:
        self.root = root
        self.build = root / "build/linker"
        self.source = self.build / "source"
        self.sdk = root / "private-sdk-units"
        self.manifest = root / "config/linker/units.v1.json"
        self.arm9 = b"ABCDEFGH"
        self.arm9i = b"\0\0xy"
        (root / "build/rom/sections").mkdir(parents=True, exist_ok=True)
        (root / "config/linker").mkdir(parents=True, exist_ok=True)
        (root / "build/rom/sections/arm9.bin").write_bytes(self.arm9)
        (root / "build/rom/sections/arm9i.bin").write_bytes(self.arm9i)
        self.value = {
            "schema_version": 1,
            "kind": "arm9-incremental-link-units",
            "targets": [
                self.target(
                    "arm9",
                    self.arm9,
                    [
                        self.region(
                            "static", "static", 0, self.arm9[:4],
                            "Static", "main", 0x02004000, None,
                        ),
                        self.region(
                            "autoload", "autoload", 4, self.arm9[4:],
                            "Autoload", "ITCM", 0x01FF8000, None,
                        ),
                    ],
                ),
                self.target(
                    "arm9i",
                    self.arm9i,
                    [
                        self.region(
                            "prefix", "container-prefix", 0, self.arm9i[:2],
                            None, None, None, None,
                        ),
                        self.region(
                            "ltdmain", "ltdautoload", 2, self.arm9i[2:],
                            "Ltdautoload", "LTDMAIN", None, "main",
                        ),
                    ],
                ),
            ],
        }
        self.write_manifest()

    @staticmethod
    def region(
        region_id: str,
        kind: str,
        offset: int,
        body: bytes,
        block: str | None,
        name: str | None,
        address: int | None,
        after: str | None,
    ) -> dict[str, object]:
        mode = "input" if block is not None else "generated"
        return {
            "id": region_id,
            "kind": kind,
            "target_offset": offset,
            "size": len(body),
            "sha256": sha256_bytes(body),
            "linker": {
                "mode": mode,
                "block": block,
                "name": name,
                "address": address,
                "after": after,
                "bss_size": 0,
            },
            "unit_size": len(body),
            "unit_sha256": [sha256_bytes(body)],
        }

    @staticmethod
    def target(
        image: str,
        body: bytes,
        regions: list[dict[str, object]],
    ) -> dict[str, object]:
        return {
            "id": image,
            "path": f"build/rom/sections/{image}.bin",
            "size": len(body),
            "sha256": sha256_bytes(body),
            "regions": regions,
        }

    def write_manifest(self) -> None:
        self.manifest.write_text(
            json.dumps(self.value, indent=2) + "\n",
            encoding="utf-8",
        )

    def artifact(
        self,
        root: Path,
        image: str,
        region: str,
        body: bytes,
    ) -> Path:
        path = root / image / region / "u000.bin"
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(body)
        return path


class IncrementalLinkTests(unittest.TestCase):
    def test_checked_in_manifest_is_a_complete_real_partition(self) -> None:
        manifest = load_manifest(PROJECT_ROOT / "config/linker/units.v1.json")
        self.assertEqual([target.image for target in manifest.targets], ["arm9", "arm9i"])
        self.assertEqual(sum(len(target.units) for target in manifest.targets), 80)
        if (PROJECT_ROOT / "build/rom/sections/arm9.bin").is_file():
            images = target_bytes(manifest, PROJECT_ROOT)
            self.assertEqual(len(images["arm9"]), 1193472)
            self.assertEqual(len(images["arm9i"]), 18324)

    def test_bootstrap_probe_and_compare_promote_only_exact_candidates(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            fixture = IncrementalLinkFixture(Path(temporary))
            fallback = bootstrap(fixture.manifest, fixture.root, fixture.build)
            self.assertEqual(len(fallback["units"]), 4)
            self.assertTrue(
                (fixture.build / "fallback/arm9/static/u000.bin").is_file()
            )

            fixture.artifact(fixture.source, "arm9", "static", b"ABCD")
            fixture.artifact(fixture.source, "arm9", "autoload", b"EFGX")
            fixture.artifact(fixture.sdk, "arm9", "autoload", b"EFGH")
            report = probe(
                fixture.manifest,
                fixture.root,
                fixture.build,
                fixture.source,
                fixture.sdk,
            )
            selected = {
                (item["image"], item["unit_id"]): item["provider"]
                for item in report["selections"]
            }
            self.assertEqual(selected[("arm9", "static-u000")], "source")
            self.assertEqual(selected[("arm9", "autoload-u000")], "sdk")
            self.assertEqual(selected[("arm9i", "prefix-u000")], "fallback")
            self.assertEqual(selected[("arm9i", "ltdmain-u000")], "fallback")
            attempts = [
                (item["provider"], item["unit_id"], item["exact"])
                for item in report["candidates"]
            ]
            self.assertEqual(
                attempts,
                [
                    ("source", "static-u000", True),
                    ("source", "autoload-u000", False),
                    ("sdk", "autoload-u000", True),
                ],
            )

            comparison = compare(
                fixture.manifest,
                fixture.root,
                fixture.build,
                fixture.source,
                fixture.sdk,
            )
            self.assertEqual(comparison["status"], "exact")
            self.assertEqual(comparison["summary"]["authored_bytes"], 8)
            self.assertEqual(comparison["summary"]["authored_matching_bytes"], 8)
            self.assertEqual(comparison["summary"]["credited_matching_bytes"], 8)
            self.assertEqual(comparison["summary"]["fallback_credited_bytes"], 0)
            self.assertEqual(comparison["providers"]["fallback"]["bytes"], 4)
            self.assertEqual(comparison["summary"]["probed_bytes"], 12)
            self.assertEqual(comparison["summary"]["probed_matching_bytes"], 11)
            self.assertEqual(
                (fixture.build / "candidate/arm9.bin").read_bytes(),
                fixture.arm9,
            )
            self.assertEqual(
                (fixture.build / "candidate/arm9i.bin").read_bytes(),
                fixture.arm9i,
            )

    def test_non_exact_source_is_compared_but_falls_back_without_credit(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            fixture = IncrementalLinkFixture(Path(temporary))
            bootstrap(fixture.manifest, fixture.root, fixture.build)
            fixture.artifact(fixture.source, "arm9", "static", b"ABXD")
            report = probe(
                fixture.manifest,
                fixture.root,
                fixture.build,
                fixture.source,
                None,
            )
            static = next(
                item for item in report["selections"]
                if item["image"] == "arm9" and
                item["unit_id"] == "static-u000"
            )
            self.assertEqual(static["provider"], "fallback")
            comparison = compare(
                fixture.manifest,
                fixture.root,
                fixture.build,
                fixture.source,
                None,
            )
            self.assertEqual(comparison["summary"]["authored_bytes"], 0)
            candidate = comparison["candidates"][0]
            self.assertFalse(candidate["exact"])
            self.assertEqual(candidate["matching_bytes"], 3)

    def test_wrong_sized_candidate_is_a_hard_error(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            fixture = IncrementalLinkFixture(Path(temporary))
            bootstrap(fixture.manifest, fixture.root, fixture.build)
            fixture.artifact(fixture.source, "arm9", "static", b"ABC")
            with self.assertRaisesRegex(IncrementalLinkError, "artifact size"):
                probe(
                    fixture.manifest,
                    fixture.root,
                    fixture.build,
                    fixture.source,
                    None,
                )

    def test_compare_rejects_candidate_changed_after_probe(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            fixture = IncrementalLinkFixture(Path(temporary))
            bootstrap(fixture.manifest, fixture.root, fixture.build)
            path = fixture.artifact(
                fixture.source, "arm9", "static", b"ABXD"
            )
            probe(
                fixture.manifest,
                fixture.root,
                fixture.build,
                fixture.source,
                None,
            )
            path.write_bytes(b"ABYD")
            with self.assertRaisesRegex(IncrementalLinkError, "changed after probe"):
                compare(
                    fixture.manifest,
                    fixture.root,
                    fixture.build,
                    fixture.source,
                    None,
                )

    def test_bootstrap_rejects_stale_fallback_without_force(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            fixture = IncrementalLinkFixture(Path(temporary))
            bootstrap(fixture.manifest, fixture.root, fixture.build)
            fallback = fixture.build / "fallback/arm9/static/u000.bin"
            fallback.write_bytes(b"XXXX")
            with self.assertRaisesRegex(IncrementalLinkError, "--force"):
                bootstrap(fixture.manifest, fixture.root, fixture.build)
            bootstrap(
                fixture.manifest,
                fixture.root,
                fixture.build,
                force=True,
            )
            self.assertEqual(fallback.read_bytes(), b"ABCD")

    def test_manifest_rejects_gaps_unsafe_paths_and_unknown_keys(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            fixture = IncrementalLinkFixture(Path(temporary))
            fixture.value["targets"][0]["regions"][1]["target_offset"] = 5
            fixture.write_manifest()
            with self.assertRaisesRegex(IncrementalLinkError, "contiguous offset"):
                load_manifest(fixture.manifest)

            fixture = IncrementalLinkFixture(Path(temporary))
            fixture.value["targets"][0]["path"] = "../escape.bin"
            fixture.write_manifest()
            with self.assertRaisesRegex(IncrementalLinkError, "normalized relative"):
                load_manifest(fixture.manifest)

            fixture = IncrementalLinkFixture(Path(temporary))
            fixture.value["targets"][0]["regions"][0]["typo"] = True
            fixture.write_manifest()
            with self.assertRaisesRegex(IncrementalLinkError, "unknown keys"):
                load_manifest(fixture.manifest)

    def test_cli_returns_nonzero_on_invalid_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            fixture = IncrementalLinkFixture(Path(temporary))
            fixture.value["schema_version"] = 2
            fixture.write_manifest()
            status = main(
                [
                    "bootstrap",
                    "--manifest",
                    str(fixture.manifest),
                    "--project-root",
                    str(fixture.root),
                    "--build-dir",
                    str(fixture.build),
                ]
            )
            self.assertEqual(status, 2)


if __name__ == "__main__":
    unittest.main()
