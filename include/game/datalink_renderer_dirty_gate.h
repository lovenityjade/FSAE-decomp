#ifndef GAME_DATALINK_RENDERER_DIRTY_GATE_H
#define GAME_DATALINK_RENDERER_DIRTY_GATE_H

#include "game/datalink_initial_phase.h"

#include <stdbool.h>

bool Game_ShouldRebuildDatalinkRender(
    const Game_DatalinkFlowState *state
);

#endif
