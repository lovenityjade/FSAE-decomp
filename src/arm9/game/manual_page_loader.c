#include "game/manual_page_loader.h"

#include "game/graphics_state.h"
#include "game/manual_page_transition.h"
#include "game/scrollable_tile_buffer.h"
#include "ntmv/m2d/manual_toc_panel.h"
#include "ntmv/m2d/scroll_controls.h"

#include <stddef.h>

enum {
    GAME_MANUAL_TOC_PANEL_OFFSET = 0x358,
    GAME_MANUAL_SCROLL_BUTTON_OFFSET = 0x360,
    GAME_MANUAL_SCROLL_INDICATOR_0_OFFSET = 0x364,
    GAME_MANUAL_SCROLL_INDICATOR_1_OFFSET = 0x368,
    GAME_MANUAL_SCROLL_BUFFER_OFFSET = 0x40370,
    GAME_MANUAL_CURRENT_PAGE_OFFSET = 0x4039c,
    GAME_MANUAL_FADE_STEPS = 13,
    GAME_MANUAL_PAGE_VIEWPORT_EXTENT = 0xae,
    GAME_MANUAL_PAGE_RESOURCE_TYPE = 0x6e747067
};

extern void OS_WaitVBlankIntr(void);
extern void GameManualViewer_RenderFrame_020bd00c(
    GameManualViewerPageFlow *viewer);
extern void GameManualViewer_ResetInputHistory(
    GameManualViewerPageFlow *viewer);
extern void GameManualViewer_SetBlendBrightness(
    GameManualViewerPageFlow *viewer,
    int32_t brightness);

#if UINTPTR_MAX > UINT32_MAX
extern void *GameManualViewer_ResolveHostPointer(
    const void *object,
    uint32_t offset);
extern void GameManualViewer_StoreHostPointer(
    void *object,
    uint32_t offset,
    void *value);
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

static void WriteViewerPointer(void *viewer, uint32_t offset, void *value)
{
#if UINTPTR_MAX > UINT32_MAX
    GameManualViewer_StoreHostPointer(viewer, offset, value);
#else
    uint8_t *bytes = (uint8_t *)viewer + offset;
    uintptr_t encoded = (uintptr_t)value;
    uint32_t index;

    for (index = 0; index < sizeof(void *); ++index) {
        bytes[index] = (uint8_t)(encoded >> (index * 8));
    }
#endif
}

static int32_t FadeBrightness(int32_t step)
{
    return (step * 4) / 3;
}

static int16_t FadeOffset(
    int32_t brightness,
    bool fade_out,
    bool reverse_direction)
{
    int32_t offset = 0;

    if (brightness > 6) {
        offset = ((brightness - 6) * 64) / 10;
    }
    if (!fade_out) {
        offset = -offset;
    }
    if (reverse_direction) {
        offset = -offset;
    }
    return (int16_t)offset;
}

static void RunFadeStep(
    GameManualViewerPageFlow *viewer,
    int32_t step,
    bool fade_out,
    bool reverse_direction)
{
    int32_t brightness = FadeBrightness(step);

    GameManualViewer_RenderFrame_020bd00c(viewer);
    OS_WaitVBlankIntr();
    GameManualViewer_SetBlendBrightness(viewer, brightness);
    GameManualViewer_SetPageTransitionOffset_020bd128(
        viewer,
        FadeOffset(brightness, fade_out, reverse_direction));
}

/* 0x020bdd60 */
void GameManualViewer_LoadPage_020bdd60(
    GameManualViewerPageFlow *viewer,
    const char *page_name,
    NtmvM2dArcResourceAccessor *accessor,
    bool reverse_direction)
{
    uint8_t *bytes = (uint8_t *)viewer;
    GameScrollableTileBuffer *scroll_buffer =
        (GameScrollableTileBuffer *)(bytes + GAME_MANUAL_SCROLL_BUFFER_OFFSET);
    NtmvM2dScrollButton *scroll_button = ReadViewerPointer(
        viewer, GAME_MANUAL_SCROLL_BUTTON_OFFSET);
    NtmvM2dScrollIndicator *indicators[2];
    void *allocator_context = viewer;
    const void *resource;
    int32_t step;
    int32_t index;

    indicators[0] = ReadViewerPointer(
        viewer, GAME_MANUAL_SCROLL_INDICATOR_0_OFFSET);
    indicators[1] = ReadViewerPointer(
        viewer, GAME_MANUAL_SCROLL_INDICATOR_1_OFFSET);

    for (step = 0; step < GAME_MANUAL_FADE_STEPS; ++step) {
        RunFadeStep(viewer, step, true, reverse_direction);
    }

    GameManualViewer_ResetInputHistory(viewer);
    scroll_button->item_count = 0;
    NtmvM2dScrollButton_SetScrollOffset(scroll_button, 0);
    for (index = 0; index < 2; ++index) {
        indicators[index]->base.reserved_1a = 0;
        NtmvM2dScrollIndicator_SetScrollOffset(indicators[index], 0);
    }

    GameManualViewer_UnloadCurrentPage(viewer);
    resource = accessor->vtable->get_localized_file(
        accessor,
        GAME_MANUAL_PAGE_RESOURCE_TYPE,
        page_name,
        NULL);
    if (resource != NULL) {
        GameManualPage *page = GameManualPage_Allocate_020bcd60(
            &allocator_context);

        WriteViewerPointer(viewer, GAME_MANUAL_CURRENT_PAGE_OFFSET, page);
        (void)GameManualPage_Load_020b4f00(
            page, &allocator_context, resource, accessor);
        page->viewport_extent = GAME_MANUAL_PAGE_VIEWPORT_EXTENT;
        GameScrollableTileBuffer_SetContent(scroll_buffer, page);

        scroll_button->item_count = page->content_extent;
        NtmvM2dScrollButton_SetScrollOffset(
            scroll_button, scroll_buffer->visible_start);
        for (index = 0; index < 2; ++index) {
            indicators[index]->base.reserved_1a =
                (uint16_t)page->content_extent;
            NtmvM2dScrollIndicator_SetScrollOffset(
                indicators[index], scroll_buffer->visible_start);
        }
    }

    OS_WaitVBlankIntr();
    Game_SetBgPaletteColor0Both(
        ((GameManualPage *)ReadViewerPointer(
            viewer, GAME_MANUAL_CURRENT_PAGE_OFFSET))->palette_color);

    for (step = GAME_MANUAL_FADE_STEPS; step > 0;) {
        --step;
        RunFadeStep(viewer, step, false, reverse_direction);
    }

    if (((NtmvM2dManualTocPanel *)ReadViewerPointer(
            viewer, GAME_MANUAL_TOC_PANEL_OFFSET))->secondary_state_active == 0) {
        Game_SetDefaultMainBlendAlpha();
    }
}
