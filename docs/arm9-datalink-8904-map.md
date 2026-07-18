# Datalink renderer at 0x020A8904

## Boundary

- Catalogued body: `0x020A8904..0x020A8D53` (`1104` bytes).
- Literal pool: `0x020A8D54..0x020A8D77` (`9` words).
- Next true function: `0x020A8D78` (`60` bytes).

The entry has its own nine-register prologue and 132-byte stack frame. The
epilogue is at `0x020A8D4C..0x020A8D53`; `0x020A8D78` begins with a distinct
prologue. No internal branch target is promoted to a function boundary.

This function has two levels of work. It first issues scene commands for each
ready slot on every call. It then reads state dirty byte `+0x43`; zero returns
immediately, while nonzero clears the byte and rebuilds the slot-row rendering
buffers. The rebuild depends on state selection `+0x40`, active participant
`+0x2C`, the participant target word reached through `+0x0C`, and the three
0x50-byte records at `0x021350B0`.

## CFG regions

| Region | Observed role |
| --- | --- |
| `0x020A8904..0x020A8933` | Prologue and copy of the 80-byte callback/render descriptor table to the stack. |
| `0x020A8934..0x020A895F` | Visit slots 0..2; ready slots issue scene commands 0, 1, or 2. |
| `0x020A8960..0x020A8973` | Dirty-byte `+0x43` gate and early return. |
| `0x020A8974..0x020A89C7` | Clear dirty byte, copy two 0x600-byte buffers, and publish a render-buffer pointer at owner `+0x934`. |
| `0x020A89C8..0x020A89EF` | Per-slot setup, clear the ten-value local descriptor array, and query readiness. |
| `0x020A89F0..0x020A8B30` | Ready-slot path: build row metadata, derive ten values from the 0x50-byte slot record, and invoke ten indirect render callbacks. |
| `0x020A8B34..0x020A8BCF` | Unavailable-slot path: emit placeholder entries for ten main cells and three auxiliary cells. |
| `0x020A8BD0..0x020A8C7F` | Selected-row and active-participant variant rendering. |
| `0x020A8C80..0x020A8CC7` | Conditional per-row auxiliary refresh and loop back for the next slot. |
| `0x020A8CC8..0x020A8D4B` | Flush and upload two 0x600-byte buffers and one 0xA80-byte buffer. |
| `0x020A8D4C..0x020A8D53` | Epilogue and return. |

## Direct dependencies

- Datalink readiness and scene commands: `0x020A7324`, `0x020AD964`.
- Buffer operations: `0x02015B94`, `0x02015D0C`, `0x020184B8`.
- Row rendering: `0x0209B394`, `0x0209B3D8`, `0x0209B4C0`,
  `0x0209B508`, and ten calls through the copied descriptor table.
- Auxiliary refresh and uploads: `0x0209CF68`, `0x0209DA7C`, `0x0209DAA0`.

## Literal pool

| Address | Value | Observed role |
| --- | --- | --- |
| `0x020A8D54` | `0x020DE7E4` | 80-byte callback/render descriptor table. |
| `0x020A8D58` | `0x0217E6F0` | Datalink scene owner. |
| `0x020A8D5C` | `0x0217A23C` | Owner of the two destination buffer objects. |
| `0x020A8D60` | `0x02180D9C` | Pointer to the first 0x600-byte source buffer. |
| `0x020A8D64` | `0x02180DA0` | Pointer to the second 0x600-byte source buffer. |
| `0x020A8D68` | `0x0217D348` | Pointer to the render owner containing field `+0x934`. |
| `0x020A8D6C` | `0x02124BB4` | Per-slot 0x34-byte row metadata array. |
| `0x020A8D70` | `0x021350B0` | Datalink player-slot array. |
| `0x020A8D74` | `0x02180DA4` | Pointer to the final 0xA80-byte upload source. |

## Recovery decision

The complete parent is recovered as `Game_UpdateDatalinkRenderer`. Its
signature preserves the sole flow-state argument at entry. A shared host-safe
binding global represents the target pointer graph, while all branch helpers
remain internal to this one true function boundary. The name is a semantic
integration candidate; no Ghidra registry change is made by this pass.

## Recovered units

`Game_CopyDatalinkRenderDescriptors` recovers the coherent 80-byte table copy
within prologue region `0x020A8904..0x020A8933`. The prologue and stack-frame
mechanics remain owned by the parent function. Each raw eight-byte descriptor
preserves its two byte parameters, zero halfword, and 32-bit callback address.

`Game_IssueReadyDatalinkSlotCommands` recovers
`0x020A8934..0x020A895F`. It queries slots zero through two in order and issues
an immediate same-index scene command for each ready slot. Both units remain
internal semantic helpers; neither is a Ghidra function or a semantic-symbol
candidate.

The next coherent region is the dirty-byte gate at
`0x020A8960..0x020A8973`.

`Game_ShouldRebuildDatalinkRender` recovers that gate as a single volatile
unsigned-byte predicate. It leaves clearing of `+0x43` to the following region
and remains an internal helper outside the Ghidra function registry.

