from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[3]


class PictureRecoveryTests(unittest.TestCase):
    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")

        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "picture_test"
            subprocess.run(
                [
                    compiler,
                    "-std=c99",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-I",
                    str(PROJECT_ROOT / "include"),
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/pane.c"),
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/picture.c"),
                    str(Path(__file__).with_name("picture_harness.c")),
                    "-o",
                    str(executable),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )
            subprocess.run([str(executable)], check=True, cwd=PROJECT_ROOT)


if __name__ == "__main__":
    unittest.main()
