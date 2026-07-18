#include "game/datalink_phase_thirty.h"

#include <stddef.h>

enum {
    DATALINK_PHASE_THIRTY_POSITION_X = 224,
    DATALINK_PHASE_THIRTY_POSITION_Y = 192,
    DATALINK_FX_ONE = 0x1000,
    DATALINK_PHASE_THIRTY_SCENE_COMMAND = 0x35,
    DATALINK_PHASE_THIRTY_ADVANCE_FLAG = 1,
    DATALINK_PHASE_THIRTY_NEXT_SYSTEM_STATE = 15,
    DATALINK_PHASE_THIRTY_NEXT_TIMER = 60,
    DATALINK_PHASE_THIRTY_NEXT_PHASE = 31
};

/* Opaque 0x020AD964 scene-object command. */
extern void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command
);

/* Opaque 0x020AC308 screen-update commit. */
extern void Game_CommitDatalinkScreenUpdate(void);

/* Opaque 0x020AC038 surrounding state change. */
extern void Game_ChangeDatalinkState(
    int state,
    int participant_count,
    int argument_2,
    int argument_3
);

_Static_assert(
    offsetof(
        Game_DatalinkPhaseThirtyObjectOwner,
        position_transition_duration_69c
    ) == 0x69C,
    "datalink phase thirty position duration offset"
);
_Static_assert(
    offsetof(Game_DatalinkPhaseThirtyObjectOwner, position_x_fx_6a4) ==
        0x6A4,
    "datalink phase thirty X offset"
);
_Static_assert(
    offsetof(Game_DatalinkPhaseThirtyObjectOwner, position_y_fx_6a8) ==
        0x6A8,
    "datalink phase thirty Y offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, phase_41) == 0x41,
    "datalink flow phase offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, phase_timer_45) == 0x45,
    "datalink phase timer offset"
);

/*
 * Semantic phase-thirty helper for 0x020A8714..0x020A8793 inside the
 * catalogued FUN_020A8198.  This block is not a Ghidra function boundary and
 * must not be promoted in the function registry.  The instruction at
 * 0x020A8794 is already the phase-thirty-one entry.
 *
 * The fixed object position is rewritten on every frame.  A nonzero unsigned
 * timer is then decremented and skips all commands.  At zero, scene command
 * 0x35 runs before the volatile advance halfword is read.  Bit zero gates the
 * ordered screen commit and state-change calls.  The timer is written to 60
 * before the phase advances to 31.
 */
void Game_UpdateDatalinkPhaseThirty(Game_DatalinkFlowState *state)
{
    volatile int32_t *position_duration =
        &gGameDatalinkPhaseThirtyObjectOwner
             .position_transition_duration_69c;
    volatile int32_t *position_x =
        &gGameDatalinkPhaseThirtyObjectOwner.position_x_fx_6a4;
    volatile int32_t *position_y =
        &gGameDatalinkPhaseThirtyObjectOwner.position_y_fx_6a8;
    volatile uint8_t *timer = &state->phase_timer_45;
    volatile uint8_t *phase = &state->phase_41;
    uint8_t timer_value;
    uint16_t advance_flags;

    *position_duration = 0;
    *position_x = DATALINK_PHASE_THIRTY_POSITION_X * DATALINK_FX_ONE;
    *position_y = DATALINK_PHASE_THIRTY_POSITION_Y * DATALINK_FX_ONE;

    timer_value = *timer;
    if (timer_value != 0U) {
        *timer = (uint8_t)(timer_value - 1U);
        return;
    }

    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkSceneOwner,
        DATALINK_PHASE_THIRTY_SCENE_COMMAND
    );
    advance_flags = gGameDatalinkPhaseThirtyAdvanceFlags;
    if ((advance_flags & DATALINK_PHASE_THIRTY_ADVANCE_FLAG) == 0U) {
        return;
    }

    Game_CommitDatalinkScreenUpdate();
    Game_ChangeDatalinkState(
        DATALINK_PHASE_THIRTY_NEXT_SYSTEM_STATE,
        0,
        0,
        0
    );
    *timer = DATALINK_PHASE_THIRTY_NEXT_TIMER;
    *phase = DATALINK_PHASE_THIRTY_NEXT_PHASE;
}
