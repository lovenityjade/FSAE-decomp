# Datalink state machine at 0x020A8198

## Function boundary

- Catalog/Ghidra body: `0x020A8198..0x020A883B` (`1700` bytes).
- Literal/data pool: `0x020A883C..0x020A88A7` (`27` words).
- Next catalogued game function: `0x020A88A8` (`88` bytes).

The function is a frame-driven state machine. Its phase byte is at state
offset `+0x41`; selection, redraw, and countdown fields occur at `+0x40`,
`+0x42/+0x43`, and `+0x45` respectively. It should be recovered as phase
handlers plus a common render tail rather than as one monolithic C function.

## CFG regions

| Region | Purpose |
| --- | --- |
| `0x020A8198..0x020A81F7` | Common preamble: copy the six object-map bytes and invoke the active participant callback. |
| `0x020A81F8..0x020A826F` | Phase decode, 0..10 jump table, and routing for phases 15, 16, 30, and 31. |
| `0x020A8270..0x020A82BB` | Phases 0/1: choose mode, set phase 5, and conditionally load resource `0x0834`. |
| `0x020A82BC..0x020A833F` | Phase 5: update display bits, wait for a transition, then create two object transitions and enter phase 6. |
| `0x020A8340..0x020A8377` | Phase 6: wait for transition completion, enter phase 10, and optionally divert to phase 30 with timer 180. |
| `0x020A8378..0x020A86AF` | Phase 10: input/selection handler, transition creation, object visibility updates, and selection redraw. |
| `0x020A86B0..0x020A86DF` | Phase 15: wait for the primary transition, set completion flags, and rebuild the screen. |
| `0x020A86E0..0x020A8713` | Phase 16: wait for the primary transition, push result `1`, and queue the next transition. |
| `0x020A8714..0x020A8793` | Phase 30: update a fixed object position, count down, start command `0x35`, and enter phase 31. |
| `0x020A8794..0x020A87C7` | Phase 31: run the slot cleanup at timer 15, count down, and return to phase 6 at zero. |
| `0x020A87C8..0x020A883B` | Common render tail: command 4, optional redraw, three selection commands, then command `0x17`. |

Phases 2..4 and 7..9 route directly to the common tail. Other unhandled phase
values between the explicitly routed high phases also perform only that tail.

### Phase-10 subdivision

| Region | Purpose |
| --- | --- |
| `0x020A8378..0x020A83F7` | Participant guard and input decode, including directional selection writes. |
| `0x020A83F8..0x020A8413` | Input 2 cancellation: mode 3, sound `0x44`, phase 15. |
| `0x020A8414..0x020A843B` | Input 1 confirmation prelude: sound `0x3E`, selection mirrors, readiness query. |
| `0x020A843C..0x020A84BF` | Ready-slot lead transitions and dirty-flag write; third BL is at `0x020A84BC`. |
| `0x020A84C0..0x020A856F` | Ready-slot three-way object-transition fanout. |
| `0x020A8570..0x020A85C3` | Ready-slot visibility cleanup for non-selected rows. |
| `0x020A85C4..0x020A85D3` | Ready-slot transition queue and branch to the common finalizer. |
| `0x020A85D4..0x020A85E3` | Unavailable-slot header: phase 16 and opaque SDK mode-2 wrapper. |
| `0x020A85E4..0x020A861B` | Unavailable-slot first transition. |
| `0x020A861C..0x020A864B` | Unavailable-slot second transition. |
| `0x020A864C..0x020A86AF` | Shared selection clamp, optional sound `0x3D`, cursor position and command 3. |

## Direct dependencies

- Datalink units: `0x020A8138`, `0x020A8148`, `0x020A8168`, `0x020A8178`,
  `0x020A88A8`, and `0x020A8904`.
- State/slot helpers: `0x020A7324`, `0x020A76D0`.
- Transition/object helpers: `0x020AC898`, `0x020ACCA0`, `0x020ACD74`,
  `0x020ACDD8`, `0x020AD964`.
- Screen/system helpers: `0x02053258`, `0x02053340`, `0x02076FE0`,
  `0x020AC038`, and `0x020AC308`.
- The SDK-classified wrapper at `0x020A8158` remains opaque and is not
  promoted into the recovered game module.

## Literal pool

