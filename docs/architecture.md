# Project architecture

## Matching target

The project treats ARM9, ARM7, ARM9i, and ARM7i as four independently measured
link products. A final SRL/TAD container is not the primary matching unit:
Nintendo signatures and product-specific packaging metadata are preserved as
private reference material and are not required to measure reconstructed code.

The canonical matching hierarchy is:

1. object or translation unit;
2. executable section within one processor image;
3. complete processor image;
4. combined four-image code result.

Every aggregate percentage is weighted by byte size. Function counts are
reported separately and never substituted for byte-weighted progress.

## Private-input pipeline

Preparation operates only on copies. It verifies known input hashes, extracts
the DS Blowfish table from a personally dumped BIOS, decrypts the `0x800`-byte
secure-area payload, decrypts the two TWL modcrypt regions, and extracts the
four processor images plus NitroFS. Generated artifacts live below `build/`
and remain ignored.

## Build environments

Host-independent orchestration and analysis run on Linux. The original
CodeWarrior ARM and Nintendo TWL SDK tools run on the authorized Windows build
host under `D:\FourSwordAnniversary`. Only source, configuration, and small
reference hashes should be synchronized routinely; private SDK and target
binaries remain in the remote `private` area.

The Windows build must be reproducible from scripted environment variables and
command lines. IDE project files may be used for investigation but cannot be
the sole build path.

## Progress contract

The progress generator owns the computed metrics. At minimum it reports, for
each of ARM9, ARM7, ARM9i, and ARM7i:

- total and matched code bytes;
- total and matched data bytes when classified;
- total and matched functions;
- total and fully linked translation units;
- SDK/library code versus game code when identification is proven.

The dashboard polls generated JSON and displays the timestamp and provenance
of every measurement. A stale or invalid measurement must be visible rather
than silently presented as current.

## Source policy

Reconstructed source and original project tooling may be versioned. ROMs,
BIOS images, official SDK contents, extracted assets, decompiler dumps, and
large copied assembly corpora may not. Small facts such as addresses, hashes,
symbol names, and independently written interfaces are stored as text.
