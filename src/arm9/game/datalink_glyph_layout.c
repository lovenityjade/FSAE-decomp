#include "game/datalink_glyph_layout.h"
#include "game/datalink_controller.h"

#include <stdint.h>

enum {
    DATALINK_GLYPH_PARTICIPANT_MASK = 0x0F,
    DATALINK_GLYPH_LAYOUT_MASK = 0x70,
    DATALINK_GLYPH_LAYOUT_ENABLED = 0x80,
    DATALINK_GLYPH_INPUT_MODE_MASK = 0x1F00,
    DATALINK_GLYPH_INPUT_MODE_ACTIVE = 0x1000
};

typedef struct Game_DatalinkGlyphCameraState {
    uint8_t unknown_000[0xF8];
    int32_t horizontal_offset_fx_f8;
} Game_DatalinkGlyphCameraState;

extern volatile int8_t gGameDatalinkGlyphPhase;
extern Game_DatalinkGlyphCameraState gGameDatalinkGlyphCameraState;
extern volatile uint32_t gGameDatalinkGlyphInputState;
extern const uint32_t gGameDatalinkGlyphLayoutSlots[];
extern const uint32_t gGameDatalinkGlyphLayoutParticipants[];
extern const uint32_t gGameDatalinkTwoParticipantLayouts[];

extern int MATH_CountPopulation(uint32_t value);
extern void OS_Terminate(void);

extern uint32_t gGameDatalinkGlyphPrimaryTableAddress;
extern uint32_t gGameDatalinkGlyphAlternateTableAddress;
extern int32_t gGameDatalinkGlyphCellIndexOffset;
extern const uint8_t gGameDatalinkGlyphCellXRecords[];
extern const uint8_t gGameDatalinkGlyphCellYRecords[];
extern const uint8_t gGameDatalinkGlyphCellRecords[];
extern const int32_t gGameDatalinkGlyphKeyboardModePoints[][2];
extern uint32_t gGameDatalinkGlyphResourceAddress;
extern uint32_t (*gGameDatalinkGlyphCellCallback)(
    Game_DatalinkGlyphMenuState *state,
    int cell_index
);

extern Game_DatalinkTweenRecord gGameDatalinkGlyphCursorTweenA;
extern Game_DatalinkTweenRecord gGameDatalinkGlyphCursorTweenB;
extern Game_DatalinkTweenRecord gGameDatalinkGlyphCursorTweenC;
extern Game_DatalinkTweenRecord gGameDatalinkGlyphCursorTweenD;
extern int32_t gGameDatalinkGlyphCursorPoint[2];
extern int32_t gGameDatalinkGlyphSecondaryPoint[2];
extern int32_t gGameDatalinkGlyphSecondaryYOffset;
extern int32_t gGameDatalinkGlyphFooterPoint[2];
extern int32_t gGameDatalinkGlyphFooterYOffset;
extern uint32_t gGameDatalinkGlyphCursorTweenDuration;
extern uint32_t gGameDatalinkGlyphFooterTweenDuration;

extern uint32_t Game_GetDatalinkGlyphResourceAddress(void);
extern void MI_CpuFill8(void *destination, uint8_t value, uint32_t size);
extern void MIi_CpuClearFast(
    uint32_t value,
    void *destination,
    uint32_t size
);
extern void DC_FlushRange(const void *buffer, uint32_t size);

typedef struct Game_DatalinkGlyphCellRecord {
    uint8_t unknown_00[8];
    uint32_t character_address_08;
    uint32_t character_index_0c;
    uint8_t unknown_10[8];
    uint32_t width_tiles_18;
    uint32_t height_tiles_1c;
    uint8_t unknown_20[0x14];
} Game_DatalinkGlyphCellRecord;

typedef struct Game_DatalinkGlyphGraphicsRuntime {
    uint8_t unknown_000[0x6AC];
    uint32_t resource_address_6ac;
    uint8_t unknown_6b0[4];
    uint32_t character_memory_address_6b4;
} Game_DatalinkGlyphGraphicsRuntime;

typedef struct Game_DatalinkGlyphRenderer {
    uint8_t unknown_00[0x38];
    uint32_t resource_address_38;
} Game_DatalinkGlyphRenderer;

