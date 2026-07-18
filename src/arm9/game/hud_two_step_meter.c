#include "game/hud_two_step_meter.h"

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
    HUD_METER_Y = 6,
    HUD_METER_PALETTE = 0,
    HUD_METER_EMPTY_TILE = 384,
    HUD_METER_FILLED_TILE = 1
};

/*
 * 0x020A6658
 *
 * Proof anchors:
 * - both positions are reset through 0x0209B5A4 at (2, 6) and (4, 6),
 *   with palette 0 and tile 384;
 * - step 1 replaces only the left glyph through 0x0209B3D8;
 * - step 2 replaces the right glyph and then falls through to the left one;
 * - every other value leaves both glyphs in their reset state;
 * - sibling routines at 0x020A66F0 and 0x020A6788 repeat the same two-step
 *   structure at rows 11 and 16 with different filled tiles and palettes.
 */
void Game_DrawTopHudTwoStepMeter(int step, uint16_t *screen_tilemap)
{
    Game_Reset2x2TileGlyph(
        screen_tilemap,
        HUD_METER_LEFT_X,
        HUD_METER_Y,
        HUD_METER_PALETTE,
        HUD_METER_EMPTY_TILE
    );
    Game_Reset2x2TileGlyph(
        screen_tilemap,
        HUD_METER_RIGHT_X,
        HUD_METER_Y,
        HUD_METER_PALETTE,
        HUD_METER_EMPTY_TILE
    );

    if (step == 2) {
        Game_Set2x2TileGlyph(
            screen_tilemap,
            HUD_METER_RIGHT_X,
            HUD_METER_Y,
            HUD_METER_PALETTE,
            HUD_METER_FILLED_TILE
        );
    } else if (step != 1) {
        return;
    }

    Game_Set2x2TileGlyph(
        screen_tilemap,
        HUD_METER_LEFT_X,
        HUD_METER_Y,
        HUD_METER_PALETTE,
        HUD_METER_FILLED_TILE
    );
}
