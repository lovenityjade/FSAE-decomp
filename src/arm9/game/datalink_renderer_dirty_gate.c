#include "game/datalink_renderer_dirty_gate.h"

#include <stddef.h>
#include <stdint.h>

_Static_assert(
    offsetof(Game_DatalinkFlowState, selection_dirty_43) == 0x43,
    "datalink renderer dirty flag offset"
);

/*
 * Semantic internal predicate for 0x020A8960..0x020A8973 inside the
 * catalogued FUN_020A8904.  The target performs one unsigned byte read from
 * state+0x43.  Zero returns from the parent renderer; every nonzero value
 * continues into the rebuild path.  Clearing the flag belongs to the next
 * region.  This helper is not a promoted Ghidra function.
 */
bool Game_ShouldRebuildDatalinkRender(
    const Game_DatalinkFlowState *state
)
{
    const volatile uint8_t *dirty = &state->selection_dirty_43;

    return *dirty != 0U;
}
