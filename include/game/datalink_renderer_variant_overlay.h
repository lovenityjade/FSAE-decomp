#ifndef GAME_DATALINK_RENDERER_VARIANT_OVERLAY_H
#define GAME_DATALINK_RENDERER_VARIANT_OVERLAY_H

#include "game/datalink_initial_phase.h"
#include "game/datalink_renderer_bindings.h"

#include <stdint.h>

void Game_RenderDatalinkRowVariantOverlay(
    Game_DatalinkFlowState *state,
    volatile Game_DatalinkRenderPrimaryBinding *buffers,
    uint16_t *secondary_tilemap,
    uint8_t slot
);

#endif