typedef struct Game_DatalinkGlyphCursorDisplay {
    uint8_t unknown_000[0xCD5];
    uint8_t display_mode_cd5;
    uint8_t unknown_cd6[0x26];
    int32_t animation_state_cfc;
    uint8_t unknown_d00[4];
    int32_t x_fx_d04;
    int32_t y_fx_d08;
} Game_DatalinkGlyphCursorDisplay;

typedef struct Game_DatalinkGlyphSubDisplay {
    uint8_t unknown_00[0xF5];
    uint8_t display_mode_f5;
} Game_DatalinkGlyphSubDisplay;

typedef struct Game_DatalinkGlyphCellObject {
    uint8_t unknown_00[0x5C];
    int16_t glyph_x_offset_5c;
    volatile int16_t glyph_y_offset_5e;
} Game_DatalinkGlyphCellObject;

typedef struct Game_DatalinkFont {
    uint32_t resource_address;
} Game_DatalinkFont;

typedef struct Game_DatalinkGlyphWidths {
    int8_t left;
    uint8_t glyph_width;
    uint8_t character_width;
} Game_DatalinkGlyphWidths;

extern Game_DatalinkGlyphGraphicsRuntime gGameDatalinkGlyphGraphicsRuntime;
extern Game_DatalinkGlyphCellRecord gGameDatalinkGlyphCellRecordTable[71];
extern Game_DatalinkGlyphRenderer gGameDatalinkGlyphRenderer;
extern Game_DatalinkGlyphCursorDisplay gGameDatalinkGlyphCursorDisplay;
extern Game_DatalinkGlyphSubDisplay gGameDatalinkGlyphSubDisplay;
extern volatile uint32_t gGameDatalinkGlyphDisplayInputState;
extern uint8_t *gGameDatalinkGlyphCellUploadBuffer;

extern void Game_PrepareDatalinkGlyphResourceContext(void);
extern uint32_t Game_GetDatalinkGlyphResourceContext(void);
extern Game_DatalinkFont *Game_GetDatalinkGlyphFont(void);
extern void Game_InitializeDatalinkGlyphRenderer(
    Game_DatalinkGlyphRenderer *renderer,
    uint32_t resource_context,
    void *state_control
);
extern Game_DatalinkGlyphCellObject *Game_GetDatalinkGlyphCellObject(
    uint32_t cell_index
);
extern void Game_FinalizeDatalinkGlyphCell(
    uint32_t cell_index,
    int argument
);
extern void Game_UploadDatalinkGlyphCharacterData(
    int engine,
    const void *source,
    uint32_t destination_offset,
    uint32_t size
);
extern uint32_t NNS_G2dFontFindGlyphIndex(
    const Game_DatalinkFont *font,
    uint16_t character_code
);
extern const Game_DatalinkGlyphWidths *NNS_G2dFontGetCharWidthsFromIndex(
    const Game_DatalinkFont *font,
    uint32_t glyph_index
);

/*
 * 0x020AE280..0x020AE28F (16 bytes).
 * Callback used by the transition code to install the -1 phase sentinel.
 */
void Game_SetDatalinkGlyphPhaseSentinel_020ae280(void)
{
    gGameDatalinkGlyphPhase = -1;
}

/*
 * 0x020AE294..0x020AE2AF (28 bytes).
 * Resets the phase byte and places the camera one tile before its origin.
 */
void Game_ResetDatalinkGlyphPhase_020ae294(void)
{
    gGameDatalinkGlyphPhase = 0;
    gGameDatalinkGlyphCameraState.horizontal_offset_fx_f8 = -0x10;
}

/*
 * 0x020AE2B8..0x020AE2DB (36 bytes).
 * Selects phase one and rewrites only input-mode bits 8..12.
 */
void Game_ActivateDatalinkGlyphPhase_020ae2b8(void)
{
    gGameDatalinkGlyphPhase = 1;
    gGameDatalinkGlyphInputState =
        (gGameDatalinkGlyphInputState & ~DATALINK_GLYPH_INPUT_MODE_MASK) |
        DATALINK_GLYPH_INPUT_MODE_ACTIVE;
}

