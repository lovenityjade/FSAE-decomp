#ifndef GAME_DATALINK_GLYPH_ACTIONS_H
#define GAME_DATALINK_GLYPH_ACTIONS_H

#include "game/datalink_glyph_layout.h"

#include <stdint.h>

int Game_GetDatalinkGlyphKeyboardMode_020af050(
    const Game_DatalinkGlyphMenuState *state,
    int column,
    int row
);

void Game_EnterDatalinkGlyphConfirmColumn_020af0b0(
    Game_DatalinkGlyphMenuState *state
);

void Game_DeleteDatalinkGlyph_020af104(
    Game_DatalinkGlyphMenuState *state
);

void Game_SynchronizeDatalinkGlyphPage_020af17c(
    Game_DatalinkGlyphMenuState *state
);

void Game_CommitDatalinkGlyphCell_020af1f4(
    Game_DatalinkGlyphMenuState *state
);

int Game_DatalinkGlyphNameHasNonSpace_020af344(
    const Game_DatalinkGlyphMenuState *state
);

void Game_ClampDatalinkGlyphRowForMode_020af380(
    const Game_DatalinkGlyphMenuState *state,
    int8_t *row
);

void Game_SaveDatalinkGlyphName_020af424(
    Game_DatalinkGlyphMenuState *state
);

void Game_UpdateDatalinkGlyphSaveTransition_020af4a0(
    Game_DatalinkGlyphMenuState *state,
    uint32_t unused_second_argument,
    uint32_t unused_third_argument,
    uint32_t unused_fourth_argument
);

#endif
