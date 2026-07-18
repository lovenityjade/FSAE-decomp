#include "game/datalink_phase_ten_unavailable.h"

#include <stddef.h>

enum {
    DATALINK_UNAVAILABLE_PHASE = 16,
    DATALINK_UNAVAILABLE_TRANSITION_MODE = 1,
    DATALINK_FX_ONE = 0x1000
};

/*
 * SDK-classified wrapper 0x020A8158.  It tail-dispatches selector two to the
 * opaque shared target and is deliberately not promoted into a game module.
 */
extern void Game_EnterOpaqueSdkDatalinkModeTwo(void *state);

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
 * Extracted unavailable header 0x020A85D4..0x020A85E3 inside FUN_020A8198.
 * This semantic helper is not a promoted Ghidra function boundary.
 */
void Game_BeginDatalinkPhaseTenUnavailable(Game_DatalinkFlowState *state)
{
    state->phase_41 = DATALINK_UNAVAILABLE_PHASE;
    Game_EnterOpaqueSdkDatalinkModeTwo(state);
}

/*
 * Extracted first transition 0x020A85E4..0x020A861B.
 *
 * Duration is loaded through target address 0x0217E844 after the SDK wrapper.
 * Object 0x0217EC8C moves to ROM X=255 and ROM Y=0 plus the dynamic offset at
 * 0x0217E834.  The captured duration is returned for the second sub-unit.
 */
int32_t Game_StartDatalinkPhaseTenUnavailableFirstTransition(void)
{
    int32_t captured_duration =
        gGameDatalinkUnavailableTransitionDuration;
    int32_t target_y = gGameDatalinkUnavailableFirstYOffset;

    Game_StartDatalinkObjectTransition(
        &gGameDatalinkPhaseFiveFirstObject,
        255 * DATALINK_FX_ONE,
        target_y * DATALINK_FX_ONE,
        captured_duration,
        DATALINK_UNAVAILABLE_TRANSITION_MODE
    );
    return captured_duration;
}

/*
 * Extracted second transition 0x020A861C..0x020A864B.
 *
 * Its dynamic Y offset at 0x0217E840 is loaded only after the first transition
 * returns.  Object 0x0217F314 moves to ROM position (24,180)+offset, reusing
 * the duration captured before the first call.
 */
void Game_StartDatalinkPhaseTenUnavailableSecondTransition(
    int32_t captured_duration
)
{
    int32_t target_y = 180 + gGameDatalinkUnavailableSecondYOffset;

    Game_StartDatalinkObjectTransition(
        &gGameDatalinkPhaseFiveSecondObject,
        24 * DATALINK_FX_ONE,
        target_y * DATALINK_FX_ONE,
        captured_duration,
        DATALINK_UNAVAILABLE_TRANSITION_MODE
    );
}

/* Semantic composition of the three internal regions; not a Ghidra symbol. */
void Game_HandleDatalinkPhaseTenUnavailable(Game_DatalinkFlowState *state)
{
    int32_t captured_duration;

    Game_BeginDatalinkPhaseTenUnavailable(state);
    captured_duration =
        Game_StartDatalinkPhaseTenUnavailableFirstTransition();
    Game_StartDatalinkPhaseTenUnavailableSecondTransition(captured_duration);
}
