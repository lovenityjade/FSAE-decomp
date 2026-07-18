from __future__ import annotations

import base64
from pathlib import Path
import tempfile
import unittest

from tools.windows import windows_toolchain as windows
from tools.windows import smoke_build


class SyncCommandTests(unittest.TestCase):
    def test_windows_path_converts_to_msys(self) -> None:
        self.assertEqual(
            windows.windows_to_msys_path(r"D:\FourSwordAnniversary\workspace"),
            "/d/FourSwordAnniversary/workspace",
        )

    def test_sync_command_has_required_exclusions(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            command = windows.build_rsync_command(Path(directory), dry_run=True)
        patterns = [
            command[index + 1]
            for index, item in enumerate(command[:-1])
            if item == "--exclude"
        ]
        for required in (
            "*.nds",
            "*.srl",
            "*.tad",
            "bios*.bin",
            "firmware*.bin",
            "*Nintendo*SDK*.zip",
            "/private/",
            "/toolchains/",
            "/build/",
        ):
            self.assertIn(required, patterns)
        self.assertIn("--dry-run", command)
        self.assertIn("--omit-dir-times", command)
        self.assertNotIn("--delete-delay", command)
        self.assertNotIn("--delete-excluded", command)
        self.assertEqual(
            command[-1],
            "sekailink-windows:/d/FourSwordAnniversary/workspace/",
        )

    def test_delete_stale_is_explicit(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            command = windows.build_rsync_command(Path(directory), delete=True)
        self.assertIn("--delete-delay", command)
        self.assertNotIn("--delete-excluded", command)


class RemoteCommandTests(unittest.TestCase):
    def decode_launcher(self, command: list[str]) -> str:
        index = command.index("-EncodedCommand")
        return windows.decode_powershell(command[index + 1])

    def test_check_command_constructs_encoded_launcher(self) -> None:
        command = windows.build_remote_toolchain_command(action="Check")
        launcher = self.decode_launcher(command)
        self.assertEqual(command[0], "ssh")
        self.assertIn("remote_toolchain.ps1", launcher)
        self.assertIn("Action='Check'", launcher)
        self.assertIn(r"Workspace='D:\FourSwordAnniversary\workspace'", launcher)
        self.assertNotIn("BuildCommandBase64", launcher)

    def test_build_command_is_utf8_base64_not_shell_interpolation(self) -> None:
        build = 'python tools\\build.py --name "Anniversary test" & echo done'
        command = windows.build_remote_toolchain_command(action="Build", build_command=build)
        launcher = self.decode_launcher(command)
        encoded = base64.b64encode(build.encode("utf-8")).decode("ascii")
        self.assertIn("Action='Build'", launcher)
        self.assertIn(f"BuildCommandBase64='{encoded}'", launcher)
        self.assertNotIn(build, launcher)

    def test_explicit_compiler_paths_with_spaces_are_encoded_safely(self) -> None:
        command = windows.build_remote_toolchain_command(
            action="Check",
            mwccarm=r"D:\Private Tools\mwccarm.exe",
            mwldarm=r"D:\Private Tools\mwldarm.exe",
        )
        launcher = self.decode_launcher(command)
        self.assertIn(r"Mwccarm='D:\Private Tools\mwccarm.exe'", launcher)
        self.assertIn(r"Mwldarm='D:\Private Tools\mwldarm.exe'", launcher)
        self.assertNotIn(r"D:\Private Tools", " ".join(command))

    def test_rejects_unsafe_host(self) -> None:
        with self.assertRaises(windows.WindowsToolchainError):
            windows.build_remote_toolchain_command(host="host; whoami", action="Check")

    def test_build_requires_command(self) -> None:
        with self.assertRaises(windows.WindowsToolchainError):
            windows.build_remote_toolchain_command(action="Build")


class SmokeBuildTests(unittest.TestCase):
    def test_compile_command_uses_separate_arguments(self) -> None:
        self.assertEqual(
            smoke_build.compile_command(
                r"D:\Private Tools\mwccarm.exe",
                Path(r"D:\workspace\tools\windows\tests\fixtures\smoke.c"),
                Path(r"D:\workspace\build\smoke.o"),
            ),
            [
                r"D:\Private Tools\mwccarm.exe",
                "-c",
                r"D:\workspace\tools\windows\tests\fixtures\smoke.c",
                "-o",
                r"D:\workspace\build\smoke.o",
            ],
        )


if __name__ == "__main__":
    unittest.main()
