#ifndef GAME_DATALINK_PHASE_FIVE_H
#define GAME_DATALINK_PHASE_FIVE_H

#include "game/datalink_initial_phase.h"
#include "game/datalink_pair_position.h"

#include <stdint.h>

extern volatile uint32_t gGameDatalinkDisplayControl;
extern Game_DatalinkPositionObject gGameDatalinkPhaseFiveGateObject;
extern Game_DatalinkPositionObject gGameDatalinkPhaseFiveFirstObject;
extern Game_DatalinkPositionObject gGameDatalinkPhaseFiveSecondObject;
extern int32_t gGameDatalinkPhaseFiveTransitionDuration;

void Game_UpdateDatalinkPhaseFive(Game_DatalinkFlowState *state);

#endif
