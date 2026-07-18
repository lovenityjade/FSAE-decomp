# Four Swords Anniversary Edition decompilation

[![Public CI](https://github.com/lovenityjade/FSAE-decomp/actions/workflows/public-ci.yml/badge.svg)](https://github.com/lovenityjade/FSAE-decomp/actions/workflows/public-ci.yml)

This repository is the working area for a complete, matching decompilation of
the European revision 0 release of *The Legend of Zelda: Four Swords
Anniversary Edition* (`KQ9V`). The long-term target is a reproducible rebuild
of all four executable images and a local dashboard that reports measured
progress globally and per section.

No ROM, BIOS dump, SDK archive, extracted game asset, or other copyrighted
Nintendo data belongs in version control. Supply legally obtained inputs in
the repository root; the preparation tools verify their hashes and place all
derived material below ignored build directories.

## Verified target

| Image | ROM offset | Load address | Size |
| --- | ---: | ---: | ---: |
| ARM9 | `0x00004000` | `0x02004000` | `0x00123600` |
| ARM7 | `0x00127600` | `0x02380000` | `0x00025860` |
| ARM9i | `0x00DDB000` | `0x02400000` | `0x00004794` |
| ARM7i | `0x00DDF800` | `0x02E80000` | `0x00049710` |

The target has no ARM9 or ARM7 overlays. Its NitroFS contains 29 files. The
ARM9 and ARM7 module parameters identify TWL SDK 5.5 patch 4; CodeWarrior ARM
4.0 build 1051 is the initial compiler candidate.

## Project status

The foundation is under construction. Preparation, binary maps, matching
configuration, and dashboard commands will be documented here as they become
verified. Progress percentages are generated from machine-readable project
data and must not be edited by hand.

## Unified commands

```sh
make help
make diagnose
make prepare
make map
make validate
make match
make progress-validate
make serve
make test
```

Remote toolchain entry points are `make windows-sync`, `make windows-check`,
and `make windows-build`. All private inputs, SDK locations, hosts, and paths
are supplied at runtime. See
[docs/build-orchestration.md](docs/build-orchestration.md) for the required
variables and strict matching-proof behavior.

## Repository boundaries

- `analysis/` contains reusable binary-analysis utilities.
- `config/` contains target metadata, sections, splits, and symbol maps.
- `dashboard/` contains the local progress website.
- `tools/rom/` prepares private inputs into ignored build artifacts.
- `tools/progress/` computes progress metrics consumed by the dashboard.
- `src/` and `include/` will contain reconstructed source code.
- `build/` is generated and ignored.

See [docs/architecture.md](docs/architecture.md) for the build and measurement
model.

Validated large blocks are checkpointed directly to the public `main` branch.
See [docs/publication.md](docs/publication.md) for the publication and private-
material gates.

The unit state machine and four-lane ownership model are documented in
[docs/acceleration-workflow.md](docs/acceleration-workflow.md).

## License

The independently written source code and tooling in this repository are
available under the [MIT License](LICENSE). Nintendo software, SDK material,
ROM images, extracted assets, and generated binary fallbacks are not part of
that license and must never be committed.
