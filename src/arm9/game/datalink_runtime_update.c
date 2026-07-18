#include "game/datalink_runtime_update.h"

#include <stddef.h>

/* Calls virtual method +0x0C on the interface stored at the target address. */
extern void Game_UpdateDatalinkInterface(uint32_t interface_address);

extern void Game_TriggerDatalinkPendingEffect(
    int event_id,
    uint32_t argument_14,
    int zero,
    uint32_t argument_10
);

/* 0x020A777C processes the runtime state before the final interface update. */
extern void Game_ProcessDatalinkRuntimeState(
    Game_DatalinkRuntimeUpdateState *state
);

_Static_assert(
    offsetof(Game_DatalinkRuntimeUpdateState, interfaces_0c) == 0x0C,
    "Datalink runtime interface array offset"
);
_Static_assert(
    offsetof(Game_DatalinkRuntimeUpdateState, current_interface_2c) == 0x2C,
    "Datalink runtime current interface offset"
);
_Static_assert(
    offsetof(Game_DatalinkRuntimeUpdateState, interface_b_38) == 0x38,
    "Datalink runtime interface B offset"
);
_Static_assert(
    offsetof(Game_DatalinkRuntimeUpdateState, state_41) == 0x41,
    "Datalink runtime state byte offset"
);
_Static_assert(
    offsetof(Game_DatalinkRuntimeUpdateState, state_43) == 0x43,
    "Datalink runtime processed flag offset"
);
_Static_assert(
    offsetof(Game_DatalinkRuntimeUpdateState, pending_count_48) == 0x48,
    "Datalink runtime pending count offset"
);
_Static_assert(
    offsetof(Game_DatalinkRuntimeUpdateState, pending_values_4c) == 0x4C,
    "Datalink runtime pending value stack offset"
);

/*
 * 0x020A761C..0x020A76C7 (Ghidra/catalog body size: 172 bytes).
 * Literal pool: 0x020A76C8..0x020A76CF; next function: 0x020A76D0.
 *
 * If global byte 0x0217D30C is nonzero, only virtual method +0x0C on
 * interface B (+0x38) is called.  Otherwise:
 * - an empty pending stack clears +0x41;
 * - a nonempty stack is popped from +0x4C in LIFO order, its low byte is
 *   stored at +0x41, and 0x0209DAA0 is invoked with event 1;
 * - +0x43 is set to one and 0x020A777C processes the runtime;
 * - the signed byte +0x2C is read after processing and selects one of the
 *   eight interface addresses beginning at +0x0C for virtual update +0x0C.
 */
void Game_UpdateDatalinkRuntime(Game_DatalinkRuntimeUpdateState *state)
{
    if (gGameDatalinkAbortUpdate != 0U) {
        Game_UpdateDatalinkInterface(state->interface_b_38);
        return;
    }

    if (state->pending_count_48 == 0U) {
        state->state_41 = 0;
    } else {
        uint32_t pending_index = state->pending_count_48 - 1U;

        state->pending_count_48 = pending_index;
        state->state_41 = (uint8_t)state->pending_values_4c[pending_index];
        Game_TriggerDatalinkPendingEffect(
            1,
            gGameDatalinkEffectArgument14,
            0,
            gGameDatalinkEffectArgument10
        );
    }

    state->state_43 = 1;
    Game_ProcessDatalinkRuntimeState(state);
    Game_UpdateDatalinkInterface(
        state->interfaces_0c[state->current_interface_2c]
    );
}
