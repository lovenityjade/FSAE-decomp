#include "game/datalink_phase_six.h"

#include <stddef.h>

enum {
    DATALINK_PHASE_TEN = 10,
    DATALINK_PHASE_THIRTY = 30,
    DATALINK_PHASE_THIRTY_TIMER = 180,
    DATALINK_TRANSITION_COMPLETE_FX = 0x1000
};

/* Opaque 0x020ACD74 transition-progress query. */
extern int32_t Game_GetDatalinkObjectTransitionProgressFx(
    const Game_DatalinkPositionObject *object
);

/*
 * 0x020A76D0 scans the three global Datalink slot outcomes.  It remains an
 * opaque dependency and does not consume the state pointer left in r0 by the
 * caller decompilation.
 */
extern int Game_ProcessDatalinkSlotOutcome(void);

_Static_assert(
    offsetof(Game_DatalinkFlowState, phase_41) == 0x41,
    "datalink flow phase offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, phase_timer_45) == 0x45,
    "datalink phase timer offset"
);

/*
 * Phase-six unit 0x020A8340..0x020A8377 inside FUN_020A8198.
 *
 * Progress for the first transition created by phase five is compared
 * exactly with 0x1000.  Incomplete progress has no other effects.  On
 * completion the phase is written to ten before 0x020A76D0 runs.  A zero
 * outcome leaves phase ten active; any nonzero outcome replaces it with
 * phase thirty and initializes the +0x45 countdown to 180, in that order.
 */
void Game_UpdateDatalinkPhaseSix(Game_DatalinkFlowState *state)
{
    if (Game_GetDatalinkObjectTransitionProgressFx(
            &gGameDatalinkPhaseFiveFirstObject
        ) != DATALINK_TRANSITION_COMPLETE_FX) {
        return;
    }

    state->phase_41 = DATALINK_PHASE_TEN;
    if (Game_ProcessDatalinkSlotOutcome() != 0) {
        state->phase_41 = DATALINK_PHASE_THIRTY;
        state->phase_timer_45 = DATALINK_PHASE_THIRTY_TIMER;
    }
}
