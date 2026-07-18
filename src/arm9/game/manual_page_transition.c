#include "game/manual_page_transition.h"

#include "game/circular_tile_upload.h"
#include "game/manual_display.h"
#include "ntmv/m2d/manual_toc_panel.h"
#include "ntmv/ui_renderer.h"

enum {
    GAME_MANUAL_RENDERER_0_OFFSET = 0x340,
    GAME_MANUAL_RENDERER_1_OFFSET = 0x344,
    GAME_MANUAL_TOC_PANEL_OFFSET = 0x358,
    GAME_MANUAL_SCROLL_BUFFER_OFFSET = 0x40370,
    GAME_MANUAL_TOC_SECONDARY_STATE_OFFSET = 0xad,
    GAME_MANUAL_WINDOW_ENABLE_MASK = 0xe000,
    GAME_MANUAL_WINDOW_0_ENABLE = 0x2000,
    GAME_MANUAL_MAIN_BG3_AFFINE_REGISTER = 0x04000030,
    GAME_MANUAL_SUB_BG3_AFFINE_REGISTER = 0x04001030,
    GAME_MANUAL_SUB_VIEWPORT_TOP = 0x12,
    GAME_MANUAL_SUB_VIEWPORT_BOTTOM = 0xae,
    GAME_MANUAL_MAIN_VIEWPORT_BOTTOM = 0x142
};

typedef int32_t GameManualAffineMatrix22[4];

extern void MTX_Identity22_(GameManualAffineMatrix22 matrix);
extern void G2x_SetBGyAffine_(
    uintptr_t affine_register,
    const GameManualAffineMatrix22 matrix,
    int32_t center_x,
    int32_t center_y,
    int32_t x,
    int32_t y);
extern void Game_UploadSubBg3Screen_020bd2c0(
    const uint16_t *source,
    uint32_t destination_offset,
    uint32_t byte_count);
extern void Game_UploadMainBg3Screen_020bd2cc(
    const uint16_t *source,
    uint32_t destination_offset,
    uint32_t byte_count);

#if UINTPTR_MAX > UINT32_MAX
extern void *GameManualViewer_ResolveHostPointer(
    const void *object,
    uint32_t offset);
#endif

static void *ReadViewerPointer(const void *viewer, uint32_t offset)
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

static bool UsesSubScreenOnly(const GameManualViewerPageFlow *viewer)
{
    const uint8_t *toc_panel = ReadViewerPointer(
        viewer, GAME_MANUAL_TOC_PANEL_OFFSET);
    return toc_panel[GAME_MANUAL_TOC_SECONDARY_STATE_OFFSET] != 0;
}

static void SetTransitionWindowEnabled(
    const GameManualViewerPageFlow *viewer,
    bool enabled)
{
    uint32_t sub_display_control = gGameSubDisplayControl_04001000;

    if (enabled) {
        if ((sub_display_control & GAME_MANUAL_WINDOW_ENABLE_MASK) == 0) {
            if (!UsesSubScreenOnly(viewer)) {
                gGameMainDisplayControl_04000000 =
                    (gGameMainDisplayControl_04000000 &
                        ~GAME_MANUAL_WINDOW_ENABLE_MASK) |
                    GAME_MANUAL_WINDOW_0_ENABLE;
            }
            gGameSubDisplayControl_04001000 =
                (sub_display_control & ~GAME_MANUAL_WINDOW_ENABLE_MASK) |
                GAME_MANUAL_WINDOW_0_ENABLE;
        }
    } else if ((sub_display_control & GAME_MANUAL_WINDOW_ENABLE_MASK) != 0) {
        if (!UsesSubScreenOnly(viewer)) {
            gGameMainDisplayControl_04000000 &=
                ~GAME_MANUAL_WINDOW_ENABLE_MASK;
        }
        gGameSubDisplayControl_04001000 =
            sub_display_control & ~GAME_MANUAL_WINDOW_ENABLE_MASK;
    }
}

/* 0x020bd128 */
void GameManualViewer_SetPageTransitionOffset_020bd128(
    GameManualViewerPageFlow *viewer,
    int32_t horizontal_offset)
{
    uint8_t *bytes = (uint8_t *)viewer;
    GameScrollableTileBuffer *scroll_buffer =
        (GameScrollableTileBuffer *)(
            bytes + GAME_MANUAL_SCROLL_BUFFER_OFFSET);
    GameManualAffineMatrix22 identity;
    GameCircularTileUploadRange sub_range = {
        0,
        GAME_MANUAL_SUB_VIEWPORT_BOTTOM
    };
    GameCircularTileUploadRange main_range = {
        GAME_MANUAL_SUB_VIEWPORT_BOTTOM,
        GAME_MANUAL_MAIN_VIEWPORT_BOTTOM
    };
    int32_t sub_y =
        scroll_buffer->visible_start - GAME_MANUAL_SUB_VIEWPORT_TOP;

    NtmvUiRenderer_FlushOam(ReadViewerPointer(
        viewer, GAME_MANUAL_RENDERER_0_OFFSET));
    NtmvUiRenderer_FlushOam(ReadViewerPointer(
        viewer, GAME_MANUAL_RENDERER_1_OFFSET));
    NtmvM2dManualTocPanel_PublishWindowRegisters_020baef4(
        ReadViewerPointer(viewer, GAME_MANUAL_TOC_PANEL_OFFSET));

    MTX_Identity22_(identity);
    G2x_SetBGyAffine_(
        GAME_MANUAL_SUB_BG3_AFFINE_REGISTER,
        identity,
        0,
        0,
        horizontal_offset,
        sub_y);
    G2x_SetBGyAffine_(
        GAME_MANUAL_MAIN_BG3_AFFINE_REGISTER,
        identity,
        0,
        0,
        horizontal_offset,
        sub_y + 0xc0);

    SetTransitionWindowEnabled(viewer, horizontal_offset != 0);
    if (horizontal_offset != 0) {
        GameManualViewer_SetRevealWindowOffset(
            (GameManualViewerDisplay *)viewer,
            horizontal_offset);
    }

    Game_UploadCircularTileRows(
        scroll_buffer,
        &sub_range,
        Game_UploadSubBg3Screen_020bd2c0);
    Game_UploadCircularTileRows(
        scroll_buffer,
        &main_range,
        Game_UploadMainBg3Screen_020bd2cc);
    scroll_buffer->previous_start = scroll_buffer->visible_start;
    scroll_buffer->previous_end = scroll_buffer->visible_end;
}
