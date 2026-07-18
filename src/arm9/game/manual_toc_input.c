#include "game/manual_toc_input.h"

#include "game/manual_display.h"
#include "game/sound_manager.h"
#include "game/scrollable_tile_buffer.h"
#include "ntmv/m2d/manual_toc_panel.h"
#include "ntmv/m2d/scroll_controls.h"

enum {
    GAME_MANUAL_SOUND_MANAGER_OFFSET = 0x0fc,
    GAME_MANUAL_TOC_PANEL_OFFSET = 0x358,
    GAME_MANUAL_SCROLL_BUTTON_OFFSET = 0x360,
    GAME_MANUAL_SCROLL_BUFFER_OFFSET = 0x40370,
    GAME_MANUAL_UPLOAD_REQUIRED_OFFSET = 0x40398,
    GAME_MANUAL_CURRENT_BUTTONS_OFFSET = 0x403a0,
    GAME_MANUAL_HELD_BUTTONS_OFFSET = 0x403a4,
    GAME_MANUAL_INTERACTION_MODE_OFFSET = 0x4061e
};

enum {
    GAME_MANUAL_BUTTON_A = 0x01,
    GAME_MANUAL_BUTTON_SELECT = 0x04,
    GAME_MANUAL_BUTTON_START = 0x08,
    GAME_MANUAL_BUTTON_RIGHT = 0x10,
    GAME_MANUAL_BUTTON_LEFT = 0x20,
    GAME_MANUAL_BUTTON_UP = 0x40,
    GAME_MANUAL_BUTTON_DOWN = 0x80,
    GAME_MANUAL_INTERACTION_DPAD_SCROLL = 4
};

extern bool NtmvM2dManualTocPanel_IsAnimating_020bb730(
    const NtmvM2dManualTocPanel *panel);
extern bool NtmvM2dManualTocPanel_MovePrevious_020bb450(
    NtmvM2dManualTocPanel *panel);
extern bool NtmvM2dManualTocPanel_MoveNext_020bb56c(
    NtmvM2dManualTocPanel *panel);
extern bool NtmvM2dManualTocPanel_ToggleGuideRequest_020bb698(
    NtmvM2dManualTocPanel *panel,
    uint8_t *show_guide);
extern bool NtmvM2dManualTocPanel_ToggleSecondaryState_020bb6c8(
    NtmvM2dManualTocPanel *panel);

#if UINTPTR_MAX > UINT32_MAX
extern void *GameManualViewer_ResolveHostPointer(
    const void *object,
    uint32_t offset);
#endif

static void *ReadViewerPointer(
    GameManualViewerPageFlow *viewer,
    uint32_t offset)
{
#if UINTPTR_MAX > UINT32_MAX
    return GameManualViewer_ResolveHostPointer(viewer, offset);
#else
    const uint8_t *bytes = (const uint8_t *)viewer + offset;
    uintptr_t value = 0;
    uint32_t index;

    for (index = 0; index < sizeof(void *); ++index) {
        value |= (uintptr_t)bytes[index] << (index * 8);
    }
    return (void *)value;
#endif
}

/* 0x020bd2d8 */
bool GameManualViewer_HandleTocPointer(
    GameManualViewerPageFlow *viewer,
    const NtmvM2dItemsPointerState *pointer)
{
    uint8_t *bytes = (uint8_t *)viewer;
    NtmvM2dManualTocAction action;
    bool handled = NtmvM2dManualTocPanel_HandlePointer(
        (NtmvM2dManualTocPanel *)ReadViewerPointer(
            viewer, GAME_MANUAL_TOC_PANEL_OFFSET),
        &action,
        pointer);

    if (handled) {
        switch (action.kind) {
        case NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_1:
            GameManualViewer_RequestExit(viewer);
            break;
        case NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_2:
            GameManualViewer_SetScrollIndicatorHeight(
                (GameManualViewerDisplay *)viewer, action.value == 1);
            break;
        case NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_3:
            GameManualViewer_ShowRequestedPage(
                viewer, action.value == 1);
            break;
        case NTMV_M2D_MANUAL_TOC_ACTION_PRIMARY_ITEM:
        case NTMV_M2D_MANUAL_TOC_ACTION_SECONDARY_ITEM:
            GameManualViewer_ShowSelectedPage(viewer);
            break;
        default:
            break;
        }

        if (pointer->pressed != 0) {
            *(uint16_t *)(bytes + GAME_MANUAL_INTERACTION_MODE_OFFSET) = 2;
        }
    }

    if (pointer->released != 0) {
        *(uint16_t *)(bytes + GAME_MANUAL_INTERACTION_MODE_OFFSET) = 0;
        handled = true;
    }
    return handled;
}

/* 0x020bd3b0 */
bool GameManualViewer_HandleScrollPointer(
    GameManualViewerPageFlow *viewer,
    const NtmvM2dScrollPointerState *pointer)
{
    uint8_t *bytes = (uint8_t *)viewer;
    NtmvM2dScrollButton *scroll_button = ReadViewerPointer(
        viewer, GAME_MANUAL_SCROLL_BUTTON_OFFSET);
    NtmvM2dScrollButtonAction action;
    bool handled;

    if (scroll_button == 0) {
        return false;
    }

    handled = NtmvM2dScrollButton_HandlePointer(
        scroll_button, &action, pointer);
    if (handled) {
        if (action.activated == 0) {
            if (pointer->pressed != 0) {
                (void)GameSoundManager_PlaySequenceArc(
                    (GameSoundManager *)(
                        bytes + GAME_MANUAL_SOUND_MANAGER_OFFSET),
                    5,
                    0x80);
            }
        } else if (action.activated == 1) {
            uint16_t *interaction_mode = (uint16_t *)(
                bytes + GAME_MANUAL_INTERACTION_MODE_OFFSET);

            if (*interaction_mode != 1) {
                bytes[GAME_MANUAL_UPLOAD_REQUIRED_OFFSET] = 1;
                *interaction_mode = 1;
            }
            (void)GameScrollableTileBuffer_RequestStep(
                (GameScrollableTileBuffer *)(
                    bytes + GAME_MANUAL_SCROLL_BUFFER_OFFSET),
                action.decrement != 0,
                action.increment != 0);
        }

        if (pointer->released != 0) {
            *(uint16_t *)(bytes + GAME_MANUAL_INTERACTION_MODE_OFFSET) = 0;
        }
    }
    return handled;
}

