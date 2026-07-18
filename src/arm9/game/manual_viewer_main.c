#include "game/manual_viewer_main.h"

#include "game/graphics_state.h"
#include "game/input_state.h"
#include "game/manual_display.h"
#include "game/manual_frame.h"
#include "game/manual_page_pointer.h"
#include "game/manual_page_transition.h"
#include "game/manual_toc_input.h"

#include <stdint.h>

enum {
    GAME_MANUAL_BUTTON_STATE_OFFSET = 0x403a0,
    GAME_MANUAL_TOUCH_SAMPLER_OFFSET = 0x403a6,
    GAME_MANUAL_INTERACTION_MODE_OFFSET = 0x4061e,
    GAME_MANUAL_EXIT_REQUESTED_OFFSET = 0x40621,
    GAME_MANUAL_FADE_FRAME_COUNT = 17,
    GAME_MANUAL_SLEEP_TRIGGER = 4,
    GAME_MANUAL_LID_CLOSED_MASK = 0x8000
};

enum GameManualInteractionMode {
    GAME_MANUAL_INTERACTION_ANY = 0,
    GAME_MANUAL_INTERACTION_SCROLL = 1,
    GAME_MANUAL_INTERACTION_TOC = 2,
    GAME_MANUAL_INTERACTION_PAGE_DRAG = 3,
    GAME_MANUAL_INTERACTION_DPAD_SCROLL = 4
};

extern void OS_WaitVBlankIntr(void);
extern void GX_DispOn(void);
extern void PM_GoSleepMode(
    uint32_t trigger, uint32_t key_logic, uint32_t lcd_power);
static bool DispatchInput(
    GameManualViewerPageFlow *viewer,
    int16_t interaction_mode,
    const GameTouchState *touch)
{
    const NtmvM2dScrollPointerState *scroll_pointer =
        (const NtmvM2dScrollPointerState *)touch;
    const NtmvM2dItemsPointerState *toc_pointer =
        (const NtmvM2dItemsPointerState *)touch;

    switch (interaction_mode) {
    case GAME_MANUAL_INTERACTION_ANY:
        if (GameManualViewer_HandleScrollPointer(viewer, scroll_pointer)) {
            return true;
        }
        if (GameManualViewer_HandleTocPointer(viewer, toc_pointer)) {
            return true;
        }
        if (GameManualViewer_HandlePagePointer_020bd474(viewer, touch)) {
            return true;
        }
        if (GameManualViewer_HandleDpadScroll(viewer)) {
            return true;
        }
        return GameManualViewer_HandleButtons(viewer);
    case GAME_MANUAL_INTERACTION_SCROLL:
        return GameManualViewer_HandleScrollPointer(viewer, scroll_pointer);
    case GAME_MANUAL_INTERACTION_TOC:
        return GameManualViewer_HandleTocPointer(viewer, toc_pointer);
    case GAME_MANUAL_INTERACTION_PAGE_DRAG:
        return GameManualViewer_HandlePagePointer_020bd474(viewer, touch);
    case GAME_MANUAL_INTERACTION_DPAD_SCROLL:
        return GameManualViewer_HandleDpadScroll(viewer);
    default:
        return false;
    }
}

static void RenderMainLoopFrame(
    GameManualViewerPageFlow *viewer,
    int32_t brightness)
{
    GameManualViewer_RenderFrame_020bd00c(viewer);
    OS_WaitVBlankIntr();
    Game_SetMasterBrightnessBoth(brightness);
    GameManualViewer_SetPageTransitionOffset_020bd128(viewer, 0);
}

/* 0x020bce14..0x020bcfff */
void GameManualViewer_RunMainLoop_020bce14(
    GameManualViewerPageFlow *viewer)
{
    uint8_t *bytes = (uint8_t *)viewer;
    GameButtonState *buttons =
        (GameButtonState *)(bytes + GAME_MANUAL_BUTTON_STATE_OFFSET);
    GameTouchSampler *touch_sampler =
        (GameTouchSampler *)(bytes + GAME_MANUAL_TOUCH_SAMPLER_OFFSET);
    int16_t *interaction_mode =
        (int16_t *)(bytes + GAME_MANUAL_INTERACTION_MODE_OFFSET);
    GameTouchState touch;
    int32_t brightness;

    OS_WaitVBlankIntr();
    GX_DispOn();
    gGameSubDisplayControl_04001000 |= 0x00010000u;

    for (brightness = GAME_MANUAL_FADE_FRAME_COUNT - 1;
         brightness >= 0;
         --brightness) {
        RenderMainLoopFrame(viewer, brightness);
    }

    while (bytes[GAME_MANUAL_EXIT_REQUESTED_OFFSET] == 0) {
        GameButtonState_Update(buttons);
        if ((gGameXyButtonState_02ffffa8 & GAME_MANUAL_LID_CLOSED_MASK) != 0) {
            PM_GoSleepMode(GAME_MANUAL_SLEEP_TRIGGER, 0, 0);
        }

        touch.x = 0;
        touch.y = 0;
        (void)GameTouchSampler_Update(touch_sampler, &touch);
        (void)DispatchInput(viewer, *interaction_mode, &touch);

        GameManualViewer_RenderFrame_020bd00c(viewer);
        OS_WaitVBlankIntr();
        GameManualViewer_SetPageTransitionOffset_020bd128(viewer, 0);
    }

    for (brightness = 0;
         brightness < GAME_MANUAL_FADE_FRAME_COUNT;
         ++brightness) {
        RenderMainLoopFrame(viewer, brightness);
    }
    GameManualViewer_Shutdown((GameManualViewerDisplay *)viewer);
}
