#include "game/datalink_glyph_menu_update.h"

#include "game/datalink_glyph_actions.h"
#include "game/datalink_glyph_page.h"
#include "game/datalink_phase_ten_finalizer.h"
#include "game/datalink_scene_command.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_GLYPH_INPUT_CONFIRM = 0x001,
    DATALINK_GLYPH_INPUT_CANCEL = 0x002,
    DATALINK_GLYPH_INPUT_FOCUS_FOOTER = 0x008,
    DATALINK_GLYPH_INPUT_DOWN = 0x010,
    DATALINK_GLYPH_INPUT_UP = 0x020,
    DATALINK_GLYPH_INPUT_LEFT = 0x040,
    DATALINK_GLYPH_INPUT_RIGHT = 0x080,
    DATALINK_GLYPH_INPUT_NEXT_PAGE = 0x100,
    DATALINK_GLYPH_INPUT_PREVIOUS_PAGE = 0x200,

    DATALINK_GLYPH_COLUMN_COUNT = 5,
    DATALINK_GLYPH_MAX_COLUMN = 5,
    DATALINK_GLYPH_MAX_ROW = 12,
    DATALINK_GLYPH_MAX_KEYBOARD_MODE = 4,
    DATALINK_GLYPH_PAGE_COUNT = 2,
    DATALINK_GLYPH_EXIT_HEADER_SELECTION = 6,
    DATALINK_GLYPH_TRANSITION_MOVING = 2,
    DATALINK_GLYPH_REMOVE_FLASH_FRAMES = 8,
    DATALINK_GLYPH_FX_ONE = 0x1000,
    DATALINK_GLYPH_SCENE_CURSOR_STATE_OFFSET = 0xcfc,
    DATALINK_GLYPH_SCENE_CURSOR_X_OFFSET = 0xd04,
    DATALINK_GLYPH_SCENE_CURSOR_Y_OFFSET = 0xd08,
    DATALINK_GLYPH_SCENE_COMMAND_CURSOR = 0x19,
    DATALINK_GLYPH_SCENE_COMMAND_KEYBOARD_MODE = 0x26,
    DATALINK_GLYPH_SCENE_COMMAND_HEADER = 0x25,
    DATALINK_GLYPH_SOUND_MOVE = 0x3d,
    DATALINK_GLYPH_SOUND_PAGE = 0x3e,
    DATALINK_GLYPH_SOUND_EXIT = 0x44,
    DATALINK_GLYPH_BLINK_VISIBLE_FRAMES = 30,
    DATALINK_GLYPH_BLINK_PERIOD = 60
};

extern void Game_UpdateDirectionalInputRepeat_0202a2a0(void);
extern void Game_PlayDatalinkSoundEffect(uint32_t sound_id);
extern int32_t FX_Sqrt(int32_t value);


_Static_assert(
    offsetof(Game_DatalinkGlyphRuntimeOwner, active_player_slot_53d) ==
        0x53d,
    "glyph runtime active-player offset");
_Static_assert(
    offsetof(Game_DatalinkGlyphPanelDisplay, header_animation_state_11c) ==
        0x11c,
    "glyph header animation-state offset");
_Static_assert(
    offsetof(Game_DatalinkGlyphPanelDisplay, cursor_animation_state_174) ==
        0x174,
    "glyph cursor animation-state offset");

static uint8_t *Game_DatalinkSceneBytes(void)
{
    return (uint8_t *)&gGameDatalinkSceneOwner;
}

static int32_t Game_ReadDatalinkSceneS32(unsigned int offset)
{
    const uint8_t *bytes = Game_DatalinkSceneBytes() + offset;

    return (int32_t)((uint32_t)bytes[0] |
        ((uint32_t)bytes[1] << 8) |
        ((uint32_t)bytes[2] << 16) |
        ((uint32_t)bytes[3] << 24));
}

static void Game_WriteDatalinkSceneS32(
    unsigned int offset,
    int32_t value)
{
    uint8_t *bytes = Game_DatalinkSceneBytes() + offset;
    uint32_t word = (uint32_t)value;

    bytes[0] = (uint8_t)word;
    bytes[1] = (uint8_t)(word >> 8);
    bytes[2] = (uint8_t)(word >> 16);
    bytes[3] = (uint8_t)(word >> 24);
}

static uint8_t *Game_DatalinkGlyphTransitionState(
    Game_DatalinkGlyphMenuState *state)
{
    return &state->transition_state_44;
}