/* 0x020AE318..0x020AE333 (28 bytes). */
uint32_t Game_GetDatalinkGlyphLayoutSlot_020ae318(
    int participant_index,
    const uint32_t *layout_state)
{
    const uint8_t *table = (const uint8_t *)gGameDatalinkGlyphLayoutSlots;
    uint32_t byte_offset = *layout_state & DATALINK_GLYPH_LAYOUT_MASK;

    return *(const uint32_t *)(
        table + byte_offset + (uint32_t)participant_index * sizeof(uint32_t)
    );
}

/* 0x020AE338..0x020AE353 (28 bytes). */
uint32_t Game_GetDatalinkGlyphLayoutParticipant_020ae338(
    int participant_index,
    const uint32_t *layout_state)
{
    const uint8_t *table =
        (const uint8_t *)gGameDatalinkGlyphLayoutParticipants;
    uint32_t byte_offset = *layout_state & DATALINK_GLYPH_LAYOUT_MASK;

    return *(const uint32_t *)(
        table + byte_offset + (uint32_t)participant_index * sizeof(uint32_t)
    );
}

/*
 * 0x020AE358..0x020AE3AF (88 bytes).
 * Participant zero selects the base layout; values 1..3 occupy low-mask bits.
 */
void Game_AddDatalinkGlyphParticipant_020ae358(
    uint32_t participant,
    uint32_t *layout_state)
{
    if (participant == 0U) {
        *layout_state = (*layout_state & ~UINT32_C(0x7F)) | 1U;
        return;
    }

    (void)MATH_CountPopulation(
        *layout_state & DATALINK_GLYPH_PARTICIPANT_MASK
    );
    *layout_state =
        (*layout_state & ~UINT32_C(0x0F)) |
        (*layout_state & DATALINK_GLYPH_PARTICIPANT_MASK) |
        ((UINT32_C(1) << (participant & UINT32_C(0xFF))) &
         DATALINK_GLYPH_PARTICIPANT_MASK);
}

/*
 * 0x020AE3B0..0x020AE4A3 (244 bytes).
 * Removes one participant bit.  When dynamic layout ordering is enabled by
 * bit 7, the remaining order is compacted and its three-bit layout code is
 * recomputed in bits 4..6.
 */
void Game_RemoveDatalinkGlyphParticipant_020ae3b0(
    uint32_t participant,
    uint32_t *layout_state)
{
    uint32_t ordered_participants[4];
    uint32_t previous_count;
    uint32_t removed_index;
    uint32_t next_state;
    uint32_t index;

    if (participant == 0U) {
        *layout_state &= ~UINT32_C(0x7F);
        return;
    }

    previous_count = (uint32_t)MATH_CountPopulation(
        *layout_state & DATALINK_GLYPH_PARTICIPANT_MASK
    );
    removed_index = Game_GetDatalinkGlyphLayoutSlot_020ae318(
        (int)participant,
        layout_state
    );
    next_state =
        (*layout_state & ~UINT32_C(0x0F)) |
        ((*layout_state & DATALINK_GLYPH_PARTICIPANT_MASK) &
         ~(UINT32_C(1) << (participant & UINT32_C(0xFF))));
    *layout_state = next_state;

    if ((next_state & DATALINK_GLYPH_LAYOUT_ENABLED) == 0U) {
        return;
    }

    for (index = 0; index < 4U; ++index) {
        ordered_participants[index] =
            Game_GetDatalinkGlyphLayoutParticipant_020ae338(
                (int)index,
                layout_state
            );
    }

    for (index = removed_index; index + 1U < previous_count; ++index) {
        uint32_t temporary = ordered_participants[index];
        ordered_participants[index] = ordered_participants[index + 1U];
        ordered_participants[index + 1U] = temporary;
    }

    *layout_state =
        (*layout_state & ~DATALINK_GLYPH_LAYOUT_MASK) |
        ((Game_ResolveDatalinkGlyphLayout_020ae4a4(
              (int)(previous_count - 1U),
              ordered_participants
          ) & UINT32_C(7)) << 4);
}

/*
 * 0x020AE4A4..0x020AE53B (152 bytes).
 * Encodes the ordering of up to three active participant identifiers.
 */
