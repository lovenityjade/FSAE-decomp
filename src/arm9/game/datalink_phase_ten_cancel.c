#include "game/datalink_phase_ten_cancel.h"

#include "game/datalink_mode_dispatch.h"

#include <stddef.h>

enum {
    DATALINK_CANCEL_SOUND_ID = 0x44,
    DATALINK_CANCELLED_PHASE = 15
};

/* Opaque 0x02076FE0 sound-effect dispatcher. */
extern void Game_PlayDatalinkSoundEffect(uint32_t sound_id);

_Static_assert(
    offsetof(Game_DatalinkFlowState, phase_41) == 0x41,
    "datalink flow phase offset"
);

/*
 * Phase-ten cancel action 0x020A83F8..0x020A8413 inside FUN_020A8198.
 *
 * The phase decoder reaches this block only for input value two.  It invokes
 * the mode-three wrapper first, emits sound 0x44 second, writes phase fifteen
 * last, and then branches to the shared selection finalizer at 0x020A864C.
 * That finalizer is intentionally a separate recovery unit.
 */
void Game_CancelDatalinkPhaseTen(Game_DatalinkFlowState *state)
{
    Game_EnterDatalinkModeThree(state);
    Game_PlayDatalinkSoundEffect(DATALINK_CANCEL_SOUND_ID);
    state->phase_41 = DATALINK_CANCELLED_PHASE;
}