| Address | Value | Observed role |
| --- | --- | --- |
| `0x020A883C` | `0x020DE7D0` | Six-byte object-pair map. |
| `0x020A8840` | `0x00000834` | Initial screen resource ID. |
| `0x020A8844` | `0x04001000` | Hardware display register. |
| `0x020A8848` | `0x0217FE14` | Transition object. |
| `0x020A884C` | `0x020DE864` | Coordinate data. |
| `0x020A8850` | `0x0217EC3C` | Transition parameter pointer. |
| `0x020A8854` | `0x0217EC8C` | Transition object. |
| `0x020A8858` | `0x020DE84C` | Coordinate data. |
| `0x020A885C` | `0x0217F314` | Transition object. |
| `0x020A8860` | `0x0212C510` | Input halfword. |
| `0x020A8864` | `0x0217E6F0` | Datalink scene owner. |
| `0x020A8868` | `0x02171DF8` | Selection mirror. |
| `0x020A886C` | `0x020DE838` | Transition parameter pointer. |
| `0x020A8870` | `0x0217FE6C` | Transition object. |
| `0x020A8874` | `0x020DE918` | Y-coordinate table. |
| `0x020A8878` | `0x020DE914` | X-coordinate table. |
| `0x020A887C` | `0x0217EC34` | Transition object. |
| `0x020A8880` | `0x0217EB2C` | Positioned-object array. |
| `0x020A8884` | `0x020DE8FC` | Three-object coordinates. |
| `0x020A8888` | `0x020DE92C` | Paired-object coordinates. |
| `0x020A888C` | `0x0217E844` | Transition parameter pointer. |
| `0x020A8890` | `0x0217E834` | Y-offset pointer. |
| `0x020A8894` | `0x0217E840` | Y-offset pointer. |
| `0x020A8898` | `0x0217E6F0` | Datalink scene owner (duplicate). |
| `0x020A889C` | `0x0217D23C` | Completion flag owner. |
| `0x020A88A0` | `0x020DE854` | Phase-30 coordinate pair. |
| `0x020A88A4` | `0x0217F6F0` | Phase-30 object owner. |

## Recovered units

`Game_EnterDatalinkInitialPhase` recovers only the phase-0/phase-1 region.
Its pointer graph lookup and the screen loader remain opaque dependencies.

`Game_UpdateDatalinkPhaseFive` recovers `0x020A82BC..0x020A833F`. It preserves
the display-register write on every frame, exact `0x1000` progress gate, two
ordered mode-two transitions, and the final phase-six write.

`Game_UpdateDatalinkPhaseSix` recovers `0x020A8340..0x020A8377`. It preserves
the exact progress gate, writes phase ten before processing the slot outcome,
and only then may divert to phase thirty with timer 180.

`Game_CancelDatalinkPhaseTen` recovers the input-2 action at
`0x020A83F8..0x020A8413`: mode three, sound `0x44`, then phase fifteen. Its
continuation into the shared finalizer remains a separate unit.

`Game_PrepareDatalinkPhaseTenConfirmation` recovers
`0x020A8414..0x020A843B`: sound `0x3E`, post-sound unsigned selection read,
two ordered selection mirrors, and the readiness branch predicate.

`Game_StartDatalinkPhaseTenReadyLeadTransitions` recovers the corrected
coherent boundary `0x020A843C..0x020A84BF`: dirty flag, three ordered
transitions, entry-selection row offset, and post-call selection reload.

`Game_StartDatalinkPhaseTenReadyFanout` recovers
`0x020A84C0..0x020A856F`: three rows of primary/paired calls, fixed pair map,
captured entry-selection offset, and nine ordered mode-two transitions.

`Game_HideUnselectedDatalinkRows` semantically extracts
`0x020A8570..0x020A85C3` without promoting an internal Ghidra function. It
reloads the signed selection for each row and clears primary/paired visibility
bytes for every non-selected row.

`Game_QueueDatalinkPhaseTenReadyTransition` semantically extracts
`0x020A85C4..0x020A85D3`: capture state+0x38, one opaque transition-stack
call, then continuation to the shared finalizer. It is not registered as a
Ghidra function.