uint32_t Game_ResolveDatalinkGlyphLayout_020ae4a4(
    int participant_count,
    const uint32_t *participants)
{
    uint32_t second;

    if (participant_count < 2) {
        return 0;
    }
    if (participant_count == 2) {
        return gGameDatalinkTwoParticipantLayouts[participants[1]];
    }

    second = participants[1];
    if (second == 1U) {
        return participants[2] != 2U ? 1U : 0U;
    }
    if (second == 2U) {
        return participants[2] == 1U ? 2U : 4U;
    }
    if (second == 3U) {
        return participants[2] == 1U ? 3U : 5U;
    }

    OS_Terminate();
    return 0;
}

/* 0x020AE540..0x020AE54F (16 bytes). */
uint32_t Game_SelectDatalinkGlyphTable_020ae540(
    const Game_DatalinkGlyphMenuState *state,
    int alternate_page)
{
    (void)state;
    return alternate_page != 0
        ? gGameDatalinkGlyphAlternateTableAddress
        : gGameDatalinkGlyphPrimaryTableAddress;
}

/*
 * 0x020AE558..0x020AE5B7 (96 bytes).
 * Resolves either one of six inline glyphs or an entry in the active page.
 */
int Game_BuildDatalinkGlyphDescriptor_020ae558(
    const Game_DatalinkGlyphMenuState *state,
    Game_DatalinkGlyphDescriptor *descriptor,
    int cell_index)
{
    uint32_t glyph_address;

    cell_index += gGameDatalinkGlyphCellIndexOffset;
    if (cell_index < 0 || cell_index > 5) {
        glyph_address = Game_SelectDatalinkGlyphTable_020ae540(
            state,
            state->font_page_47
        );
        glyph_address += (uint32_t)(cell_index - 6) * sizeof(uint16_t);
    } else {
        glyph_address =
            (uint32_t)(uintptr_t)&state->fixed_glyphs_34[cell_index];
    }

    descriptor->resource_address = Game_GetDatalinkGlyphResourceAddress();
    descriptor->glyph_address = glyph_address;
    descriptor->glyph_size = sizeof(uint16_t);
    return 1;
}

static int32_t Game_ReadDatalinkGlyphRecordS32(
    const uint8_t *records,
    uint32_t byte_offset)
{
    const int32_t *value = (const int32_t *)(records + byte_offset);
    return *value;
}

/*
 * 0x020AE830..0x020AE8AF (128 bytes).
 * Converts cell-table coordinates into the cursor's pixel position.
 */
void Game_GetDatalinkGlyphCursorPoint_020ae830(
    Game_DatalinkGlyphPoint *point,
    const Game_DatalinkGlyphMenuState *state)
{
    if (state->keyboard_mode_46 == 0) {
        uint32_t byte_offset =
            (uint32_t)(state->selected_cell_4a + 6) * UINT32_C(0x34);
        point->x =
            Game_ReadDatalinkGlyphRecordS32(
                gGameDatalinkGlyphCellXRecords,
                byte_offset
            ) * 8 + 6;
        point->y =
            Game_ReadDatalinkGlyphRecordS32(
                gGameDatalinkGlyphCellYRecords,
                byte_offset
            ) * 8 + 6;
    } else {
        uint32_t index = (uint32_t)(state->keyboard_mode_46 - 1);
        point->x = gGameDatalinkGlyphKeyboardModePoints[index][0] + 0x14;
        point->y = gGameDatalinkGlyphKeyboardModePoints[index][1] + 8;
    }
}

/* 0x020AE8C0..0x020AE903 (68 bytes). */
void Game_GetDatalinkGlyphHeaderPoint_020ae8c0(
    Game_DatalinkGlyphPoint *point,
    const Game_DatalinkGlyphMenuState *state)
{
    uint32_t byte_offset =
        (uint32_t)state->header_selection_42 * UINT32_C(0x34);

    point->x = Game_ReadDatalinkGlyphRecordS32(
        gGameDatalinkGlyphCellRecords,
        byte_offset + 0x10U
    ) * 8 + 8;
    point->y = Game_ReadDatalinkGlyphRecordS32(
        gGameDatalinkGlyphCellRecords,
        byte_offset + 0x14U
    ) * 8 + 8;
}

/*
 * 0x020AE908..0x020AE94B (68 bytes).
 * Inline cells use state+0x34; later cells index the current glyph page.
 */
