# ARM9 acceleration workflow

The ARM9 rebuild uses a matching-first unit pipeline. Analysis coverage and
linked matching evidence are separate measurements; only the latter advances
the primary completion counter.

## Unit states

Each configured ARM9 or ARM9i interval is in exactly one state:

- `fallback`: target bytes are present only in an ignored private build object;
- `probe`: a reconstructed source or SDK object is being compared at the
  interval's target placement;
- `promoted`: the source/SDK provider has the exact target size and hash and a
  canonical link preserved all previously promoted intervals.

Fallback coverage is useful build scaffolding, not matching evidence. The
project is complete only when fallback coverage reaches zero.

## Parallel ownership

The four concurrent lanes do not overlap:

| Lane | Ownership |
| --- | --- |
| Linker/SDK agent | Link manifests, private fallback generation, SDK objects, ARM9i |
| Low ARM9 agent | `0x02004000..0x0205FFFF` |
| Middle ARM9 agent | `0x02060000..0x0209FFFF` |
| Coordinator | `0x020A0000..0x020DF4FF`, shared contracts and publication |

Workers may add source and tests within their lane. Only the coordinator edits
shared ABI headers, central manifests, matching evidence, dashboard snapshots,
or Git history.

## Block gate

A block normally closes at 16 KiB of candidate code or 32 functions. It is
accepted only after:

1. source-specific host and ARM checks pass;
2. CodeWarrior produces every candidate object;
3. probes report exact size/hash or remain fallback without credit;
4. the canonical link preserves all earlier promoted intervals;
5. progress validation and the public-material audit pass.

The accepted block, its updated dashboard snapshot, tests, and evidence form a
single Git commit and public push. See [publication.md](publication.md).
