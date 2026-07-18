# ARM9/ARM9i incremental linker

This directory has three deliberately separate pipelines.

`incremental.py` is a byte-level proof and selection tool. Its `compare`
command reconstructs container images from exact source/SDK units plus private
fallback bytes. It does not invoke a linker and its output is always named a
`candidate` or `reconstruction`, never a linked image. Fallback bytes are
reported separately and receive zero matching credit.

`codewarrior_driver.py` is the real production-link driver. It wraps every
selected exact input unit in a validated ARM relocatable ELF section, generates
an LSF and response file, runs the private SDK `makelcf` tool and then invokes
CodeWarrior `mwldarm`. Container regions marked `generated` in
`config/linker/units.v1.json` are not wrapped as inputs: the SDK LCF template
must generate the ARM9 autoload table and the ARM9i LTD prefix/table.

`arm9i_sdk_prepare.py` is the private-input boundary for ARM9i SDK objects. It
reads the public selection in `config/build/arm9i.json`, verifies each selected
archive by SHA-256, inventories only the required archive members and can
extract those members into a content-addressed set below `build/linker/`.
Neither the external SDK root nor unselected member names or contents are
recorded in its reports.

## Prerequisites

- User-prepared ARM9 and ARM9i target sections under `build/rom/sections/`.
- An ARM-capable Clang for generating private fallback ELF objects. Override it
  with `FSAE_ARM_OBJECT_COMPILER` or `--object-compiler`.
- The private TwlSDK tree selected by `TWLSDK_ROOT` or `--sdk-root`. The driver
  requires `include/twl/specfiles/ARM9-TS.lcf.template` below that root.
- `makelcf` selected by `MAKELCF` or `--makelcf`.
- CodeWarrior ARM linker 2.0 build 99 selected by `MWLDARM` or `--mwldarm`.
- For the public compile-and-link smoke, CodeWarrior ARM compiler 4.0 build
  1051 selected by `MWCCARM` or `--mwccarm`.
- A privately managed CodeWarrior license that permits all requested tools.
  FLEXlm diagnostics such as `Win32_Plugins_ARM` or `-10,32` stop immediately
  with exit code 3. The driver contains no license bypass or clock workaround.

Private SDKs, tools, licenses, ROM-derived units and linked products must stay
outside Git. All generated artifacts below are written under `build/linker/`.

## Workflow

Generate and validate the private unit fallback:

```sh
python3 tools/linker/incremental.py bootstrap
```

Probe exact source/SDK units. Source units default to
`build/linker/source/<image>/<region>/uNNN.bin`. SDK unit staging can be passed
with `--sdk-dir` or `FSAE_LINKER_SDK_UNITS`.

```sh
python3 tools/linker/incremental.py probe
python3 tools/linker/incremental.py compare
```

`compare.v1.json` distinguishes reconstruction equality from credited equality.
Only exact promoted source/SDK units contribute to `credited_matching_bytes`;
`fallback_credited_bytes` is always zero.

Verify the configured ARM9i archives and generate a metadata-only inventory:

```sh
python3 tools/linker/arm9i_sdk_prepare.py inventory \
  --sdk-root "$TWLSDK_ROOT"
```

Extract exactly the 15 unique members selected by the current ARM9i plan:

```sh
python3 tools/linker/arm9i_sdk_prepare.py extract \
  --sdk-root "$TWLSDK_ROOT"
```

The generated `inventory.v1.json` contains only configured archive/member
names, sizes and SHA-256 metadata. `extraction.v1.json` points to the selected
content-addressed set under `build/linker/arm9i-sdk/sets/`. The tool rejects
archive hash mismatches, absent or duplicate selected members, archive paths
escaping the SDK root and any output path resolving outside this repository's
`build/` directory. It never copies a complete archive.

Validate an existing extracted set without reading the external SDK again:

```sh
python3 tools/linker/arm9i_sdk_prepare.py validate
```

Opt in to those verified objects when preparing the production link:

```sh
python3 tools/linker/codewarrior_driver.py prepare \
  --arm9i-sdk-set build/linker/arm9i-sdk
```

This replaces only the two flat ARM9i fallback wrappers and their synthetic
BSS object with the 15 relocatable SDK members, in the exact `link_order` from
`config/build/arm9i.json`. ARM9 static/ITCM/DTCM inputs are unchanged. The
driver revalidates `inventory.v1.json`, `extraction.v1.json`, every member hash,
the exact set contents and the ARM relocatable ELF identity before compiling
any wrapper. Paths for SDK members in the LSF and response are relative to the
project, and preparation reports never contain the absolute SDK-set root.

Without `--arm9i-sdk-set`, preparation retains the original fallback-wrapper
mode. Both modes explicitly report `credited_matching_bytes: 0` and
`fallback_credited_bytes: 0`; only an exact linked output comparison may earn
matching credit.

Build and validate the production ELF input objects, LSF and response:

```sh
python3 tools/linker/codewarrior_driver.py prepare
```

Run the real production link:

```sh
python3 tools/linker/codewarrior_driver.py link \
  --sdk-root "$TWLSDK_ROOT" \
  --arm9i-sdk-set build/linker/arm9i-sdk \
  --makelcf "$MAKELCF" \
  --mwldarm "$MWLDARM"
```

The driver calls `makelcf -V4 production.lsf ARM9-TS.lcf.template`, captures
its LCF on standard output, and invokes `mwldarm` with the verified ARM946E-S,
interworking, standard-library, closure-map and `_start` flags. A successful
command must produce a valid 32-bit ARM executable ELF; a zero exit code without
the declared output is rejected.

Exercise the full compiler/makelcf/linker command chain with public fixtures:

```sh
python3 tools/linker/codewarrior_driver.py smoke \
  --sdk-root "$TWLSDK_ROOT" \
  --mwccarm "$MWCCARM" \
  --makelcf "$MAKELCF" \
  --mwldarm "$MWLDARM"
```

The synthetic tests use injected command runners and contain no SDK, ROM,
compiler or license material:

```sh
python3 -m unittest discover -s tools/linker/tests -v
```
