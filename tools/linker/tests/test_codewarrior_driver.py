from __future__ import annotations

from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest

from tools.linker.codewarrior_driver import (
    CodeWarriorLicenseError,
    invoke_production_link,
    prepare_production,
    read_elf_sections,
    smoke_compile_link,
)
from tools.linker.incremental import bootstrap, probe
from tools.linker.tests.test_incremental import IncrementalLinkFixture


PROJECT_ROOT = Path(__file__).resolve().parents[3]


def write_minimal_arm_elf(path: Path, elf_type: int) -> None:
    names = b"\0.shstrtab\0"
    section_offset = 52
    names_offset = section_offset + 80
    ident = b"\x7fELF" + bytes((1, 1, 1)) + bytes(9)
    header = ident + struct.pack(
        "<HHIIIIIHHHHHH",
        elf_type,
        40,
        1,
        0,
        0,
        section_offset,
        0,
        52,
        0,
        0,
        40,
        2,
        1,
    )
    null_section = bytes(40)
    names_section = struct.pack(
        "<IIIIIIIIII",
        1,
        3,
        0,
        0,
        names_offset,
        len(names),
        0,
        0,
        1,
        0,
    )
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(header + null_section + names_section + names)


class FakeCodeWarriorRunner:
    def __init__(self, *, license_failure: bool = False) -> None:
        self.calls: list[list[str]] = []
        self.license_failure = license_failure

    def __call__(
        self,
        command: tuple[str, ...] | list[str],
        cwd: Path,
    ) -> subprocess.CompletedProcess[str]:
        del cwd
        call = list(command)
        self.calls.append(call)
        executable = Path(call[0]).name.lower()
        if executable == "mwccarm":
            output = Path(call[call.index("-o") + 1])
            write_minimal_arm_elf(output, 1)
            return subprocess.CompletedProcess(call, 0, "compiled", "")
        if executable == "makelcf":
            return subprocess.CompletedProcess(
                call,
                0,
                "MEMORY\n{\n}\nSECTIONS\n{\n}\n",
                "",
            )
        if executable == "mwldarm":
            if self.license_failure:
                return subprocess.CompletedProcess(
                    call,
                    1,
                    "",
                    "FLEXlm checkout failed for Win32_Plugins_ARM (-10,32)",
                )
            output = Path(call[call.index("-o") + 1])
            write_minimal_arm_elf(output, 2)
            return subprocess.CompletedProcess(call, 0, "linked", "")
        raise AssertionError(f"unexpected fake command: {call}")


