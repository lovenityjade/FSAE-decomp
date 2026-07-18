#include "game/manual_page_loader.h"

#include "game/graphics_state.h"
#include "game/scrollable_tile_buffer.h"
#include "ntmv/m2d/manual_toc_panel.h"
#include "ntmv/m2d/scroll_controls.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static uint8_t sViewer[0x40610];
static GameManualPage sPage;
static uint32_t sResource;
static GameManualPage *sCurrentPage;
static NtmvM2dManualTocPanel sTocPanel;
static NtmvM2dScrollButton sScrollButton;
static NtmvM2dScrollIndicator sIndicators[2];
static unsigned int sRenderCount;
static unsigned int sWaitCount;
static unsigned int sBlendCount;
static unsigned int sOffsetCount;
static int32_t sBrightness[26];
static int16_t sOffsets[26];
static unsigned int sResetCount;
static unsigned int sUnloadCount;
static unsigned int sResourceCount;
static unsigned int sAllocateCount;
static unsigned int sLoadCount;
static unsigned int sSetContentCount;
static unsigned int sScrollSetCount;
static unsigned int sIndicatorSetCount;
static unsigned int sPaletteCount;
static unsigned int sDefaultBlendCount;

static GameScrollableTileBuffer *ScrollBuffer(void)
{
    return (GameScrollableTileBuffer *)(sViewer + 0x40370);
}

void *GameManualViewer_ResolveHostPointer(
    const void *object,
    uint32_t offset)
{
    assert(object == sViewer);
    switch (offset) {
    case 0x358:
        return &sTocPanel;
    case 0x360:
        return &sScrollButton;
    case 0x364:
        return &sIndicators[0];
    case 0x368:
        return &sIndicators[1];
    case 0x4039c:
        return sCurrentPage;
    default:
        assert(0);
        return 0;
    }
}

void GameManualViewer_StoreHostPointer(
    void *object,
    uint32_t offset,
    void *value)
{
    assert(object == sViewer);
    assert(offset == 0x4039c);
    sCurrentPage = value;
}

void GameManualViewer_RenderFrame_020bd00c(
    GameManualViewerPageFlow *viewer)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    ++sRenderCount;
}

void OS_WaitVBlankIntr(void)
{
    ++sWaitCount;
}

void GameManualViewer_SetBlendBrightness(
    GameManualViewerPageFlow *viewer,
    int32_t brightness)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    assert(sBlendCount < 26);
    sBrightness[sBlendCount++] = brightness;
}

void GameManualViewer_SetPageTransitionOffset_020bd128(
    GameManualViewerPageFlow *viewer,
    int16_t offset)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    assert(sOffsetCount < 26);
    sOffsets[sOffsetCount++] = offset;
}

void GameManualViewer_ResetInputHistory(GameManualViewerPageFlow *viewer)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    assert(sRenderCount == 13);
    assert(sWaitCount == 13);
    ++sResetCount;
}

void GameManualViewer_UnloadCurrentPage(GameManualViewerPageFlow *viewer)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    assert(sResetCount == 1);
    sCurrentPage = 0;
    ++sUnloadCount;
}

static void *GetLocalizedFile(
    NtmvM2dArcResourceAccessor *accessor,
    uint32_t resource_type,
    const char *name,
    void *unused_result)
{
    (void)unused_result;
    assert(accessor == (NtmvM2dArcResourceAccessor *)(sViewer + 0x40500));
    assert(resource_type == 0x6e747067);
    assert(strcmp(name, "100root") == 0);
    assert(sUnloadCount == 1);
    ++sResourceCount;
    return &sResource;
}

static const NtmvM2dArcResourceAccessorVTable sAccessorVTable = {
    .get_localized_file = GetLocalizedFile,
};

GameManualPage *GameManualPage_Allocate_020bcd60(
    void *const *allocator_context)
{
    assert(*allocator_context == sViewer);
    assert(sResourceCount == 1);
    ++sAllocateCount;
    return &sPage;
}

bool GameManualPage_Load_020b4f00(
    GameManualPage *page,
    void *allocator_context,
    const void *resource,
    NtmvM2dArcResourceAccessor *accessor)
{
    assert(page == &sPage);
    assert(*(void **)allocator_context == sViewer);
    assert(resource == &sResource);
    assert(accessor == (NtmvM2dArcResourceAccessor *)(sViewer + 0x40500));
    assert(sAllocateCount == 1);
    ++sLoadCount;
    return true;
}

