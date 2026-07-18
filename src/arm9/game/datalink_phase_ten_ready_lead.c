#include "game/datalink_phase_ten_ready_lead.h"

#include <stddef.h>

enum {
    DATALINK_SELECTION_COUNT = 3,
    DATALINK_SELECTION_ROW_HEIGHT = 48,
    DATALINK_READY_TRANSITION_DURATION = 20,
    DATALINK_READY_TRANSITION_MODE = 2,
    DATALINK_FX_ONE = 0x1000
};

/* ROM tables at 0x020DE914 and 0x020DE918, read with an eight-byte stride. */
static const int32_t sDatalinkReadyLeadX[DATALINK_SELECTION_COUNT] = {
    9, 9, 9
};
static const int32_t sDatalinkReadyLeadY[DATALINK_SELECTION_COUNT] = {
    29, 77, 125
};

/* Opaque 0x020ACCA0 transition setup. */
extern void Game_StartDatalinkObjectTransition(
    Game_DatalinkPositionObject *object,
    int32_t target_x_fx,
    int32_t target_y_fx,
    int32_t duration,
    int mode
);

_Static_assert(
    offsetof(Game_DatalinkFlowState, selected_participant_40) == 0x40,
    "datalink selected participant offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, selection_dirty_43) == 0x43,
    "datalink selection dirty offset"
);

/*
 * Ready lead-transition unit 0x020A843C..0x020A84BF inside FUN_020A8198.
 *
 * The third BL instruction is physically at 0x020A84BC, so the coherent unit
 * ends at 0x020A84BF; the following fanout begins at 0x020A84C0.
 *
 * The signed selection at entry determines the shared Y target (row*48) for
 * objects 0x0217FE14 and 0x0217FE6C.  The +0x43 dirty byte is set before either
 * call.  Selection is loaded again after those calls to index the ROM X/Y
 * tables for object 0x0217EC34, while the Y adjustment still uses the entry
 * selection.  All three transitions use ROM duration 20 and mode two.
 */
void Game_StartDatalinkPhaseTenReadyLeadTransitions(
    Game_DatalinkFlowState *state
)
{
    int initial_selection = state->selected_participant_40;
    int32_t row_offset = initial_selection * DATALINK_SELECTION_ROW_HEIGHT;
    int table_selection;

    state->selection_dirty_43 = 1;
    Game_StartDatalinkObjectTransition(
        &gGameDatalinkPhaseFiveGateObject,
        0,
        row_offset * DATALINK_FX_ONE,
        DATALINK_READY_TRANSITION_DURATION,
        DATALINK_READY_TRANSITION_MODE
    );
    Game_StartDatalinkObjectTransition(
        &gGameDatalinkPhaseTenReadySecondLeadObject,
        0,
        row_offset * DATALINK_FX_ONE,
        DATALINK_READY_TRANSITION_DURATION,
        DATALINK_READY_TRANSITION_MODE
    );

    table_selection = state->selected_participant_40;
    Game_StartDatalinkObjectTransition(
        &gGameDatalinkPhaseTenReadyThirdLeadObject,
        sDatalinkReadyLeadX[table_selection] * DATALINK_FX_ONE,
        (sDatalinkReadyLeadY[table_selection] - row_offset) *
            DATALINK_FX_ONE,
        DATALINK_READY_TRANSITION_DURATION,
        DATALINK_READY_TRANSITION_MODE
    );
}
