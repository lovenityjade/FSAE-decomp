# Datalink functions after 0x020A8D54

## Boundary map

| Range | Classification |
| --- | --- |
| `0x020A8D54..0x020A8D77` | Nine-word literal pool owned by `Game_UpdateDatalinkRenderer` at `0x020A8904`. |
| `0x020A8D78..0x020A8DB3` | Complete 60-byte row-resource resolver. |
| `0x020A8DB4..0x020A8DBB` | Resolver literals: `-0x2328` and slot-record base `0x021350B0`. |
| `0x020A8DBC..0x020A8E9B` | Complete 224-byte flow-state serializer. |
| `0x020A8E9C..0x020A8F7B` | Complete 224-byte flow-state deserializer. |
| `0x020A8F7C..0x020A8F7F` | Unreferenced `bx lr` null stub outside the catalogued deserializer body. |
| `0x020A8F80..0x020A8FE3` | Complete 100-byte controller initialization method. |
| `0x020A8FE4..0x020A8FEF` | Three-word literal pool for `0x020A8F80`. |
| `0x020A8FF0..0x020A903B` | Complete 76-byte controller transition method. |
| `0x020A903C..0x020A9047` | Literal pool for `0x020A8FF0`. |
| `0x020A9048..0x020A9053` | Independent 12-byte trampoline to `0x02053340`, referenced from the controller vtable. |
| `0x020A9054..0x020A9063` | Complete 16-byte zero-variant preparation wrapper. |
| `0x020A9064..0x020A92AF` | Complete 588-byte controller-animation update. |
| `0x020A92B0..0x020A92DF` | Twelve-word literal pool owned by `0x020A9064`. |
| `0x020A92E0..0x020A947F` | Complete 416-byte internal variant routine with its own prologue; absent from the catalog and not part of `0x020A9064`. |
| `0x020A9480..0x020A9493` | Five-word literal pool owned by the internal variant routine. |
| `0x020A9494..0x020A949F` | Complete 12-byte tail wrapper selecting variant zero. |
| `0x020A94A0..0x020A94A3` | Wrapper literal pointing to `0x020A92E0`. |
| `0x020A94A4..0x020A94AF` | Complete 12-byte tail wrapper selecting variant one. |
| `0x020A94B0..0x020A94B3` | Wrapper literal pointing to `0x020A92E0`. |
| `0x020A94B4..0x020A9A5B` | Complete 1,448-byte controller-selection state machine. |
| `0x020A9A5C..0x020A9A9F` | Seventeen-word literal pool owned by `0x020A94B4`. |
| `0x020A9AA0..0x020A9CCF` | Complete 560-byte controller tilemap refresh. |
| `0x020A9CD0..0x020A9CDF` | Four-word literal pool owned by `0x020A9AA0`. |
| `0x020A9CE0..0x020A9DF7` | Complete 280-byte two-descriptor controller build. |
| `0x020A9DF8..0x020A9E0B` | Five-word literal pool owned by `0x020A9CE0`. |
| `0x020A9E0C..0x020AA44F` | Complete 1,604-byte participant-selection controller. |
| `0x020AA450..0x020AA4AB` | Twenty-three-word literal pool owned by `0x020A9E0C`. |
| `0x020AA4AC..` | Next independent catalogued game function. |

## Vtable proof

The table beginning at `0x02124B88` contains `0x020A8DBC` at method offset
`+0x14`, `0x020A8E9C` at `+0x18`, and `0x020A8D78` at `+0x1C`. The first two
methods dispatch the same respective virtual offset on the three member
addresses stored at flow-state `+0x34`, `+0x38`, and `+0x3C`. Their remaining
payload operations are exact inverses.

## Recovered semantics

`Game_ResolveDatalinkRowResource` converts renderer IDs beginning at `0x2328`
to the 16-byte view at offset eight of the corresponding 0x50-byte record at
`0x021350B0`. This closes the relation with the metadata IDs written by
`Game_RenderReadyDatalinkRow`.

`Game_SerializeDatalinkFlowState` and
`Game_DeserializeDatalinkFlowState` dispatch their three child members in
order and then transfer the same 40-byte parent payload. Four bytes at
`+0x40..+0x43` are followed by the word at `+0x48` and the 32-byte stack at
`+0x4C`; transient bytes `+0x44..+0x47` are deliberately excluded.

All three functions are represented completely and receive standalone source
annotations. No address from a pool, stub, or partially mapped function is
promoted.

## Controller-method batch

The table beginning at `0x02124B64` contains `0x020A8F80` as its first method,
`0x020A8FF0` at offset `+0x0C`, and trampoline `0x020A9048` at `+0x10`.

`Game_InitializeDatalinkController` guards static-arena interface D at
`+0x100`, constructs the controller base, publishes address `0x0217E3D4`,
clears bytes `+0x5B`, `+0x5C`, and `+0x5F`, selects mode one, initializes the
interface, and finally clears word `+0x30`.

`Game_BeginDatalinkControllerTransition` follows completion-owner byte
`+0xD0`. A nonzero byte pushes the interface and forwarded transition argument
onto the Datalink scene stack. Zero resets the controller transition fields
and loads screen resource `0x0835`.

`Game_RebuildDatalinkControllerScreen` represents the three-instruction
trampoline to `0x02053340`. Because `0x020A9048` is absent from the catalog, it
keeps a `Range:` annotation. `Game_PrepareDatalinkControllerVariant` completely
recovers `0x020A9054`, forwarding its owner argument to the zero-variant
wrapper and returning two. The three catalogued functions `0x020A8F80`,
`0x020A8FF0`, and `0x020A9054` receive standalone annotations.

## Controller-animation CFG at 0x020A9064

| Range | Role |
| --- | --- |
| `0x020A9064..0x020A90DB` | Prologue and local copies of the two six-byte ROM tables. |
| `0x020A90DC..0x020A90F7` | Dispatch controller byte `+0x5D`: phase zero, phase one, or common tail. |
| `0x020A90F8..0x020A91FF` | Phase zero: select variant one, configure eleven tweens, increment phase. |
| `0x020A9200..0x020A920F` | Phase-one completion query on tween record 11. |
| `0x020A9210..0x020A9267` | Completion: activate nine records, mark flow dirty, pop transition, load `0x0834`. |
| `0x020A9268..0x020A92A3` | Common three-slot scene-command loop using signed selection byte `+0x5B`. |
| `0x020A92A4..0x020A92AF` | Return zero and epilogue. |

`Game_UpdateDatalinkControllerAnimation` represents the complete function.
The ROM tables resolve to tween indices `5..10`, command pairs `11..16`, base
positions `(44,48)`, `(44,96)`, `(44,144)`, participant positions `(24,30)`,
`(24,78)`, `(24,126)`, duration 20, and mode one. Phase zero configures the
two standalone tweens plus records `0..2` and `5..10`. Completed phase one
activates those nine indexed records in slot order before touching the bound
flow state. Later phases still execute the common command loop.

The function receives a standalone `0x020A9064` annotation only. Its internal
helpers are not promoted.

## Controller-variant routine at 0x020A92E0

The non-catalogued target has an explicit `push` prologue at `0x020A92E0`,
balanced returns at `0x020A9334`, `0x020A93F4`, and `0x020A947C`, and a
five-word literal pool beginning at `0x020A9480`. Both independent tail
wrappers load `0x020A92E0` from their own literals. These facts prove the
routine boundary without treating any pool word as code. If the analysis
catalog is regenerated, this address is therefore eligible for an explicit
`create_if_missing` guard; no catalog or Ghidra configuration was modified as
part of this semantic recovery.

The six-byte index table at `0x020DE790` selects tween records `(11,12)`,
`(13,14)`, and `(15,16)`. The coordinate table at `0x020DE8CC` contains
`(128,96)`, `(84,136)`, and `(172,136)`; the duration words used by both
branches are 20. Variant zero marks each selected record active, clears its
elapsed word, seeds its fixed-point position 144 pixels below the unshifted
coordinate, and configures mode two motion to that coordinate. Variant one
configures mode one motion to the vertically offset coordinate without the
direct record writes. Any other selector returns immediately.