static uint8_t *Game_DatalinkGlyphBlinkCounter(
    Game_DatalinkGlyphMenuState *state)
{
    return &state->frame_counter_45;
}

static int Game_ClampInt(int value, int minimum, int maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

static void Game_RefreshDatalinkGlyphPage(
    Game_DatalinkGlyphMenuState *state)
{
    Game_UpdateDatalinkGlyphPage_020af8c8(state);
    Game_UploadDatalinkGlyphCells_020ae72c(state);
    Game_PlayDatalinkSoundEffect(DATALINK_GLYPH_SOUND_PAGE);
}

static void Game_HandleDatalinkGlyphInput(
    Game_DatalinkGlyphMenuState *state,
    uint16_t input,
    int8_t previous_column,
    int8_t previous_row)
{
    int8_t previous_mode = state->keyboard_mode_46;

    switch (input) {
    case DATALINK_GLYPH_INPUT_CONFIRM:
        Game_CommitDatalinkGlyphCell_020af1f4(state);
        break;
    case DATALINK_GLYPH_INPUT_CANCEL:
        Game_DeleteDatalinkGlyph_020af104(state);
        state->transition_counter_48 =
            DATALINK_GLYPH_REMOVE_FLASH_FRAMES;
        Game_DrawDatalinkGlyphModeIndicators_020af7ec(state, 1);
        break;
    case DATALINK_GLYPH_INPUT_FOCUS_FOOTER:
        Game_EnterDatalinkGlyphConfirmColumn_020af0b0(state);
        break;
    case DATALINK_GLYPH_INPUT_DOWN:
        if (state->keyboard_mode_46 == 0) {
            ++state->selected_row_4d;
        } else {
            ++state->keyboard_mode_46;
            if (state->keyboard_mode_46 <
                DATALINK_GLYPH_MAX_KEYBOARD_MODE + 1) {
                Game_PlayDatalinkSoundEffect(DATALINK_GLYPH_SOUND_MOVE);
            } else {
                state->keyboard_mode_46 =
                    DATALINK_GLYPH_MAX_KEYBOARD_MODE;
            }
        }
        break;
    case DATALINK_GLYPH_INPUT_UP:
        if (state->keyboard_mode_46 == 0) {
            --state->selected_row_4d;
        } else {
            --state->keyboard_mode_46;
            if (state->keyboard_mode_46 == 0) {
                state->keyboard_mode_46 = 1;
            } else {
                Game_PlayDatalinkSoundEffect(DATALINK_GLYPH_SOUND_MOVE);
            }
        }
        break;
    case DATALINK_GLYPH_INPUT_LEFT:
        if (state->keyboard_mode_46 == 0) {
            --state->selected_column_4c;
        } else {
            Game_ClampDatalinkGlyphRowForMode_020af380(
                state, &state->selected_row_4d);
            state->keyboard_mode_46 = 0;
            Game_PlayDatalinkSoundEffect(DATALINK_GLYPH_SOUND_MOVE);
        }
        break;
    case DATALINK_GLYPH_INPUT_RIGHT:
        if (state->keyboard_mode_46 == 0) {
            ++state->selected_column_4c;
        } else {
            Game_ClampDatalinkGlyphRowForMode_020af380(
                state, &state->selected_row_4d);
        }
        break;
    case DATALINK_GLYPH_INPUT_NEXT_PAGE:
        ++state->font_page_47;
        if (state->font_page_47 >= DATALINK_GLYPH_PAGE_COUNT) {
            state->font_page_47 = 0;
        }
        if (previous_mode > 0 && previous_mode < 3) {
            ++state->keyboard_mode_46;
            if (state->keyboard_mode_46 > 2) {
                state->keyboard_mode_46 = 1;
            }
        }
        Game_RefreshDatalinkGlyphPage(state);
        break;
    case DATALINK_GLYPH_INPUT_PREVIOUS_PAGE:
        --state->font_page_47;
        if (state->font_page_47 < 0) {
            state->font_page_47 = DATALINK_GLYPH_PAGE_COUNT - 1;
        }
        if (previous_mode > 0 && previous_mode < 3) {
            --state->keyboard_mode_46;
            if (state->keyboard_mode_46 == 0) {
                state->keyboard_mode_46 = 2;
            }
        }
        Game_RefreshDatalinkGlyphPage(state);
        break;
    default:
        break;
    }

    if (state->keyboard_mode_46 == 0) {
        state->selected_row_4d = (int8_t)Game_ClampInt(
            state->selected_row_4d, 0, DATALINK_GLYPH_MAX_ROW);
        state->selected_column_4c = (int8_t)Game_ClampInt(
            state->selected_column_4c, 0, DATALINK_GLYPH_MAX_COLUMN);
        if (previous_column != state->selected_column_4c) {
            Game_PlayDatalinkSoundEffect(DATALINK_GLYPH_SOUND_MOVE);
        }
        if (previous_row != state->selected_row_4d) {
            Game_PlayDatalinkSoundEffect(DATALINK_GLYPH_SOUND_MOVE);
        }
        state->keyboard_mode_46 =
            Game_GetDatalinkGlyphKeyboardMode_020af050(
                state,
                state->selected_column_4c,
                state->selected_row_4d);
        if (state->keyboard_mode_46 != 0) {
            state->selected_column_4c = 4;
        }
    }
}

static void Game_UpdateDatalinkGlyphFooterTween(
    Game_DatalinkGlyphMenuState *state)
{
    int32_t target_y;
    int mode;

    if (Game_GetDatalinkTweenProgress(
            &gGameDatalinkGlyphCursorTweenD) != DATALINK_GLYPH_FX_ONE) {
        return;
    }
    target_y = gGameDatalinkGlyphFooterPoint[1];
    if (state->header_selection_42 == 0) {
        mode = 2;
    } else {
        target_y += gGameDatalinkGlyphFooterYOffset;
        mode = 1;
    }
    Game_ConfigureDatalinkTween(
        &gGameDatalinkGlyphCursorTweenD,
        gGameDatalinkGlyphFooterPoint[0] << 12,
        target_y << 12,
        gGameDatalinkGlyphFooterTweenDuration,
        mode);
}

static int32_t Game_GetDatalinkGlyphMotionDuration(
    int32_t old_x_fx,
    int32_t old_y_fx,
    const Game_DatalinkGlyphPoint *target)
{
    int32_t delta_x = (old_x_fx >> 12) - target->x;
    int32_t delta_y = (old_y_fx >> 12) - target->y;
    int32_t distance = FX_Sqrt(
        delta_x * delta_x + delta_y * delta_y);

    return (int32_t)(
        ((int64_t)INT32_C(0x2aaaaaab) * distance >> 39) -
        (distance >> 31));
}

static void Game_UpdateDatalinkGlyphCursorMotion(
    Game_DatalinkGlyphMenuState *state,
    const Game_DatalinkGlyphPoint *target)
{
    uint8_t *transition_state = Game_DatalinkGlyphTransitionState(state);

    if (*transition_state == DATALINK_GLYPH_TRANSITION_MOVING &&
        Game_GetDatalinkTweenProgress(
            &gGameDatalinkGlyphMotionTweenA) == DATALINK_GLYPH_FX_ONE) {
        *transition_state = 0;
        Game_DrawDatalinkGlyphModeIndicators_020af7ec(state, 1);
    }

    if (*transition_state == DATALINK_GLYPH_TRANSITION_MOVING) {
        return;
    }
    if (*transition_state == 0) {
        Game_WriteDatalinkSceneS32(
            DATALINK_GLYPH_SCENE_CURSOR_STATE_OFFSET, 0);
        Game_WriteDatalinkSceneS32(
            DATALINK_GLYPH_SCENE_CURSOR_X_OFFSET, target->x << 12);
        Game_WriteDatalinkSceneS32(
            DATALINK_GLYPH_SCENE_CURSOR_Y_OFFSET, target->y << 12);
        gGameDatalinkGlyphPanelDisplay.cursor_animation_state_174 = 0;
        gGameDatalinkGlyphPanelDisplay.cursor_x_fx_17c = target->x << 12;
        gGameDatalinkGlyphPanelDisplay.cursor_y_fx_180 = target->y << 12;
    } else {
        int32_t target_x_fx = target->x << 12;
        int32_t target_y_fx = target->y << 12;
        int32_t duration = Game_GetDatalinkGlyphMotionDuration(
            Game_ReadDatalinkSceneS32(
                DATALINK_GLYPH_SCENE_CURSOR_X_OFFSET),
            Game_ReadDatalinkSceneS32(
                DATALINK_GLYPH_SCENE_CURSOR_Y_OFFSET),
            target);

        Game_ConfigureDatalinkTween(
            &gGameDatalinkGlyphMotionTweenA,
            target_x_fx,
            target_y_fx,
            (uint32_t)duration,
            2);
        Game_ConfigureDatalinkTween(
            &gGameDatalinkGlyphMotionTweenB,
            target_x_fx,
            target_y_fx,
            (uint32_t)duration,
            2);
        *transition_state = DATALINK_GLYPH_TRANSITION_MOVING;
    }
}

/* 0x020aea40..0x020af01f */
void Game_UpdateDatalinkGlyphMenu_020aea40(
    Game_DatalinkGlyphMenuState *state)
{
    Game_DatalinkGlyphPoint cursor;
    Game_DatalinkGlyphPoint header;
    int8_t previous_mode = state->keyboard_mode_46;
    int8_t previous_column;
    int8_t previous_row;
    uint8_t *transition_state = Game_DatalinkGlyphTransitionState(state);
    uint8_t *blink_counter = Game_DatalinkGlyphBlinkCounter(state);

    state->selected_column_4c =
        (int8_t)(state->selected_cell_4a % DATALINK_GLYPH_COLUMN_COUNT);
    state->selected_row_4d =
        (int8_t)(state->selected_cell_4a / DATALINK_GLYPH_COLUMN_COUNT);
    Game_UpdateDirectionalInputRepeat_0202a2a0();
    previous_column = state->selected_column_4c;
    previous_row = state->selected_row_4d;

    if (*transition_state == 0) {
        uint16_t input = gGameDatalinkControllerInput;

        if (input == DATALINK_GLYPH_INPUT_CANCEL &&
            state->header_selection_42 == 0) {
            Game_ConfigureDatalinkGlyphCursorTweens_020ae968();
            state->phase_49 = 3;
            gGameDatalinkGlyphGraphicsRuntime.active_player_slot_53d = 0;
            Game_PlayDatalinkSoundEffect(DATALINK_GLYPH_SOUND_EXIT);
            return;
        }
        Game_HandleDatalinkGlyphInput(
            state, input, previous_column, previous_row);
        Game_UpdateDatalinkGlyphFooterTween(state);
    }

    Game_SynchronizeDatalinkGlyphPage_020af17c(state);
    state->selected_cell_4a = (int16_t)(
        state->selected_column_4c +
        state->selected_row_4d * DATALINK_GLYPH_COLUMN_COUNT);
    Game_GetDatalinkGlyphCursorPoint_020ae830(&cursor, state);

    if (state->transition_counter_48 > 0) {
        --state->transition_counter_48;
        if (state->transition_counter_48 == 0) {
            Game_DrawDatalinkGlyphModeIndicators_020af7ec(state, 1);
        }
    }
    Game_UpdateDatalinkGlyphCursorMotion(state, &cursor);

    if (previous_mode != state->keyboard_mode_46 &&
        Game_GetDatalinkTweenProgress(
            &gGameDatalinkGlyphMotionTweenA) == DATALINK_GLYPH_FX_ONE) {
        Game_DrawDatalinkGlyphModeIndicators_020af7ec(state, 1);
    }

    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkSceneOwner,
        state->keyboard_mode_46 == 0 ||
            *transition_state == DATALINK_GLYPH_TRANSITION_MOVING
            ? DATALINK_GLYPH_SCENE_COMMAND_CURSOR
            : DATALINK_GLYPH_SCENE_COMMAND_KEYBOARD_MODE);

    if (state->header_selection_42 ==
        DATALINK_GLYPH_EXIT_HEADER_SELECTION) {
        return;
    }
    Game_GetDatalinkGlyphHeaderPoint_020ae8c0(&header, state);
    gGameDatalinkGlyphPanelDisplay.header_animation_state_11c = 0;
    gGameDatalinkGlyphPanelDisplay.header_x_fx_124 = header.x << 12;
    gGameDatalinkGlyphPanelDisplay.header_y_fx_128 = header.y << 12;
    if (*blink_counter < DATALINK_GLYPH_BLINK_VISIBLE_FRAMES) {
        Game_ApplyDatalinkSceneCommand(
            &gGameDatalinkSceneOwner,
            DATALINK_GLYPH_SCENE_COMMAND_HEADER);
    }
    *blink_counter = (uint8_t)(*blink_counter + 1);
    if (*blink_counter > DATALINK_GLYPH_BLINK_PERIOD) {
        *blink_counter = 0;
    }
}
