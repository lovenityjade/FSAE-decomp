#ifndef GAME_DATALINK_GLYPH_MODIFIERS_H
#define GAME_DATALINK_GLYPH_MODIFIERS_H

#include "game/datalink_glyph_layout.h"

#include <stdint.h>

int Game_TranslateDatalinkGlyphCode_020af978(
    Game_DatalinkGlyphMenuState *state,
    uint32_t raw_code
);

int Game_IsDatalinkGlyphModifierBoundaryA_020afa1c(uint32_t glyph_code);
int Game_IsDatalinkGlyphModifierBoundaryB_020afa4c(uint32_t glyph_code);
int Game_CanApplyDatalinkGlyphModifierA_020afa7c(uint32_t glyph_code);
int Game_CanApplyDatalinkGlyphModifierB_020afb4c(uint32_t glyph_code);
uint32_t Game_ApplyDatalinkGlyphModifierA_020afb94(uint32_t glyph_code);
uint32_t Game_ApplyDatalinkGlyphModifierB_020afc0c(uint32_t glyph_code);

#endif
