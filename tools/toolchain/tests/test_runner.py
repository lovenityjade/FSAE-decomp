from __future__ import annotations

import os
from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
RUNNER = ROOT / "tools" / "toolchain" / "run_codewarrior_compat.sh"


class CompatibilityRunnerTests(unittest.TestCase):
    def run_runner(self, environment: dict[str, str]) -> subprocess.CompletedProcess[str]:
        clean_environment = {
            "PATH": os.environ.get("PATH", ""),
            "HOME": os.environ.get("HOME", ""),
            **environment,
        }
        return subprocess.run(
            [str(RUNNER), "-version"],
            cwd=ROOT,
            env=clean_environment,
            text=True,
            capture_output=True,
            check=False,
        )

    def test_requires_tool_path(self) -> None:
        result = self.run_runner({})
        self.assertEqual(result.returncode, 1)
        self.assertIn("set CW_TOOL", result.stderr)

    def test_rejects_missing_proprietary_inputs_before_wine(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            result = self.run_runner(
                {
                    "CW_TOOL": str(root / "mwccarm.exe"),
                    "CW_LICENSE_FILE": str(root / "license.dat"),
                    "FAKETIME_LIB": str(root / "libfaketime.so.1"),
                }
            )
        self.assertEqual(result.returncode, 2)
        self.assertIn("tool not found", result.stderr)


if __name__ == "__main__":
    unittest.main()
