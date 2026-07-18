#include "game/datalink_controller.h"

#include "game/datalink_renderer.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_CONTROLLER_ROW_COUNT = 3,
    DATALINK_CONTROLLER_ROW_STRIDE = 6,
    DATALINK_CONTROLLER_MAIN_CELL_COUNT = 10,
    DATALINK_CONTROLLER_MAIN_X = 8,
    DATALINK_CONTROLLER_MAIN_FIRST_Y = 6,
    DATALINK_CONTROLLER_MAIN_SECOND_Y = 7,
    DATALINK_CONTROLLER_MAIN_PALETTE = 13,
    DATALINK_CONTROLLER_MAIN_FIRST_TILE = 0x22,
    DATALINK_CONTROLLER_MAIN_SECOND_TILE = 0x42,
    DATALINK_CONTROLLER_AUX_CELL_COUNT = 3,
    DATALINK_CONTROLLER_AUX_X = 0x1A,
    DATALINK_CONTROLLER_AUX_FIRST_Y = 4,
    DATALINK_CONTROLLER_AUX_SECOND_Y = 5,
    DATALINK_CONTROLLER_AUX_PALETTE = 4,
    DATALINK_CONTROLLER_AUX_FIRST_TILE = 2,
    DATALINK_CONTROLLER_AUX_SECOND_TILE = 0x22,
    DATALINK_CONTROLLER_OVERLAY_X = 2,
    DATALINK_CONTROLLER_OVERLAY_Y = 4,
    DATALINK_CONTROLLER_OVERLAY_WIDTH = 0x1C,
    DATALINK_CONTROLLER_OVERLAY_HEIGHT = 4,
    DATALINK_CONTROLLER_SELECTED_PALETTE = 4,
    DATALINK_CONTROLLER_DEFAULT_PALETTE = 3,
    DATALINK_CONTROLLER_RENDER_BUFFER_SIZE = 0x600,
    DATALINK_CONTROLLER_PRIMARY_TARGET = 1,
    DATALINK_CONTROLLER_SECONDARY_TARGET = 2
};

/* Host-safe representation of SDK MIi_CpuCopyFast at 0x02015B5C. */
extern void Game_CopyDatalinkControllerTilemapBytes(
    const uint8_t *source,
    uint8_t *destination,
    uint32_t size
);

/* Opaque 0x0209B394 single-tile writer. */
extern void Game_SetDatalinkRenderTile(
    uint16_t *tilemap,
    int x,
    int y,
    int palette,
    uint16_t tile_id
);

/* Opaque 0x0209B4C0 rectangle clear. */
extern void Game_ClearDatalinkRenderRectangle(
    uint16_t *tilemap,
    int x,
    int y,
    int width,
    int height
);

/* Opaque 0x0209B508 rectangle palette update. */
extern void Game_SetDatalinkRenderRectanglePalette(
    uint16_t *tilemap,
    int x,
    int y,
    int width,
    int height,
    int palette
);

/* Opaque 0x0209CF68 per-row auxiliary refresh. */
extern void Game_RefreshDatalinkRowAuxiliary(int slot);

/* Opaque 0x020184B8 cache-range flush. */
extern void Game_FlushDatalinkRenderRange(
    const void *buffer,
    uint32_t size
);

/* Opaque 0x0209DA7C render-buffer transfer. */
extern void Game_TransferDatalinkRenderBuffer(
    int target,
    const void *source,
    uint32_t destination_offset,
    uint32_t size
);

_Static_assert(
    offsetof(Game_DatalinkController, participant_count_5a) == 0x5A,
    "datalink controller tilemap participant-count offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, selected_row_5b) == 0x5B,
    "datalink controller tilemap selected-row offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, auxiliary_state_5f) == 0x5F,
    "datalink controller tilemap dirty offset"
);

static void Game_RenderDatalinkControllerTilemapRow(
    uint16_t *tilemap,
    int row,
    const Game_DatalinkController *controller
)
{
    int row_base = row * DATALINK_CONTROLLER_ROW_STRIDE;
    int cell;
    int palette;

    for (cell = 0; cell < DATALINK_CONTROLLER_MAIN_CELL_COUNT; ++cell) {
        Game_SetDatalinkRenderTile(
            tilemap,
            DATALINK_CONTROLLER_MAIN_X + cell,
            row_base + DATALINK_CONTROLLER_MAIN_FIRST_Y,
            DATALINK_CONTROLLER_MAIN_PALETTE,
            DATALINK_CONTROLLER_MAIN_FIRST_TILE
        );
        Game_SetDatalinkRenderTile(
            tilemap,
            DATALINK_CONTROLLER_MAIN_X + cell,
            row_base + DATALINK_CONTROLLER_MAIN_SECOND_Y,
            DATALINK_CONTROLLER_MAIN_PALETTE,
            DATALINK_CONTROLLER_MAIN_SECOND_TILE
        );
    }

    for (cell = 0; cell < DATALINK_CONTROLLER_AUX_CELL_COUNT; ++cell) {
        Game_SetDatalinkRenderTile(
            tilemap,
            DATALINK_CONTROLLER_AUX_X + cell,
            row_base + DATALINK_CONTROLLER_AUX_FIRST_Y,
            DATALINK_CONTROLLER_AUX_PALETTE,
            DATALINK_CONTROLLER_AUX_FIRST_TILE
        );
        Game_SetDatalinkRenderTile(
            tilemap,
            DATALINK_CONTROLLER_AUX_X + cell,
            row_base + DATALINK_CONTROLLER_AUX_SECOND_Y,
            DATALINK_CONTROLLER_AUX_PALETTE,
            DATALINK_CONTROLLER_AUX_SECOND_TILE
        );
    }

    palette = (controller->selected_row_5b == row) ?
        DATALINK_CONTROLLER_SELECTED_PALETTE :
        DATALINK_CONTROLLER_DEFAULT_PALETTE;
    Game_SetDatalinkRenderRectanglePalette(
        tilemap,
        DATALINK_CONTROLLER_OVERLAY_X,
        row_base + DATALINK_CONTROLLER_OVERLAY_Y,
        DATALINK_CONTROLLER_OVERLAY_WIDTH,
        DATALINK_CONTROLLER_OVERLAY_HEIGHT,
        palette
    );
    Game_RefreshDatalinkRowAuxiliary(row);
}

