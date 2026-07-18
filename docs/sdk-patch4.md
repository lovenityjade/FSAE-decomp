# External TwlSDK 5.5 patch4 reconstruction

This project does not contain or redistribute TwlSDK. The preparer accepts two
legally obtained, hash-locked archives and writes a deterministic reconstruction
outside the repository. It refuses an output path anywhere below the repository
root.

## Inputs and diagnostic

The supported archives are:

| Archive | Files | SHA-256 |
| --- | ---: | --- |
| `TwlSDK-5_5-20100715a.zip` | 6,377 | `4df838fde941e6240abe1dd0f3aa1b4ade56f42ac235101753105973d84d64b7` |
| `TwlSDK-5_5_patch5-20111213.zip` | 202 | `983314422fc1867f15dceb4dfc98410c8c3993412866bc6d5341fb45ecac5563` |

Run a read-only diagnostic first:

```sh
python3 tools/sdk/prepare_patch4.py diagnose \
  --base-archive /private/TwlSDK-5_5-20100715a.zip \
  --patch5-archive /private/TwlSDK-5_5_patch5-20111213.zip
```

The command validates the complete archive hashes, expected roots, file counts,
uncompressed sizes, regular-file types, safe member names, unique relative paths,
and overlay classification. Modified or repacked archives are refused even when
their visible directory layout looks correct.

## Preparation

Choose a destination outside this checkout:

```sh
python3 tools/sdk/prepare_patch4.py prepare \
  --base-archive /private/TwlSDK-5_5-20100715a.zip \
  --patch5-archive /private/TwlSDK-5_5_patch5-20111213.zip \
  --output /private/twlsdk-5.5-patch4
```

The result contains `TwlSDK/` and `preparation-manifest.json`. Repeating the
command validates the complete tree digest and reports `already-prepared`
without rewriting it. An existing unexpected tree is refused; `--force` is
required to replace it atomically.

The canonical output contains 6,378 files and 375,712,225 bytes. Its structural
SHA-256 (path, size and content hash for every sorted file) is
`6e8c23a00893920743f0ddc625bf5d0e5ccc6254cf3aea4631da81844f785d7b`.
This is the reconstruction digest defined by this project, not the hash of an
official Nintendo patch4 archive.

The policy in `config/sdk-patch4.json` performs these operations:

1. Extract all 6,377 base files.
2. Classify all 202 cumulative patch files.
3. Overlay 180 files carrying fixes 1 through 4.
4. Keep the base form of 20 patch5-only files: `mb_child_TWL.srl`, the SNDEX
   source/header/demo/docs, the twelve `libsndex` archives, and the manual index.
5. Omit the two new patch5 package metadata files, for which no byte-exact
   patch4 substitute exists.
6. Rewrite both official version headers to release step `30004` and the
   patch4 date representation.
7. Replace exactly 37 little-endian release words in exactly 35 rebuilt files,
   from patch5 `35 75 05 05` to patch4 `34 75 05 05`.
8. Refuse any remaining base release word, unexpected pre-existing patch4 word,
   transform count mismatch, or final tree digest mismatch.

No CodeWarrior invocation or license handling is part of this tool.

## What is proved and what is inferred

Proved:

- Both source packages and their complete member layouts are identified by
  SHA-256.
- Nintendo's cumulative patch5 changelog explicitly enumerates patch1
  (`commondefs`), patch2 (Chinese ISBN screen), patch3 (`makelcf`, NA and banner
  restrictions), patch4 (WM channel initialization), then patch5 (SNDEX target
  restrictions and the `mb_child_TWL` debug-log correction).
- ARM9 and ARM7 in the target game independently contain release step `30004`.
- For the target `racoon` Rom component, patch5 with the release byte normalized
  to patch4 agrees byte-for-byte with the game payload outside its documented
  16-byte SDK footer. The game's ARM7 hash is
  `a96394d986dced7cbe6f3b3d2f82464b3970b82f336e3d26500419dbe380de1c`.
  Its complete 300,816-byte LTD/ARM7i payload is identical, with SHA-256
  `3703c27125294aa754fb4bc64f6f78e6cac93ae0b4eb4d72cafbcc591aaaa3ef`.

Inferred and deliberately recorded as such in every generated manifest:

- The header date encoding is derived from the official patch4 date
  `2011-07-12` and the format visible in the base and patch5 headers.
- Because fixes 1 through 4 do not list SNDEX or `mb_child_TWL`, their base forms
  are used to remove the two patch5 changes.
- Other December 2011 rebuilt libraries, tools, TEF/NEF and packaged outputs are
  treated as cumulative patch4 content after release-marker normalization when
  they are outside the documented patch5 change set.

There is no supplied official patch4 archive hash. Consequently, the final tree
digest proves this explicit deterministic policy; it must not be cited as proof
that every signed package or build metadata byte equals an unavailable official
patch4 archive. The distinction is especially important for TAD/signature data
and rebuilt TEF/NEF/library metadata.

## Tests

```sh
python3 -m unittest discover -s tools/sdk/tests -v
```

The tests create miniature legal synthetic archives. They cover cumulative
overlay, patch5-only exclusion, text and binary release markers, whole-tree
validation, repeat-run idempotence, wrong hashes, unsafe zip paths, mismatched
classification rules, and refusal to write SDK data inside the repository.
