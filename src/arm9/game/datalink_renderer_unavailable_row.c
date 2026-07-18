#include "game/datalink_renderer_unavailable_row.h"

enum {
    DATALINK_ROW_STRIDE = 6,
    DATALINK_MAIN_CELL_COUNT = 10,
    DATALINK_MAIN_X = 8,
    DATALINK_MAIN_FIRST_Y = 6,
    DATALINK_MAIN_SECOND_Y = 7,
    DATALINK_AUX_CELL_COUNT = 3,
    DATALINK_AUX_X = 0x1A,
    DATALINK_AUX_FIRST_Y = 4,
    DATALINK_AUX_SECOND_Y = 5,
    DATALINK_UNAVAILABLE_PALETTE = 4,
    DATALINK_TILE_EMPTY = 2,
    DATALINK_TILE_UNAVAILABLE = 0x22,
    DATALINK_TILE_UNAVAILABLE_ALT = 0x42
};

/* Opaque 0x0209B394 single-tile writer. */
extern void Game_SetDatalinkRenderTile(
    uint16_t *tilemap,
    int x,
    int y,
    int palette,
    uint16_t tile_id
);

/*
 * Semantic internal helper for the unavailable-row branch
 * Range: 0x020A8B34..0x020A8BCF inside catalogued FUN_020A8904.  The first
 * loop emits two ordered tiles at X 8..17 on row pair slot*6+6/+7.  The
 * second emits two ordered tiles at X 26..28 on row pair slot*6+4/+5.  All
 * calls use palette four.  This helper is not a promoted Ghidra function.
 */
void Game_RenderUnavailableDatalinkRow(
    uint16_t *tilemap,
    uint8_t slot
)
{
    int row_base = slot * DATALINK_ROW_STRIDE;
    int cell;

    for (cell = 0; cell < DATALINK_MAIN_CELL_COUNT; ++cell) {
        Game_SetDatalinkRenderTile(
            tilemap,
            DATALINK_MAIN_X + cell,
            row_base + DATALINK_MAIN_FIRST_Y,
            DATALINK_UNAVAILABLE_PALETTE,
            DATALINK_TILE_UNAVAILABLE
        );
        Game_SetDatalinkRenderTile(
            tilemap,
            DATALINK_MAIN_X + cell,
            row_base + DATALINK_MAIN_SECOND_Y,
            DATALINK_UNAVAILABLE_PALETTE,
            DATALINK_TILE_UNAVAILABLE_ALT
        );
    }

    for (cell = 0; cell < DATALINK_AUX_CELL_COUNT; ++cell) {
        Game_SetDatalinkRenderTile(
            tilemap,
            DATALINK_AUX_X + cell,
            row_base + DATALINK_AUX_FIRST_Y,
            DATALINK_UNAVAILABLE_PALETTE,
            DATALINK_TILE_EMPTY
        );
        Game_SetDatalinkRenderTile(
            tilemap,
            DATALINK_AUX_X + cell,
            row_base + DATALINK_AUX_SECOND_Y,
            DATALINK_UNAVAILABLE_PALETTE,
            DATALINK_TILE_UNAVAILABLE
        );
    }
}
