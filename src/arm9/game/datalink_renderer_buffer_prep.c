#include "game/datalink_renderer_buffer_prep.h"

#include <stddef.h>

enum {
    DATALINK_RENDER_BUFFER_SIZE = 0x600
};

/* Opaque 0x02015D0C byte-copy routine, source before destination. */
extern void Game_CopyDatalinkRenderBytes(
    const uint8_t *source,
    uint8_t *destination,
    uint32_t size
);

_Static_assert(
    offsetof(Game_DatalinkFlowState, selection_dirty_43) == 0x43,
    "datalink renderer dirty flag offset"
);

/*
 * Semantic internal helper for 0x020A8974..0x020A89C7 inside the catalogued
 * FUN_020A8904.  The binding fields represent the pointer graph reached from
 * literals 0x0217A23C, 0x02180D9C, 0x02180DA0, and 0x0217D348 without
 * imposing target pointer offsets on a host build.
 *
 * The dirty byte is cleared before any buffer binding is read.  Primary
 * source/destination are captured for the first 0x600-byte copy.  Secondary
 * bindings are loaded only after that call returns.  After the second copy,
 * the primary destination is reloaded and published, preserving the distinct
 * load at 0x020A89BC.  This helper is not a promoted Ghidra function.
 */
void Game_PrepareDatalinkRenderBuffers(
    Game_DatalinkFlowState *state,
    volatile Game_DatalinkRenderBufferBindings *buffers
)
{
    volatile uint8_t *dirty = &state->selection_dirty_43;
    const uint8_t *source;
    uint8_t *destination;

    *dirty = 0;

    source = buffers->primary_source;
    destination = buffers->primary_destination;
    Game_CopyDatalinkRenderBytes(
        source,
        destination,
        DATALINK_RENDER_BUFFER_SIZE
    );

    source = buffers->secondary_source;
    destination = buffers->secondary_destination;
    Game_CopyDatalinkRenderBytes(
        source,
        destination,
        DATALINK_RENDER_BUFFER_SIZE
    );

    buffers->primary_buffer = buffers->primary_destination;
}