`Game_ConfigureDatalinkControllerVariant` represents the complete internal
body. `Game_SelectDatalinkControllerVariantZero` and
`Game_SelectDatalinkControllerVariantOne` represent the two complete tail
wrappers. All three receive standalone source annotations; pools remain data.

## Controller-selection CFG at 0x020A94B4

| Range | Role |
| --- | --- |
| `0x020A94B4..0x020A955B` | Prologue, copies of three ROM byte tables, and phase dispatch. |
| `0x020A95AC..0x020A9667` | Phase zero visibility fanout and early return while tween 11 is incomplete. |
| `0x020A9668..0x020A974B` | Input dispatch: four-way navigation, cancel result three, and confirm entry. |
| `0x020A974C..0x020A9963` | Three row-dependent confirmation paths and their transition setup. |
| `0x020A9964..0x020A99A8` | Clamp selected row, publish cursor coordinates, issue cursor command three. |
| `0x020A99AC..0x020A99CC` | Phase five screen-commit handshake. |
| `0x020A99D0..0x020A99E0` | Phase ten completion result five. |
| `0x020A99E4..0x020A99F4` | Phase twenty completion result four. |
| `0x020A99F8..0x020A9A10` | Phase twenty-five transition completion and interface queue. |
| `0x020A9A14..0x020A9A5B` | Common three-command render tail and return. |

`Game_UpdateDatalinkControllerSelection` represents the entire catalogued
body. Phase zero synchronizes object groups `(0,5,6)`, `(1,7,8)`, and
`(2,9,10)` with active-slot byte `0x02180C2D`. Until tween 11 reaches fixed
point `0x1000`, commands 12, 14, and 16 run and the routine returns early.

The signed neighbor table at `0x020DE7D8` describes left, right, up, and down
neighbors for each of the three rows, with `-1` blocking movement. Successful
movement plays sound `0x3D`; cancel plays `0x44` and returns three. Confirm
plays `0x3E`. Row zero configures records 4 and 23, locks the bound owner,
pushes return phase one, and enters phase 25. Row one selects state zero/one
when zero/all three player slots are ready, otherwise it enters phase 20. Row
two selects state two when none are ready, otherwise it enters phase 10.

The cursor table resolves to `(40,97)`, `(48,137)`, and `(136,137)`. The two
confirmation targets are record 4 at `(255,-32)` and record 23 at `(24,204)`,
both duration 12 in mode one. Phase five controls scene byte `+0xC27`; phases
ten and twenty return five and four after tween completion; phase 25 queues
the controller interface after primary-transition completion. All non-early
paths finish with selected commands 11/13/15 and unselected commands
12/14/16. Only the complete `0x020A94B4` body receives a standalone source
annotation; the literal pool remains data.

## Controller tilemap refresh at 0x020A9AA0

| Range | Role |
| --- | --- |
| `0x020A9AA0..0x020A9ABB` | Prologue and early return when controller byte `+0x5F` is zero. |
| `0x020A9ABC..0x020A9B27` | Clear dirty byte, copy two `0x600`-byte maps, publish primary map, load row count. |
| `0x020A9B28..0x020A9C2F` | Render each active row, apply selected/default palette, refresh row auxiliary state. |
| `0x020A9C30..0x020A9C6F` | Clear every remaining row through row two. |
| `0x020A9C70..0x020A9CCF` | Flush and transfer primary target one followed by secondary target two. |

`Game_RefreshDatalinkControllerTilemaps` represents the complete body. The
four pool pointers reproduce the established renderer binding graph rooted at
`0x0217A23C`, `0x02180D9C`, `0x02180DA0`, and `0x0217D348`. They are kept in
the pool and are not interpreted as instructions.

For each row below unsigned controller count `+0x5A`, the first loop writes
tile `0x22`/`0x42` pairs at X 8..17 and row offsets six/seven using palette
13. The second writes tile `2`/`0x22` pairs at X 26..28 and offsets four/five
using palette four. The row rectangle is `(2, row*6+4, 28, 4)` and receives
palette four exactly for signed selected row `+0x5B`, otherwise palette
three. Each rendered row immediately refreshes its auxiliary state. The same
rectangle is cleared for unused rows.

The primary binding is reloaded after its cache flush before transfer,
whereas the secondary destination is captured before rendering and retained
through its flush and transfer. Both transfers cover exactly `0x600` bytes at
destination offset zero. Only `0x020A9AA0..0x020A9CCF` receives a standalone
source annotation; pool `0x020A9CD0..0x020A9CDF` and function `0x020A9CE0`
are excluded.

## Controller descriptor build at 0x020A9CE0

| Range | Role |
| --- | --- |
| `0x020A9CE0..0x020A9D0B` | Prepare rendering, acquire context, and measure resource `0x084F`. |
| `0x020A9D0C..0x020A9D43` | Center measured width in 256 units and derive signed quotient/remainder by eight. |
| `0x020A9D44..0x020A9D93` | Populate two 0x34-byte descriptors and derive their buffer addresses. |
| `0x020A9D94..0x020A9DC7` | Publish layout value, resolve optional owner `+0x30`, render descriptor set. |
| `0x020A9DC8..0x020A9DF7` | Flush and transfer the final `0xA80`-byte range at offset `0x6400`. |

`Game_BuildDatalinkControllerDescriptors` represents the complete body. It
shares the established descriptor structures at `0x02124BB4`, renderer at
`0x02124C50`, layout source at `0x021806F0`, and buffer-base binding at
`0x02180DA4`. Literal resource ID `0x084F` and those four addresses occupy
pool `0x020A9DF8..0x020A9E0B`, which remains data.

Both descriptors use resource `0x084F`, type two, and mode 14. Their variant
indices are five and eleven. The preserved row word at descriptor `+0x0C`
selects `buffer_base + row*32`. The centered signed offset is decomposed with
C11 truncation toward zero into quotient `+0x10` and 16-bit remainder
`+0x24`; tests include both positive and negative offsets. Only descriptors
zero and one are changed.

A zero controller owner address is forwarded unchanged. A nonzero owner is
advanced by `0x30` before rendering. The buffer base is reloaded after cache
flush before the target-one transfer, matching the target load order. Only
`0x020A9CE0..0x020A9DF7` receives a standalone annotation; its pool and the
independent function at `0x020A9E0C` are excluded.

## Participant-selection controller at 0x020A9E0C

| Range | Role |
| --- | --- |
| `0x020A9E0C..0x020A9EA3` | Prologue, six-byte tween-pair table copy, phase dispatch. |
| `0x020A9EA4..0x020AA047` | Phase zero initialization, missing-slot discovery, tween seeding, resource/descriptors. |
| `0x020AA048..0x020AA244` | Phase five transition wait, navigation, confirm/cancel, clamp and cursor update. |
| `0x020AA248..0x020AA2FC` | Phase ten countdown, timed slot transfer, owner commit and early return. |
| `0x020AA300..0x020AA3CC` | Phase fifteen exit wait, stack restore, object reposition and result two. |
| `0x020AA3D0..0x020AA44F` | Common tilemap refresh, participant commands, fixed commands 30/23 and return zero. |

`Game_UpdateDatalinkControllerParticipantSelection` represents the complete
catalogued body. Phase zero saves the two panel Y values on the controller
stack, resets the panel/overlay positions, and configures records 30 and 23
toward `(255,0)` and `(24,180)` with duration 12 in mode two. It builds the
list of unavailable player slots, seeds their record pairs `(5,6)`, `(7,8)`,
or `(9,10)` at missing-row positions `(24,30)`, `(24,78)`, `(24,126)`, then
loads resource `0x0836`, rebuilds two descriptors, and marks the tilemap dirty.

