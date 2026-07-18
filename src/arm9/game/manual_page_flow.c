#include "game/manual_page_flow.h"

#include "game/manual_viewer_lifecycle.h"
#include "game/scrollable_tile_buffer.h"
#include "ntmv/m2d/page_header_panel.h"

enum {
    GAME_MANUAL_TILEMAP_CONTEXT_OFFSET = 0x0f0,
    GAME_MANUAL_PAGE_INDEX_OFFSET = 0x348,
    GAME_MANUAL_HEADER_PANEL_OFFSET = 0x354,
    GAME_MANUAL_TOC_PANEL_OFFSET = 0x358,
    GAME_MANUAL_SCROLL_BUFFER_OFFSET = 0x40370,
    GAME_MANUAL_CURRENT_PAGE_OFFSET = 0x4039c,
    GAME_MANUAL_SHARED_RESOURCES_OFFSET = 0x403f8,
    GAME_MANUAL_PAGE_STATE_OFFSET = 0x40608,
    GAME_MANUAL_EXIT_REQUESTED_OFFSET = 0x40621
};

static const char sManualResourceName[] = "manual"; /* 0x02127178 */
static const char sFallbackGuidePage[] = "000guide"; /* 0x02127188 */

extern void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size);
extern void NtmvM2dManualTocPanel_TriggerExitFeedback_020bb70c(
    NtmvM2dManualTocPanel *panel);

#if UINTPTR_MAX > UINT32_MAX
extern void *GameManualViewer_ResolveHostPointer(
    const void *object,
    uint32_t offset);
extern void GameManualViewer_StoreHostPointer(
    void *object,
    uint32_t offset,
    void *value);
#endif

static void *ReadEmbeddedPointer(const void *object, uint32_t offset)
{
#if UINTPTR_MAX > UINT32_MAX
    return GameManualViewer_ResolveHostPointer(object, offset);
#else
    const uint8_t *bytes = (const uint8_t *)object + offset;
    uintptr_t value = 0;
    uint32_t index;

    for (index = 0; index < sizeof(void *); ++index) {
        value |= (uintptr_t)bytes[index] << (index * 8);
    }
    return (void *)value;
#endif
}

static void WriteEmbeddedPointer(void *object, uint32_t offset, void *value)
{
#if UINTPTR_MAX > UINT32_MAX
    GameManualViewer_StoreHostPointer(object, offset, value);
#else
    uint8_t *bytes = (uint8_t *)object + offset;
    uintptr_t encoded = (uintptr_t)value;
    uint32_t index;

    for (index = 0; index < sizeof(void *); ++index) {
        bytes[index] = (uint8_t)(encoded >> (index * 8));
    }
#endif
}

/* 0x020bda3c */
void GameManualViewer_ShowRequestedPage(
    GameManualViewerPageFlow *viewer,
    bool show_guide)
{
    NtmvM2dManualTocPanel *toc_panel;

    if (show_guide) {
        GameManualViewer_ShowGuideFallback(viewer);
        return;
    }

    toc_panel = ReadEmbeddedPointer(viewer, GAME_MANUAL_TOC_PANEL_OFFSET);
    if (NtmvM2dManualTocPanel_GetPrimarySelection_020bac20(toc_panel) != -1) {
        GameManualViewer_ShowSelectedPage(viewer);
    }
}

/* 0x020bda70 */
void GameManualViewer_RequestExit(GameManualViewerPageFlow *viewer)
{
    uint8_t *bytes = (uint8_t *)viewer;
    NtmvM2dManualTocPanel *toc_panel = ReadEmbeddedPointer(
        viewer, GAME_MANUAL_TOC_PANEL_OFFSET);

    NtmvM2dManualTocPanel_TriggerExitFeedback_020bb70c(toc_panel);
    bytes[GAME_MANUAL_EXIT_REQUESTED_OFFSET] = 1;
}

/* 0x020bda90 */
void GameManualViewer_SetTilemapContext(
    GameManualViewerPageFlow *viewer,
    const uint32_t context_words[3])
{
    uint8_t *bytes = (uint8_t *)viewer;
    uint32_t *stored_context = (uint32_t *)(
        bytes + GAME_MANUAL_TILEMAP_CONTEXT_OFFSET);
    GameScrollableTileBuffer *scroll_buffer =
        (GameScrollableTileBuffer *)(bytes + GAME_MANUAL_SCROLL_BUFFER_OFFSET);
    uint32_t index;

    for (index = 0; index < 3; ++index) {
        stored_context[index] = context_words[index];
    }
    scroll_buffer->tilemap = stored_context;
}

/* 0x020bdfc0 */
void GameManualViewer_UnloadCurrentPage(GameManualViewerPageFlow *viewer)
{
    uint8_t *bytes = (uint8_t *)viewer;
    void *current_page = ReadEmbeddedPointer(
        viewer, GAME_MANUAL_CURRENT_PAGE_OFFSET);

    if (current_page != 0) {
        void *allocator_context = viewer;
        GameManualPage_DestroyAndFree_020bc7b8(
            &allocator_context, current_page);
        WriteEmbeddedPointer(
            viewer, GAME_MANUAL_CURRENT_PAGE_OFFSET, 0);
        GameScrollableTileBuffer_SetContent(
            (GameScrollableTileBuffer *)(
                bytes + GAME_MANUAL_SCROLL_BUFFER_OFFSET),
            0);
    }
}

