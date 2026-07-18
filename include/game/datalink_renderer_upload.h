#ifndef GAME_DATALINK_RENDERER_UPLOAD_H
#define GAME_DATALINK_RENDERER_UPLOAD_H

#include "game/datalink_renderer_bindings.h"

void Game_UploadDatalinkRenderBuffers(
    volatile Game_DatalinkRenderUploadBindings *buffers,
    uint8_t *captured_secondary_buffer
);

#endif