Phase five waits for record 30 to reach `0x1000`. Left/right inputs decrement
or increment the signed selection before it is clamped to `0..count-1`.
Movement marks controller `+0x5F`, plays sound `0x3D`, and selects cursor
positions `(9,29)`, `(9,77)`, `(9,125)`. Confirm enters phase ten with a
60-frame countdown, changes surrounding state to three, writes display bits
`0x1900` under mask `0x1F00`, hides the overlay, and deactivates records
5..10. Cancel plays `0x44`, configures records 30/23 toward `(255,-32)` and
`(24,204)` in mode one, deactivates those pairs, and enters phase 15.

Phase ten clears the shared ready byte every frame. At countdown 30 it
transfers the active source slot to the player slot mapped by the initial
selection. Expiry removes the two saved stack entries, resets both panel
objects, pops the owner transition, clears owner redraw suppression, marks its
selection dirty, selects participant zero, invokes the owner refresh, commits
the screen, and returns early. Phase 15 waits for record 30, restores both
saved Y values, repositions owner pairs/triplet by the negated fixed-point Y,
rebuilds owner descriptors, selects variant zero, stores `0x0835` in
controller `+0x60`, and returns two.

The common tail refreshes controller tilemaps and maps each displayed row's
underlying player-slot ID through the six-byte `0x020DE7A8` command table.
Selected rows issue 5/7/9 and unselected rows 6/8/10, followed by commands 30
and 23. Pool `0x020AA450..0x020AA4AB` contains all table/global/resource
pointers, input/display addresses, and constants; it remains data. Only the
body through `0x020AA44F` receives a standalone annotation, and independent
function `0x020AA4AC` is excluded.

## Participant-action controller at 0x020AA4AC

| Range | Role |
| --- | --- |
| `0x020AA4AC..0x020AA580` | Prologue, ten-byte local table copy and six-phase dispatch. |
| `0x020AA584..0x020AA6C8` | Phase zero row/overlay initialization and resource `0x0838`. |
| `0x020AA6CC..0x020AA87C` | Phase five tween wait, two-choice input, removal setup and cursor update. |
| `0x020AA880..0x020AA8C0` | Phase fifteen removal countdown and screen-commit handoff. |
| `0x020AA8C4..0x020AA90C` | Phase sixteen two-frame owner commit. |
| `0x020AA910..0x020AA9FC` | Phase twenty exit tween setup. |
| `0x020AAA00..0x020AAA50` | Phase twenty-five exit wait and result one. |
| `0x020AAA54..0x020AAAA7` | Common selection-dependent scene commands and return. |

`Game_UpdateDatalinkControllerParticipantAction` represents the complete
1,532-byte catalogued body. The ROM tables at `0x020DE784` and `0x020DE7B0`
resolve respectively to action pairs `(17,18)/(15,16)` and the underlying
participant pairs `(5,6)/(7,8)/(9,10)`. Phase zero seeds both action variants
64 pixels below their final positions `(84,112)` and `(172,112)`, configures
them in mode two for 12 frames, exposes scene object `+0xEE7`, and configures
records 31 and 23 toward `(255,0)` and `(24,180)`. The direct scene backing
record 31 begins hidden at `(255,-32)`. Resource `0x0838` is loaded before
phase five begins with selection zero.

Phase five waits for record 17 to reach `0x1000`. Inputs `0x10` and `0x20`
move the signed selection across the two rows, clamp it to `0..1`, play sound
`0x3D` on a real change, and place the cursor at `(48,113)` or `(136,113)`.
Cancel or confirming row zero selects the ordinary exit. Confirming row one
writes display value `0x1900` under mask `0x1F00`, enters phase 15 with a
60-frame countdown, changes surrounding state to seven, and deactivates the
four action records, participant records 5..10, and base records 0..2. It also
clears scene bytes `+0xEE7` and `+0xC27`.

At countdown 30, phase 15 removes the active player slot. Expiry commits the
screen, installs a two-frame phase-16 delay, then pops the owner transition,
clears redraw suppression, marks the owner selection dirty, selects participant
zero, invokes the owner refresh, and resets the controller selection. The
ordinary exit animates the four action records 64 pixels downward, record 31
to `(255,-32)`, and record 23 to `(24,204)` in mode one while playing sound
`0x44`. Phase 25 waits again on record 17, restores record 23 toward
`(24,180)`, stores resource `0x0835` in controller `+0x60`, and returns one.

Every non-early path issues the selected action command (17 or 15), the other
row's unselected command (18 or 16), then command 31. The 20-word literal pool
at `0x020AAAA8..0x020AAAF7` is data and is not part of the recovered body. The
independent mode dispatcher at `0x020AAAF8..0x020AAB7B`, the one-instruction
stub at `0x020AAB7C..0x020AAB7F`, and the next catalogued function at
`0x020AAB80` are all explicitly excluded.

## Menu-controller lifecycle at 0x020AAB80

| Range | Role |
| --- | --- |
| `0x020AAB80..0x020AAC1B` | 156-byte guarded constructor and two-interface initialization. |
| `0x020AAC1C..0x020AAC2F` | Five-word constructor literal pool, excluded. |
| `0x020AAC30..0x020AACEB` | 188-byte resume, history-pop and resource-selection method. |
| `0x020AACEC..0x020AACFB` | Four-word resume-method literal pool, excluded. |
| `0x020AACFC..0x020AAD03` | Noncatalogued overlay trampoline, excluded. |
| `0x020AAD04..0x020AAD07` | Trampoline target literal, excluded. |
| `0x020AAD08..` | Next catalogued animation helper, excluded from this lifecycle lot. |

`Game_InitializeDatalinkMenuController` first guards two static interface
objects through arena words `+0x08` and `+0x10`. The first guarded object is at
`0x0217E354` (arena `+0x80`) and receives vtable `0x02124B40`; the second
global slot at `0x0217E498` receives `0x02124B1C`. Base construction precedes
controller mode two and interface-address stores at `+0x30/+0x34`. Both first
virtual methods run before the base finalizer receives the primary interface;
history depth `+0x38` and selection `+0x5D` are cleared only afterward.

`Game_ResumeDatalinkMenuController` preserves the incoming third and fourth
virtual-method arguments for the transition path. When global byte
`0x0217D30C` is set and completion-owner byte `0x0217D30D` is three or four,
the scene and secondary interface slot are forwarded to the transition stack
and the method returns without consuming history. Other completion states
clear the global byte and continue normally.

An empty history clears controller phase `+0x5E`. A nonempty history decrements
depth `+0x38`, sets phase `+0x5E` to one, and restores the low byte of the last
word from `+0x3C` into selection `+0x5D`. The overlay availability query chooses
resource `0x083B`; if its primary choice is unavailable, selection is forced
to one and resource `0x07EF` is loaded instead. The pools, trampoline, literal,
and `Game` candidate beginning at `0x020AAD08` receive no annotation from this
lot.

## Directional menu animations at 0x020AAD08

| Range | Role |
| --- | --- |
| `0x020AAD08..0x020AAE2B` | 292-byte entrance from the right. |
| `0x020AAE2C..0x020AAE53` | Ten-word literal pool, excluded. |
| `0x020AAE54..0x020AAF27` | 212-byte exit to the left. |
| `0x020AAF28..0x020AAF4B` | Nine-word literal pool, excluded. |
| `0x020AAF4C..0x020AB06F` | 292-byte entrance from the left. |
| `0x020AB070..0x020AB097` | Ten-word literal pool, excluded. |
| `0x020AB098..0x020AB16B` | 212-byte exit to the right. |
| `0x020AB16C..0x020AB18F` | Nine-word literal pool, excluded. |
| `0x020AB190..` | Next catalogued menu controller, excluded. |

The four helpers form one symmetric animation group over tween records 19
through 23. ROM table `0x020DE87C` supplies the two paired positions
`(128,64)` and `(128,120)`, while `0x020DE84C` supplies auxiliary position
`(24,180)`. All configurations use duration 20 from `0x020DE838`.

`Game_EnterDatalinkMenuFromRight` seeds the first four scene records at X 384,
64/120 and `Game_EnterDatalinkMenuFromLeft` seeds them at X -128. Both seed
the fifth scene record at `(24,204)` and configure tweens 19/20, 21/22 and 23
toward their centered positions in mode two. The five scene records occupy
duration/X/Y fields beginning at `+0xAEC`, `+0xB44`, `+0xB9C`, `+0xBF4`, and
`+0xC4C`, each with stride `0x58` except for the already identified shared
ready byte at `+0xC27` between the fourth and fifth records.

