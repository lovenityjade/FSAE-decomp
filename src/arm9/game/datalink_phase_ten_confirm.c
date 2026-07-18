#include "game/datalink_phase_ten_confirm.h"

#include "game/datalink_slot_readiness.h"

#include <stddef.h>

enum {
    DATALINK_CONFIRM_SOUND_ID = 0x3E
};

/* Opaque 0x02076FE0 sound-effect dispatcher. */
extern void Game_PlayDatalinkSoundEffect(uint32_t sound_id);

_Static_assert(
    offsetof(Game_DatalinkFlowState, selected_participant_40) == 0x40,
    "datalink selected participant offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, phase_41) == 0x41,
    "datalink flow phase offset"
);

/*
 * Phase-ten confirmation prelude 0x020A8414..0x020A843B inside
 * FUN_020A8198.
 *
 * Input value one reaches this block.  It emits sound 0x3E before reading the
 * selected byte at +0x40, copies that unsigned byte first to scene+0x53D
 * (target owner 0x0217E6F0) and then to mirror+0x04 (owner 0x02171DF8), and
 * finally calls 0x020A7324 with the same value.  A zero readiness result
 * selects the unavailable branch at 0x020A85D4; any nonzero result selects
 * the ready branch at 0x020A843C.
 */
bool Game_PrepareDatalinkPhaseTenConfirmation(
    Game_DatalinkFlowState *state
)
{
    uint8_t selected_participant;

    Game_PlayDatalinkSoundEffect(DATALINK_CONFIRM_SOUND_ID);
    selected_participant = (uint8_t)state->selected_participant_40;
    gGameDatalinkSceneSelectedParticipant = selected_participant;
    gGameDatalinkSelectedParticipantMirror = selected_participant;

    return Game_IsDatalinkPlayerSlotReady(selected_participant) != 0;
}
