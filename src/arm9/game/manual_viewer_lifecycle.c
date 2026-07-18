#include "game/manual_viewer_lifecycle.h"

#include "game/input_state.h"
#include "game/scrollable_tile_buffer.h"
#include "game/sound_manager.h"
#include "ntmv/m2d/resource_accessor.h"

enum {
    GAME_MANUAL_SOUND_MANAGER_OFFSET = 0x0fc,
    GAME_MANUAL_RENDERER_0_OFFSET = 0x340,
    GAME_MANUAL_RENDERER_1_OFFSET = 0x344,
    GAME_MANUAL_PAGE_INDEX_OFFSET = 0x348,
    GAME_MANUAL_ROOT_PANEL_0_OFFSET = 0x34c,
    GAME_MANUAL_ROOT_PANEL_1_OFFSET = 0x350,
    GAME_MANUAL_HEADER_PANEL_OFFSET = 0x354,
    GAME_MANUAL_TOC_PANEL_OFFSET = 0x358,
    GAME_MANUAL_PAGE_RESOURCE_OFFSET = 0x35c,
    GAME_MANUAL_SCROLL_BUTTON_OFFSET = 0x360,
    GAME_MANUAL_SCROLL_INDICATOR_0_OFFSET = 0x364,
    GAME_MANUAL_SCROLL_INDICATOR_1_OFFSET = 0x368,
    GAME_MANUAL_SCROLL_BUFFER_OFFSET = 0x40370,
    GAME_MANUAL_CURRENT_PAGE_OFFSET = 0x4039c,
    GAME_MANUAL_POINTER_HISTORY_OFFSET = 0x403a0,
    GAME_MANUAL_TOUCH_SAMPLER_OFFSET = 0x403a6,
    GAME_MANUAL_SHARED_ACCESSOR_OFFSET = 0x403f8,
    GAME_MANUAL_LOCALIZED_ACCESSOR_OFFSET = 0x40500,
    GAME_MANUAL_PAGE_STATE_OFFSET = 0x40608,
    GAME_MANUAL_POINTER_AUX_OFFSET = 0x4060c,
    GAME_MANUAL_INPUT_STATE_OFFSET = 0x4060e,
    GAME_MANUAL_FLAGS_OFFSET = 0x40620
};

static void ClearWord(uint8_t *bytes, uint32_t offset)
{
    *(uint32_t *)(bytes + offset) = 0;
}

static void ClearHalfword(uint8_t *bytes, uint32_t offset)
{
    *(uint16_t *)(bytes + offset) = 0;
}

/* 0x020bc588 */
GameManualViewerPageFlow *GameManualViewer_Construct_020bc588(
    GameManualViewerPageFlow *viewer)
{
    uint8_t *bytes = (uint8_t *)viewer;
    uint32_t index;

    ClearWord(bytes, 0x10);
    ClearWord(bytes, 0x14);
    bytes[0x80] = 0;
    bytes[0xec] = 0;
    GameSoundManager_InitEmpty(
        (GameSoundManager *)(bytes + GAME_MANUAL_SOUND_MANAGER_OFFSET));

    ClearWord(bytes, GAME_MANUAL_RENDERER_0_OFFSET);
    ClearWord(bytes, GAME_MANUAL_RENDERER_1_OFFSET);
    ClearWord(bytes, GAME_MANUAL_PAGE_INDEX_OFFSET);
    ClearWord(bytes, GAME_MANUAL_HEADER_PANEL_OFFSET);
    ClearWord(bytes, GAME_MANUAL_TOC_PANEL_OFFSET);
    ClearWord(bytes, GAME_MANUAL_PAGE_RESOURCE_OFFSET);
    ClearWord(bytes, GAME_MANUAL_SCROLL_BUTTON_OFFSET);
    GameScrollableTileBuffer_Init(
        (GameScrollableTileBuffer *)(
            bytes + GAME_MANUAL_SCROLL_BUFFER_OFFSET));

    ClearWord(bytes, GAME_MANUAL_CURRENT_PAGE_OFFSET);
    ClearHalfword(bytes, GAME_MANUAL_POINTER_HISTORY_OFFSET);
    ClearHalfword(bytes, GAME_MANUAL_POINTER_HISTORY_OFFSET + 2);
    ClearHalfword(bytes, GAME_MANUAL_POINTER_HISTORY_OFFSET + 4);
    GameTouchSampler_Init(
        (GameTouchSampler *)(bytes + GAME_MANUAL_TOUCH_SAMPLER_OFFSET));
    NtmvM2dArcResourceAccessor_Init(
        (NtmvM2dArcResourceAccessor *)(
            bytes + GAME_MANUAL_SHARED_ACCESSOR_OFFSET));
    NtmvM2dArcResourceAccessor_Init(
        (NtmvM2dArcResourceAccessor *)(
            bytes + GAME_MANUAL_LOCALIZED_ACCESSOR_OFFSET));

    ClearWord(bytes, GAME_MANUAL_PAGE_STATE_OFFSET);
    ClearHalfword(bytes, GAME_MANUAL_POINTER_AUX_OFFSET);
    ClearHalfword(bytes, GAME_MANUAL_INPUT_STATE_OFFSET);
    ClearHalfword(bytes, GAME_MANUAL_INPUT_STATE_OFFSET + 2);
    ClearHalfword(bytes, GAME_MANUAL_INPUT_STATE_OFFSET + 4);
    ClearHalfword(bytes, GAME_MANUAL_INPUT_STATE_OFFSET + 6);
    ClearHalfword(bytes, GAME_MANUAL_INPUT_STATE_OFFSET + 0x0e);
    ClearHalfword(bytes, GAME_MANUAL_INPUT_STATE_OFFSET + 0x10);
    for (index = 0; index < 5; ++index) {
        bytes[GAME_MANUAL_FLAGS_OFFSET + index] = 0;
    }
    for (index = 0; index < 3; ++index) {
        ClearWord(bytes, 0xf0 + index * 4);
    }
    for (index = 0; index < 2; ++index) {
        ClearWord(bytes, GAME_MANUAL_ROOT_PANEL_0_OFFSET + index * 4);
        ClearWord(
            bytes,
            GAME_MANUAL_SCROLL_INDICATOR_0_OFFSET + index * 4);
    }
    return viewer;
}
