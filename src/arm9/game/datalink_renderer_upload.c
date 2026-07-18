#include "game/datalink_renderer_upload.h"

enum {
    DATALINK_RENDER_BUFFER_SIZE = 0x600,
    DATALINK_RENDER_FINAL_OFFSET = 0x6400,
    DATALINK_RENDER_FINAL_SIZE = 0xA80,
    DATALINK_RENDER_PRIMARY_TARGET = 1,
    DATALINK_RENDER_SECONDARY_TARGET = 2
};

/* Opaque 0x020184B8 cache-range flush. */
extern void Game_FlushDatalinkRenderRange(
    const void *buffer,
    uint32_t size
);

/* Opaque 0x0209DA7C transfer for the two 0x600-byte render buffers. */
extern void Game_TransferDatalinkRenderBuffer(
    int target,
    const void *source,
    uint32_t destination_offset,
    uint32_t size
);

/* Opaque 0x0209DAA0 transfer for the final tile range. */
extern void Game_TransferDatalinkRenderTiles(
    int target,
    const void *source,
    uint32_t destination_offset,
    uint32_t size
);

/*
 * Semantic internal helper for the upload operation
 * Range: 0x020A8CC8..0x020A8D4B inside catalogued FUN_020A8904.
 *
 * The secondary pointer is the value captured by the parent at 0x020A89C8.
 * In contrast, the primary pointer is reloaded after both initial flushes,
 * and the final source base is reloaded after its flush.  Volatile bindings
 * preserve those observable loads without imposing target-global addresses
 * on host tests.  This helper is not promoted in Ghidra.
 */
void Game_UploadDatalinkRenderBuffers(
    volatile Game_DatalinkRenderUploadBindings *buffers,
    uint8_t *captured_secondary_buffer
)
{
    uint8_t *source;

    source = buffers->primary_buffer;
    Game_FlushDatalinkRenderRange(source, DATALINK_RENDER_BUFFER_SIZE);
    Game_FlushDatalinkRenderRange(
        captured_secondary_buffer,
        DATALINK_RENDER_BUFFER_SIZE
    );

    source = buffers->primary_buffer;
    Game_TransferDatalinkRenderBuffer(
        DATALINK_RENDER_PRIMARY_TARGET,
        source,
        0,
        DATALINK_RENDER_BUFFER_SIZE
    );
    Game_TransferDatalinkRenderBuffer(
        DATALINK_RENDER_SECONDARY_TARGET,
        captured_secondary_buffer,
        0,
        DATALINK_RENDER_BUFFER_SIZE
    );

    source = buffers->final_source_base + DATALINK_RENDER_FINAL_OFFSET;
    Game_FlushDatalinkRenderRange(source, DATALINK_RENDER_FINAL_SIZE);

    source = buffers->final_source_base + DATALINK_RENDER_FINAL_OFFSET;
    Game_TransferDatalinkRenderTiles(
        DATALINK_RENDER_PRIMARY_TARGET,
        source,
        DATALINK_RENDER_FINAL_OFFSET,
        DATALINK_RENDER_FINAL_SIZE
    );
}
