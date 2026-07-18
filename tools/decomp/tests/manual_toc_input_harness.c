#include "game/manual_toc_input.h"

#include "game/manual_display.h"
#include "game/sound_manager.h"
#include "game/scrollable_tile_buffer.h"
#include "ntmv/m2d/manual_toc_panel.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static uint8_t sViewer[0x40620];
static NtmvM2dManualTocPanel sTocPanel;
static bool sHandled;
static NtmvM2dManualTocAction sAction;
static unsigned int sExitCount;
static unsigned int sHeightCount;
static unsigned int sRequestedCount;
static unsigned int sSelectedCount;
static bool sLastValue;
static NtmvM2dScrollButton sScrollButton;
static bool sScrollButtonAvailable = true;
static bool sScrollHandled;
static NtmvM2dScrollButtonAction sScrollAction;
static unsigned int sSoundCount;
static unsigned int sStepCount;
static bool sExpectedDecrement;
static bool sExpectedIncrement;
static bool sStepResult = true;
static bool sTocAnimating;
static unsigned int sHoverCount;
static bool sExpectedHoverDecrement;
static bool sExpectedHoverIncrement;
static bool sMovePreviousResult;
static bool sMoveNextResult;
static bool sGuideHandled;
static uint8_t sGuideValue;
static bool sSecondaryToggleValue;
static unsigned int sMovePreviousCount;
static unsigned int sMoveNextCount;
static unsigned int sGuideCount;
static unsigned int sSecondaryToggleCount;

void *GameManualViewer_ResolveHostPointer(
    const void *object,
    uint32_t offset)
{
    assert(object == sViewer);
    if (offset == 0x358) {
        return &sTocPanel;
    }
    if (offset == 0x360) {
        return sScrollButtonAvailable ? &sScrollButton : 0;
    }
    assert(0);
    return 0;
}

bool NtmvM2dManualTocPanel_HandlePointer(
    NtmvM2dManualTocPanel *panel,
    NtmvM2dManualTocAction *action,
    const NtmvM2dItemsPointerState *pointer)
{
    assert(panel == &sTocPanel);
    assert(pointer != 0);
    *action = sAction;
    return sHandled;
}

void GameManualViewer_RequestExit(GameManualViewerPageFlow *viewer)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    ++sExitCount;
}

void GameManualViewer_SetScrollIndicatorHeight(
    GameManualViewerDisplay *viewer,
    bool expanded)
{
    assert(viewer == (GameManualViewerDisplay *)sViewer);
    sLastValue = expanded;
    ++sHeightCount;
}

void GameManualViewer_ShowRequestedPage(
    GameManualViewerPageFlow *viewer,
    bool show_guide)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    sLastValue = show_guide;
    ++sRequestedCount;
}

void GameManualViewer_ShowSelectedPage(GameManualViewerPageFlow *viewer)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    ++sSelectedCount;
}

bool NtmvM2dScrollButton_HandlePointer(
    NtmvM2dScrollButton *button,
    NtmvM2dScrollButtonAction *action,
    const NtmvM2dScrollPointerState *pointer)
{
    assert(button == &sScrollButton);
    assert(pointer != 0);
    *action = sScrollAction;
    return sScrollHandled;
}

GameSoundEntry *GameSoundManager_PlaySequenceArc(
    GameSoundManager *manager,
    int32_t sequence_id,
    int32_t channel_priority)
{
    assert(manager == (GameSoundManager *)(sViewer + 0xfc));
    assert(sequence_id == 5);
    assert(channel_priority == 0x80);
    ++sSoundCount;
    return 0;
}

bool GameScrollableTileBuffer_RequestStep(
    GameScrollableTileBuffer *buffer,
    bool decrement,
    bool increment)
{
    assert(buffer == (GameScrollableTileBuffer *)(sViewer + 0x40370));
    assert(decrement == sExpectedDecrement);
    assert(increment == sExpectedIncrement);
    ++sStepCount;
    return sStepResult;
}

bool NtmvM2dManualTocPanel_IsAnimating_020bb730(
    const NtmvM2dManualTocPanel *panel)
{
    assert(panel == &sTocPanel);
    return sTocAnimating;
}

void NtmvM2dScrollButton_SetDirectionalHover(
    NtmvM2dScrollButton *button,
    bool decrement,
    bool increment)
{
    assert(button == &sScrollButton);
    assert(decrement == sExpectedHoverDecrement);
    assert(increment == sExpectedHoverIncrement);
    ++sHoverCount;
}

