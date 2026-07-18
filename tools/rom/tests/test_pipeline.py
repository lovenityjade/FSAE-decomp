from __future__ import annotations

import hashlib
from pathlib import Path
import struct
import tempfile
import unittest

from tools.rom import pipeline


def synthetic_table() -> bytes:
    return bytes((index * 73 + 19) & 0xFF for index in range(pipeline.BLOWFISH_SIZE))


class BlowfishTests(unittest.TestCase):
    def test_encrypt_decrypt_are_inverse(self) -> None:
        magic = list(struct.unpack("<1042I", synthetic_table()))
        original = (0x12345678, 0x9ABCDEF0)
        encrypted = pipeline._encrypt_block(magic, *original)
        self.assertEqual(pipeline._decrypt_block(magic, *encrypted), original)

    def test_secure_area_round_trip_fixture(self) -> None:
        table = synthetic_table()
        game_code = int.from_bytes(b"TEST", "little")
        plaintext = bytearray((index * 29 + 7) & 0xFF for index in range(0x800))
        struct.pack_into("<II", plaintext, 0, 0x72636E65, 0x6A624F79)

        first_magic, first_args = pipeline._init1(table, game_code)
        second_magic = first_magic.copy()
        second_args = first_args.copy()
        second_args[1] = (second_args[1] << 1) & pipeline.MASK32
        second_args[2] >>= 1
        pipeline._init2(second_magic, second_args)

        encrypted = bytearray(plaintext)
        word0, word1 = struct.unpack_from("<II", plaintext, 0)
        word1, word0 = pipeline._encrypt_block(second_magic, word1, word0)
        word1, word0 = pipeline._encrypt_block(first_magic, word1, word0)
        struct.pack_into("<II", encrypted, 0, word0, word1)
        for offset in range(8, len(encrypted), 8):
            word0, word1 = struct.unpack_from("<II", plaintext, offset)
            word1, word0 = pipeline._encrypt_block(second_magic, word1, word0)
            struct.pack_into("<II", encrypted, offset, word0, word1)

        rom = bytearray(0x5000)
        rom[0x0C:0x10] = b"TEST"
        rom[0x4000:0x4800] = encrypted
        before_header = bytes(rom[:0x1000])
        pipeline.decrypt_secure_area(rom, table)

        expected = bytearray(plaintext)
        struct.pack_into("<II", expected, 0, 0xE7FFDEFF, 0xE7FFDEFF)
        self.assertEqual(rom[0x4000:0x4800], expected)
        self.assertEqual(rom[:0x1000], before_header)

    def test_extract_table_uses_bios_offset(self) -> None:
        bios = bytes(index & 0xFF for index in range(pipeline.BIOS7_SIZE))
        self.assertEqual(
            pipeline.extract_blowfish_table(bios),
            bios[pipeline.BLOWFISH_OFFSET : pipeline.BLOWFISH_OFFSET + pipeline.BLOWFISH_SIZE],
        )


def make_nitrofs_rom() -> bytes:
    rom = bytearray(0x1000)
    fnt_offset = 0x200
    fat_offset = 0x300
    file0_offset = 0x400
    file1_offset = 0x410

    root_subtable = b"\x05a.bin" + b"\x83sub" + struct.pack("<H", 0xF001) + b"\x00"
    child_subtable = b"\x05b.bin\x00"
    root_offset = 16
    child_offset = root_offset + len(root_subtable)
    main_table = struct.pack("<IHH", root_offset, 0, 2) + struct.pack(
        "<IHH", child_offset, 1, 0xF000
    )
    fnt = main_table + root_subtable + child_subtable
    fat = struct.pack("<IIII", file0_offset, file0_offset + 3, file1_offset, file1_offset + 4)
    struct.pack_into("<IIII", rom, 0x40, fnt_offset, len(fnt), fat_offset, len(fat))
    rom[fnt_offset : fnt_offset + len(fnt)] = fnt
    rom[fat_offset : fat_offset + len(fat)] = fat
    rom[file0_offset : file0_offset + 3] = b"one"
    rom[file1_offset : file1_offset + 4] = b"two!"
    return bytes(rom)


class NitroFsTests(unittest.TestCase):
    def test_list_and_extract_nested_files(self) -> None:
        rom = make_nitrofs_rom()
        entries = pipeline.list_nitrofs(rom)
        self.assertEqual([entry["path"] for entry in entries], ["a.bin", "sub/b.bin"])
        with tempfile.TemporaryDirectory() as directory:
            extracted = pipeline.extract_nitrofs(rom, Path(directory))
            self.assertEqual((Path(directory) / "a.bin").read_bytes(), b"one")
            self.assertEqual((Path(directory) / "sub" / "b.bin").read_bytes(), b"two!")
            self.assertEqual(
                extracted[1]["sha256"], hashlib.sha256(b"two!").hexdigest()
            )

    def test_rejects_unsafe_file_name(self) -> None:
        rom = bytearray(make_nitrofs_rom())
        root_subtable_offset = struct.unpack_from("<I", rom, 0x200)[0]
        name_offset = 0x200 + root_subtable_offset + 1
        rom[name_offset : name_offset + 5] = b"../x!"
        with self.assertRaises(pipeline.PipelineError):
            pipeline.list_nitrofs(rom)


class ValidationTests(unittest.TestCase):
    def test_rejects_hash_mismatch(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "input.bin"
            path.write_bytes(b"wrong")
            with self.assertRaisesRegex(pipeline.PipelineError, "SHA-256 mismatch"):
                pipeline._verify_file(
                    path,
                    label="fixture",
                    expected_sha256="0" * 64,
                    expected_size=5,
                )

    def test_change_validator_rejects_header_mutation(self) -> None:
        original = bytes(0x6000)
        changed = bytearray(original)
        changed[0x10] = 1
        header = {"modcrypt": []}
        with self.assertRaisesRegex(pipeline.PipelineError, "offset 0x10"):
            pipeline.validate_decryption_changes(original, bytes(changed), header)

    def test_change_validator_requires_each_encrypted_region(self) -> None:
        original = bytes(0x6000)
        changed = bytearray(original)
        changed[pipeline.SECURE_AREA_OFFSET] = 1
        header = {"modcrypt": [{"index": 0, "offset": 0x5000, "size": 0x10}]}
        with self.assertRaisesRegex(pipeline.PipelineError, "modcrypt 0 was not decrypted"):
            pipeline.validate_decryption_changes(original, bytes(changed), header)


if __name__ == "__main__":
    unittest.main()