static void Game_ClearUnusedDatalinkControllerRows(
    uint16_t *tilemap,
    int first_unused_row
)
{
    int row;

    for (row = first_unused_row;
         row < DATALINK_CONTROLLER_ROW_COUNT;
         ++row) {
        Game_ClearDatalinkRenderRectangle(
            tilemap,
            DATALINK_CONTROLLER_OVERLAY_X,
            row * DATALINK_CONTROLLER_ROW_STRIDE +
                DATALINK_CONTROLLER_OVERLAY_Y,
            DATALINK_CONTROLLER_OVERLAY_WIDTH,
            DATALINK_CONTROLLER_OVERLAY_HEIGHT
        );
    }
}

/*
 * 0x020A9AA0
 *
 * Complete 560-byte controller tilemap refresh through 0x020A9CCF. A zero
 * dirty byte at controller +0x5F returns immediately. Otherwise the byte is
 * cleared, two 0x600-byte backing maps are copied, and the primary binding is
 * published. Rows below controller +0x5A receive their fixed main and
 * auxiliary tiles, selected/default palette four/three, and an auxiliary
 * refresh. Remaining rows are cleared. The primary and captured secondary
 * maps are then flushed and uploaded to targets one and two. The four words
 * at 0x020A9CD0..0x020A9CDF are a literal pool and remain data; the next
 * function begins at 0x020A9CE0.
 */
void Game_RefreshDatalinkControllerTilemaps(
    Game_DatalinkController *controller
)
{
    volatile Game_DatalinkRendererBindings *bindings =
        &gGameDatalinkRendererBindings;
    volatile uint8_t *dirty = &controller->auxiliary_state_5f;
    const uint8_t *source;
    uint8_t *destination;
    uint8_t *captured_secondary;
    uint8_t *primary;
    uint16_t *tilemap;
    const volatile uint8_t *participant_count_address =
        &controller->participant_count_5a;
    int participant_count;
    int row;

    if (*dirty == 0U) {
        return;
    }
    *dirty = 0;

    source = bindings->primary_source;
    destination = bindings->primary_destination;
    Game_CopyDatalinkControllerTilemapBytes(
        source,
        destination,
        DATALINK_CONTROLLER_RENDER_BUFFER_SIZE
    );

    source = bindings->secondary_source;
    destination = bindings->secondary_destination;
    Game_CopyDatalinkControllerTilemapBytes(
        source,
        destination,
        DATALINK_CONTROLLER_RENDER_BUFFER_SIZE
    );

    bindings->primary_buffer = bindings->primary_destination;
    captured_secondary = bindings->secondary_destination;
    tilemap = (uint16_t *)(void *)captured_secondary;
    participant_count = *participant_count_address;

    row = 0;
    while (row < participant_count) {
        Game_RenderDatalinkControllerTilemapRow(
            tilemap,
            row,
            controller
        );
        ++row;
        participant_count = *participant_count_address;
    }
    Game_ClearUnusedDatalinkControllerRows(tilemap, participant_count);

    primary = bindings->primary_buffer;
    Game_FlushDatalinkRenderRange(
        primary,
        DATALINK_CONTROLLER_RENDER_BUFFER_SIZE
    );
    primary = bindings->primary_buffer;
    Game_TransferDatalinkRenderBuffer(
        DATALINK_CONTROLLER_PRIMARY_TARGET,
        primary,
        0,
        DATALINK_CONTROLLER_RENDER_BUFFER_SIZE
    );
    Game_FlushDatalinkRenderRange(
        captured_secondary,
        DATALINK_CONTROLLER_RENDER_BUFFER_SIZE
    );
    Game_TransferDatalinkRenderBuffer(
        DATALINK_CONTROLLER_SECONDARY_TARGET,
        captured_secondary,
        0,
        DATALINK_CONTROLLER_RENDER_BUFFER_SIZE
    );
}