bool NtmvM2dManualTocPanel_MovePrevious_020bb450(
    NtmvM2dManualTocPanel *panel)
{
    assert(panel == &sTocPanel);
    ++sMovePreviousCount;
    return sMovePreviousResult;
}

bool NtmvM2dManualTocPanel_MoveNext_020bb56c(
    NtmvM2dManualTocPanel *panel)
{
    assert(panel == &sTocPanel);
    ++sMoveNextCount;
    return sMoveNextResult;
}

bool NtmvM2dManualTocPanel_ToggleGuideRequest_020bb698(
    NtmvM2dManualTocPanel *panel,
    uint8_t *show_guide)
{
    assert(panel == &sTocPanel);
    *show_guide = sGuideValue;
    ++sGuideCount;
    return sGuideHandled;
}

bool NtmvM2dManualTocPanel_ToggleSecondaryState_020bb6c8(
    NtmvM2dManualTocPanel *panel)
{
    assert(panel == &sTocPanel);
    ++sSecondaryToggleCount;
    return sSecondaryToggleValue;
}

static bool Run(
    bool handled,
    uint16_t kind,
    uint16_t value,
    uint8_t pressed,
    uint8_t released)
{
    NtmvM2dItemsPointerState pointer;

    memset(&pointer, 0, sizeof(pointer));
    sHandled = handled;
    sAction.kind = kind;
    sAction.value = value;
    pointer.pressed = pressed;
    pointer.released = released;
    return GameManualViewer_HandleTocPointer(
        (GameManualViewerPageFlow *)sViewer, &pointer);
}

static bool RunScroll(
    bool handled,
    uint16_t activated,
    uint8_t decrement,
    uint8_t increment,
    uint8_t pressed,
    uint8_t released)
{
    NtmvM2dScrollPointerState pointer;

    memset(&pointer, 0, sizeof(pointer));
    sScrollHandled = handled;
    sScrollAction.activated = activated;
    sScrollAction.decrement = decrement;
    sScrollAction.increment = increment;
    pointer.pressed = pressed;
    pointer.released = released;
    sExpectedDecrement = decrement != 0;
    sExpectedIncrement = increment != 0;
    return GameManualViewer_HandleScrollPointer(
        (GameManualViewerPageFlow *)sViewer, &pointer);
}

