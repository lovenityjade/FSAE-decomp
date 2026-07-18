#include "game/time_counter.h"

/*
 * Helper 0x0209B680 writes a two-tile-high glyph at (x, y), using the supplied
 * palette and tile index.  The descriptive name remains provisional.
 */
extern void Game_SetVerticalTileGlyph(
    uint16_t *screen_tilemap,
    int x,
    int y,
    int palette,
    uint16_t tile_index
);

enum {
    TIME_COUNTER_X = 26,
    TIME_COUNTER_Y = 21,
    TIME_COUNTER_PALETTE = 3,
    TIME_COUNTER_DIGIT_TILE_BASE = 8,
    TIME_COUNTER_COLON_TILE = 7,
    TIME_COUNTER_MAX_MINUTES = 99
};

static uint16_t Game_TimeDigitTile(int digit)
{
    return (uint16_t)(digit + TIME_COUNTER_DIGIT_TILE_BASE);
}

/*
 * 0x020A650C
 *
 * Proof anchors:
 * - signed divide-by-60 uses multiplier 0x88888889;
 * - signed divide-by-10 uses multiplier 0x66666667;
 * - only the minute quotient is clamped to 99;
 * - five calls target 0x0209B680 at x=26..30, y=21, palette=3;
 * - glyph 7 is fixed between the two digit pairs.
 */
void Game_DrawTimeCounter(int total_seconds, uint16_t *screen_tilemap)
{
    int raw_minutes = total_seconds / 60;
    int seconds = total_seconds - raw_minutes * 60;
    int minutes = raw_minutes;

    if (minutes > TIME_COUNTER_MAX_MINUTES) {
        minutes = TIME_COUNTER_MAX_MINUTES;
    }

    Game_SetVerticalTileGlyph(
        screen_tilemap,
        TIME_COUNTER_X,
        TIME_COUNTER_Y,
        TIME_COUNTER_PALETTE,
        Game_TimeDigitTile(minutes / 10)
    );
    Game_SetVerticalTileGlyph(
        screen_tilemap,
        TIME_COUNTER_X + 1,
        TIME_COUNTER_Y,
        TIME_COUNTER_PALETTE,
        Game_TimeDigitTile(minutes - (minutes / 10) * 10)
    );
    Game_SetVerticalTileGlyph(
        screen_tilemap,
        TIME_COUNTER_X + 2,
        TIME_COUNTER_Y,
        TIME_COUNTER_PALETTE,
        TIME_COUNTER_COLON_TILE
    );
    Game_SetVerticalTileGlyph(
        screen_tilemap,
        TIME_COUNTER_X + 3,
        TIME_COUNTER_Y,
        TIME_COUNTER_PALETTE,
        Game_TimeDigitTile(seconds / 10)
    );
    Game_SetVerticalTileGlyph(
        screen_tilemap,
        TIME_COUNTER_X + 4,
        TIME_COUNTER_Y,
        TIME_COUNTER_PALETTE,
        Game_TimeDigitTile(seconds - (seconds / 10) * 10)
    );
}
