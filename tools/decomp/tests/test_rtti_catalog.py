from __future__ import annotations

import struct
import sys
from pathlib import Path
import unittest


sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import rtti_catalog  # noqa: E402


class RttiCatalogTestCase(unittest.TestCase):
    def test_recovers_class_vtable_and_function_metadata(self) -> None:
        load = 0x02000000
        image = bytearray(0x300)
        typeinfo = load + 0x100
        name = load + 0x200
        struct.pack_into("<II", image, 0x100, 0x02000080, name)
        struct.pack_into("<IIII", image, 0x120, 0, typeinfo, load + 0x10, load + 0x20)
        image[0x200 : 0x200 + len(b"N4test5ClassE\0")] = b"N4test5ClassE\0"
        export = {
            "functions": [
                {"entry": "02000010", "name": "FUN_02000010", "body_bytes": 12},
                {"entry": "02000020", "name": "FUN_02000020", "body_bytes": 20},
            ]
        }
        report = rtti_catalog.recover_rtti(
            bytes(image), load, load, load + 0x80, export, demangle=lambda _: "test::Class"
        )
        self.assertEqual(
            report["summary"], {"classes": 1, "vtable_slots": 2, "unique_virtual_functions": 2}
        )
        item = report["classes"][0]
        self.assertEqual(item["name"], "test::Class")
        self.assertEqual(item["typeinfo_address"], typeinfo)
        self.assertEqual(item["vtable_address_point"], load + 0x128)
        self.assertEqual(item["virtual_functions"][1]["recovered_name"], "FUN_02000020")

    def test_ignores_type_name_without_typeinfo_reference(self) -> None:
        image = bytearray(0x100)
        image[0x20 : 0x20 + len(b"N4test5ClassE\0")] = b"N4test5ClassE\0"
        report = rtti_catalog.recover_rtti(
            bytes(image), 0x02000000, 0x02000000, 0x02000040, demangle=lambda x: x
        )
        self.assertEqual(report["summary"]["classes"], 0)


if __name__ == "__main__":
    unittest.main()