The next coherent region is rebuild-buffer preparation at
`0x020A8974..0x020A89C7`.

`Game_PrepareDatalinkRenderBuffers` recovers that preparation region through a
host-safe buffer-binding context. It preserves dirty clear before all pointer
loads, two ordered 0x600-byte copies, post-first-copy secondary binding loads,
and the post-second-copy primary-destination reload used for publication. The
context models the target pointer graph without changing the parent function
boundary or adding a Ghidra symbol.

The next coherent operation is the per-row value clear and readiness query at
`0x020A89CC..0x020A89EF`.

`Game_PrepareDatalinkRenderRow` recovers that operation as an explicit clear of
ten 32-bit values followed by exactly one low-byte slot readiness query. It
does not absorb the parent-held buffer pointer loaded at `0x020A89C8` and
remains an internal helper outside the Ghidra function registry.

The next branches are the substantially larger ready-row path at
`0x020A89F0..0x020A8B30` and unavailable-row path at
`0x020A8B34..0x020A8BCF`.

`Game_RenderUnavailableDatalinkRow` recovers the unavailable branch as 26
ordered single-tile writes: two rows across ten main cells, then two rows
across three auxiliary cells. Slot-scaled Y coordinates, palette four, and
tile IDs `2`, `0x22`, and `0x42` are preserved. The helper remains internal
and does not create a Ghidra function boundary.

`Game_RenderReadyDatalinkRow` recovers the ready branch at
`0x020A89F0..0x020A8B30`. It writes the slot-scaled 0x34-byte metadata entry,
emits the fixed two-by-two glyph and single tile, derives ten values from the
volatile 0x50-byte slot record, reloads the primary tilemap, and dispatches
all ten copied descriptors in order. Raw 32-bit callback addresses are kept
host-safe behind a semantic indirect-call dispatcher. The helper remains
internal and does not create a Ghidra boundary.

`Game_RenderDatalinkRowVariantOverlay` recovers
`0x020A8BD0..0x020A8C7F`. A selected row applies palette four to the
28-by-four rectangle without resolving participant type. A non-selected row
of active type one clears that rectangle first in the secondary tilemap and
then in a freshly reloaded primary tilemap. Every other type applies palette
three to the secondary tilemap. The helper remains internal and does not
create a Ghidra boundary.

`Game_RefreshDatalinkRowAuxiliaryIfNeeded` recovers the conditional operation
at `0x020A8C80..0x020A8CBB`. It refreshes selected rows without a type lookup;
non-selected rows resolve the active participant and skip only for target type
one. Parent loop mechanics at `0x020A8CBC..0x020A8CC7` remain outside the
helper and no Ghidra boundary is added.

`Game_UploadDatalinkRenderBuffers` recovers the final upload operation at
`0x020A8CC8..0x020A8D4B`. It preserves two ordered 0x600-byte flushes and
transfers, followed by the 0xA80-byte flush and transfer at source and target
offset `0x6400`. The parent-captured secondary pointer remains stable, while
the primary pointer is reloaded after both initial flushes and the final
source base is reloaded after its flush. The epilogue remains outside the
helper, and no Ghidra boundary is added.

## Whole-parent coverage audit

| Range | Semantic status |
| --- | --- |
| `0x020A8904..0x020A8933` | Parent locals and descriptor-table copy represent the prologue-owned state. |
| `0x020A8934..0x020A895F` | Three-slot ready-command loop recovered. |
| `0x020A8960..0x020A8973` | Dirty gate and early-return decision recovered. |
| `0x020A8974..0x020A89C7` | Dirty clear, two copies, pointer reload, and publication recovered. |
| `0x020A89C8` | Parent captures the secondary tilemap once and retains it through all rows and upload. |
| `0x020A89CC..0x020A89EF` | Ten-value clear and per-slot readiness query recovered. |
| `0x020A89F0..0x020A8B30` | Ready-row branch recovered. |
| `0x020A8B34..0x020A8BCF` | Unavailable-row branch recovered. |
| `0x020A8BD0..0x020A8C7F` | Selected/active variant overlay recovered. |
| `0x020A8C80..0x020A8CBB` | Conditional row auxiliary refresh recovered. |
| `0x020A8CBC..0x020A8CC7` | Parent three-slot loop represents the increment and back-edge. |
| `0x020A8CC8..0x020A8D4B` | Ordered flush and upload sequence recovered. |
| `0x020A8D4C..0x020A8D53` | Parent return represents the machine epilogue. |

`Game_UpdateDatalinkRenderer` now composes the shared local descriptor and
value arrays, the clean-frame early return, dirty rebuild, retained secondary
tilemap, ready/unavailable row branches, overlays, auxiliary refreshes,
three-row back-edge, and final uploads. Its orchestration tests cover both the
clean early return and a dirty `ready / unavailable / ready` frame. The dirty
test mutates the binding's secondary source during row one and verifies that
rows two and upload still receive the value captured at `0x020A89C8`; it also
verifies that primary binding mutations remain visible to the final upload.

The complete source therefore carries the standalone `0x020A8904` annotation.
All leaf helpers retain only `Range:` annotations and remain outside Ghidra's
function registry.
