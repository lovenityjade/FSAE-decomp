# Public checkpoints

The canonical public repository is
[`lovenityjade/FSAE-decomp`](https://github.com/lovenityjade/FSAE-decomp).
Only the coordinator publishes to `main`. Worker output is integrated, tested,
and compared locally before it becomes a public checkpoint.

## Block boundary

A normal ARM9 block closes after 16 KiB of candidate code or 32 recovered
functions, whichever occurs first. A smaller block may close only for an SDK
object boundary, an executable boundary, or an urgent correctness repair.

The block is publishable only when its host and ARM syntax checks pass, the
CodeWarrior probe completes, the canonical link preserves every previously
promoted interval, and progress collection accepts the resulting evidence.
Fallback bytes may keep a private image structurally complete, but they never
increase the matching counters.

## Publication gate

Before a direct push to `main`, the coordinator must:

1. regenerate `dashboard/progress.json` atomically from accepted evidence;
2. run the public and relevant private tests;
3. run `make public-audit`;
4. inspect the staged paths and commit one coherent block;
5. push without rewriting public history.

Commit subjects use the affected subsystem and outcome, for example
`arm9: promote lobby block`. The commit body records address ranges, matched
bytes, compiler profile, comparison proof, and test commands.

## Private boundary

ROMs, BIOS dumps, SDK archives and trees, compiler installations, licenses,
fallback objects, extracted resources, and generated linker products remain
outside Git. Public files may contain independently derived metadata such as
addresses, sizes, hashes, and symbol names.
