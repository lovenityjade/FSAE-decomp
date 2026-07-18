from __future__ import annotations

import hashlib
import json
import shutil
import struct
import tempfile
import unittest
from pathlib import Path

from tools.linker.arm9i_sdk_prepare import (
    atomic_json,
    canonical_sha256,
    extract_members,
    parse_plan,
)
from tools.linker.codewarrior_driver import (
    CodeWarriorDriverError,
    load_arm9i_sdk_inputs,
    prepare_production,
    read_elf_sections,
)
from tools.linker.incremental import bootstrap, probe
from tools.linker.tests.test_incremental import IncrementalLinkFixture


def arm_elf(marker: int, *, duplicate_debug_names: bool = False) -> bytes:
    names = b"\0.shstrtab\0.marker\0"
    section_offset = 52
    names_offset = section_offset + 120
    marker_offset = names_offset + len(names)
    ident = b"\x7fELF" + bytes((1, 1, 1)) + bytes(9)
    header = ident + struct.pack(
        "<HHIIIIIHHHHHH", 1, 40, 1, 0, 0, section_offset, 0,
        52, 0, 0, 40, 3, 1,
    )
    null = bytes(40)
    names_section = struct.pack(
        "<IIIIIIIIII", 1, 3, 0, 0, names_offset, len(names), 0, 0, 1, 0,
    )
    marker_section = struct.pack(
        "<IIIIIIIIII", 11, 1, 2, 0, marker_offset, 1, 0, 0, 1, 0,
    )
    payload = header + null + names_section + marker_section + names + bytes((marker,))
    if not duplicate_debug_names:
        return payload

    duplicate_offset = len(payload)
    duplicate = struct.pack(
        "<IIIIIIIIII", 11, 1, 0, 0, duplicate_offset, 1, 0, 0, 1, 0,
    )
    header = ident + struct.pack(
        "<HHIIIIIHHHHHH", 1, 40, 1, 0, 0, section_offset, 0,
        52, 0, 0, 40, 4, 1,
    )
    names_offset = section_offset + 160
    marker_offset = names_offset + len(names)
    names_section = struct.pack(
        "<IIIIIIIIII", 1, 3, 0, 0, names_offset, len(names), 0, 0, 1, 0,
    )
    marker_section = struct.pack(
        "<IIIIIIIIII", 11, 1, 2, 0, marker_offset, 1, 0, 0, 1, 0,
    )
    duplicate_offset = marker_offset + 1
    duplicate = struct.pack(
        "<IIIIIIIIII", 11, 1, 0, 0, duplicate_offset, 1, 0, 0, 1, 0,
    )
    return (
        header + null + names_section + marker_section + duplicate
        + names + bytes((marker, marker ^ 0xFF))
    )


