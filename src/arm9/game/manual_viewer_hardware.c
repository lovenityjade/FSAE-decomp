#include "game/manual_viewer_lifecycle.h"

#include "game/graphics_state.h"
#include "game/input_state.h"
#include "game/manual_display.h"

#include <stdint.h>

enum {
    GAME_MANUAL_BUTTON_STATE_OFFSET = 0x403a0,
    GAME_MANUAL_TOUCH_SAMPLER_OFFSET = 0x403a6,
    GAME_MANUAL_DMA_OVERRIDDEN_OFFSET = 0x40622,
    GAME_MANUAL_PREVIOUS_VBLANK_OFFSET = 0x40623,
    GAME_MANUAL_PREVIOUS_EXTERNAL_STATE_OFFSET = 0x40624
};

void OSi_InitCommon(void);

/* 0x020bc8bc */
void GameManualViewer_InitializeHardware_020bc8bc(
    GameManualViewerPageFlow *viewer,
    const uint32_t initial_context[4])
{
    uint8_t *bytes = (uint8_t *)viewer;
    uint32_t index;
    bool previous_external_state;
    bool previous_vblank_state;

    for (index = 0; index < 4; ++index) {
        ((uint32_t *)bytes)[index] = initial_context[index];
    }

    OSi_InitCommon();
    Game_GraphicsPreInitHook_02012c3c();
    previous_external_state =
        Game_IsExternalDisplayStateActive_0218a870();
    bytes[GAME_MANUAL_PREVIOUS_EXTERNAL_STATE_OFFSET] =
        previous_external_state ? 1 : 0;
    if (!previous_external_state) {
        Game_SetExternalDisplayState_0218a860(1);
    }

    previous_vblank_state = GX_VBlankIntr(true);
    bytes[GAME_MANUAL_PREVIOUS_VBLANK_OFFSET] =
        previous_vblank_state ? 1 : 0;
    GX_Init();
    if (!Game_IsGraphicsDefaultDmaConfigured_020bc528()) {
        GX_SetDefaultDMA(3);
        bytes[GAME_MANUAL_DMA_OVERRIDDEN_OFFSET] = 1;
    }
    GX_DispOff();
    gGameSubDisplayControl_04001000 &= ~0x00010000u;

    GameButtonState_Update(
        (GameButtonState *)(bytes + GAME_MANUAL_BUTTON_STATE_OFFSET));
    GameTouchSampler_Start(
        (GameTouchSampler *)(bytes + GAME_MANUAL_TOUCH_SAMPLER_OFFSET));
    (void)GX_VBlankIntr(true);
}
