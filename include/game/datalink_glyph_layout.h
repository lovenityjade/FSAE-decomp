#ifndef GAME_DATALINK_GLYPH_LAYOUT_H
#define GAME_DATALINK_GLYPH_LAYOUT_H

#include <stdint.h>

typedef struct Game_DatalinkGlyphMenuState {
    uint8_t unknown_00[0x34];
    uint16_t fixed_glyphs_34[7];
    uint8_t header_selection_42;
    int8_t transition_countdown_43;
    uint8_t transition_state_44;
    uint8_t frame_counter_45;
    int8_t keyboard_mode_46;
    int8_t font_page_47;
    uint8_t transition_counter_48;
    uint8_t phase_49;
    int16_t selected_cell_4a;
    int8_t selected_column_4c;
    int8_t selected_row_4d;
    uint8_t completion_timer_4e;
} Game_DatalinkGlyphMenuState;

typedef struct Game_DatalinkGlyphDescriptor {
    uint32_t resource_address;
    uint32_t glyph_address;
    uint32_t glyph_size;
} Game_DatalinkGlyphDescriptor;

typedef struct Game_DatalinkGlyphPoint {
    int32_t x;
    int32_t y;
} Game_DatalinkGlyphPoint;

void Game_SetDatalinkGlyphPhaseSentinel_020ae280(void);
void Game_ResetDatalinkGlyphPhase_020ae294(void);
void Game_ActivateDatalinkGlyphPhase_020ae2b8(void);

uint32_t Game_GetDatalinkGlyphLayoutSlot_020ae318(
    int participant_index,
    const uint32_t *layout_state
);

uint32_t Game_GetDatalinkGlyphLayoutParticipant_020ae338(
    int participant_index,
    const uint32_t *layout_state
);

void Game_AddDatalinkGlyphParticipant_020ae358(
    uint32_t participant,
    uint32_t *layout_state
);

void Game_RemoveDatalinkGlyphParticipant_020ae3b0(
    uint32_t participant,
    uint32_t *layout_state
);

uint32_t Game_ResolveDatalinkGlyphLayout_020ae4a4(
    int participant_count,
    const uint32_t *participants
);

uint32_t Game_SelectDatalinkGlyphTable_020ae540(
    const Game_DatalinkGlyphMenuState *state,
    int alternate_page
);

int Game_BuildDatalinkGlyphDescriptor_020ae558(
    const Game_DatalinkGlyphMenuState *state,
    Game_DatalinkGlyphDescriptor *descriptor,
    int cell_index
);

void Game_GetDatalinkGlyphCursorPoint_020ae830(
    Game_DatalinkGlyphPoint *point,
    const Game_DatalinkGlyphMenuState *state
);

void Game_GetDatalinkGlyphHeaderPoint_020ae8c0(
    Game_DatalinkGlyphPoint *point,
    const Game_DatalinkGlyphMenuState *state
);

uint16_t Game_GetDatalinkGlyphCode_020ae908(
    const Game_DatalinkGlyphMenuState *state,
    int cell_index
);

uint32_t Game_DispatchDatalinkGlyphCell_020ae954(
    Game_DatalinkGlyphMenuState *state
);

void Game_ConfigureDatalinkGlyphCursorTweens_020ae968(void);

void Game_InitializeDatalinkGlyphMenu_020ae5bc(
    Game_DatalinkGlyphMenuState *state
);

void Game_SetupDatalinkGlyphDisplay_020ae634(
    Game_DatalinkGlyphMenuState *state,
    uint32_t unused_second_argument,
    uint32_t unused_third_argument,
    uint32_t unused_fourth_argument
);

void Game_UploadDatalinkGlyphCells_020ae72c(
    Game_DatalinkGlyphMenuState *state
);

void Game_CenterDatalinkGlyphCell_020ae7a8(
    Game_DatalinkGlyphMenuState *state,
    uint32_t cell_index,
    int available_width
);

#endif
