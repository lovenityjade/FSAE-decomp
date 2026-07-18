#include "game/manual_display.h"

#include "ntmv/m2d/scroll_controls.h"

enum {
    GAME_MANUAL_MODE_PANEL_OFFSET = 0x358,
    GAME_MANUAL_SCROLL_INDICATOR_OFFSET = 0x364,
    GAME_MANUAL_INPUT_HISTORY_OFFSET = 0x40616,
    GAME_MANUAL_INPUT_MODE_OFFSET = 0x4061c,
    GAME_MANUAL_MODE_PANEL_FLAG_OFFSET = 0xad,
    GAME_MANUAL_COMPACT_HEIGHT = 0xae,
    GAME_MANUAL_EXPANDED_HEIGHT = 0x142
};

static void *ReadEmbeddedPointer(const void *object, uint32_t offset)
{
    const uint8_t *bytes = (const uint8_t *)object + offset;
    uintptr_t value = 0;
    uint32_t index;

    for (index = 0; index < sizeof(void *); ++index) {
        value |= (uintptr_t)bytes[index] << (index * 8);
    }
    return (void *)value;
}

static bool UsesSubScreenOnly(const GameManualViewerDisplay *viewer)
{
    const uint8_t *mode_panel = ReadEmbeddedPointer(
        viewer, GAME_MANUAL_MODE_PANEL_OFFSET);
    return mode_panel[GAME_MANUAL_MODE_PANEL_FLAG_OFFSET] != 0;
}

/* 0x020bc574 */
int32_t GameManualViewer_GetPointerContentTop_020bc574(
    const NtmvM2dManualTocPanel *toc_panel)
{
    const uint8_t *bytes = (const uint8_t *)toc_panel;

    return bytes[GAME_MANUAL_MODE_PANEL_FLAG_OFFSET] != 0 ?
        0 : 0x94;
}

/* 0x020be244 */
void GameManualViewer_Shutdown(GameManualViewerDisplay *viewer)
{
    uint8_t *bytes = (uint8_t *)viewer;

    TP_RequestAutoSamplingStopAsync();
    TP_WaitBusy(4);
    TP_CheckError(4);
    NNS_SndStopSoundAll();
    GX_DispOff();
    gGameSubDisplayControl_04001000 &= ~0x00010000u;
    if (bytes[0x40622] != 0) {
        GX_SetDefaultDMA(UINT32_MAX);
    }
    GX_Init();
    if (bytes[0x40623] != 0) {
        GX_VBlankIntr(true);
    }
    if (bytes[0x40624] == 0) {
        Game_SetExternalDisplayState_0218a860(0);
    }
    if (bytes[0x80] != 0) {
        NNS_FndUnmountArchive(bytes + 0x18);
    }
    if (bytes[0xec] != 0) {
        NNS_FndUnmountArchive(bytes + 0x84);
    }
}

/* 0x020be2f8 */
void GameManualViewer_ResetInputHistory(GameManualViewerDisplay *viewer)
{
    uint8_t *bytes = (uint8_t *)viewer;
    int16_t *history = (int16_t *)(bytes + GAME_MANUAL_INPUT_HISTORY_OFFSET);
    int32_t index;

    for (index = 0; index < 3; ++index) {
        history[index] = 0;
    }
    *(int16_t *)(bytes + GAME_MANUAL_INPUT_MODE_OFFSET) = 0;
}

/* 0x020be32c */
void GameManualViewer_SetBlendBrightness(
    GameManualViewerDisplay *viewer,
    int32_t brightness)
{
    if (!UsesSubScreenOnly(viewer)) {
        G2x_SetBlendBrightness_(0x04000050u, 0x28, brightness);
    }
    G2x_SetBlendBrightness_(0x04001050u, 0x28, brightness);
}

/* 0x020be370 */
void GameManualViewer_SetRevealWindowOffset(
    GameManualViewerDisplay *viewer,
    int32_t horizontal_offset)
{
    uint16_t left = 0;
    uint16_t right = 0x100;
    uint16_t horizontal_range;

    if (horizontal_offset > 0) {
        right = (uint16_t)(0x100 - horizontal_offset);
    } else {
        left = (uint16_t)-horizontal_offset;
    }
    horizontal_range = (uint16_t)(
        ((uint16_t)(left << 8) & 0xff00u) | (right & 0xffu));

    if (!UsesSubScreenOnly(viewer)) {
        gGameMainWindow0Registers_04000040[0] = horizontal_range;
        gGameMainWindow0Registers_04000040[2] = 0x0094;
    }
    gGameSubWindow0Registers_04001040[0] = horizontal_range;
    gGameSubWindow0Registers_04001040[2] = 0x12c0;
}

/* 0x020be3e0 */
void GameManualViewer_SetScrollIndicatorHeight(
    GameManualViewerDisplay *viewer,
    bool compact_layout)
{
    NtmvM2dScrollIndicator *indicator = ReadEmbeddedPointer(
        viewer, GAME_MANUAL_SCROLL_INDICATOR_OFFSET);
    int16_t height = compact_layout ?
        GAME_MANUAL_COMPACT_HEIGHT : GAME_MANUAL_EXPANDED_HEIGHT;
    NtmvM2dPoint position = {
        indicator->base.base.local_position.x,
        (int16_t)(height / 2 + 0x12),
    };

    indicator->base.base.vtable->set_position(
        (NtmvM2dUIElement *)indicator, &position);
    NtmvM2dScrollIndicator_RepositionChildren(indicator, height);
}
