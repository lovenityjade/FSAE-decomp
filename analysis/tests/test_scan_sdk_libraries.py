from __future__ import annotations

import unittest

from analysis.scan_sdk_libraries import runtime_mapping


class RuntimeMappingTests(unittest.TestCase):
    def test_ltd_payload_skips_file_prefix(self) -> None:
        binary_map = {
            "executables": [
                {
                    "id": "arm9i",
                    "ltd_autoload": {
                        "table_binary_offset": 0x4784,
                        "records": [
                            {"destination": 0x021870C0, "size": 0x4780},
                        ],
                    },
                }
            ]
        }
        self.assertIsNone(runtime_mapping(binary_map, "arm9i", 0, 4))
        self.assertEqual(
            runtime_mapping(binary_map, "arm9i", 4, 8),
            (0x021870C0, "ltd_autoload"),
        )
        self.assertEqual(
            runtime_mapping(binary_map, "arm9i", 36, 40),
            (0x021870E0, "ltd_autoload"),
        )

    def test_multiple_ltd_records_advance_file_and_memory_independently(self) -> None:
        binary_map = {
            "executables": [
                {
                    "id": "arm7i",
                    "ltd_autoload": {
                        "table_binary_offset": 0x34,
                        "records": [
                            {"destination": 0x03040000, "size": 0x10},
                            {"destination": 0x02F88000, "size": 0x20},
                        ],
                    },
                }
            ]
        }
        self.assertEqual(
            runtime_mapping(binary_map, "arm7i", 4, 4),
            (0x03040000, "ltd_autoload"),
        )
        self.assertEqual(
            runtime_mapping(binary_map, "arm7i", 0x14, 4),
            (0x02F88000, "ltd_autoload"),
        )


if __name__ == "__main__":
    unittest.main()