class Arm9iSdkDriverTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.build = self.root / "build/linker"
        self.sdk_set = self.build / "arm9i-sdk"
        self.plan_path = self.root / "config/build/arm9i.json"
        self.plan_path.parent.mkdir(parents=True)
        objects = [f"object_{index:02d}.o" for index in range(15)]
        link_order = [
            {"kind": "sdk_object", "archive": f"lib{index % 2}.a", "object": name}
            for index, name in enumerate(objects)
        ]
        value = {
            "schema_version": 1,
            "kind": "arm9i-build-plan",
            "archives": [
                {"name": "lib0.a", "path": "lib/lib0.a", "sha256": "0" * 64},
                {"name": "lib1.a", "path": "lib/lib1.a", "sha256": "1" * 64},
            ],
            "object_ranges": link_order,
        }
        self.plan_path.write_text(json.dumps(value), encoding="utf-8")
        plan = parse_plan(self.plan_path)
        payloads = {
            (entry["archive"], entry["member"]): arm_elf(
                index, duplicate_debug_names=index == 0
            )
            for index, entry in enumerate(plan["link_order"])
        }
        archives = []
        for index, (archive, members) in enumerate(plan["required"].items()):
            archives.append({
                "index": index,
                "name": archive,
                "configured_path": plan["archives"][archive]["path"],
                "sha256": plan["archives"][archive]["sha256"],
                "members": [
                    {
                        "name": member,
                        "size": len(payloads[(archive, member)]),
                        "sha256": hashlib.sha256(payloads[(archive, member)]).hexdigest(),
                    }
                    for member in members
                ],
            })
        basis = {
            "plan_sha256": plan["plan_sha256"],
            "archives": archives,
            "link_order": plan["link_order"],
        }
        inventory = {
            "schema_version": 1,
            "kind": "arm9i-sdk-member-inventory",
            "plan_sha256": plan["plan_sha256"],
            "selection_sha256": canonical_sha256(basis),
            "archive_count": 2,
            "selected_member_count": 15,
            "archives": archives,
            "link_order": plan["link_order"],
        }
        self.sdk_set.mkdir(parents=True)
        report = extract_members(self.sdk_set, inventory, payloads)
        atomic_json(self.sdk_set / "inventory.v1.json", inventory)
        atomic_json(self.sdk_set / "extraction.v1.json", report)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def test_returns_fifteen_elfs_in_plan_order_without_absolute_paths(self) -> None:
        loaded = load_arm9i_sdk_inputs(
            self.sdk_set, self.plan_path, self.root, self.build
        )
        records = loaded["records"]
        selection = loaded["selection_sha256"]
        self.assertEqual(len(records), 15)
        self.assertEqual([entry["member"] for entry in records], [
            f"object_{index:02d}.o" for index in range(15)
        ])
        self.assertTrue(selection)
        self.assertNotIn(str(self.sdk_set), json.dumps(records))
        self.assertTrue(all(entry["credited_bytes"] == 0 for entry in records))

    def test_accepts_codewarrior_duplicate_section_names(self) -> None:
        report = json.loads(
            (self.sdk_set / "extraction.v1.json").read_text(encoding="utf-8")
        )
        first = self.sdk_set / report["members"][0]["path"]
        with self.assertRaisesRegex(CodeWarriorDriverError, "duplicate ELF section"):
            read_elf_sections(first, 1)
        loaded = load_arm9i_sdk_inputs(
            self.sdk_set, self.plan_path, self.root, self.build
        )
        self.assertEqual(len(loaded["records"]), 15)

    def test_rejects_mutation_and_path_outside_build(self) -> None:
        report = json.loads((self.sdk_set / "extraction.v1.json").read_text())
        (self.sdk_set / report["members"][0]["path"]).write_bytes(b"changed")
        with self.assertRaisesRegex(CodeWarriorDriverError, "invalid ARM9i SDK set"):
            load_arm9i_sdk_inputs(
                self.sdk_set, self.plan_path, self.root, self.build
            )
        with self.assertRaisesRegex(CodeWarriorDriverError, "beneath"):
            load_arm9i_sdk_inputs(
                self.root, self.plan_path, self.root, self.build
            )

    def test_prepare_replaces_arm9i_fallback_with_exact_plan_order(self) -> None:
        fixture = IncrementalLinkFixture(self.root)
        fixture.value["targets"][0]["regions"][0]["linker"]["bss_size"] = 8
        fixture.value["targets"][1]["regions"][1]["linker"]["bss_size"] = 4
        fixture.write_manifest()
        bootstrap(fixture.manifest, fixture.root, fixture.build)
        probe(
            fixture.manifest,
            fixture.root,
            fixture.build,
            fixture.source,
            None,
        )
        compiler = shutil.which("clang")
        if compiler is None:
            self.skipTest("clang is unavailable")
        report = prepare_production(
            fixture.manifest,
            fixture.root,
            fixture.build,
            fixture.source,
            None,
            compiler,
            arm9i_sdk_set=self.sdk_set,
            arm9i_plan_path=self.plan_path,
        )
        self.assertEqual(report["arm9i_input_mode"], "extracted-sdk")
        self.assertEqual(report["arm9i_sdk_object_count"], 15)
        self.assertEqual(report["object_count"], 18)
        self.assertEqual(report["credited_matching_bytes"], 0)
        self.assertEqual(report["fallback_credited_bytes"], 0)
        sdk_records = [
            entry for entry in report["objects"]
            if entry["provider"] == "arm9i-sdk-extracted"
        ]
        expected_members = [f"object_{index:02d}.o" for index in range(15)]
        self.assertEqual([entry["member"] for entry in sdk_records], expected_members)
        self.assertTrue(all(entry["credited_bytes"] == 0 for entry in sdk_records))
        self.assertFalse(any(
            entry["image"] == "arm9i" and entry["provider"] in ("fallback", "generated-bss")
            for entry in report["objects"]
        ))

        response = (fixture.build / report["response"]).read_text(encoding="utf-8")
        response_paths = [Path(json.loads(line)) for line in response.splitlines()]
        self.assertEqual([path.name for path in response_paths[-15:]], expected_members)
        self.assertTrue(all(not path.is_absolute() for path in response_paths[-15:]))
        lsf = (fixture.build / report["lsf"]).read_text(encoding="utf-8")
        ltdmain = lsf[lsf.index("Ltdautoload LTDMAIN"):]
        positions = [ltdmain.index(member) for member in expected_members]
        self.assertEqual(positions, sorted(positions))
        self.assertNotIn(".fsae.arm9i.ltdmain", ltdmain)
        self.assertNotIn(str(self.sdk_set.resolve()), json.dumps(report))
        self.assertNotIn(str(self.sdk_set.resolve()), response)
        self.assertNotIn(str(self.sdk_set.resolve()), lsf)

    def test_rejects_integrity_valid_non_elf_member(self) -> None:
        inventory_path = self.sdk_set / "inventory.v1.json"
        extraction_path = self.sdk_set / "extraction.v1.json"
        inventory = json.loads(inventory_path.read_text(encoding="utf-8"))
        report = json.loads(extraction_path.read_text(encoding="utf-8"))
        first = report["members"][0]
        member_path = self.sdk_set / first["path"]
        payload = b"not an ELF object"
        digest = hashlib.sha256(payload).hexdigest()
        inventory_member = inventory["archives"][0]["members"][0]
        inventory_member["size"] = len(payload)
        inventory_member["sha256"] = digest
        basis = {
            "plan_sha256": inventory["plan_sha256"],
            "archives": inventory["archives"],
            "link_order": inventory["link_order"],
        }
        old_selection = inventory["selection_sha256"]
        new_selection = canonical_sha256(basis)
        inventory["selection_sha256"] = new_selection
        old_set = self.sdk_set / "sets" / old_selection
        new_set = self.sdk_set / "sets" / new_selection
        old_set.rename(new_set)
        first["size"] = len(payload)
        first["sha256"] = digest
        first["path"] = first["path"].replace(old_selection, new_selection, 1)
        report["selection_sha256"] = new_selection
        for entry in report["members"][1:]:
            entry["path"] = entry["path"].replace(old_selection, new_selection, 1)
        (self.sdk_set / first["path"]).write_bytes(payload)
        atomic_json(inventory_path, inventory)
        atomic_json(extraction_path, report)
        with self.assertRaisesRegex(CodeWarriorDriverError, "not an ARM relocatable ELF"):
            load_arm9i_sdk_inputs(
                self.sdk_set, self.plan_path, self.root, self.build
            )


if __name__ == "__main__":
    unittest.main()
