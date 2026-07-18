#include "game/datalink_glyph_actions.h"

#include "game/datalink_controller.h"
#include "game/datalink_screen_update.h"

#include <stdint.h>

enum {
    DATALINK_GLYPH_MAX_NAME_LENGTH = 6,
    DATALINK_GLYPH_SPACE = 0x20,
    DATALINK_GLYPH_SOUND_ADD = 0x93,
    DATALINK_GLYPH_SOUND_DELETE = 0x94,
    DATALINK_GLYPH_SOUND_PAGE = 0x3E,
    DATALINK_GLYPH_SOUND_INVALID = 0x40,
    DATALINK_GLYPH_SOUND_COMPLETE = 0x44
};

typedef struct Game_DatalinkGlyphPlayerRecord {
    uint8_t active_00;
    uint8_t unknown_01[7];
    uint16_t name_08[DATALINK_GLYPH_MAX_NAME_LENGTH];
    uint8_t unknown_14[0x3C];
} Game_DatalinkGlyphPlayerRecord;

extern volatile uint8_t gGameDatalinkGlyphPlayerIndex;
extern Game_DatalinkGlyphPlayerRecord gGameDatalinkGlyphPlayerRecords[];
extern Game_DatalinkTweenRecord gGameDatalinkGlyphConfirmTween;
extern int32_t gGameDatalinkGlyphConfirmPoint[2];
extern uint32_t gGameDatalinkGlyphConfirmTweenDuration;

extern void Game_PlayDatalinkSoundEffect(uint32_t sound_id);
extern void Game_UpdateDatalinkGlyphPage_020af8c8(
    Game_DatalinkGlyphMenuState *state
);
extern int Game_TranslateDatalinkGlyphCode_020af978(
    const Game_DatalinkGlyphMenuState *state,
    uint32_t raw_code
);
extern void Game_FinalizeDatalinkGlyphCell(
    uint32_t cell_index,
    int argument
);
extern void Game_BeginDatalinkGlyphPlayerTransition(uint32_t player_index);
extern void Game_FinalizeDatalinkGlyphSave(void);
extern void Game_ActivateDatalinkGlyphPlayer(uint32_t player_index);
extern void Game_CommitDatalinkGlyphPlayer(uint32_t player_index);

/*
 * 0x020AF050..0x020AF0AF (96 bytes).
 * The last grid column is split into four action bands; ordinary cells retain
 * the mode already stored in the menu state.
 */
int Game_GetDatalinkGlyphKeyboardMode_020af050(
    const Game_DatalinkGlyphMenuState *state,
    int column,
    int row)
{
    if (column == 5) {
        if (row >= 0 && row < 3) {
            return 1;
        }
        if (row >= 3 && row < 7) {
            return 2;
        }
        if (row >= 7 && row < 10) {
            return 3;
        }
        if (row >= 10 && row < 13) {
            return 4;
        }
    }
    return state->keyboard_mode_46;
}

/*
 * 0x020AF0B0..0x020AF0FF (80 bytes).
 * Moves the selection to column four of its current five-column row and
 * enters confirmation mode.
 */
void Game_EnterDatalinkGlyphConfirmColumn_020af0b0(
    Game_DatalinkGlyphMenuState *state)
{
    int row;

    if (state->keyboard_mode_46 == 4) {
        return;
    }
    row = state->selected_cell_4a / 5;
    state->selected_row_4d = (int8_t)row;
    state->selected_column_4c = 4;
    state->selected_cell_4a = (int16_t)(row * 5 + 4);
    state->transition_state_44 = 1;
    state->keyboard_mode_46 = 4;
}

/*
 * 0x020AF104..0x020AF17B (120 bytes).
 * Deletes the last inline name glyph and refreshes every remaining slot.
 */
void Game_DeleteDatalinkGlyph_020af104(
    Game_DatalinkGlyphMenuState *state)
{
    uint32_t index;

    if (state->header_selection_42 == 0) {
        return;
    }
    --state->header_selection_42;
    if ((int8_t)state->header_selection_42 < 0) {
        state->header_selection_42 = 0;
    }
    state->fixed_glyphs_34[state->header_selection_42] = 0;

    for (index = 0; index < (uint32_t)state->header_selection_42 + 1U;
         ++index) {
        Game_FinalizeDatalinkGlyphCell(index, 0);
    }
    Game_PlayDatalinkSoundEffect(DATALINK_GLYPH_SOUND_DELETE);
}

/*
 * 0x020AF17C..0x020AF1F3 (120 bytes).
 * Keeps the external glyph page synchronized with keyboard modes one and two.
 */
void Game_SynchronizeDatalinkGlyphPage_020af17c(
    Game_DatalinkGlyphMenuState *state)
{
    if (state->keyboard_mode_46 == 1) {
        if (state->font_page_47 == 0) {
            return;
        }
        state->font_page_47 = 0;
    } else if (state->keyboard_mode_46 == 2) {
        if (state->font_page_47 == 1) {
            return;
        }
        state->font_page_47 = 1;
    } else {
        return;
    }

    Game_UpdateDatalinkGlyphPage_020af8c8(state);
    Game_UploadDatalinkGlyphCells_020ae72c(state);
    Game_PlayDatalinkSoundEffect(DATALINK_GLYPH_SOUND_PAGE);
}

/*
 * 0x020AF1F4..0x020AF337 (324 bytes).
 * Dispatches the selected key, appends a translated glyph, handles delete,
 * or begins confirmation depending on the active keyboard mode.
 */
