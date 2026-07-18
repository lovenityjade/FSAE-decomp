#include "game/datalink_phase_thirty_one.h"

#include "game/datalink_player_slot.h"

#include <stddef.h>

enum {
    DATALINK_PHASE_THIRTY_ONE_CLEANUP_TIMER = 15,
    DATALINK_PHASE_THIRTY_ONE_NEXT_PHASE = 6
};

/* Opaque 0x020AC308 screen-update commit. */
extern void Game_CommitDatalinkScreenUpdate(void);

_Static_assert(
    offsetof(Game_DatalinkFlowState, phase_41) == 0x41,
    "datalink flow phase offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, phase_timer_45) == 0x45,
    "datalink phase timer offset"
);

/*
 * Semantic phase-thirty-one helper for 0x020A8794..0x020A87C7 inside the
 * catalogued FUN_020A8198.  This internal block is not promoted in the Ghidra
 * function registry.  The cleanup dependency at 0x020A88A8 is a separate
 * true function boundary and remains opaque here.
 *
 * The first volatile timer read alone gates cleanup at value 15.  A distinct
 * post-cleanup reload then controls the countdown.  Any nonzero value is
 * decremented and returns even when the result becomes zero.  A value already
 * equal to zero advances the phase to six before committing the screen
 * update.
 */
void Game_UpdateDatalinkPhaseThirtyOne(Game_DatalinkFlowState *state)
{
    volatile uint8_t *timer = &state->phase_timer_45;
    volatile uint8_t *phase = &state->phase_41;
    uint8_t cleanup_timer;
    uint8_t countdown_timer;

    cleanup_timer = *timer;
    if (cleanup_timer == DATALINK_PHASE_THIRTY_ONE_CLEANUP_TIMER) {
        Game_CleanupDatalinkPlayerSlots();
    }

    countdown_timer = *timer;
    if (countdown_timer != 0U) {
        *timer = (uint8_t)(countdown_timer - 1U);
        return;
    }

    *phase = DATALINK_PHASE_THIRTY_ONE_NEXT_PHASE;
    Game_CommitDatalinkScreenUpdate();
}