uint16_t Game_GetDatalinkGlyphCode_020ae908(
    const Game_DatalinkGlyphMenuState *state,
    int cell_index)
{
    int glyph_index = Game_ReadDatalinkGlyphRecordS32(
        gGameDatalinkGlyphCellRecords,
        (uint32_t)cell_index * UINT32_C(0x34)
    ) + gGameDatalinkGlyphCellIndexOffset;

    if (cell_index < 6) {
        return state->fixed_glyphs_34[cell_index];
    }

    return *(const uint16_t *)(uintptr_t)(
        Game_SelectDatalinkGlyphTable_020ae540(state, state->font_page_47) +
        (uint32_t)(glyph_index - 6) * sizeof(uint16_t)
    );
}

/* 0x020AE954..0x020AE963 (16 bytes), a tail-dispatch adapter. */
uint32_t Game_DispatchDatalinkGlyphCell_020ae954(
    Game_DatalinkGlyphMenuState *state)
{
    return gGameDatalinkGlyphCellCallback(state, state->selected_cell_4a + 6);
}

/*
 * 0x020AE968..0x020AEA17 (176 bytes).
 * Starts the four cursor/panel tweens from the positions published by the
 * glyph menu.  Coordinates use the game's 20.12 fixed-point convention.
 */
void Game_ConfigureDatalinkGlyphCursorTweens_020ae968(void)
{
    int32_t cursor_x_fx =
        (gGameDatalinkGlyphCursorPoint[0] - 0x100) * 0x1000;
    int32_t cursor_y_fx = gGameDatalinkGlyphCursorPoint[1] * 0x1000;

    Game_ConfigureDatalinkTween(
        &gGameDatalinkGlyphCursorTweenA,
        cursor_x_fx,
        cursor_y_fx,
        gGameDatalinkGlyphCursorTweenDuration,
        1
    );
    Game_ConfigureDatalinkTween(
        &gGameDatalinkGlyphCursorTweenB,
        cursor_x_fx,
        cursor_y_fx,
        gGameDatalinkGlyphCursorTweenDuration,
        1
    );
    Game_ConfigureDatalinkTween(
        &gGameDatalinkGlyphCursorTweenC,
        gGameDatalinkGlyphSecondaryPoint[0] * 0x1000,
        (gGameDatalinkGlyphSecondaryPoint[1] - 0x20) * 0x1000,
        gGameDatalinkGlyphCursorTweenDuration,
        1
    );
    Game_ConfigureDatalinkTween(
        &gGameDatalinkGlyphCursorTweenD,
        gGameDatalinkGlyphFooterPoint[0] * 0x1000,
        (gGameDatalinkGlyphFooterPoint[1] +
         gGameDatalinkGlyphFooterYOffset) * 0x1000,
        gGameDatalinkGlyphFooterTweenDuration,
        1
    );
}

/*
 * 0x020AE5BC..0x020AE62B (112 bytes).
 * Initializes the menu state and all 71 character-cell descriptors.
 */
void Game_InitializeDatalinkGlyphMenu_020ae5bc(
    Game_DatalinkGlyphMenuState *state)
{
    uint32_t character_index = 0x240U;
    uint32_t cell_index;

    Game_DatalinkStateStack_Init_020acd98(
        (Game_DatalinkStateStack *)state
    );
    *(uint32_t *)((uint8_t *)state + 4) = 5U;
    state->phase_49 = 0;
    state->selected_cell_4a = 0;
    state->font_page_47 = 0;
    state->keyboard_mode_46 = 0;

    for (cell_index = 0; cell_index < 71U; ++cell_index) {
        Game_DatalinkGlyphCellRecord *record =
            &gGameDatalinkGlyphCellRecordTable[cell_index];
        record->character_index_0c = character_index;
        record->width_tiles_18 = 2;
        record->height_tiles_1c = 2;
        record->character_address_08 =
            gGameDatalinkGlyphGraphicsRuntime.character_memory_address_6b4 +
            character_index * 0x20U;
        character_index += 4U;
    }
}

/*
 * 0x020AE634..0x020AE717 (228 bytes).
 * Clears the glyph character arena, binds its renderer and publishes the
 * initial cursor position to both display engines.
 */
