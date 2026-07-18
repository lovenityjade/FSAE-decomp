#ifndef GAME_DATALINK_RENDERER_BUFFER_PREP_H
#define GAME_DATALINK_RENDERER_BUFFER_PREP_H

#include "game/datalink_initial_phase.h"
#include "game/datalink_renderer_bindings.h"

#include <stdint.h>

void Game_PrepareDatalinkRenderBuffers(
    Game_DatalinkFlowState *state,
    volatile Game_DatalinkRenderBufferBindings *buffers
);

#endif
