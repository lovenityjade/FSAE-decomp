#ifndef GAME_DATALINK_RENDERER_AUXILIARY_H
#define GAME_DATALINK_RENDERER_AUXILIARY_H

#include "game/datalink_initial_phase.h"

#include <stdint.h>

void Game_RefreshDatalinkRowAuxiliaryIfNeeded(
    Game_DatalinkFlowState *state,
    uint8_t slot
);

#endif
