# ROM preparation pipeline

The repository never contains a ROM, BIOS, SDK archive, extracted key table, or
TWLTool executable. Those inputs remain local and all generated artifacts live
under the ignored `build/rom/` directory.

## Required local inputs

- A personally dumped European Four Swords Anniversary Edition SRL/ROM.
- A personally dumped 16 KiB `bios7.bin` from a DS/DSi.
- The upstream `twltool-v1.6.zip` release archive.
- Wine, available as `wine` on `PATH`.

The pipeline accepts only the project's known target hashes. A wrong revision,
bad dump, renamed firmware image, or modified tool fails before anything is
written. It also checks the title, game code, DSi unit code, section ranges,
FNT/FAT structure, and modcrypt alignment.

Current verified inputs:

| Input | Size | SHA-256 |
| --- | ---: | --- |
| EUR ROM (`KQ9V`) | 14,848,000 | `3a880dce73ace38f923eb2c3f3e497ca00749c7349ab14c99ad88710c45a8be8` |
| `bios7.bin` | 16,384 | `ba65f690eb04ec92db67c0e299e21ad71de087d6d5de8a9cb17a62eaab563c17` |
| TWLTool v1.6 zip | 953,171 | `a242eba6b10b5a7527a2df88a00c4029d65ebc69bbfb80045a73311e0071296b` |

## Diagnose without writing

From the repository root:

```sh
python3 tools/rom/pipeline.py diagnose \
  --rom "Zelda-_Four_Swords-Anniversary_Edition-Nintendo (EUR).nds" \
  --bios7 bios7.bin \
  --twltool-zip "$HOME/Downloads/twltool-v1.6.zip"
```

`prepare --dry-run` performs the same checks. The JSON report shows the exact
section ranges, modcrypt ranges, NitroFS file count, tool path, and destination.

## Prepare analysis artifacts

```sh
python3 tools/rom/pipeline.py prepare \
  --rom "Zelda-_Four_Swords-Anniversary_Edition-Nintendo (EUR).nds" \
  --bios7 bios7.bin \
  --twltool-zip "$HOME/Downloads/twltool-v1.6.zip"
```

Pass `--force` only to replace a previous generated `build/rom/`. Paths can also
be changed with `--output`; `TWLTOOL_ZIP` supplies the default TWLTool path.

Preparation proceeds as follows:

1. Verify all input sizes and SHA-256 hashes.
2. Extract the 4,168-byte DS Blowfish table from `bios7.bin` offset `0x30`.
3. Copy the ROM and decrypt only its `0x4000..0x47ff` secure area. The 4 KiB DSi
   extended header is byte-for-byte preserved.
4. Extract the verified TWLTool executable into a temporary build directory and
   apply its two modcrypt regions to the copy. The original ROM is never passed
   to TWLTool, whose input mode is read/write.
5. Reject the result if any byte outside the secure-area or declared modcrypt
   ranges changed, if one of those ranges remained encrypted, or if the final
   decrypted-ROM hash differs from the known target.
6. Extract ARM9, ARM7, ARM9i, ARM7i, and every NitroFS file.
7. Record hashes, offsets, load addresses, and entry points in `manifest.json`.

The final layout is:

```text
build/rom/
  decrypted.nds
  manifest.json
  twltool.log
  keys/nds-blowfish.bin
  sections/{arm9,arm7,arm9i,arm7i}.bin
  nitrofs/...
```

Every file in this tree is derived from local copyrighted inputs and must remain
ignored. The original ROM and BIOS are read only. Output is first assembled in a
temporary sibling directory and moved into place only after all validation and
extraction steps succeed.

## Tests

```sh
python3 -m unittest discover -s tools/rom/tests -v
```

Unit tests use synthetic key tables and NitroFS images. An integration diagnostic
runs automatically only when the verified local ROM, BIOS, and TWLTool archive
are present; no proprietary fixture is checked in.
