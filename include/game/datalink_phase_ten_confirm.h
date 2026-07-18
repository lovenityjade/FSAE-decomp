#ifndef GAME_DATALINK_PHASE_TEN_CONFIRM_H
#define GAME_DATALINK_PHASE_TEN_CONFIRM_H

#include "game/datalink_initial_phase.h"

#include <stdbool.h>
#include <stdint.h>

extern uint8_t gGameDatalinkSceneSelectedParticipant;
extern uint8_t gGameDatalinkSelectedParticipantMirror;

bool Game_PrepareDatalinkPhaseTenConfirmation(
    Game_DatalinkFlowState *state
);

#endif