`Game_ExitDatalinkMenuToLeft` targets X -128 and
`Game_ExitDatalinkMenuToRight` targets X 384 for the first four tweens. Both
move record 23 to `(24,204)` in mode one without rewriting the direct scene
fields. Each catalogued body ends before its own literal pool. The pool after
the right-side exit and the independent 784-byte controller at `0x020AB190`
are explicitly excluded.

## Eight-phase menu controller at 0x020AB190

| Range | Role |
| --- | --- |
| `0x020AB190..0x020AB203` | Prologue, four-byte command table copy, context/status queries and eight-way dispatch. |
| `0x020AB204..0x020AB228` | Phases zero/one directional entrances and transition to selection. |
| `0x020AB22C..0x020AB3A8` | Phase two display setup, tween wait, input, history push, clamp and cursor. |
| `0x020AB3AC..0x020AB3C4` | Phase three cancel-exit wait and scene transition pop. |
| `0x020AB3C8..0x020AB3E8` | Phase four primary-action wait and scene transition push. |
| `0x020AB3EC..0x020AB414` | Phase five wireless-selection extraction and commit start. |
| `0x020AB418..0x020AB434` | Phase six asynchronous commit wait/finalization. |
| `0x020AB438..0x020AB450` | Phase seven one-byte resource delay. |
| `0x020AB454..0x020AB49F` | Common two-row commands, fixed command 23 and return. |
| `0x020AB4A0..0x020AB4C3` | Nine-word literal pool, excluded. |
| `0x020AB4C4..` | Next catalogued background-clear helper, excluded. |

`Game_UpdateDatalinkMenuController` represents the complete 784-byte body.
Every invocation obtains the selection context and current connection state
before dispatching controller phase `+0x5E`. Phase zero clears the menu
background, enters from the right, and selects phase two; phase one enters
from the left and reaches the same selection phase.

Phase two writes display value `0x1900` under mask `0x1F00`, then waits for
tween 19 to reach `0x1000`. Inputs `0x40/0x80` move selection `+0x5D` across
the two rows. Cancel selects phase three, exits right, and plays `0x44`.
Confirming row zero is accepted only while connection state is zero: it starts
the primary action with the context, pushes the signed selection into the word
history at `+0x3C`, selects phase four, exits left, and plays `0x3E`.
Confirming row one starts the secondary action and performs the same history
push/left exit for phase five.

The availability query chooses clamp range `0..1` or restricted range `1..1`.
A real selection change plays `0x3D`; cursor positions from ROM are `(40,65)`
and `(40,121)`, followed by scene command three. Phase three pops the scene
transition after tween 19 completes. Phase four waits for both tween completion
and connection state two, then pushes the scene with controller slot `+0x34`.
ARM inspection confirms this call has exactly those two meaningful arguments;
the incoming virtual-method arguments are not relayed.

Phase five checks bit `0x40`, performs a second status read, maps its low
nibble, stores the resulting byte at `0x0212C7C8`, begins asynchronous commit,
and selects phase six. Commit completion finalizes the wireless selection,
sets phase seven and countdown `+0x5C` to one. The next phase decrements that
byte and writes resource `0x84` to `0x021806F0 + 0x53E` when it reaches zero.

The common tail maps selected/unselected rows through ROM command pairs
`(19,20)` and `(21,22)`, then issues command 23. Pool
`0x020AB4A0..0x020AB4C3` contains the command/cursor tables, scene/tween/input
addresses and shared bytes; it remains data. No trampoline lies between this
pool and independent function `0x020AB4C4`, which is explicitly excluded.

## Menu-background clear at 0x020AB4C4

| Range | Role |
| --- | --- |
| `0x020AB4C4..0x020AB51B` | 88-byte clear, cache flush and dual-target upload helper. |
| `0x020AB51C..0x020AB51F` | Buffer-binding pointer literal, excluded. |
| `0x020AB520..0x020AB523` | Independent one-instruction `bx lr` stub, excluded. |
| `0x020AB524..` | Next catalogued mode-three controller constructor, excluded. |

`Game_ClearDatalinkMenuBackground` is the complete helper invoked by phase
zero of `Game_UpdateDatalinkMenuController`. It reads the transfer buffer
binding at `0x02180D9C`, clears `0x800` bytes to zero, flushes the same-sized
cache range, then transfers the buffer to render targets one and two at
destination offset zero.

The target reloads the pointer stored at `0x02180D9C` before the clear, flush,
primary transfer, and secondary transfer rather than retaining the first
loaded buffer value. The semantic binding remains volatile and the dedicated
harness mutates it after each operation to verify all four reload points.
Only `0x020AB4C4..0x020AB51B` belongs to this annotation. Literal
`0x020AB51C`, stub `0x020AB520`, and the distinct guarded constructor beginning
at `0x020AB524` remain excluded.

## Extended-menu lifecycle at 0x020AB524

| Range | Role |
| --- | --- |
| `0x020AB524..0x020AB5DF` | 188-byte guarded mode-three controller constructor. |
| `0x020AB5E0..0x020AB5F7` | Six-word constructor literal pool, excluded. |
| `0x020AB5F8..0x020AB6B7` | 192-byte resume/history method. |
| `0x020AB6B8..0x020AB6CB` | Five-word resume-method literal pool, excluded. |
| `0x020AB6CC..0x020AB6CF` | Independent one-instruction `bx lr` stub, excluded. |
| `0x020AB6D0..` | Next catalogued directional animation, excluded. |

`Game_InitializeDatalinkExtendedMenuController` guards two static objects.
Arena guard `+0x1C` controls the compound interface at `0x0217E4FC`, whose
vtable words at offsets zero, `+0x30`, and `+0xD0` become `0x02126414`,
`0x0212643C`, and `0x02124B40`. Arena guard zero controls the primary inline
interface at arena `+0x20` (`0x0217E2F4`), also with vtable `0x02124B40`.

Base construction precedes the controller interface stores at `+0x30/+0x34`
and both first virtual calls. Mode three is installed before the base
finalizer receives the primary interface. Selection `+0x5E`, forced-phase byte
`+0x60`, halfword `+0x5C`, phase `+0x5F`, and history depth `+0x38` are cleared
only after finalization.

`Game_ResumeDatalinkExtendedMenuController` checks shared abort byte
`0x0217D30C`. With completion-owner byte `0x0217D30D` equal to four, it
forwards the scene, secondary interface, and incoming third/fourth arguments
to the transition operation and returns without clearing the abort byte or
consuming history. Other abort states optionally convert fallback byte
`0x0217D30E == -1` into forced byte `+0x60`, clear both shared bytes as
appropriate, and select a nonzero resumed state.

A nonempty word history at `+0x3C` is popped through depth `+0x38` and replaces
that resumed state. Halfword `+0x5C` is always cleared. Forced byte `+0x60`
selects phase seven; otherwise a zero/nonzero restored word selects phase zero
or one. Resource `0x083C` is then loaded. The six- and five-word pools, the
stub at `0x020AB6CC`, and the distinct animation beginning at `0x020AB6D0`
remain excluded.

## Extended-menu directional animations at 0x020AB6D0

| Range | Role |
| --- | --- |
| `0x020AB6D0..0x020AB7F3` | 292-byte entrance from the right. |
| `0x020AB7F4..0x020AB81B` | Ten-word literal pool, excluded. |
| `0x020AB81C..0x020AB93F` | 292-byte entrance from the left. |
| `0x020AB940..0x020AB967` | Ten-word literal pool, excluded. |
| `0x020AB968..0x020ABA3B` | 212-byte exit to the right. |
| `0x020ABA3C..0x020ABA5F` | Nine-word literal pool, excluded. |
| `0x020ABA60..0x020ABB33` | 212-byte exit to the left. |
| `0x020ABB34..0x020ABB57` | Nine-word literal pool, excluded. |
| `0x020ABB58..` | Next catalogued extended-menu controller, excluded. |

