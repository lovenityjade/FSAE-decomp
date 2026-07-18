#ifndef GAME_DATALINK_PHASE_THIRTY_H
#define GAME_DATALINK_PHASE_THIRTY_H

#include "game/datalink_phase_ten_finalizer.h"

#include <stdint.h>

typedef struct Game_DatalinkPhaseThirtyObjectOwner {
    uint8_t unknown_000[0x69C];
    int32_t position_transition_duration_69c;
    uint8_t unknown_6a0[4];
    int32_t position_x_fx_6a4;
    int32_t position_y_fx_6a8;
} Game_DatalinkPhaseThirtyObjectOwner;

extern Game_DatalinkPhaseThirtyObjectOwner
    gGameDatalinkPhaseThirtyObjectOwner;
extern volatile uint16_t gGameDatalinkPhaseThirtyAdvanceFlags;

void Game_UpdateDatalinkPhaseThirty(Game_DatalinkFlowState *state);

#endif
