#include "game/hud_four_step_meter.h"

/* External helper 0x0209B2F4 draws a sequential rectangular region into a tilemap. */
extern void Game_DrawTileRectangle(
    uint16_t *screen_tilemap,
    int x,
    int y,
    int width,
    int height,
    int palette,
    uint16_t tile_index
);

enum {
    HUD_METER_X = 27,
    HUD_METER_FIRST_Y = 4,
    HUD_METER_SLOT_SPACING = 4,
    HUD_METER_SLOT_COUNT = 4,
    HUD_METER_WIDTH = 2,
    HUD_METER_HEIGHT = 3,
    HUD_METER_PALETTE = 0,
    HUD_METER_EMPTY_TILE = 384,
    HUD_METER_FILLED_TOP_HEIGHT = 2,
    HUD_METER_FILLED_TOP_TILE = 18,
    HUD_METER_FILLED_BOTTOM_HEIGHT = 1,
    HUD_METER_FILLED_BOTTOM_TILE = 20
};

static void Game_ResetRightHudMeterSlot(
    uint16_t *screen_tilemap,
    int y
)
{
    Game_DrawTileRectangle(
        screen_tilemap,
        HUD_METER_X,
        y,
        HUD_METER_WIDTH,
        HUD_METER_HEIGHT,
        HUD_METER_PALETTE,
        HUD_METER_EMPTY_TILE
    );
}

static void Game_FillRightHudMeterSlot(
    uint16_t *screen_tilemap,
    int y
)
{
    Game_DrawTileRectangle(
        screen_tilemap,
        HUD_METER_X,
        y,
        HUD_METER_WIDTH,
        HUD_METER_FILLED_TOP_HEIGHT,
        HUD_METER_PALETTE,
        HUD_METER_FILLED_TOP_TILE
    );
    Game_DrawTileRectangle(
        screen_tilemap,
        HUD_METER_X,
        y + HUD_METER_FILLED_TOP_HEIGHT,
        HUD_METER_WIDTH,
        HUD_METER_FILLED_BOTTOM_HEIGHT,
        HUD_METER_PALETTE,
        HUD_METER_FILLED_BOTTOM_TILE
    );
}

/*
 * 0x020A6830
 *
 * Proof anchors:
 * - four 2x3 slots at x=27 and y=4,8,12,16 are first drawn with palette 0
 *   and tile 384;
 * - the jump table accepts only values 0..4, with 0 drawing no filled slot;
 * - each filled slot is a 2x2 rectangle from tile 18 followed by a 2x1
 *   rectangle from tile 20;
 * - cases 4 through 1 fall through, producing slots from the requested
 *   bottommost position back toward y=4;
 * - initialization passes the HUD state field at +0x418 and the same tilemap
 *   pointer at +0x934 used by the adjacent HUD routines.
 */
void Game_DrawRightHudFourStepMeter(int step, uint16_t *screen_tilemap)
{
    int slot;

    for (slot = 0; slot < HUD_METER_SLOT_COUNT; ++slot) {
        Game_ResetRightHudMeterSlot(
            screen_tilemap,
            HUD_METER_FIRST_Y + slot * HUD_METER_SLOT_SPACING
        );
    }

    if (step < 1 || step > HUD_METER_SLOT_COUNT) {
        return;
    }

    for (slot = step - 1; slot >= 0; --slot) {
        Game_FillRightHudMeterSlot(
            screen_tilemap,
            HUD_METER_FIRST_Y + slot * HUD_METER_SLOT_SPACING
        );
    }
}