The four functions form the complete directional-animation group used by the
mode-three controller. ROM table `0x020DE89C` again supplies paired positions
`(128,64)` and `(128,120)`, while `0x020DE84C` supplies auxiliary position
`(24,180)`. Duration is 20, horizontal travel is 256 pixels, and auxiliary
exit travel is 24 pixels.

`Game_EnterDatalinkExtendedMenuFromRight` seeds the four direct scene records
at X 384 and `Game_EnterDatalinkExtendedMenuFromLeft` seeds them at X -128.
The records begin at `+0xD54`, `+0xDAC`, `+0xE04`, and `+0xE5C`; their target
tweens are 26/27 and 28/29. Both entrances seed the shared fifth record at
`+0xC4C` to `(24,204)` and configure all five tweens toward centered positions
in mode two.

`Game_ExitDatalinkExtendedMenuToRight` targets X 384 and
`Game_ExitDatalinkExtendedMenuToLeft` targets X -128 in mode one. Both target
tween 23 at `(24,204)` without rewriting the direct scene records. Every body
ends before its own literal pool; the final nine-word pool and independent
controller at `0x020ABB58` are excluded.

## Eight-phase extended-menu controller at 0x020ABB58

| Range | Role |
| --- | --- |
| `0x020ABB58..0x020ABBDC` | Prologue, four-byte command-table copy, session update, context/status queries and eight-way dispatch. |
| `0x020ABBE0..0x020ABC14` | Phases zero/one request setup and directional entrances. |
| `0x020ABC18..0x020ABD54` | Phase two display setup, tween wait, input handling, clamp and cursor placement. |
| `0x020ABD58..0x020ABD8C` | Phase three cancel-exit wait and connection-dependent transition handling. |
| `0x020ABD90..0x020ABDB0` | Phase four primary-action connection wait. |
| `0x020ABDB4..0x020ABDCC` | Phase five exit-tween wait and resource handoff. |
| `0x020ABDD0..0x020ABE04` | Phase six exit-tween wait, history push and scene transition. |
| `0x020ABE08..0x020ABEE0` | Phase seven result dispatch or timed recovery; all paths return early. |
| `0x020ABEE4..0x020ABF2B` | Common two-row commands, fixed command 23 and return. |
| `0x020ABF2C..0x020ABF5B` | Twelve-word literal pool, excluded. |
| `0x020ABF5C..` | Next catalogued function, excluded. |

`Game_UpdateDatalinkExtendedMenuController` represents the complete 980-byte
mode-three controller body. Every invocation copies the low nibble of global
word `0x0212C530` to session byte `0x021349C8`, then obtains the selection
context and connection state before dispatching phase byte `+0x5F`. Phases
zero and one store request count four in session halfword `+0x04`, select
phase two, and enter respectively from the right or left.

Phase two writes display value `0x1900` under mask `0x1F00` and waits for
tween 26 to reach `0x1000`. Inputs `0x40/0x80` move signed row `+0x5E` through
the clamp range `0..1`. A real movement plays sound `0x3D`, places the cursor
at ROM coordinate `(32,65)` or `(32,121)`, and issues scene command three.
Cancel is accepted immediately while disconnected, or after the context-ready
query succeeds while connected; it exits right, plays sound `0x44`, and
selects phase three. Confirming row zero calls the primary-action preparation
helper and selects phase four only when it succeeds. Confirming row one exits
left and selects phase six. Both confirmation paths play sound `0x3E`.

Phase three waits for tween 26. Connection state zero pops the extended-menu
scene transition; states other than zero and three perform the context-ready
query without consuming its result. Phase four waits for connection state
eight, begins the secondary menu action, clears session word `+0x24`, exits
left, and selects phase five. After tween completion, phase five stores
resource `0x84` in the screen-resource byte. Phase six instead pushes history
word one at the current `+0x38` depth and starts the scene transition through
the secondary interface address at controller `+0x34`.

Forced phase-seven handling maps context results two, three, and all other
values to surrounding states 12, 14, and 13, clears force byte `+0x60`, and
returns. Its unforced path treats halfword `+0x5C` as signed: values through
60 increment and return; later frames place the recovery object at ROM
coordinate `(224,192)` and issue command `0x35`. Confirm input then commits
the screen, clears the counter, restores phase zero, and plays sound `0x3E`.
All phase-seven paths deliberately bypass the common command tail.

Every other path selects ROM command pair `(26,27)` for row zero and `(28,29)`
for row one, then issues command 23. Pool `0x020ABF2C..0x020ABF5B` contains the
command table, session/scene/input/tween owners and the three coordinate
tables; it remains data. No stub or trampoline lies before the distinct
function at `0x020ABF5C`, which is explicitly excluded.

## Extended-menu primary session preparation at 0x020ABF5C

| Range | Role |
| --- | --- |
| `0x020ABF5C..0x020ABF77` | Prologue, selection/render-context queries and initial connection-state read. |
| `0x020ABF78..0x020ABFAC` | Disconnected-state start and blocking VBlank/update loop until state two. |
| `0x020ABFB0..0x020ABFF3` | State gate, active-slot payload selection and wireless-session start. |
| `0x020ABFF4..0x020AC00B` | Failure sound, Boolean returns and epilogue. |
| `0x020AC00C..0x020AC013` | Two-word literal pool, excluded. |
| `0x020AC014..0x020AC027` | Five independent `bx lr` null stubs, excluded. |
| `0x020AC028..0x020AC033` | Independent row-resource adapter trampoline, excluded. |
| `0x020AC034..0x020AC037` | Adapter literal `-0x30`, excluded. |
| `0x020AC038..` | Next catalogued function, excluded. |

`Game_PrepareDatalinkExtendedMenuPrimaryAction` is the complete 176-byte
helper called when row zero is confirmed in the extended menu. It first
obtains the selection context, preserves the otherwise-unused rendering-
context query, and reads the current connection state. State zero begins the
primary connection action with argument zero, then repeatedly waits for
VBlank, updates the connection using the selection context, and rereads its
state until it becomes two. An initially nonzero state other than two is
rejected immediately.

Once ready, scene byte `0x021806F0 + 0x53D` selects one of the `0x50`-byte
player-slot records at `0x021350B0`. The session initializer receives the
14-byte payload beginning at record offset eight, participant limit four,
shared-data stride `0x34`, and the original selection context. A successful
initializer returns one. Failure plays sound `0x40` and returns zero.

The only body-owned literals are the scene-owner and player-slot bases at
`0x020AC00C..0x020AC013`. The five null stubs and the adapter at
`0x020AC014..0x020AC037` have independent entry semantics; the adapter adjusts
its first argument by `-0x30` and branches to the previously recovered
row-resource resolver. None is part of this function or promoted by this lot.

## Datalink screen-update commit at 0x020AC308

| Range | Role |
| --- | --- |
| `0x020AC038..0x020AC307` | Independent preceding state-change function and its pool, excluded. |
| `0x020AC308..0x020AC31F` | Prologue, update-state binding and screen-buffer capture. |
| `0x020AC324..0x020AC374` | Nested rectangle-clear loops over a 32-column tilemap. |
| `0x020AC378..0x020AC3B0` | Conditional `0x600`-byte cache flush, target-zero transfer and upload-flag clear. |
| `0x020AC3B4..0x020AC3C7` | Shared-value restoration and epilogue. |
| `0x020AC3C8..0x020AC3CF` | Two-word literal pool, excluded. |
| `0x020AC3D0..` | Next independent catalogued function, excluded. |

`Game_CommitDatalinkScreenUpdate` represents the complete 192-byte body. The
state at `0x0217E604` supplies signed-byte rectangle coordinates and dimensions
at `+0x0C..+0x0F`, plus the screen buffer at `+0x48`. Each selected halfword is
cleared using `x + y * 32`. Zero width or height skips the corresponding loop;
the ARM comparisons otherwise treat the sign-extended dimensions as unsigned
loop bounds, matching the target's positive-dimension state invariant.

