# ARM9 semantic names

`config/decomp/arm9-symbols.json` is the public, reviewable source for semantic
function names applied to `arm9.analysis.elf`. These names improve navigation in
the Ghidra analysis project. They are not source recovery, byte matching, or
evidence of matching progress.

Each entry is guarded by:

- the exact ARM9 target and analysis-ELF SHA-256 values;
- an exact function entry address;
- an allowlist of current names, including the desired name for idempotence;
- a confidence label and rationale.

Validate the public data and all external identity checks without opening Ghidra:

```sh
python3 tools/decomp/apply_semantic_symbols.py validate
```

Check the real Ghidra project without changing it:

```sh
python3 tools/decomp/apply_semantic_symbols.py apply --dry-run
```

Apply the guarded names transactionally:

```sh
python3 tools/decomp/apply_semantic_symbols.py apply
```

The wrapper processes only `arm9.analysis.elf`, disables analysis, invokes only
`ApplySemanticSymbols.java`, and validates a fresh JSON report. The Java script
resolves every address and verifies every current name before starting a
transaction. Any rename failure rolls the complete transaction back.

Rare source- and vtable-proven entries that the initial Ghidra pass left as a
`LAB_*` label use an explicit `create_if_missing: true` guard. Their current
label (or the `NO_SYMBOL` sentinel) must still appear in `expected_names`.
Disassembly, function creation and naming then occur in the same transaction.
