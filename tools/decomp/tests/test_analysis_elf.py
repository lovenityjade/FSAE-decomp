from __future__ import annotations

import json
from pathlib import Path
import struct
import tempfile
import unittest

from tools.decomp.analysis_elf import AnalysisElfError, create_analysis_elf, sha256


class AnalysisElfTests(unittest.TestCase):
    def fixture(self, root: Path) -> tuple[Path, Path, Path]:
        image = bytes(range(96))
        image_path = root / "arm9.bin"
        image_path.write_bytes(image)
        binary_map = {
            "executables": [
                {
                    "id": "arm9",
                    "rom": {"size": len(image), "sha256": sha256(image)},
                    "memory": {"load_address": 0x02000000, "entry_address": 0x02000008},
                    "entry_isa": "ARM",
                    "module_params": {
                        "autoload_data_start": 0x02000040,
                        "autoload_list_start": 0x02000050,
                        "autoload_records": [
                            {"destination": 0x01FF8000, "size": 0x10},
                        ],
                    },
                }
            ]
        }
        map_path = root / "map.json"
        map_path.write_text(json.dumps(binary_map), encoding="utf-8")
        signatures = {
            "image": "arm9",
            "image_sha256": sha256(image),
            "matches": [
                {
                    "name": "SDK_Arm",
                    "binary_offset": 0x10,
                    "address": 0x02000010,
                    "size": 8,
                    "isa": "ARM",
                },
                {
                    "name": "SDK_Thumb",
                    "binary_offset": 0x44,
                    "address": 0x01FF8004,
                    "size": 6,
                    "isa": "Thumb",
                },
            ],
        }
        signature_path = root / "signatures.json"
        signature_path.write_text(json.dumps(signatures), encoding="utf-8")
        return image_path, map_path, signature_path

    def test_emits_runtime_segments_symbols_and_reproducible_bytes(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            image, binary_map, signatures = self.fixture(root)
            first = root / "first.elf"
            second = root / "second.elf"
            report = create_analysis_elf(image, binary_map, "arm9", first, signatures)
            create_analysis_elf(image, binary_map, "arm9", second, signatures)
            self.assertEqual(first.read_bytes(), second.read_bytes())
            self.assertEqual(report["symbols"], 3)

            data = first.read_bytes()
            self.assertEqual(data[:4], b"\x7fELF")
            fields = struct.unpack_from("<HHIIIIIHHHHHH", data, 16)
            self.assertEqual(fields[1], 40)  # EM_ARM
            self.assertEqual(fields[3], 0x02000008)
            phoff, phentsize, phnum = fields[4], fields[8], fields[9]
            self.assertEqual(phnum, 2)
            first_ph = struct.unpack_from("<IIIIIIII", data, phoff)
            second_ph = struct.unpack_from("<IIIIIIII", data, phoff + phentsize)
            self.assertEqual((first_ph[2], first_ph[4]), (0x02000000, 0x40))
            self.assertEqual((second_ph[2], second_ph[4]), (0x01FF8000, 0x10))
            self.assertIn(b"SDK_Thumb\0", data)

    def test_rejects_wrong_input_hash(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            image, binary_map, signatures = self.fixture(root)
            image.write_bytes(b"x" * 96)
            with self.assertRaisesRegex(AnalysisElfError, "SHA-256"):
                create_analysis_elf(image, binary_map, "arm9", root / "bad.elf", signatures)

    def test_rejects_autoload_gap(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            image, binary_map, signatures = self.fixture(root)
            value = json.loads(binary_map.read_text(encoding="utf-8"))
            value["executables"][0]["module_params"]["autoload_list_start"] += 4
            binary_map.write_text(json.dumps(value), encoding="utf-8")
            with self.assertRaisesRegex(AnalysisElfError, "payload ends"):
                create_analysis_elf(image, binary_map, "arm9", root / "bad.elf", signatures)

    def test_rejects_stale_runtime_symbol_address(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            image, binary_map, signatures = self.fixture(root)
            value = json.loads(signatures.read_text(encoding="utf-8"))
            value["matches"][1]["address"] += 4
            signatures.write_text(json.dumps(value), encoding="utf-8")
            with self.assertRaisesRegex(AnalysisElfError, "expected"):
                create_analysis_elf(image, binary_map, "arm9", root / "bad.elf", signatures)


if __name__ == "__main__":
    unittest.main()
