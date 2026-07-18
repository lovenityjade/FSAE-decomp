"""Optional local integration tests; skipped when proprietary inputs are absent."""

from __future__ import annotations

import argparse
from pathlib import Path
import unittest

from tools.rom import pipeline


ROOT = Path(__file__).resolve().parents[3]
ROM = ROOT / "Zelda-_Four_Swords-Anniversary_Edition-Nintendo (EUR).nds"
BIOS7 = ROOT / "bios7.bin"
TWLTOOL = Path.home() / "Downloads" / "twltool-v1.6.zip"


@unittest.skipUnless(ROM.is_file() and BIOS7.is_file() and TWLTOOL.is_file(), "local dumps absent")
class RealInputTests(unittest.TestCase):
    def test_diagnostic_matches_target_layout(self) -> None:
        with self.subTest("verified local user inputs"):
            args = argparse.Namespace(
                rom=ROM,
                bios7=BIOS7,
                twltool_zip=TWLTOOL,
                output=ROOT / "build" / "rom",
                wine=None,
            )
            result = pipeline.diagnose(args)
            self.assertEqual(result["status"], "ready")
            self.assertEqual(result["nitrofs_file_count"], 29)
            self.assertEqual(set(result["header"]["sections"]), {"arm9", "arm7", "arm9i", "arm7i"})


if __name__ == "__main__":
    unittest.main()
