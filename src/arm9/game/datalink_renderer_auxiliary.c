#include "game/datalink_renderer_auxiliary.h"

#include <stddef.h>

enum {
    DATALINK_AUXILIARY_SKIP_PARTICIPANT_TYPE = 1
};

/* Opaque state+0x0C[index] target-type lookup used elsewhere in Datalink. */
extern int Game_GetDatalinkParticipantType(
    const Game_DatalinkFlowState *state,
    int participant_index
);

/* Opaque 0x0209CF68 per-row auxiliary refresh. */
extern void Game_RefreshDatalinkRowAuxiliary(int slot);

_Static_assert(
    offsetof(Game_DatalinkFlowState, active_participant_index_2c) == 0x2C,
    "datalink auxiliary active participant offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, selected_participant_40) == 0x40,
    "datalink auxiliary selected participant offset"
);

/*
 * Semantic internal helper for the conditional operation
 * Range: 0x020A8C80..0x020A8CBB inside catalogued FUN_020A8904.  Parent loop
 * increment and branch at 0x020A8CBC..0x020A8CC7 remain outside this unit.
 *
 * A fresh signed selection read is compared with the current slot.  The
 * selected row refreshes immediately without resolving participant type.  A
 * non-selected row performs a fresh signed active-index read and skips only
 * when its target type equals one.  This helper is not promoted in Ghidra.
 */
void Game_RefreshDatalinkRowAuxiliaryIfNeeded(
    Game_DatalinkFlowState *state,
    uint8_t slot
)
{
    const volatile int8_t *selected = &state->selected_participant_40;
    const volatile int8_t *active_index =
        &state->active_participant_index_2c;

    if (*selected != slot &&
        Game_GetDatalinkParticipantType(state, *active_index) ==
            DATALINK_AUXILIARY_SKIP_PARTICIPANT_TYPE) {
        return;
    }

    Game_RefreshDatalinkRowAuxiliary(slot);
}