State word `+0x08` records whether the screen buffer was externally supplied.
When set, the captured buffer is flushed for exactly `0x600` bytes, transferred
to target zero at destination offset zero through `0x0209DA7C`, and the flag is
cleared only after that transfer. A direct VRAM-backed buffer bypasses all
three operations.

Finally, saved state word `+0x04` is always copied to shared global
`0x02117E60`, including zero-area and direct-buffer paths. The body owns only
the two literals `0x0217E604` and `0x02117E60` at
`0x020AC3C8..0x020AC3CF`. The predecessor through `0x020AC307`, this pool, and
the distinct prologue at `0x020AC3D0` remain outside the recovered function.

## Datalink resource access wrappers at 0x020AC3D0

| Range | Role |
| --- | --- |
| `0x020AC3D0..0x020AC403` | Complete 52-byte archive-base plus resource-offset resolver. |
| `0x020AC404..0x020AC40B` | Two-word archive-base pointer pool, excluded. |
| `0x020AC40C..0x020AC427` | Complete 28-byte resource-size selector; no pool. |
| `0x020AC428..` | Next independent catalogued function, excluded. |

`Game_GetDatalinkResourceAddress` forwards the resource ID to one of two
offset-table readers. A zero selector uses `0x0209B700` and adds the returned
offset to the base pointer stored at `0x0212C55C`; a nonzero selector uses
`0x0209B734` and base-pointer owner `0x0212C560`. The caller immediately after
this lot derives that selector from bit 31 of its encoded resource ID, but the
wrapper itself accepts any nonzero value.

`Game_GetDatalinkResourceSize` performs the corresponding selection between
`0x0209B714` and `0x0209B748`. Both helpers calculate size by subtracting the
selected resource offset from the following index offset. ARM inspection
confirms that the resource ID remains in `r0` across each wrapper dispatch,
correcting the raw decompiler's missing arguments and missing return value for
the second wrapper.

The only pool belongs to the address resolver and contains `0x0212C560` and
`0x0212C55C`. The size wrapper ends at its own balanced return and has no
literals before the separate prologue at `0x020AC428`.

## Datalink background-resource loader at 0x020AC428

| Range | Role |
| --- | --- |
| `0x020AC428..0x020AC44F` | Prologue and optional character-resource delegation. |
| `0x020AC450..0x020AC4E7` | Optional palette allocation, copy, unpack, flush, transfer and release. |
| `0x020AC4E8..0x020AC573` | Optional screen allocation, copy, unpack, flush and transfer. |
| `0x020AC574..0x020AC593` | Optional screen-file release and balanced returns. |
| `0x020AC594..` | Next independent catalogued function, excluded. |

`Game_LoadDatalinkBackgroundResources` represents the complete 364-byte body
and owns no literal pool. Its resource owner preserves the target word layout:
palette file/data at `+0x00/+0x04`, character file/data at `+0x08/+0x0C`, and
screen file/data at `+0x10/+0x14`.

A character ID other than `-1` is forwarded first to the independent loader
at `0x020AC594`, with destination offset zero and the original release flags.
Palette and screen IDs use `UINT32_MAX` as their independent absence sentinel.
For each present resource, bit 31 selects the alternate archive and the lower
31 bits select the index passed through the recovered address/size wrappers.

Both direct paths allocate the exact archive size with alignment `0x20` and
copy the complete file before unpacking it. Palette files use the byte-copy
primitive, flush the pointer/size exposed by the unpacked palette descriptor,
and transfer through `0x0209DB38` at destination offset zero. Screen files use
the fast-copy primitive and flush the inline payload at unpacked-screen offset
`+0x0C`; `0x0209DAA0` receives the sign-extended 16-bit screen target,
destination offset zero, and the unpacked size at `+0x08`.

Release-flag bit two frees and clears the palette file/data pair after its
transfer. Bit four performs the same operation for the screen pair. Bit one is
passed to the delegated character loader. Body SHA-256 from the ROM is
`52ed558b63ff91df5c58c2831d9c4922746f687016aa008b89bd8ad274225641`.
The next prologue begins directly at `0x020AC594`; there are no intervening
literals, alignment words, stubs or trampolines.

## Datalink sprite OAM submission at 0x020AC8B4

| Range | Role |
| --- | --- |
| `0x020AC594..0x020AC8B3` | Independent preceding resource/cell helpers and their pools, excluded. |
| `0x020AC8B4..0x020AC8F7` | Prologue, active gate and optional 60-frame age gate. |
| `0x020AC8F8..0x020AC953` | Cell-to-OAM expansion and per-entry priority rewrite. |
| `0x020AC954..0x020AC96F` | Copy of the 96-byte DS OAM dimension table to the local frame. |
| `0x020AC970..0x020ACA0B` | Shape/size lookup and multi-OAM bounds reduction. |
| `0x020ACA0C..0x020ACA87` | Horizontal, wrapped-Y and actor-position clipping decisions. |
| `0x020ACA88..0x020ACAAB` | Conditional OAM-manager submission, count return and epilogue. |
| `0x020ACAAC..0x020ACAB3` | Two-word literal pool, excluded. |
| `0x020ACAB4..` | Next independent catalogued function, excluded. |

`Game_SubmitDatalinkSpriteOams` represents the complete 504-byte body. An
inactive sprite returns zero. A nonzero age-gate byte also suppresses the
sprite while its age byte is below 60. Accepted sprites expand the current
cell through `NNS_G2dMakeCellToOams`, using capacity from the caller, no affine
matrix, the sprite's `+0x30` fixed-point translation, and null callbacks.

Every generated entry has attribute-two bits 10 and 11 replaced by sprite
byte `+0x01`. The copied table at `0x020DEA58` contains the canonical DS OAM
dimensions: four square sizes, four horizontal sizes, and four vertical
sizes. Shape from attribute-zero bits 14..15 and size from attribute-one bits
14..15 select one pair. As in the target, all entries in the generated cell
use the first entry's selected dimensions while reducing minimum X, minimum
right edge, minimum Y, and maximum bottom edge.

The clipping tail rejects horizontal bounds outside `0..255`, the specific
lower-right hardware-wrap combination beginning at Y 192, and sprites whose
object Y is below 192 while their normalized bottom has wrapped above the
screen. Y normalization is modulo 256. A nonzero surviving count is submitted
to OAM manager `0x0217A23C`; the count is returned either way.

Ghidra's reported indirect jump at `0x020ACA64` is a false positive. Direct
ARM inspection shows only arithmetic Y normalization and conditional
branches throughout `0x020ACA28..0x020ACA87`; there is no jump table or
indirect call in the body. The actual body-owned table is the dimension data
reached through literal `0x020DEA58`. Pool `0x020ACAAC..0x020ACAB3` contains
that address and OAM-manager address `0x0217A23C`.

Body SHA-256 from the ROM is
`1727e1810c72ef22ab7061193606fc064141b311adf52551093f70f9e74d2211`.
The pool and the separate prologue at `0x020ACAB4` remain excluded.

## Datalink sinusoidal vector interpolation at 0x020ACB04

| Range | Role |
| --- | --- |
| `0x020ACAB4..0x020ACB03` | Independent preceding linear interpolation helper, excluded. |
| `0x020ACB04..0x020ACB73` | Complete 112-byte ease-in instruction body. |
| `0x020ACB74..0x020ACB7B` | Ease-in sine-table and signed-scale literal pool, excluded from `body_bytes`. |
| `0x020ACB7C..0x020ACBEB` | Complete 112-byte ease-out instruction body. |
| `0x020ACBEC..0x020ACBF3` | Ease-out sine-table and signed-scale literal pool, excluded from `body_bytes`. |
| `0x020ACBF4..` | Next independent catalogued function, excluded. |

`Game_DatalinkInterpolateVectorEaseInSine` and
`Game_DatalinkInterpolateVectorEaseOutSine` both accept the same Q12 progress
domain and two-component vectors as the linear helper at `0x020ACAB4`. They
sample the sine half-cycle from NitroSDK's interleaved signed Q12
`FX_SinCosTable_` at `0x020C91FC`. Progress `0..0x1000` maps to sine indices
`0..0x800`; the cosine halfwords are skipped.

