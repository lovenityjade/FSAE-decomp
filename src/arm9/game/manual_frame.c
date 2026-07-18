#include "game/manual_frame.h"

#include "game/sound_manager.h"
#include "game/scrollable_tile_buffer.h"
#include "ntmv/m2d/manual_toc_panel.h"
#include "ntmv/m2d/scroll_controls.h"
#include "ntmv/m2d/ui_panel.h"

enum {
    GAME_MANUAL_SOUND_MANAGER_OFFSET = 0x0fc,
    GAME_MANUAL_RENDER_BUFFER_OFFSET = 0x370,
    GAME_MANUAL_RENDER_BUFFER_SIZE = 0x40000,
    GAME_MANUAL_RENDER_CONTEXT_0_OFFSET = 0x340,
    GAME_MANUAL_RENDER_CONTEXT_1_OFFSET = 0x344,
    GAME_MANUAL_ROOT_PANEL_0_OFFSET = 0x34c,
    GAME_MANUAL_ROOT_PANEL_1_OFFSET = 0x350,
    GAME_MANUAL_TOC_PANEL_OFFSET = 0x358,
    GAME_MANUAL_SCROLL_BUTTON_OFFSET = 0x360,
    GAME_MANUAL_SCROLL_INDICATOR_0_OFFSET = 0x364,
    GAME_MANUAL_SCROLL_INDICATOR_1_OFFSET = 0x368,
    GAME_MANUAL_SCROLL_BUFFER_OFFSET = 0x40370
};

extern void DC_FlushRange(const void *address, uint32_t size);

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

static void RenderRootPanel(
    NtmvM2dUIPanel *panel,
    void *render_context)
{
    static const NtmvM2dPoint zero = {0, 0};
    const NtmvM2dUIPanelVTable *vtable =
        (const NtmvM2dUIPanelVTable *)panel->base.vtable;

    vtable->render(panel, &render_context, &zero);
}

/* 0x020bd00c */
void GameManualViewer_RenderFrame_020bd00c(
    GameManualViewerPageFlow *viewer)
{
    uint8_t *bytes = (uint8_t *)viewer;
    GameScrollableTileBuffer *scroll_buffer =
        (GameScrollableTileBuffer *)(bytes + GAME_MANUAL_SCROLL_BUFFER_OFFSET);
    NtmvM2dScrollButton *scroll_button = ReadViewerPointer(
        viewer, GAME_MANUAL_SCROLL_BUTTON_OFFSET);
    NtmvM2dScrollIndicator *indicator_0 = ReadViewerPointer(
        viewer, GAME_MANUAL_SCROLL_INDICATOR_0_OFFSET);
    NtmvM2dScrollIndicator *indicator_1 = ReadViewerPointer(
        viewer, GAME_MANUAL_SCROLL_INDICATOR_1_OFFSET);
    void *render_context_0 = ReadViewerPointer(
        viewer, GAME_MANUAL_RENDER_CONTEXT_0_OFFSET);
    void *render_context_1 = ReadViewerPointer(
        viewer, GAME_MANUAL_RENDER_CONTEXT_1_OFFSET);
    uint8_t upload_required = 0;

    GameSoundManager_Tick(
        (GameSoundManager *)(bytes + GAME_MANUAL_SOUND_MANAGER_OFFSET));
    if (GameScrollableTileBuffer_Tick(
            scroll_buffer, &upload_required)) {
        NtmvM2dScrollButton_SetScrollOffset(
            scroll_button, scroll_buffer->visible_start);
        NtmvM2dScrollIndicator_SetScrollOffset(
            indicator_0, scroll_buffer->visible_start);
        NtmvM2dScrollIndicator_SetScrollOffset(
            indicator_1, scroll_buffer->visible_start);
    }
    if (upload_required != 0) {
        (void)GameSoundManager_PlaySequenceArc(
            (GameSoundManager *)(bytes + GAME_MANUAL_SOUND_MANAGER_OFFSET),
            11,
            0x80);
    }

    GameScrollableTileBuffer_UpdateContent(scroll_buffer);
    DC_FlushRange(
        bytes + GAME_MANUAL_RENDER_BUFFER_OFFSET,
        GAME_MANUAL_RENDER_BUFFER_SIZE);
    NtmvM2dScrollButton_TickChildren(scroll_button);
    NtmvM2dScrollIndicator_UpdateVisibility(
        indicator_0, render_context_1);
    NtmvM2dScrollIndicator_UpdateVisibility(
        indicator_1, render_context_0);
    NtmvM2dManualTocPanel_Update(
        (NtmvM2dManualTocPanel *)ReadViewerPointer(
            viewer, GAME_MANUAL_TOC_PANEL_OFFSET));
    RenderRootPanel(
        (NtmvM2dUIPanel *)ReadViewerPointer(
            viewer, GAME_MANUAL_ROOT_PANEL_0_OFFSET),
        render_context_1);
    RenderRootPanel(
        (NtmvM2dUIPanel *)ReadViewerPointer(
            viewer, GAME_MANUAL_ROOT_PANEL_1_OFFSET),
        render_context_0);
}
