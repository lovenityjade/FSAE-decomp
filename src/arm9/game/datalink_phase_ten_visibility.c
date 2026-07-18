#include "game/datalink_phase_ten_visibility.h"

#include <stddef.h>

enum {
    DATALINK_VISIBILITY_ROW_COUNT = 3
};

/* Six-byte map copied from ROM address 0x020DE7D0 by the parent preamble. */
static const uint8_t sDatalinkVisibilityPairs
    [DATALINK_VISIBILITY_ROW_COUNT][2] = {
        {5, 6},
        {7, 8},
        {9, 10}
    };

_Static_assert(
    offsetof(Game_DatalinkFlowState, selected_participant_40) == 0x40,
    "datalink selected participant offset"
);
_Static_assert(
    sizeof(Game_DatalinkPositionObject) == 0x58,
    "datalink positioned-object stride"
);
_Static_assert(
    offsetof(Game_DatalinkPositionObject, visible_03) == 0x03,
    "datalink object visibility offset"
);

static void Game_HideDatalinkObject(int object_index)
{
    volatile uint8_t *visibility =
        &gGameDatalinkPositionObjects[object_index].visible_03;

    *visibility = 0;
}

/*
 * Extracted phase-ten visibility unit 0x020A8570..0x020A85C3 inside the
 * catalogued function FUN_020A8198.  This helper is not a promoted Ghidra
 * function boundary.
 *
 * Rows zero through two are visited in ascending order.  The signed selected
 * byte at state+0x40 is reloaded for every comparison.  For each non-selected
 * row, byte +0x03 is cleared in this exact order: the primary object (0..2),
 * then the first and second mapped objects (5..10).  The selected row and
 * unrelated objects three/four are untouched.
 */
void Game_HideUnselectedDatalinkRows(Game_DatalinkFlowState *state)
{
    const volatile int8_t *selected_participant =
        &state->selected_participant_40;
    int row;

    for (row = 0; row < DATALINK_VISIBILITY_ROW_COUNT; ++row) {
        if (row != *selected_participant) {
            Game_HideDatalinkObject(row);
            Game_HideDatalinkObject(sDatalinkVisibilityPairs[row][0]);
            Game_HideDatalinkObject(sDatalinkVisibilityPairs[row][1]);
        }
    }
}
