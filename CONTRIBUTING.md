# Contributing

Contributions must keep the repository buildable without distributing any
Nintendo binary. Do not commit ROMs, BIOS dumps, SDK files, extracted assets,
or generated decompiler output.

Matching claims require a reproducible comparison result. Record the target
image, address range, compiler identity and flags, and the command used to
verify the bytes. Keep guessed names or boundaries explicitly marked as
provisional until supported by binary or SDK evidence.

Progress data is generated. Update the underlying split, symbol, or comparison
result and let the progress tooling recalculate the dashboard.

Run `make public-audit` before every public push. The audit must cover all
non-ignored files for the initial import and all tracked files in CI.
