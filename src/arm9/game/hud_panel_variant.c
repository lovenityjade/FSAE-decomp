#include "game/hud_panel_variant.h"

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
    HUD_PANEL_Y = 21,
    HUD_PANEL_HEIGHT = 2,
    HUD_PANEL_PALETTE = 4,
    HUD_LEFT_PANEL_X = 6,
    HUD_LEFT_PANEL_WIDTH = 16,
    HUD_RIGHT_PANEL_X = 22,
    HUD_RIGHT_PANEL_WIDTH = 4
};

/* 0x02124AC8: seven halfwords before the next table at 0x02124AD6. */
static const uint16_t sLeftPanelTiles[] = {
    0x0090,
    0x00D0,
    0x00C0,
    0x0110,
    0x0100,
    0x0150,
    0x0140
};

/* 0x02124AD6: twelve four-tile-aligned source positions. */
static const uint16_t sRightPanelTiles[] = {
    0x0040,
    0x0044,
    0x0048,
    0x004C,
    0x0050,
    0x0054,
    0x0058,
    0x005C,
    0x0080,
    0x0084,
    0x0088,
    0x008C
};

/*
 * 0x020A6480
 *
 * Loads one halfword from 0x02124AC8 and draws a 16x2 rectangle at (6, 21)
 * with palette 4.  The caller supplies the HUD state field at +0x400 and the
 * same tilemap pointer at +0x934 as the adjacent counters and meters.
 */
void Game_DrawLeftHudPanelVariant(int variant, uint16_t *screen_tilemap)
{
    Game_DrawTileRectangle(
        screen_tilemap,
        HUD_LEFT_PANEL_X,
        HUD_PANEL_Y,
        HUD_LEFT_PANEL_WIDTH,
        HUD_PANEL_HEIGHT,
        HUD_PANEL_PALETTE,
        sLeftPanelTiles[variant]
    );
}

/*
 * 0x020A64C8
 *
 * Loads one halfword from 0x02124AD6 and draws a 4x2 rectangle at (22, 21)
 * with palette 4.  Its caller uses the adjacent HUD state field at +0x404.
 */
void Game_DrawRightHudPanelVariant(int variant, uint16_t *screen_tilemap)
{
    Game_DrawTileRectangle(
        screen_tilemap,
        HUD_RIGHT_PANEL_X,
        HUD_PANEL_Y,
        HUD_RIGHT_PANEL_WIDTH,
        HUD_PANEL_HEIGHT,
        HUD_PANEL_PALETTE,
        sRightPanelTiles[variant]
    );
}
