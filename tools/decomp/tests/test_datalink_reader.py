from __future__ import annotations

import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[3]


class DatalinkReaderTests(unittest.TestCase):
    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")

        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "datalink_reader_test"
            subprocess.run(
                [
                    compiler,
                    "-std=c99",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-I",
                    str(PROJECT_ROOT / "include"),
                    str(PROJECT_ROOT / "src/arm9/game/datalink_reader.c"),
                    str(Path(__file__).with_name("datalink_reader_harness.c")),
                    "-o",
                    str(executable),
                ],
                check=True,
                cwd=PROJECT_ROOT,
            )
            subprocess.run([str(executable)], check=True, cwd=PROJECT_ROOT)


if __name__ == "__main__":
    unittest.main()
