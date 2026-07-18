#include "game/hud_two_step_meter_stack.h"

/* External helper 0x0209B5A4 clears and replaces a two-by-two tile glyph. */
extern void Game_Reset2x2TileGlyph(
    uint16_t *screen_tilemap,
    int x,
    int y,
    int palette,
    uint16_t tile_index
);

/* External helper 0x0209B3D8 replaces a two-by-two tile glyph without first clearing it. */
extern void Game_Set2x2TileGlyph(
    uint16_t *screen_tilemap,
    int x,
    int y,
    int palette,
    uint16_t tile_index
);

enum {
    HUD_METER_LEFT_X = 2,
    HUD_METER_RIGHT_X = 4,
    HUD_METER_EMPTY_PALETTE = 0,
    HUD_METER_EMPTY_TILE = 384,
    HUD_METER_MIDDLE_Y = 11,
    HUD_METER_MIDDLE_FILLED_PALETTE = 1,
    HUD_METER_MIDDLE_FILLED_TILE = 3,
    HUD_METER_BOTTOM_Y = 16,
    HUD_METER_BOTTOM_FILLED_PALETTE = 2,
    HUD_METER_BOTTOM_FILLED_TILE = 5
};

static void Game_DrawHudTwoStepMeterMember(
    int step,
    uint16_t *screen_tilemap,
    int y,
    int filled_palette,
    uint16_t filled_tile
)
{
    Game_Reset2x2TileGlyph(
        screen_tilemap,
        HUD_METER_LEFT_X,
        y,
        HUD_METER_EMPTY_PALETTE,
        HUD_METER_EMPTY_TILE
    );
    Game_Reset2x2TileGlyph(
        screen_tilemap,
        HUD_METER_RIGHT_X,
        y,
        HUD_METER_EMPTY_PALETTE,
        HUD_METER_EMPTY_TILE
    );

    if (step == 2) {
        Game_Set2x2TileGlyph(
            screen_tilemap,
            HUD_METER_RIGHT_X,
            y,
            filled_palette,
            filled_tile
        );
    } else if (step != 1) {
        return;
    }

    Game_Set2x2TileGlyph(
        screen_tilemap,
        HUD_METER_LEFT_X,
        y,
        filled_palette,
        filled_tile
    );
}

/*
 * 0x020A66F0
 *
 * Resets (2, 11) and (4, 11) with palette 0/tile 384.  Step 1 replaces
 * only the left glyph with palette 1/tile 3; step 2 replaces right then
 * left.  Other values retain both reset glyphs.
 */
void Game_DrawMiddleHudTwoStepMeter(int step, uint16_t *screen_tilemap)
{
    Game_DrawHudTwoStepMeterMember(
        step,
        screen_tilemap,
        HUD_METER_MIDDLE_Y,
        HUD_METER_MIDDLE_FILLED_PALETTE,
        HUD_METER_MIDDLE_FILLED_TILE
    );
}

/*
 * 0x020A6788
 *
 * Resets (2, 16) and (4, 16) with palette 0/tile 384.  Step 1 replaces
 * only the left glyph with palette 2/tile 5; step 2 replaces right then
 * left.  Other values retain both reset glyphs.
 */
void Game_DrawBottomHudTwoStepMeter(int step, uint16_t *screen_tilemap)
{
    Game_DrawHudTwoStepMeterMember(
        step,
        screen_tilemap,
        HUD_METER_BOTTOM_Y,
        HUD_METER_BOTTOM_FILLED_PALETTE,
        HUD_METER_BOTTOM_FILLED_TILE
    );
}