int main(void)
{
    uint16_t *mode = (uint16_t *)(sViewer + 0x4061e);

    memset(sViewer, 0, sizeof(sViewer));
    assert(!Run(false, NTMV_M2D_MANUAL_TOC_ACTION_NONE, 0, 0, 0));
    assert(*mode == 0);

    *mode = 7;
    assert(Run(false, NTMV_M2D_MANUAL_TOC_ACTION_NONE, 0, 0, 1));
    assert(*mode == 0);

    assert(Run(true, NTMV_M2D_MANUAL_TOC_ACTION_NONE, 0, 1, 0));
    assert(*mode == 2);

    assert(Run(true, NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_1, 0, 0, 0));
    assert(sExitCount == 1);

    assert(Run(true, NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_2, 1, 0, 0));
    assert(sHeightCount == 1 && sLastValue);
    assert(Run(true, NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_2, 2, 0, 0));
    assert(sHeightCount == 2 && !sLastValue);

    assert(Run(true, NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_3, 1, 0, 0));
    assert(sRequestedCount == 1 && sLastValue);
    assert(Run(true, NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_3, 0, 0, 0));
    assert(sRequestedCount == 2 && !sLastValue);

    assert(Run(true, NTMV_M2D_MANUAL_TOC_ACTION_PRIMARY_ITEM, 0, 0, 0));
    assert(Run(true, NTMV_M2D_MANUAL_TOC_ACTION_SECONDARY_ITEM, 0, 0, 0));
    assert(sSelectedCount == 2);

    *mode = 9;
    assert(Run(true, NTMV_M2D_MANUAL_TOC_ACTION_PRIMARY_ITEM, 0, 1, 1));
    assert(*mode == 0);
    assert(sSelectedCount == 3);

    sScrollButtonAvailable = false;
    assert(!RunScroll(true, 1, 1, 0, 0, 0));
    sScrollButtonAvailable = true;
    assert(!RunScroll(false, 0, 0, 0, 1, 0));
    assert(sSoundCount == 0 && sStepCount == 0);

    assert(RunScroll(true, 0, 0, 0, 1, 0));
    assert(sSoundCount == 1 && sStepCount == 0);

    *(uint16_t *)(sViewer + 0x4061e) = 0;
    sViewer[0x40398] = 0;
    assert(RunScroll(true, 1, 1, 0, 0, 0));
    assert(*(uint16_t *)(sViewer + 0x4061e) == 1);
    assert(sViewer[0x40398] == 1);
    assert(sStepCount == 1);

    sViewer[0x40398] = 0;
    assert(RunScroll(true, 1, 0, 1, 0, 1));
    assert(*(uint16_t *)(sViewer + 0x4061e) == 0);
    assert(sViewer[0x40398] == 0);
    assert(sStepCount == 2);

    sTocAnimating = true;
    assert(!GameManualViewer_HandleDpadScroll(
        (GameManualViewerPageFlow *)sViewer));
    sTocAnimating = false;

    *(uint16_t *)(sViewer + 0x4061e) = 0;
    *(uint16_t *)(sViewer + 0x403a0) = 0;
    assert(!GameManualViewer_HandleDpadScroll(
        (GameManualViewerPageFlow *)sViewer));

    *(uint16_t *)(sViewer + 0x403a0) = 0x40 | 0x80;
    sExpectedDecrement = true;
    sExpectedIncrement = false;
    sStepResult = false;
    assert(!GameManualViewer_HandleDpadScroll(
        (GameManualViewerPageFlow *)sViewer));
    assert(sSoundCount == 2);

    *(uint16_t *)(sViewer + 0x403a0) = 0x80;
    sExpectedDecrement = false;
    sExpectedIncrement = true;
    sExpectedHoverDecrement = false;
    sExpectedHoverIncrement = true;
    sStepResult = true;
    sViewer[0x40398] = 0;
    assert(GameManualViewer_HandleDpadScroll(
        (GameManualViewerPageFlow *)sViewer));
    assert(*(uint16_t *)(sViewer + 0x4061e) == 4);
    assert(sViewer[0x40398] == 1);
    assert(sHoverCount == 1);

    *(uint16_t *)(sViewer + 0x403a4) = 0x40;
    sExpectedDecrement = true;
    sExpectedIncrement = false;
    assert(GameManualViewer_HandleDpadScroll(
        (GameManualViewerPageFlow *)sViewer));

    *(uint16_t *)(sViewer + 0x403a4) = 0;
    sExpectedHoverDecrement = false;
    sExpectedHoverIncrement = false;
    assert(!GameManualViewer_HandleDpadScroll(
        (GameManualViewerPageFlow *)sViewer));
    assert(*(uint16_t *)(sViewer + 0x4061e) == 0);
    assert(sHoverCount == 2);

    sTocAnimating = true;
    assert(!GameManualViewer_HandleButtons(
        (GameManualViewerPageFlow *)sViewer));
    sTocAnimating = false;

    *(uint16_t *)(sViewer + 0x403a0) = 0;
    assert(!GameManualViewer_HandleButtons(
        (GameManualViewerPageFlow *)sViewer));

    sMovePreviousResult = false;
    *(uint16_t *)(sViewer + 0x403a0) = 0x20 | 0x10;
    assert(!GameManualViewer_HandleButtons(
        (GameManualViewerPageFlow *)sViewer));
    assert(sMovePreviousCount == 1 && sMoveNextCount == 0);

    sMovePreviousResult = true;
    assert(GameManualViewer_HandleButtons(
        (GameManualViewerPageFlow *)sViewer));
    assert(sSelectedCount == 4);

    sMoveNextResult = true;
    *(uint16_t *)(sViewer + 0x403a0) = 0x10;
    assert(GameManualViewer_HandleButtons(
        (GameManualViewerPageFlow *)sViewer));
    assert(sMoveNextCount == 1 && sSelectedCount == 5);

    sGuideHandled = false;
    *(uint16_t *)(sViewer + 0x403a0) = 0x04 | 0x01;
    assert(!GameManualViewer_HandleButtons(
        (GameManualViewerPageFlow *)sViewer));
    assert(sGuideCount == 1 && sSecondaryToggleCount == 0);

    sGuideHandled = true;
    sGuideValue = 1;
    assert(GameManualViewer_HandleButtons(
        (GameManualViewerPageFlow *)sViewer));
    assert(sGuideCount == 2 && sRequestedCount == 3 && sLastValue);

    sSecondaryToggleValue = true;
    *(uint16_t *)(sViewer + 0x403a0) = 0x01;
    assert(GameManualViewer_HandleButtons(
        (GameManualViewerPageFlow *)sViewer));
    assert(sSecondaryToggleCount == 1 && sHeightCount == 3 && sLastValue);

    *(uint16_t *)(sViewer + 0x403a0) = 0x08;
    assert(GameManualViewer_HandleButtons(
        (GameManualViewerPageFlow *)sViewer));
    assert(sExitCount == 2);
    return 0;
}