static void ReadScrollDirections(
    uint16_t buttons,
    bool *decrement,
    bool *increment)
{
    *decrement = (buttons & GAME_MANUAL_BUTTON_UP) != 0;
    *increment = !*decrement &&
        (buttons & GAME_MANUAL_BUTTON_DOWN) != 0;
}

/* 0x020bd830 */
bool GameManualViewer_HandleDpadScroll(GameManualViewerPageFlow *viewer)
{
    uint8_t *bytes = (uint8_t *)viewer;
    NtmvM2dManualTocPanel *toc_panel = ReadViewerPointer(
        viewer, GAME_MANUAL_TOC_PANEL_OFFSET);
    NtmvM2dScrollButton *scroll_button = ReadViewerPointer(
        viewer, GAME_MANUAL_SCROLL_BUTTON_OFFSET);
    GameScrollableTileBuffer *scroll_buffer =
        (GameScrollableTileBuffer *)(
            bytes + GAME_MANUAL_SCROLL_BUFFER_OFFSET);
    uint16_t *interaction_mode = (uint16_t *)(
        bytes + GAME_MANUAL_INTERACTION_MODE_OFFSET);
    bool decrement;
    bool increment;

    if (NtmvM2dManualTocPanel_IsAnimating_020bb730(toc_panel)) {
        return false;
    }

    if (*interaction_mode == 0) {
        ReadScrollDirections(
            *(uint16_t *)(bytes + GAME_MANUAL_CURRENT_BUTTONS_OFFSET),
            &decrement,
            &increment);
        if (decrement || increment) {
            if (!GameScrollableTileBuffer_RequestStep(
                    scroll_buffer, decrement, increment)) {
                (void)GameSoundManager_PlaySequenceArc(
                    (GameSoundManager *)(
                        bytes + GAME_MANUAL_SOUND_MANAGER_OFFSET),
                    5,
                    0x80);
            } else {
                *interaction_mode = GAME_MANUAL_INTERACTION_DPAD_SCROLL;
                NtmvM2dScrollButton_SetDirectionalHover(
                    scroll_button, decrement, increment);
                bytes[GAME_MANUAL_UPLOAD_REQUIRED_OFFSET] = 1;
                return true;
            }
        }
    } else if (*interaction_mode == GAME_MANUAL_INTERACTION_DPAD_SCROLL) {
        ReadScrollDirections(
            *(uint16_t *)(bytes + GAME_MANUAL_HELD_BUTTONS_OFFSET),
            &decrement,
            &increment);
        if (decrement || increment) {
            (void)GameScrollableTileBuffer_RequestStep(
                scroll_buffer, decrement, increment);
            return true;
        }

        *interaction_mode = 0;
        NtmvM2dScrollButton_SetDirectionalHover(
            scroll_button, false, false);
    }
    return false;
}

/* 0x020bd968 */
bool GameManualViewer_HandleButtons(GameManualViewerPageFlow *viewer)
{
    uint8_t *bytes = (uint8_t *)viewer;
    NtmvM2dManualTocPanel *toc_panel = ReadViewerPointer(
        viewer, GAME_MANUAL_TOC_PANEL_OFFSET);
    uint16_t buttons;
    bool selection_changed;

    if (NtmvM2dManualTocPanel_IsAnimating_020bb730(toc_panel)) {
        return false;
    }

    buttons = *(uint16_t *)(bytes + GAME_MANUAL_CURRENT_BUTTONS_OFFSET);
    if ((buttons & GAME_MANUAL_BUTTON_LEFT) != 0) {
        selection_changed =
            NtmvM2dManualTocPanel_MovePrevious_020bb450(toc_panel);
    } else if ((buttons & GAME_MANUAL_BUTTON_RIGHT) != 0) {
        selection_changed =
            NtmvM2dManualTocPanel_MoveNext_020bb56c(toc_panel);
    } else if ((buttons & GAME_MANUAL_BUTTON_SELECT) != 0) {
        uint8_t show_guide = 0;

        if (NtmvM2dManualTocPanel_ToggleGuideRequest_020bb698(
                toc_panel, &show_guide)) {
            GameManualViewer_ShowRequestedPage(
                viewer, show_guide != 0);
            return true;
        }
        return false;
    } else if ((buttons & GAME_MANUAL_BUTTON_A) != 0) {
        bool compact =
            NtmvM2dManualTocPanel_ToggleSecondaryState_020bb6c8(
                toc_panel);
        GameManualViewer_SetScrollIndicatorHeight(
            (GameManualViewerDisplay *)viewer, compact);
        return true;
    } else if ((buttons & GAME_MANUAL_BUTTON_START) != 0) {
        GameManualViewer_RequestExit(viewer);
        return true;
    } else {
        return false;
    }

    if (selection_changed) {
        GameManualViewer_ShowSelectedPage(viewer);
        return true;
    }
    return false;
}
