#include "game/datalink_initial_phase.h"

#include "game/datalink_mode_dispatch.h"

#include <stddef.h>

enum {
    DATALINK_INITIAL_PHASE_ZERO = 0,
    DATALINK_INITIAL_PHASE_ONE = 1,
    DATALINK_LOADING_PHASE = 5,
    DATALINK_SCREEN_RESOURCE_ID = 0x0834,
    DATALINK_PARTICIPANT_TYPE_WITHOUT_RELOAD = 1
};

/*
 * The original follows state+0x0C[index] and reads the target word at +0x04.
 * That target-owned pointer graph remains opaque to this extracted handler.
 */
extern int Game_GetDatalinkParticipantType(
    const Game_DatalinkFlowState *state,
    int participant_index
);

/* External helper 0x02053258 rebuilds the screen resources selected by the 16-bit ID. */
extern void Game_LoadDatalinkScreenResource(uint16_t resource_id);

_Static_assert(
    offsetof(Game_DatalinkFlowState, active_participant_index_2c) == 0x2C,
    "datalink active participant index offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, phase_41) == 0x41,
    "datalink flow phase offset"
);

/*
 * First extracted logical unit inside 0x020A8198:
 * state-zero path 0x020A8270..0x020A828B and state-one path
 * 0x020A828C..0x020A82BB, sharing the resource call at 0x020A8284.
 *
 * The caller dispatches here only when phase_41 is zero or one.  Both paths
 * select their corresponding opaque mode wrapper and advance to phase five.
 * Phase zero always loads resource 0x0834.  Phase one skips that load exactly
 * when the active participant's target word at +0x04 equals one.
 */
void Game_EnterDatalinkInitialPhase(Game_DatalinkFlowState *state)
{
    if (state->phase_41 == DATALINK_INITIAL_PHASE_ZERO) {
        Game_EnterDatalinkModeZero(state);
        state->phase_41 = DATALINK_LOADING_PHASE;
        Game_LoadDatalinkScreenResource(DATALINK_SCREEN_RESOURCE_ID);
        return;
    }

    Game_EnterDatalinkModeOne(state);
    state->phase_41 = DATALINK_LOADING_PHASE;
    if (Game_GetDatalinkParticipantType(
            state,
            state->active_participant_index_2c
        ) != DATALINK_PARTICIPANT_TYPE_WITHOUT_RELOAD) {
        Game_LoadDatalinkScreenResource(DATALINK_SCREEN_RESOURCE_ID);
    }
}
