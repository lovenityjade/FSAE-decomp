#ifndef GAME_DATALINK_RENDERER_BINDINGS_H
#define GAME_DATALINK_RENDERER_BINDINGS_H

#include <stdint.h>

typedef struct Game_DatalinkRendererBindings {
    uint8_t *primary_source;
    uint8_t *primary_destination;
    uint8_t *secondary_source;
    uint8_t *secondary_destination;
    uint8_t *primary_buffer;
    uint8_t *final_source_base;
} Game_DatalinkRendererBindings;

typedef Game_DatalinkRendererBindings Game_DatalinkRenderBufferBindings;
typedef Game_DatalinkRendererBindings Game_DatalinkRenderPrimaryBinding;
typedef Game_DatalinkRendererBindings Game_DatalinkRenderUploadBindings;

#endif
