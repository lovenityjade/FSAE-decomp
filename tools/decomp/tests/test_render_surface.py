from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[3]
HARNESS = Path(__file__).with_name("render_surface_harness.c")
SOURCE = PROJECT_ROOT / "src/arm9/ntmv/m2d/render_surface.c"


class RenderSurfaceRecoveryTests(unittest.TestCase):
    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")

        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "render_surface_test"
            subprocess.run(
                [
                    compiler,
                    "-std=c99",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-I",
                    str(PROJECT_ROOT / "include"),
                    str(SOURCE),
                    str(HARNESS),
                    "-o",
                    str(executable),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )
            subprocess.run([str(executable)], check=True, cwd=PROJECT_ROOT)

    def test_arm946e_s_syntax(self) -> None:
        compiler = shutil.which("clang")
        if compiler is None:
            self.skipTest("clang is unavailable")

        subprocess.run(
            [
                compiler,
                "--target=arm-none-eabi",
                "-mcpu=arm946e-s",
                "-marm",
                "-std=c99",
                "-Wall",
                "-Wextra",
                "-Werror",
                "-I",
                str(PROJECT_ROOT / "include"),
                "-fsyntax-only",
                str(SOURCE),
            ],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )


if __name__ == "__main__":
    unittest.main()
