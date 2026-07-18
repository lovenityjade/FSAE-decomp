#include "game/datalink_phase_five.h"

#include <stddef.h>

enum {
    DATALINK_PHASE_SIX = 6,
    DATALINK_TRANSITION_COMPLETE_FX = 0x1000,
    DATALINK_DISPLAY_FIELD_VALUE = 0x1F00,
    DATALINK_TRANSITION_MODE = 2,
    DATALINK_FX_ONE = 0x1000
};

static const uint32_t sDatalinkDisplayFieldClearMask = 0xFFFFE0FFU;

/* Opaque 0x020ACD74 transition-progress query. */
extern int32_t Game_GetDatalinkObjectTransitionProgressFx(
    const Game_DatalinkPositionObject *object
);

/* Opaque 0x020ACCA0 transition setup. */
extern void Game_StartDatalinkObjectTransition(
    Game_DatalinkPositionObject *object,
    int32_t target_x_fx,
    int32_t target_y_fx,
    int32_t duration,
    int mode
);

_Static_assert(
    offsetof(Game_DatalinkFlowState, phase_41) == 0x41,
    "datalink flow phase offset"
);

/*
 * Phase-five unit 0x020A82BC..0x020A833F inside FUN_020A8198.
 *
 * Every frame first replaces display-control bits 8..12 with 0x1F, then
 * queries the transition object at target address 0x0217FE14.  Any progress
 * other than exactly 0x1000 leaves phase five active.  Completion starts the
 * following two mode-two transitions in order, using the duration stored via
 * target address 0x0217EC3C:
 *
 * - object 0x0217EC8C to ROM coordinates (255, 0);
 * - object 0x0217F314 to ROM coordinates (24, 180).
 *
 * The phase byte changes to six only after both setup calls return.
 */
void Game_UpdateDatalinkPhaseFive(Game_DatalinkFlowState *state)
{
    int32_t duration;

    gGameDatalinkDisplayControl =
        (gGameDatalinkDisplayControl & sDatalinkDisplayFieldClearMask) |
        DATALINK_DISPLAY_FIELD_VALUE;

    if (Game_GetDatalinkObjectTransitionProgressFx(
            &gGameDatalinkPhaseFiveGateObject
        ) != DATALINK_TRANSITION_COMPLETE_FX) {
        return;
    }

    duration = gGameDatalinkPhaseFiveTransitionDuration;
    Game_StartDatalinkObjectTransition(
        &gGameDatalinkPhaseFiveFirstObject,
        255 * DATALINK_FX_ONE,
        0,
        duration,
        DATALINK_TRANSITION_MODE
    );
    Game_StartDatalinkObjectTransition(
        &gGameDatalinkPhaseFiveSecondObject,
        24 * DATALINK_FX_ONE,
        180 * DATALINK_FX_ONE,
        duration,
        DATALINK_TRANSITION_MODE
    );
    state->phase_41 = DATALINK_PHASE_SIX;
}