void Game_CommitDatalinkGlyphCell_020af1f4(
    Game_DatalinkGlyphMenuState *state)
{
    int translated_glyph;
    uint32_t index;

    if (state->keyboard_mode_46 == 0) {
        translated_glyph = Game_TranslateDatalinkGlyphCode_020af978(
            state,
            Game_DispatchDatalinkGlyphCell_020ae954(state)
        );
        if (translated_glyph == 0) {
            Game_PlayDatalinkSoundEffect(DATALINK_GLYPH_SOUND_INVALID);
            return;
        }

        index = state->header_selection_42;
        ++state->header_selection_42;
        state->fixed_glyphs_34[index] = (uint16_t)translated_glyph;
        if (state->header_selection_42 > DATALINK_GLYPH_MAX_NAME_LENGTH - 1) {
            state->header_selection_42 = DATALINK_GLYPH_MAX_NAME_LENGTH;
            Game_EnterDatalinkGlyphConfirmColumn_020af0b0(state);
        }

        for (index = 0; index < state->header_selection_42; ++index) {
            Game_CenterDatalinkGlyphCell_020ae7a8(state, index, 0x0F);
            Game_FinalizeDatalinkGlyphCell(index, 0);
        }
        Game_PlayDatalinkSoundEffect(DATALINK_GLYPH_SOUND_ADD);
        return;
    }

    if (state->keyboard_mode_46 == 3) {
        Game_DeleteDatalinkGlyph_020af104(state);
        return;
    }
    if (state->keyboard_mode_46 != 4) {
        return;
    }

    if (state->header_selection_42 > 0 &&
        Game_DatalinkGlyphNameHasNonSpace_020af344(state) != 0) {
        Game_ConfigureDatalinkTween(
            &gGameDatalinkGlyphConfirmTween,
            gGameDatalinkGlyphConfirmPoint[0] * 0x1000,
            (gGameDatalinkGlyphConfirmPoint[1] - 0x20) * 0x1000,
            gGameDatalinkGlyphConfirmTweenDuration,
            1
        );
        state->phase_49 = 4;
        return;
    }

    if (state->header_selection_42 == DATALINK_GLYPH_MAX_NAME_LENGTH) {
        state->header_selection_42 = DATALINK_GLYPH_MAX_NAME_LENGTH - 1;
    }
    Game_PlayDatalinkSoundEffect(DATALINK_GLYPH_SOUND_INVALID);
}

/* 0x020AF344..0x020AF37F (60 bytes). */
int Game_DatalinkGlyphNameHasNonSpace_020af344(
    const Game_DatalinkGlyphMenuState *state)
{
    uint32_t index;

    for (index = 0; index < state->header_selection_42; ++index) {
        if (state->fixed_glyphs_34[index] != DATALINK_GLYPH_SPACE) {
            return 1;
        }
    }
    return 0;
}

/*
 * 0x020AF380..0x020AF423 (164 bytes).
 * Clamps the row to the action band represented by keyboard modes one to four.
 */
void Game_ClampDatalinkGlyphRowForMode_020af380(
    const Game_DatalinkGlyphMenuState *state,
    int8_t *row)
{
    switch (state->keyboard_mode_46) {
    case 1:
        if (*row < 0 || *row >= 3) {
            *row = 0;
        }
        break;
    case 2:
        if (*row <= 2 || *row >= 7) {
            *row = 3;
        }
        break;
    case 3:
        if (*row <= 6 || *row >= 10) {
            *row = 7;
        }
        break;
    case 4:
        if (*row <= 9 || *row >= 13) {
            *row = 10;
        }
        break;
    default:
        break;
    }
}

/*
 * 0x020AF424..0x020AF497 (116 bytes).
 * Copies the entered six-code-unit name into the selected player record and
 * starts Datalink state eight.
 */
void Game_SaveDatalinkGlyphName_020af424(
    Game_DatalinkGlyphMenuState *state)
{
    Game_DatalinkGlyphPlayerRecord *record =
        &gGameDatalinkGlyphPlayerRecords[gGameDatalinkGlyphPlayerIndex];
    uint32_t index;

    for (index = 0; index < state->header_selection_42; ++index) {
        record->name_08[index] = state->fixed_glyphs_34[index];
    }
    Game_ChangeDatalinkState(8, 0, 0, 0);
    state->phase_49 = 5;
    state->transition_countdown_43 = 0x1E;
}

/*
 * 0x020AF4A0..0x020AF53F (160 bytes).
 * Drives the 30-frame save transition and selects the active-player or
 * no-player completion path.
 */
void Game_UpdateDatalinkGlyphSaveTransition_020af4a0(
    Game_DatalinkGlyphMenuState *state,
    uint32_t unused_second_argument,
    uint32_t unused_third_argument,
    uint32_t unused_fourth_argument)
{
    uint32_t player_index = gGameDatalinkGlyphPlayerIndex;
    int8_t previous_countdown;

    (void)unused_second_argument;
    (void)unused_third_argument;
    (void)unused_fourth_argument;

    if (state->transition_countdown_43 == 0x0F) {
        Game_BeginDatalinkGlyphPlayerTransition(player_index);
    }
    previous_countdown = state->transition_countdown_43;
    --state->transition_countdown_43;
    if (previous_countdown != 0) {
        return;
    }

    Game_FinalizeDatalinkGlyphSave();
    if (gGameDatalinkGlyphPlayerRecords[player_index].active_00 != 0) {
        Game_ActivateDatalinkGlyphPlayer(player_index);
        Game_ChangeDatalinkState(6, 0, 0, 0);
        state->phase_49 = 6;
        state->completion_timer_4e = 0x78;
        return;
    }

    state->phase_49 = 7;
    Game_ConfigureDatalinkGlyphCursorTweens_020ae968();
    Game_PlayDatalinkSoundEffect(DATALINK_GLYPH_SOUND_COMPLETE);
}
