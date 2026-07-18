# ARM9 public source provider

`tools/linker/source_provider.py` connects public C recovery work to the
incremental linker without confusing compilation coverage with byte-exact
matching evidence.

The provider has three deliberately separate states:

1. `plan` validates one public promotion batch, capped at 16 KiB, and writes
   deterministic `mwccarm` commands under `build/linker/source-provider/`.
2. `compile` invokes the configured CodeWarrior ARM compiler and verifies each
   result as a 32-bit little-endian ARM relocatable ELF. These objects remain
   unlinked and receive zero credit.
3. `stage` accepts raw unit candidates produced by a separate canonical link,
   writes them under `build/linker/source/`, and runs `incremental.py probe`.
   It does not call `compare`; even an exact probe remains uncredited until the
   normal exact-comparison gate runs later. Staged batches accumulate rather
   than replacing earlier public source units.

No compiler, license, object or linked candidate is versioned. The tool does
not change a clock, patch a binary, modify a license or otherwise bypass
CodeWarrior licensing. FLEXlm failures stop with exit code 3.

## Promotion manifest

The provider consumes the existing `config/linker/units.v1.json` partition and
a small, reviewable promotion manifest supplied with `--promotions`:

```json
{
  "schema_version": 1,
  "kind": "arm9-source-promotions",
  "units": [
    {
      "image": "arm9",
      "unit_id": "static-main-u023",
      "include_dirs": ["include"],
      "defines": [],
      "sources": [
        {
          "path": "src/arm9/game/example.c",
          "proof": "config/decomp/example.json"
        }
      ]
    }
  ]
}
```

The first checked-in promotion is
`config/linker/arm9-source-promotions.v1.json`. It selects exactly
`static-main-u023`, the 16 KiB ARM9 interval
`0x02060000..0x02064000`, from
`src/arm9/game/arm9_middle_02061818_raw.c` and its public recovery proof.
It is the safe default used by the Make targets; set
`ARM9_SOURCE_PROMOTIONS` or `FSAE_ARM9_SOURCE_PROMOTIONS` to select another
reviewed manifest explicitly.

Every source must be owned by an existing public
`arm9-raw-recovery-block` proof whose target SHA-256 matches the ARM9 target in
the unit manifest. The source, proof, and proof-owned header hashes are pinned
in the generated plan. Unknown fields, duplicate units/sources, unsafe paths,
stale proofs and batches above 16 KiB are rejected.

The command profile is fixed and deterministic:

```text
mwccarm -proc arm946e -nothumb -nopic -nopid -interworking -lang c99 ... -c SOURCE -o OBJECT
```

Include directories and defines are sorted before command generation. The
private compiler path is substituted only at execution and is never retained
in reports.

## Workflow

Prepare the ignored fallback index first, then provide a reviewed promotion
manifest:

```sh
python3 tools/linker/incremental.py bootstrap

python3 tools/linker/source_provider.py plan \
  --promotions config/linker/my-source-promotions.v1.json

MWCCARM=/private/path/mwccarm \
python3 tools/linker/source_provider.py compile \
  --promotions config/linker/my-source-promotions.v1.json
```

The compilation outputs are inputs for a later canonical ARM9 link. That link
must export each selected raw unit at:

```text
build/linker/source-provider/candidates/<image>/<region>/uNNN.bin
```

Each candidate must have the exact manifest size and may not exceed 16 KiB.
Stage and probe it with:

```sh
python3 tools/linker/source_provider.py stage \
  --promotions config/linker/my-source-promotions.v1.json
```

Successful stages atomically update the private generated registry at
`build/linker/source-provider/staged-artifacts.v1.json`. It records the path,
size and SHA-256 of every managed source unit from the current and previous
batches. Before a later batch is staged, the complete cumulative source tree
is checked against that registry and the configured unit manifest. Missing or
mutated managed files, extra files, stale metadata and symbolic links are
rejected. Consequently `probe` sees all previously staged exact source units
without allowing an unregistered artifact to enter the provider tree.

Equivalent Make entry points default to the checked-in `static-main-u023`
promotion and optionally use `ARM9_SOURCE_CANDIDATES`:

```sh
make arm9-source-plan
make arm9-source-compile MWCCARM=/private/path/mwccarm
make arm9-source-stage
```

Override `ARM9_SOURCE_PROMOTIONS` on any command to use a different reviewed
batch. The compile target still requires a valid private CodeWarrior compiler
and license; the default manifest does not weaken that boundary.

The stage report records `exact` and the provider selected by `probe`, but its
`credited_matching_bytes` and `fallback_credited_bytes` remain zero. Only a
subsequent explicit `incremental.py compare` can credit an exact source unit.