void Game_SetupDatalinkGlyphDisplay_020ae634(
    Game_DatalinkGlyphMenuState *state,
    uint32_t unused_second_argument,
    uint32_t unused_third_argument,
    uint32_t unused_fourth_argument)
{
    Game_DatalinkGlyphPoint cursor;
    uint32_t resource_context;

    (void)unused_second_argument;
    (void)unused_third_argument;
    (void)unused_fourth_argument;

    state->phase_49 = 0;
    state->header_selection_42 = 0;
    state->selected_cell_4a = 0;
    state->keyboard_mode_46 = 0;
    state->transition_state_44 = 0;
    state->frame_counter_45 = 0;
    state->transition_counter_48 = 0;
    MI_CpuFill8(state->fixed_glyphs_34, 0, 0x0EU);

    Game_PrepareDatalinkGlyphResourceContext();
    resource_context = Game_GetDatalinkGlyphResourceContext();
    MIi_CpuClearFast(
        0,
        (void *)(uintptr_t)
            gGameDatalinkGlyphGraphicsRuntime.character_memory_address_6b4,
        0x8000U
    );
    gGameDatalinkGlyphRenderer.resource_address_38 =
        gGameDatalinkGlyphGraphicsRuntime.resource_address_6ac;
    Game_InitializeDatalinkGlyphRenderer(
        &gGameDatalinkGlyphRenderer,
        resource_context,
        state != 0 ? (uint8_t *)state + 0x30 : 0
    );

    Game_GetDatalinkGlyphCursorPoint_020ae830(&cursor, state);
    gGameDatalinkGlyphCursorDisplay.animation_state_cfc = 0;
    gGameDatalinkGlyphCursorDisplay.x_fx_d04 = cursor.x * 0x1000;
    gGameDatalinkGlyphCursorDisplay.y_fx_d08 = cursor.y * 0x1000;
    gGameDatalinkGlyphCursorDisplay.display_mode_cd5 = 2;
    gGameDatalinkGlyphSubDisplay.display_mode_f5 = 2;
    gGameDatalinkGlyphDisplayInputState =
        (gGameDatalinkGlyphDisplayInputState & ~UINT32_C(0x1F00)) |
        UINT32_C(0x1900);
}

/*
 * 0x020AE72C..0x020AE7A3 (120 bytes).
 * Centers the page-backed glyphs, finalizes every cell and uploads 0x800
 * bytes of character data to display engine one.
 */
void Game_UploadDatalinkGlyphCells_020ae72c(
    Game_DatalinkGlyphMenuState *state)
{
    uint32_t cell_index;

    for (cell_index = 6; cell_index < 71U; ++cell_index) {
        Game_CenterDatalinkGlyphCell_020ae7a8(state, cell_index, 0x0E);
    }
    for (cell_index = 0; cell_index < 71U; ++cell_index) {
        Game_FinalizeDatalinkGlyphCell(cell_index, 0);
    }

    DC_FlushRange(gGameDatalinkGlyphCellUploadBuffer, 0x800U);
    Game_UploadDatalinkGlyphCharacterData(
        1,
        gGameDatalinkGlyphCellUploadBuffer,
        0,
        0x800U
    );
}

/*
 * 0x020AE7A8..0x020AE81F (120 bytes).
 * Looks up a character's font metrics and horizontally centers its glyph.
 */
void Game_CenterDatalinkGlyphCell_020ae7a8(
    Game_DatalinkGlyphMenuState *state,
    uint32_t cell_index,
    int available_width)
{
    uint16_t character_code =
        Game_GetDatalinkGlyphCode_020ae908(state, (int)cell_index);
    Game_DatalinkGlyphCellObject *cell =
        Game_GetDatalinkGlyphCellObject(cell_index);
    Game_DatalinkFont *font;
    const Game_DatalinkGlyphWidths *widths;
    uint32_t glyph_index;

    Game_PrepareDatalinkGlyphResourceContext();
    font = Game_GetDatalinkGlyphFont();
    glyph_index = NNS_G2dFontFindGlyphIndex(font, character_code);
    if (glyph_index == UINT32_C(0xFFFF)) {
        glyph_index = *(const uint16_t *)(uintptr_t)(
            font->resource_address + 2U
        );
    }
    widths = NNS_G2dFontGetCharWidthsFromIndex(font, glyph_index);
    cell->glyph_x_offset_5c = (int16_t)(
        ((available_width - (int)widths->glyph_width) / 2) -
        (int)widths->left
    );
    cell->glyph_y_offset_5e = cell->glyph_y_offset_5e;
}
