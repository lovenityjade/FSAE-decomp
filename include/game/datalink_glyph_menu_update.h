#ifndef GAME_DATALINK_GLYPH_MENU_UPDATE_H
#define GAME_DATALINK_GLYPH_MENU_UPDATE_H

#include "game/datalink_controller.h"
#include "game/datalink_glyph_layout.h"

#include <stdint.h>

typedef struct Game_DatalinkGlyphRuntimeOwner {
    uint8_t unknown_000[0x53d];
    uint8_t active_player_slot_53d;
    uint8_t unknown_53e[0x16e];
    uint32_t resource_address_6ac;
    uint8_t unknown_6b0[4];
    uint32_t character_memory_address_6b4;
} Game_DatalinkGlyphRuntimeOwner;

typedef struct Game_DatalinkGlyphPanelDisplay {
    uint8_t unknown_000[0x11c];
    int32_t header_animation_state_11c;
    uint8_t unknown_120[4];
    int32_t header_x_fx_124;
    int32_t header_y_fx_128;
    uint8_t unknown_12c[0x48];
    int32_t cursor_animation_state_174;
    uint8_t unknown_178[4];
    int32_t cursor_x_fx_17c;
    int32_t cursor_y_fx_180;
} Game_DatalinkGlyphPanelDisplay;

extern Game_DatalinkGlyphRuntimeOwner gGameDatalinkGlyphGraphicsRuntime;
extern Game_DatalinkGlyphPanelDisplay gGameDatalinkGlyphPanelDisplay;
extern Game_DatalinkTweenRecord gGameDatalinkGlyphCursorTweenD;
extern Game_DatalinkTweenRecord gGameDatalinkGlyphMotionTweenA;
extern Game_DatalinkTweenRecord gGameDatalinkGlyphMotionTweenB;
extern int32_t gGameDatalinkGlyphFooterPoint[2];
extern int32_t gGameDatalinkGlyphFooterYOffset;
extern uint32_t gGameDatalinkGlyphFooterTweenDuration;

/* 0x020aea40..0x020af01f; 48-byte literal pool follows. */
void Game_UpdateDatalinkGlyphMenu_020aea40(
    Game_DatalinkGlyphMenuState *state
);

#endif