The unavailable branch is safely divided into its phase/mode header at
`0x020A85D4..0x020A85E3`, first transition at `0x020A85E4..0x020A861B`, and
second transition at `0x020A861C..0x020A864B`.

`Game_BeginDatalinkPhaseTenUnavailable` and the two
`Game_StartDatalinkPhaseTenUnavailable*Transition` helpers recover those
three regions without adding Ghidra symbols. The SDK mode-two wrapper remains
opaque; duration is captured after it, and the second Y offset is loaded only
after the first transition returns.

The next phase-ten unit is the shared selection finalizer at
`0x020A864C..0x020A86AF`.

`Game_FinalizeDatalinkPhaseTenSelection` semantically extracts that finalizer
without adding a Ghidra symbol. It preserves the clamp, first selection reload
for sound `0x3D`, second post-sound reload for ROM coordinates, ordered cursor
writes, and final scene command three.

Phase ten is now closed.

`Game_UpdateDatalinkPhaseFifteen` semantically extracts
`0x020A86B0..0x020A86DF`. The block remains inside `FUN_020A8198`, so the
helper is not promoted to the Ghidra function registry. It preserves the
transition-completion gate, two ordered completion-flag writes, resource byte
`0x83`, and the final no-argument screen rebuild call.

Phase fifteen is now closed. The next state handler is phase 16 at
`0x020A86E0..0x020A8713`.

`Game_UpdateDatalinkPhaseSixteen` semantically extracts that handler without
promoting its internal block to the Ghidra function registry. It preserves the
transition-completion gate, post-increment of the return-phase depth, write of
phase one at the old stack index, subsequent `+0x3C` transition-address load,
and final scene transition push.

Phase sixteen is now closed. The next state handler is phase 30 at
`0x020A8714..0x020A8793`. The dispatch target at `0x020A8268` proves that the
instruction at `0x020A8794` is already the phase-31 entry, correcting the
earlier overlapping region boundary.

`Game_UpdateDatalinkPhaseThirty` semantically extracts the corrected phase-30
region without promoting its internal block to the Ghidra function registry.
It preserves the three fixed-position writes on every frame, unsigned timer
countdown, command `0x35`, post-command volatile advance-bit read, ordered
screen/state calls, timer 60, and final phase-31 write.

Phase thirty is now closed. The next state handler is phase 31 at
`0x020A8794..0x020A87C7`.

`Game_UpdateDatalinkPhaseThirtyOne` semantically extracts that corrected
region without promoting its internal block to the Ghidra function registry.
It preserves the first timer read that gates the true cleanup function at
`0x020A88A8`, the distinct post-cleanup timer reload, decrement-and-return for
every nonzero value, and the ordered phase-six write before screen commit at
zero.

Phase thirty-one is now closed. The remaining region of `FUN_020A8198` is the
common render tail at `0x020A87C8..0x020A883B`.

`Game_RenderDatalinkCommonTail` semantically extracts that final internal
region without promoting it to the Ghidra function registry. It preserves
command four before the volatile redraw-suppression read, the optional early
return, refresh call `0x020A8904`, three fresh signed selection reads and row
commands, and final command `0x17`.

Every coherent region of catalogued function `FUN_020A8198` is now covered by
the semantic recovery units in this document. The next true function boundary
is `0x020A88A8`.

## Neighbor boundary audit

The neighboring `FUN_020A88A8` boundary is structurally secure: its prologue
is at `0x020A88A8`, its epilogue and return are at `0x020A88FC`, and its sole
literal at `0x020A8900` is `0x021350B0`. The catalog reports an 88-byte body.
`0x020A8904` starts a distinct catalogued 1104-byte function with a new
prologue.

Semantic call and field analysis now supports the descriptive integration
candidate `Game_CleanupDatalinkPlayerSlots` for `0x020A88A8`. It visits the
three active 0x50-byte slot records, persists status six through `0x02056998`,
resets statuses six/eight through the already recovered
`Game_ResetDatalinkPlayerSlot`, clears each handled +0x47 status byte, and
finally invokes persistent-state commit `0x02056738`. The proposed confidence
is `descriptive`; no Ghidra registry or semantic-symbol configuration change
is applied by this recovery unit.

The larger true function at `0x020A8904` is bounded and subdivided, but not yet
recovered, in `docs/arm9-datalink-8904-map.md`.