/* 0x020be128 */
GameManualPageIndex *GameManualPageIndex_Allocate(
    void *const *allocator_context)
{
    GameManualPageIndex *page_index = NNS_FndAllocFromAllocator(
        *allocator_context, 8);

    if (page_index != 0) {
        return GameManualPageIndex_Construct(page_index);
    }
    return 0;
}

/* 0x020be0c4 */
void GameManualViewer_InitializePageIndex(GameManualViewerPageFlow *viewer)
{
    uint8_t *bytes = (uint8_t *)viewer;
    void *allocator_context = viewer;
    GameManualPageIndex *page_index =
        GameManualPageIndex_Allocate(&allocator_context);
    NtmvM2dManualTocPanel *toc_panel;

    WriteEmbeddedPointer(
        viewer, GAME_MANUAL_PAGE_INDEX_OFFSET, page_index);
    GameManualPageIndex_Load_020b8c78(
        page_index,
        sManualResourceName,
        (NtmvM2dArcResourceAccessor *)(bytes + 0x40500),
        &allocator_context);

    toc_panel = ReadEmbeddedPointer(viewer, GAME_MANUAL_TOC_PANEL_OFFSET);
    NtmvM2dManualTocPanel_SetTocData(
        toc_panel, (const NtmvM2dItemsTocData *)ReadEmbeddedPointer(
            viewer, GAME_MANUAL_PAGE_INDEX_OFFSET));
    if (NtmvM2dManualTocPanel_GetPrimarySelection_020bac20(toc_panel) != -1) {
        GameManualViewer_ShowSelectedPage(viewer);
    } else {
        GameManualViewer_ShowGuideFallback(viewer);
    }
}

/* 0x020be154 */
void GameManualViewer_ShowSelectedPage(GameManualViewerPageFlow *viewer)
{
    uint8_t *bytes = (uint8_t *)viewer;
    const GameManualPageIndex *page_index = ReadEmbeddedPointer(
        viewer, GAME_MANUAL_PAGE_INDEX_OFFSET);
    const NtmvM2dManualTocPanel *toc_panel = ReadEmbeddedPointer(
        viewer, GAME_MANUAL_TOC_PANEL_OFFSET);
    NtmvM2dPageHeaderPanel *header = ReadEmbeddedPointer(
        viewer, GAME_MANUAL_HEADER_PANEL_OFFSET);
    int32_t primary =
        NtmvM2dManualTocPanel_GetPrimarySelection_020bac20(toc_panel);
    int32_t secondary =
        NtmvM2dManualTocPanel_GetSecondarySelection_020bac58(toc_panel);
    const GameManualPageRecord *record =
        &page_index->records[(int16_t)primary];
    const char *page_name;
    uint32_t packed_state;
    uint32_t previous_state =
        *(uint32_t *)(bytes + GAME_MANUAL_PAGE_STATE_OFFSET);

    if (secondary == -1) {
        page_name = record->page_name;
    } else {
        int32_t refreshed_secondary =
            NtmvM2dManualTocPanel_GetSecondarySelection_020bac58(
                toc_panel);
        page_name = record->subpage_names[refreshed_secondary];
    }

    packed_state = (uint32_t)(secondary + 1) |
        ((uint32_t)(primary + 1) << 16);
    GameManualViewer_LoadPage_020bdd60(
        viewer,
        page_name,
        (NtmvM2dArcResourceAccessor *)(bytes + 0x40500),
        (int32_t)previous_state > (int32_t)packed_state);
    NtmvM2dPageHeaderPanel_SetText(
        header, record->title, (uint16_t)primary);
    *(uint32_t *)(bytes + GAME_MANUAL_PAGE_STATE_OFFSET) = packed_state;
}

/* 0x020be1f4 */
void GameManualViewer_ShowGuideFallback(GameManualViewerPageFlow *viewer)
{
    uint8_t *bytes = (uint8_t *)viewer;
    GameScrollableTileBuffer *scroll_buffer =
        (GameScrollableTileBuffer *)(bytes + GAME_MANUAL_SCROLL_BUFFER_OFFSET);
    NtmvM2dPageHeaderPanel *header = ReadEmbeddedPointer(
        viewer, GAME_MANUAL_HEADER_PANEL_OFFSET);
    const uint16_t *title;

    GameManualViewer_LoadPage_020bdd60(
        viewer,
        sFallbackGuidePage,
        (NtmvM2dArcResourceAccessor *)(
            bytes + GAME_MANUAL_SHARED_RESOURCES_OFFSET),
        true);
    title = GameScrollableTileBuffer_GetContentResource(scroll_buffer);
    NtmvM2dPageHeaderPanel_SetText(header, title, UINT16_MAX);
    *(uint32_t *)(bytes + GAME_MANUAL_PAGE_STATE_OFFSET) = 0;
}