Before converting progress to a Q9 interpolation weight, the ease-in helper
adds `sin(progress) * -0x517 >> 12`, while the ease-out helper uses positive
`0x517`. Thus both preserve the exact start and end points. At midpoint, the
negative curve has weight 93 and the positive curve weight 418 after Q9
quantization; they respectively accelerate from and decelerate toward a
near-zero slope. The adjusted Q12 value is shifted right three, then applied
independently to X and Y with the same
`start * (0x200 - weight) + end * weight >> 9` blend as the linear helper.

The catalogued `body_bytes` value of 112 is correct for both functions: it
ends immediately before each two-word literal pool. Measuring from one entry
to the following prologue produces 120 bytes because that interval includes
the eight-byte pool; this is an extent measurement, not a larger instruction
body. Both 112-byte bodies are instruction-identical and have ROM SHA-256
`b83fa61a87daa62cbbdcc41c30b88064738c49345276aa1bf182cbf66522f683`.
Including the owned pools gives distinct 120-byte extent hashes:
`c0ca6b65707879ec721ba53c6e9d7426d497cd1b3f9d5b43757f315f186cd9c6`
for ease-in and
`dcff3af326c467d6da0820c3c57b76b15c5895060e0cb10206734c001bae38e5`
for ease-out.

## Datalink tween step and interpolation at 0x020ACD1C

| Range | Role |
| --- | --- |
| `0x020ACD10..0x020ACD1B` | Three-word callback pool owned by the preceding configuration helper, excluded. |
| `0x020ACD1C..0x020ACD73` | Complete 88-byte tween-step body. |
| `0x020ACD74..` | Next independent progress helper, excluded. |

`Game_UpdateDatalinkTween` advances the record's Q12 elapsed time by the
caller's delta through `Game_AdvanceDatalinkTween_020acc7c`, retaining that
helper's completion result. It then obtains the normalized Q12 progress from
`Game_GetDatalinkTweenProgress` and calls the interpolation callback stored at
record offset `+0x10`. The callback receives a temporary output vector, the
start vector at `+0x14`, target vector at `+0x1C`, and normalized progress.
Both temporary components are finally published to current position
`+0x30/+0x34`, and the earlier completion result is returned unchanged.

The sole recovered caller at `0x020ACC2C` invokes this helper with delta
`0x1000` whenever duration `+0x28` is nonzero. This is a caller relationship,
not a dependency on that helper's remaining frame-counter or callback-update
semantics, so no part of `0x020ACC2C` is included here. The body has no literal
pool: the preceding callback-address pool ends immediately before its prologue
and the next independent prologue begins directly at `0x020ACD74`.

Body SHA-256 from the ROM is
`f568a5c25242b15d7a52a83f8df0e685f27b2fa2472c5dcc287f79576673f9fe`.

## Datalink scene OAM visibility submission at 0x020AD9A4

| Range | Role |
| --- | --- |
| `0x020AD964..0x020AD9A3` | Preceding `Game_ApplyDatalinkSceneCommand`, excluded. |
| `0x020AD9A4..0x020ADABB` | Complete 280-byte OAM bounds and submission body. |
| `0x020ADABC..0x020ADABF` | One-word dimension-table pointer pool, excluded. |
| `0x020ADAC0..0x020ADB03` | Independent adapters and null stubs, excluded. |
| `0x020ADB04..` | Next catalogued function, excluded. |

`Game_SubmitVisibleDatalinkSceneOams` receives a scene, an eight-byte OAM
entry array and a signed count. It copies the canonical 12-pair DS OAM
dimension table from `0x020DE998`, selects shape and size from the first
entry, and uses that first entry's dimensions for the entire array. The
reduction preserves the target's minimum X, minimum right edge, minimum Y and
maximum bottom edge behavior.

Submission requires a nonnegative minimum right edge, at least one X origin
below 256, at least one Y origin below 192, a nonnegative maximum bottom edge,
and a strictly positive signed count. The manager pointer is read from scene
offset `+0x2540`; the submitted count is truncated to its low 16 bits exactly
as in the ARM body. The target reads entry zero before rejecting a nonpositive
count, so the recovered contract likewise requires a readable first entry for
zero or negative counts.

The first two signed edge checks are effectively defensive for the canonical
positive dimension table and hardware-masked coordinates. Unlike the earlier
cell-to-OAM helper, this function does not reject a maximum right or bottom
edge beyond the screen; visibility is decided by the reduced entry origins.

Body SHA-256 from the ROM is
`ef2c3709822bde786760fbd11f1ab02afc0d4c89df050b9ce67cb6b65c155ccf`.
Including the excluded pointer pool produces the 284-byte extent SHA-256
`fa053d356f7e967a73a9f819d7cc7654492a7e18cc3c112d9b87a5a7c8a428a8`.

## Datalink post-OAM scene and palette pipeline at 0x020ADB04

| Range | Recovered role |
| --- | --- |
| `0x020ADB04..0x020ADB63` | 96-byte scene-graphics initialization body. |
| `0x020ADB64..0x020ADB67` | Owned sub-display-control literal pool, excluded from `body_bytes`. |
| `0x020ADB68..0x020ADBAF` | 72-byte scene-graphics release body. |
| `0x020ADBB0..0x020ADBB7` | Owned release-resource and hardware-state pool, excluded. |
| `0x020ADBB8..0x020ADCE7` | 304-byte remote-participant OAM body. |
| `0x020ADCE8..0x020ADCFB` | Five-word participant/output/OAM-manager pool, excluded. |
| `0x020ADCFC..0x020ADF83` | Independent intervening function and its data, excluded. |
| `0x020ADF84..0x020ADFDB` | 88-byte pending-resource commit body. |
| `0x020ADFDC..0x020ADFE3` | Owned state/controller pool, excluded. |
| `0x020ADFE4..0x020ADFEF` | Independent address adapter and its literal, excluded. |
| `0x020ADFF0..0x020AE117` | 296-byte palette/object upload body. |
| `0x020AE118..0x020AE12F` | Six-word palette/object global pool, excluded. |
| `0x020AE130..0x020AE267` | 312-byte palette-wave update body. |
| `0x020AE268..0x020AE27F` | Six-word state, mask, palette and sine-table pool, excluded. |
| `0x020AE280..` | Next independent glyph-layout function, excluded. |

`Game_InitializeDatalinkPostOamSceneGraphics` configures the secondary scene,
loads resource pair `0x2B/0x2A`, clears the background target, selects mode 4,
and zeros runtime fields `+0x10/+0x14`. It then selects sub-display plane bit
`0x1000` while clearing `0x1F00` and starts the positive `0x2000` transition.
Its inverse, `Game_ReleaseDatalinkPostOamSceneGraphics`, releases resource
slots 0 through 3 and resource set `0x0217A258`. The negative `0x1000`
transition is skipped only when hardware-state bytes `+0xD0/+0xD1` are
exactly `1/2`.

`Game_RenderDatalinkRemoteParticipantOams` is gated by runtime availability
and the readiness subobject at runtime `+0x1F8`. It walks up to four session
participants, skips local index `selection[0x12] & 3`, and centers the remote
markers at Y 104 with 50-pixel spacing. Marker code/flag families are
`variant+3 / 8` for layout 1 or 4, `variant+11 / 16` for layout 2, and
`variant+7 / 0` otherwise. Each generated entry uses flag `0x80000000`, is
submitted immediately through OAM manager `0x0217A23C`, and contributes to
the output owner's counter at `+0x460`. This links the post-OAM stage to the
already recovered shared `Game_DatalinkOamEntry` and
`gGameDatalinkOamManager` contract.

The palette side uses a compact state at `0x02180DA8`: signed brightness and
step at `+0/+1`, pending resource size/address at `+4/+8`, palette-dirty flag
at `+0xC`, object cursor at `+0x10`, active flag at `+0x14`, and Q12-like
phase at `+0x18`. `Game_CommitDatalinkPostOamPendingResource` resets the
controller at `0x02180E24`, installs a nonzero pending address through runtime
context `+0x44`, clears the pending pair, and releases the runtime's former
handle from `+0x58`.