class CodeWarriorDriverTests(unittest.TestCase):
    def prepare_fixture(
        self,
        temporary: str,
    ) -> tuple[IncrementalLinkFixture, Path, str]:
        fixture = IncrementalLinkFixture(Path(temporary))
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
        return fixture, fixture.root / "private-sdk", compiler

    def install_template(self, sdk_root: Path) -> Path:
        template = sdk_root / "include/twl/specfiles/ARM9-TS.lcf.template"
        template.parent.mkdir(parents=True, exist_ok=True)
        template.write_text("private template fixture\n", encoding="utf-8")
        return template

    def test_prepare_builds_named_arm_objects_and_production_placements(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            fixture, _, compiler = self.prepare_fixture(temporary)
            report = prepare_production(
                fixture.manifest,
                fixture.root,
                fixture.build,
                fixture.source,
                None,
                compiler,
            )
            self.assertEqual(report["object_count"], 5)
            lsf = (fixture.build / "codewarrior/production.lsf").read_text(
                encoding="utf-8"
            )
            response = (
                fixture.build / "codewarrior/production.response"
            ).read_text(encoding="utf-8")
            self.assertIn("Static main", lsf)
            self.assertIn("Address 0x02004000", lsf)
            self.assertIn("Autoload ITCM", lsf)
            self.assertIn("Address 0x01ff8000", lsf)
            self.assertIn("Ltdautoload LTDMAIN", lsf)
            self.assertIn("After main", lsf)
            self.assertNotIn("container-prefix", lsf)
            self.assertIn(".fsae.arm9.static.u000", lsf)
            self.assertIn(".fsae.arm9.static.bss", lsf)
            self.assertLess(lsf.index("Static main"), lsf.index("Autoload ITCM"))
            self.assertLess(
                lsf.index("Autoload ITCM"), lsf.index("Ltdautoload LTDMAIN")
            )
            self.assertEqual(len(response.strip().splitlines()), 5)
            first = fixture.build / report["objects"][0]["object"]
            sections = read_elf_sections(first, 1)
            self.assertIn(report["objects"][0]["section"], sections)
            self.assertEqual(report["objects"][0]["provider"], "fallback")

    def test_production_link_invokes_makelcf_and_mwldarm_not_concat(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            fixture, sdk_root, compiler = self.prepare_fixture(temporary)
            self.install_template(sdk_root)
            prepare_production(
                fixture.manifest,
                fixture.root,
                fixture.build,
                fixture.source,
                None,
                compiler,
            )
            runner = FakeCodeWarriorRunner()
            report = invoke_production_link(
                fixture.manifest,
                PROJECT_ROOT / "config/linker/arm9.json",
                fixture.root,
                fixture.build,
                fixture.source,
                None,
                sdk_root,
                compiler,
                "makelcf",
                "mwldarm",
                runner=runner,
            )
            self.assertEqual(report["driver"], "real-codewarrior-link")
            self.assertEqual([Path(call[0]).name for call in runner.calls], [
                "makelcf", "mwldarm"
            ])
            makelcf = runner.calls[0]
            self.assertEqual(makelcf[1], "-V4")
            self.assertTrue(makelcf[2].endswith("production.lsf"))
            self.assertTrue(makelcf[3].endswith("ARM9-TS.lcf.template"))
            link = runner.calls[1]
            self.assertIn("-proc", link)
            self.assertIn("arm946e", link)
            self.assertIn("-main", link)
            self.assertIn("_start", link)
            self.assertTrue(link[-1].startswith("@"))

    def test_smoke_runs_two_compiles_makelcf_and_link(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            sdk_root = root / "sdk"
            self.install_template(sdk_root)
            runner = FakeCodeWarriorRunner()
            report = smoke_compile_link(
                PROJECT_ROOT,
                root / "build/linker",
                sdk_root,
                "mwccarm",
                "makelcf",
                "mwldarm",
                runner=runner,
            )
            self.assertEqual(report["kind"], "arm9-codewarrior-smoke-link")
            self.assertEqual(
                [Path(call[0]).name for call in runner.calls],
                ["mwccarm", "mwccarm", "makelcf", "mwldarm"],
            )
            for command in runner.calls[:2]:
                self.assertIn("-proc", command)
                self.assertIn("arm946e", command)
                self.assertIn("-interworking", command)
                self.assertIn("-c", command)
            lsf = (
                root / "build/linker/codewarrior-smoke/smoke.lsf"
            ).read_text(encoding="utf-8")
            self.assertIn("Static main", lsf)
            self.assertIn("Autoload ITCM", lsf)
            self.assertIn("Autoload DTCM", lsf)
            self.assertIn("Ltdautoload LTDMAIN", lsf)
            self.assertNotIn("$(STATIC_OBJECT)", lsf)
            self.assertNotIn("$(LTD_OBJECT)", lsf)

    def test_license_failure_is_distinct_and_stops_link(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            sdk_root = root / "sdk"
            self.install_template(sdk_root)
            runner = FakeCodeWarriorRunner(license_failure=True)
            with self.assertRaisesRegex(CodeWarriorLicenseError, "-10,32"):
                smoke_compile_link(
                    PROJECT_ROOT,
                    root / "build/linker",
                    sdk_root,
                    "mwccarm",
                    "makelcf",
                    "mwldarm",
                    runner=runner,
                )
            self.assertFalse(
                (root / "build/linker/codewarrior-smoke/smoke.elf").exists()
            )


if __name__ == "__main__":
    unittest.main()
