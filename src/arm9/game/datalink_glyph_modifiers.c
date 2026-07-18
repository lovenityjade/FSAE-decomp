#include "game/datalink_glyph_modifiers.h"

#include <stdint.h>

typedef struct Game_DatalinkGlyphRange {
    uint16_t first;
    uint16_t count_minus_one;
} Game_DatalinkGlyphRange;

extern uint16_t gGameDatalinkGlyphModifierKeyA;
extern uint16_t gGameDatalinkGlyphModifierKeyB;
extern const uint16_t gGameDatalinkGlyphModifierBoundaryA[];
extern const uint16_t gGameDatalinkGlyphModifierBoundaryB[];
extern const Game_DatalinkGlyphRange gGameDatalinkGlyphModifierARanges[6];
extern const Game_DatalinkGlyphRange gGameDatalinkGlyphModifierBRanges[2];
extern uint16_t gGameDatalinkGlyphModifierASpecialBase;
extern uint16_t gGameDatalinkGlyphModifierAWrapUpper;
extern uint16_t gGameDatalinkGlyphModifierAWrapLower;

/*
 * 0x020AF978..0x020AFA13 (156 bytes).
 * Ordinary codes pass through.  Either of the two modifier keys transforms
 * the previous inline glyph and consumes that previous slot on success.
 */
int Game_TranslateDatalinkGlyphCode_020af978(
    Game_DatalinkGlyphMenuState *state,
    uint32_t raw_code)
{
    uint16_t previous_glyph;
    uint32_t translated;

    if (raw_code != gGameDatalinkGlyphModifierKeyA &&
        raw_code != (uint32_t)gGameDatalinkGlyphModifierKeyA + 1U) {
        return (int)raw_code;
    }
    if (state->header_selection_42 == 0) {
        return 0;
    }

    previous_glyph =
        state->fixed_glyphs_34[state->header_selection_42 - 1U];
    if (raw_code == gGameDatalinkGlyphModifierKeyB) {
        if (Game_CanApplyDatalinkGlyphModifierB_020afb4c(
                previous_glyph
            ) == 0) {
            return 0;
        }
        translated =
            Game_ApplyDatalinkGlyphModifierB_020afc0c(previous_glyph);
    } else {
        if (Game_CanApplyDatalinkGlyphModifierA_020afa7c(
                previous_glyph
            ) == 0) {
            return 0;
        }
        translated =
            Game_ApplyDatalinkGlyphModifierA_020afb94(previous_glyph);
    }

    --state->header_selection_42;
    return (int)translated;
}

static int Game_IsDatalinkGlyphInTerminatedTable(
    const uint16_t *table,
    uint32_t glyph_code)
{
    uint16_t value = *table;

    while (value != 0U) {
        if (value == glyph_code) {
            return 1;
        }
        ++table;
        value = *table;
    }
    return 0;
}

/* 0x020AFA1C..0x020AFA43 (40 bytes). */
int Game_IsDatalinkGlyphModifierBoundaryA_020afa1c(uint32_t glyph_code)
{
    return Game_IsDatalinkGlyphInTerminatedTable(
        gGameDatalinkGlyphModifierBoundaryA,
        glyph_code
    );
}

/* 0x020AFA4C..0x020AFA73 (40 bytes). */
int Game_IsDatalinkGlyphModifierBoundaryB_020afa4c(uint32_t glyph_code)
{
    return Game_IsDatalinkGlyphInTerminatedTable(
        gGameDatalinkGlyphModifierBoundaryB,
        glyph_code
    );
}

static int Game_IsDatalinkGlyphInRange(
    uint32_t glyph_code,
    const Game_DatalinkGlyphRange *range)
{
    return glyph_code >= range->first &&
        glyph_code <= (uint32_t)range->first + range->count_minus_one;
}

/*
 * 0x020AFA7C..0x020AFB2F (180 bytes).
 * Modifier A accepts six recovered contiguous ranges and a two-code special
 * pair separated by 0x4E code points.
 */
int Game_CanApplyDatalinkGlyphModifierA_020afa7c(uint32_t glyph_code)
{
    uint32_t range_index;

    for (range_index = 0; range_index < 6U; ++range_index) {
        if (Game_IsDatalinkGlyphInRange(
                glyph_code,
                &gGameDatalinkGlyphModifierARanges[range_index]
            )) {
            return 1;
        }
    }
    return glyph_code == gGameDatalinkGlyphModifierASpecialBase ||
        glyph_code ==
            (uint32_t)gGameDatalinkGlyphModifierASpecialBase + 0x4EU;
}

/*
 * 0x020AFB4C..0x020AFB8B (64 bytes).
 * Modifier B accepts either of two recovered fifteen-code ranges.
 */
int Game_CanApplyDatalinkGlyphModifierB_020afb4c(uint32_t glyph_code)
{
    return Game_IsDatalinkGlyphInRange(
               glyph_code,
               &gGameDatalinkGlyphModifierBRanges[0]
           ) ||
        Game_IsDatalinkGlyphInRange(
            glyph_code,
            &gGameDatalinkGlyphModifierBRanges[1]
        );
}

/*
 * 0x020AFB94..0x020AFC03 (112 bytes).
 * Applies the one-step modifier with the two recovered wrap cases.
 */
uint32_t Game_ApplyDatalinkGlyphModifierA_020afb94(uint32_t glyph_code)
{
    if (Game_IsDatalinkGlyphModifierBoundaryA_020afa1c(glyph_code) != 0) {
        return glyph_code == gGameDatalinkGlyphModifierAWrapUpper
            ? glyph_code - 0x4EU
            : (glyph_code - 1U) & UINT32_C(0xFFFF);
    }
    if (Game_IsDatalinkGlyphModifierBoundaryB_020afa4c(glyph_code) == 0) {
        return glyph_code == gGameDatalinkGlyphModifierAWrapLower
            ? glyph_code + 0x4EU
            : (glyph_code + 1U) & UINT32_C(0xFFFF);
    }
    return (glyph_code - 1U) & UINT32_C(0xFFFF);
}

/*
 * 0x020AFC0C..0x020AFC5B (80 bytes).
 * Applies the companion modifier with one- or two-code-unit steps.
 */
uint32_t Game_ApplyDatalinkGlyphModifierB_020afc0c(uint32_t glyph_code)
{
    if (Game_IsDatalinkGlyphModifierBoundaryA_020afa1c(glyph_code) != 0) {
        return (glyph_code + 1U) & UINT32_C(0xFFFF);
    }
    if (Game_IsDatalinkGlyphModifierBoundaryB_020afa4c(glyph_code) != 0) {
        return (glyph_code - 2U) & UINT32_C(0xFFFF);
    }
    return (glyph_code + 2U) & UINT32_C(0xFFFF);
}
