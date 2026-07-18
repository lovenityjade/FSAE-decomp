# ARM9 analysis workspace

`tools/decomp/analysis_elf.py` creates an analysis-only ELF from an executable
extracted by the ROM pipeline.  It corrects a property that raw-binary imports
cannot represent: autoload bytes sit at the end of the SRL image but execute at
independent runtime addresses.

For ARM9 the generated container maps:

- the static image at `0x02004000`;
- the first autoload payload at `0x01FF8000`;
- the second autoload payload at `0x02FE0000`.

Verified SDK function boundaries are exported as ELF symbols, including the
ARM/Thumb bit.  The ELF is deterministic and hash-checked against
`config/binary-map.json`, but it is deliberately excluded from matching totals:
it is a disassembler/decompiler input, not a reconstructed game executable.

Generate the ARM9 analysis input with:

```sh
python3 tools/decomp/analysis_elf.py \
  --image arm9 \
  --input build/rom/sections/arm9.bin \
  --binary-map config/binary-map.json \
  --signatures config/sdk-signatures-arm9.json \
  --output build/decomp/arm9.analysis.elf \
  --manifest build/decomp/arm9.analysis.json
```

For ARM9i, add `--build-plan config/build/arm9i.json`; this imports the extra
SDK functions that were classified after the first unique-signature scan.  The
generator cross-checks every symbol's file offset against its runtime address,
which prevents the four-byte TWL staging prefix from silently shifting labels.

It can then be imported normally in Ghidra as `ARM:LE:32:v5t`.  The ELF entry
point is `0x02004800`, and autoload references resolve to their actual runtime
destinations instead of false addresses at the end of the static image.

The headless helper pins that processor explicitly (Ghidra otherwise guesses a
newer ARM revision from a generic ELF container):

```sh
GHIDRA_HEADLESS=/path/to/ghidra-headless \
  tools/decomp/run_ghidra_headless.sh analyze arm9
```

A bounded recovery dump can then be produced without committing decompiler
output as source.  For example, the initial ARM9 range is exported with:

```sh
tools/decomp/run_ghidra_headless.sh export arm9 02004800 02008000
```

The generated `build/decomp/arm9/recovered.c` is only a starting point.  Its
companion JSON says explicitly that it earns no matching credit until rewritten,
compiled with the target toolchain, and compared against the executable.
