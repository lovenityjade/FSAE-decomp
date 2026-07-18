#ifndef GAME_DATALINK_PHASE_TEN_READY_LEAD_H
#define GAME_DATALINK_PHASE_TEN_READY_LEAD_H

#include "game/datalink_phase_five.h"

extern Game_DatalinkPositionObject
    gGameDatalinkPhaseTenReadySecondLeadObject;
extern Game_DatalinkPositionObject
    gGameDatalinkPhaseTenReadyThirdLeadObject;

void Game_StartDatalinkPhaseTenReadyLeadTransitions(
    Game_DatalinkFlowState *state
);

#endif
