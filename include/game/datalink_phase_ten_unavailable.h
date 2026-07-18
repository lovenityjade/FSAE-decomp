#ifndef GAME_DATALINK_PHASE_TEN_UNAVAILABLE_H
#define GAME_DATALINK_PHASE_TEN_UNAVAILABLE_H

#include "game/datalink_phase_five.h"

#include <stdint.h>

extern int32_t gGameDatalinkUnavailableTransitionDuration;
extern int32_t gGameDatalinkUnavailableFirstYOffset;
extern int32_t gGameDatalinkUnavailableSecondYOffset;

void Game_BeginDatalinkPhaseTenUnavailable(
    Game_DatalinkFlowState *state
);
int32_t Game_StartDatalinkPhaseTenUnavailableFirstTransition(void);
void Game_StartDatalinkPhaseTenUnavailableSecondTransition(
    int32_t captured_duration
);
void Game_HandleDatalinkPhaseTenUnavailable(
    Game_DatalinkFlowState *state
);

#endif