void GameScrollableTileBuffer_SetContent(
    GameScrollableTileBuffer *buffer,
    void *content)
{
    assert(buffer == ScrollBuffer());
    assert(content == &sPage);
    assert(sLoadCount == 1);
    buffer->content = content;
    buffer->visible_start = 37;
    ++sSetContentCount;
}

void NtmvM2dScrollButton_SetScrollOffset(
    NtmvM2dScrollButton *button,
    int16_t scroll_offset)
{
    assert(button == &sScrollButton);
    if (sScrollSetCount == 0) {
        assert(button->item_count == 0);
        assert(scroll_offset == 0);
    } else {
        assert(button->item_count == sPage.content_extent);
        assert(scroll_offset == 37);
        assert(sSetContentCount == 1);
    }
    ++sScrollSetCount;
}

void NtmvM2dScrollIndicator_SetScrollOffset(
    NtmvM2dScrollIndicator *indicator,
    int16_t scroll_offset)
{
    unsigned int index = sIndicatorSetCount % 2;
    assert(indicator == &sIndicators[index]);
    if (sIndicatorSetCount < 2) {
        assert(indicator->base.reserved_1a == 0);
        assert(scroll_offset == 0);
    } else {
        assert(indicator->base.reserved_1a == (uint16_t)sPage.content_extent);
        assert(scroll_offset == 37);
    }
    ++sIndicatorSetCount;
}

void Game_SetBgPaletteColor0Both(uint16_t color)
{
    assert(sWaitCount == 14);
    assert(color == sPage.palette_color);
    ++sPaletteCount;
}

void Game_SetDefaultMainBlendAlpha(void)
{
    assert(sRenderCount == 26);
    assert(sWaitCount == 27);
    ++sDefaultBlendCount;
}

static void ResetCounters(void)
{
    sRenderCount = 0;
    sWaitCount = 0;
    sBlendCount = 0;
    sOffsetCount = 0;
    sResetCount = 0;
    sUnloadCount = 0;
    sResourceCount = 0;
    sAllocateCount = 0;
    sLoadCount = 0;
    sSetContentCount = 0;
    sScrollSetCount = 0;
    sIndicatorSetCount = 0;
    sPaletteCount = 0;
    sDefaultBlendCount = 0;
}

static void CheckFade(bool reversed)
{
    static const int32_t expected_brightness[13] = {
        0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 16
    };
    static const int16_t expected_magnitude[13] = {
        0, 0, 0, 0, 0, 0, 12, 19, 25, 38, 44, 51, 64
    };
    unsigned int index;

    assert(sBlendCount == 26);
    assert(sOffsetCount == 26);
    for (index = 0; index < 13; ++index) {
        int16_t sign = reversed ? -1 : 1;
        assert(sBrightness[index] == expected_brightness[index]);
        assert(sOffsets[index] == sign * expected_magnitude[index]);
        assert(sBrightness[13 + index] == expected_brightness[12 - index]);
        assert(sOffsets[13 + index] ==
            sign * expected_magnitude[12 - index] * -1);
    }
}

static void RunCase(bool reversed, uint8_t toc_state)
{
    NtmvM2dArcResourceAccessor *accessor =
        (NtmvM2dArcResourceAccessor *)(sViewer + 0x40500);

    memset(sViewer, 0, sizeof(sViewer));
    memset(&sPage, 0, sizeof(sPage));
    memset(&sTocPanel, 0, sizeof(sTocPanel));
    memset(&sScrollButton, 0, sizeof(sScrollButton));
    memset(sIndicators, 0, sizeof(sIndicators));
    ResetCounters();
    accessor->vtable = &sAccessorVTable;
    sPage.content_extent = 231;
    sPage.palette_color = 0x1234;
    sTocPanel.secondary_state_active = toc_state;

    GameManualViewer_LoadPage_020bdd60(
        (GameManualViewerPageFlow *)sViewer,
        "100root",
        accessor,
        reversed);

    assert(sCurrentPage == &sPage);
    assert(sPage.viewport_extent == 0xae);
    assert(sRenderCount == 26);
    assert(sWaitCount == 27);
    assert(sResetCount == 1);
    assert(sUnloadCount == 1);
    assert(sResourceCount == 1);
    assert(sAllocateCount == 1);
    assert(sLoadCount == 1);
    assert(sSetContentCount == 1);
    assert(sScrollSetCount == 2);
    assert(sIndicatorSetCount == 4);
    assert(sPaletteCount == 1);
    assert(sDefaultBlendCount == (toc_state == 0 ? 1u : 0u));
    CheckFade(reversed);
}

int main(void)
{
    RunCase(false, 0);
    RunCase(true, 1);
    return 0;
}