`Game_UploadDatalinkPostOamPaletteAndObjects` always copies 16 eight-color
rows from `0x02180EE0` to OBJ palette destinations beginning at `0x05000410`
with destination stride `0x20`. While active, it publishes phase-derived
hardware fields `+0xE0/+0xE4`. A dirty phase processes the runtime's raw
object-address list at `+0x64`, advances cursor `+0x10`, and builds a batch of
up to 32 objects. The callback in vtable slot `+0x0C` terminates collection;
the terminating object is consumed by the cursor but not included in the
batch count, matching the ARM loop. Once the cursor reaches runtime count
`+0x68`, the default resource at `0x021250D4` is bound and active state is
cleared. Palette records at `0x02180FE0` have stride `0x68` and are selected
by phase bits `12..15`.

`Game_UpdateDatalinkPostOamPaletteWave` advances signed brightness until it
reaches zero or either magnitude-64 limit and publishes brightness divided by
four to hardware state `+0xF8`. An active phase normally advances by `0x80`;
when runtime is unavailable and flag halfword `0x0212C584` has bit 3 set, it
performs eight such steps. Crossing a `0x1000` phase boundary marks the
palette dirty. The 16 output rows sample NitroSDK's interleaved signed Q12
`FX_SinCosTable_` at `0x020C91FC`, with 16 table halfwords per eight-bit phase
unit and a row phase offset of `-16`. The ARM `LSL 9; LSR 16` conversion is
preserved as unsigned wrapping before clamping to RGB5 level 31; each row is
filled with eight equal RGB555 colors.

The six recovered instruction bodies total 1,168 bytes. Their body SHA-256
values in address order are:

- `6c35b87ad98dfbfde0a75906c455bf7b45f96f01ff32c79f8879769d6d082cf3`
- `ac67116b539b6dddac8f235171c29edb41f8582258bfaf03d7cbafbc823eb30e`
- `f4a6cd9c19ec50bcb3b50de99479a773eb5cd71ba389e0b30d8da661b02c2c85`
- `972b92b3915d459696943170f80b0eca4a293f3ce3528aa2cb360f756d1aa705`
- `52ee0720542ee45437c876a8bfaa5bf68be16a14a1f56020472b9090ad0676d1`
- `76de1dec4e5076152bb12678541da54daf3a0e6022ac00964bf1d2fad747dc84`

Tests also pin every owned pool word and extent SHA, including the independent
`0x020ADFE4` adapter boundary, so pool bytes cannot be accidentally counted as
recovered instructions.

## Wireless peer-list maintenance at 0x020B07BC

| Range | Recovered role |
| --- | --- |
| `0x020B07BC..0x020B07FF` | 68-byte ready-prefix count; no pool. |
| `0x020B0800..0x020B09EF` | 496-byte peer-record update/insert body. |
| `0x020B09F0..0x020B09F3` | Pointer to the eight-word tween-index table at `0x020DED24`, excluded. |
| `0x020B09F4..0x020B0A67` | 116-byte peer-visual initialization body. |
| `0x020B0A68..0x020B0A6B` | Positioned-object-array pointer `0x0217EB2C`, excluded. |
| `0x020B0A6C..0x020B0B37` | 204-byte two-sprite arrangement body. |
| `0x020B0B38..0x020B0B3F` | Positioned-object and signed-offset globals, excluded. |
| `0x020B0B40..0x020B0C33` | 244-byte peer removal and compaction body. |
| `0x020B0C34..0x020B0C37` | Positioned-object-array pointer, excluded. |
| `0x020B0C38..0x020B0D53` | 284-byte stable ready-peer sort and relayout body. |
| `0x020B0D54..0x020B0D57` | Positioned-object-array pointer, excluded. |
| `0x020B0D58..0x020B0D7B` | 36-byte peer-list clear; no pool. |
| `0x020B0D7C..` | Next independent wireless refresh function, excluded. |

The seven functions share `Game_DatalinkPeerMenuState`, already established
by the caller at `0x020B0014`: four inline 0x20-byte records begin at `+0x34`,
the four ordered pointers begin at `+0xB4`, and the live count is at `+0xC8`.
A record stores its six-byte wireless identity, positioned-object index
`+0x06`, paired sprite index `+0x07`, active byte `+0x08`, descending sort key
`+0x09`, visual attribute `+0x0A`, refresh marker `+0x0C`, 14-byte label at
`+0x0E`, and discovery age at `+0x1C`.

`Game_CountDatalinkReadyPeers_020b07bc` counts only the leading ordered
records whose discovery age is at least 9. This is deliberately a ready
prefix rather than a count over the whole list, matching all caller-side
selection and rendering gates.

`Game_UpsertDatalinkPeerRecord_020b0800` compares all six identity bytes.
Refreshing an existing peer replaces its sort key, marks it seen, increments
its age, and rebuilds visuals only if attribute `+0x0A` changed. Insertion
selects the first inactive inline slot, zero-fills the 14-byte label before the
caller's exact-size copy, and assigns positioned-object indices from ROM table
`0x020DED24`: `23, 27, 29, 2B, 2D, 2F, 31, 33` hexadecimal. The first four
slots use paired sprite indices `0/1`, `2/3`, `4/5`, and `6/7`. A new record
is appended, sound `0x43` is emitted, and the ready prefix is sorted.

`Game_InitializeDatalinkPeerVisuals_020b09f4` activates both sprites, then
resets the record-selected 0x58-byte positioned object at `0x0217EB2C`:
duration zero, X `0x80000`, and Y `(visible_index * 40 + 48) << 12`. It then
delegates to `Game_ArrangeDatalinkPeerSprites_020b0a6c`. That arrangement
uses the sprite's OAM start index and layout fields `+0x2C/+0x38/+0x3C` with
`NNS_G2dArrangeOBJ2DRect`. The first and second sprite bases come from signed
global quartet `0x021263CC`: `(-72,-8)` and `(40,-8)`, respectively; the
positioned object's Q12 X/Y is added to each pair.

`Game_RemoveDatalinkPeerRecord_020b0b40` searches by record pointer. A match
clears active/seen, emits sound `0x44`, shifts later ordered pointers down,
and immediately resets and rearranges each shifted peer for its new row. The
count is decremented after compaction; the stale final pointer is intentionally
left untouched because `+0xC8` defines the live extent.

`Game_SortDatalinkPeerRecords_020b0c38` bubble-sorts only the ready prefix by
descending unsigned sort key. Equal keys remain stable because swapping is
strictly `right > left`. If any swap occurred, every ready record receives
the same row-position reset and paired-sprite arrangement. The function
re-queries the ready-prefix length through its loops as the ARM body does.
`Game_ClearDatalinkPeerList_020b0d58` sets the live count to zero first, then
clears only the four inline active bytes.

The seven instruction bodies total exactly 1,448 bytes. Their body SHA-256
values in address order are:

- `8241ecf25331215f60c019c1b50fe48421d53046e11d5e7583067351024001d2`
- `f09310e493b49692c5bb9999a507c8a9dc9bb4ed6055ac25b4d299c30a439ca0`
- `a6e0063b011f00c8ba76a3d91d94dedcf2f3a17fb3de7ceecb54081211ffeca5`
- `b30f9cc45a30879139b27553f46d3fa1c5e2eb1aca6bd82ba61949440966b37c`
- `42aa963fd4f9ac754833d37f01c68f2a3414363c9591ee70c65b305924635517`
- `96a3d4d89acbdc78cda1ae67a4522929d6e8797b2fbbb1d5fd5ff0e65d99a333`
- `3d75704dbc9dc4909284f997a447a8b8973d40b75083d59732ad9b45aed73588`

The ROM tests pin each pool word, every body/extent hash, both no-pool
boundaries, the full tween-index table, the signed sprite offsets, and the
independent `0x020B0D7C` prologue.
