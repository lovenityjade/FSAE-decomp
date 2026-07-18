#include "game/manual_page_flow.h"

#include "game/scrollable_tile_buffer.h"
#include "ntmv/m2d/page_header_panel.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static uint8_t sViewer[0x40610];
static NtmvM2dPageHeaderPanel sHeader;
static uint16_t sTitle[] = {'G', 'u', 'i', 'd', 'e', 0};
static unsigned int sStep;
static NtmvM2dManualTocPanel sTocPanel;
static GameManualPageIndex sPageIndex;
static GameManualPageIndex *sEmbeddedPageIndex = &sPageIndex;
static uint32_t sCurrentPageStorage;
static void *sCurrentPage;
static GameManualPageRecord sRecords[2];
static int32_t sPrimary;
static int32_t sSecondaryValues[2];
static unsigned int sSecondaryCallCount;
static const char *sExpectedPageName = "000guide";
static bool sExpectedReset = true;
static const uint16_t *sExpectedTitle = sTitle;
static uint16_t sExpectedPageIndex = UINT16_MAX;
static unsigned int sExpectedHeaderStep = 2;
static bool sAllocationShouldFail;
static bool sCaptureEvents;
static char sEvents[32];
static unsigned int sEventCount;
static unsigned int sExitFeedbackCount;

static void AppendEvent(char event)
{
    if (sCaptureEvents) {
        assert(sEventCount + 1 < sizeof(sEvents));
        sEvents[sEventCount++] = event;
        sEvents[sEventCount] = '\0';
    }
}

void *GameManualViewer_ResolveHostPointer(
    const void *object,
    uint32_t offset)
{
    assert(object == sViewer);
    if (offset == 0x348) {
        return sEmbeddedPageIndex;
    }
    if (offset == 0x354) {
        return &sHeader;
    }
    if (offset == 0x358) {
        return &sTocPanel;
    }
    if (offset == 0x4039c) {
        return sCurrentPage;
    }
    assert(0);
    return 0;
}

void GameManualViewer_StoreHostPointer(
    void *object,
    uint32_t offset,
    void *value)
{
    assert(object == sViewer);
    if (offset == 0x348) {
        sEmbeddedPageIndex = value;
        return;
    }
    if (offset == 0x4039c) {
        sCurrentPage = value;
        return;
    }
    assert(0);
}

void GameManualPage_DestroyAndFree_020bc7b8(
    void *const *allocator_context,
    void *page)
{
    AppendEvent('D');
    assert(*allocator_context == sViewer);
    assert(page == &sCurrentPageStorage);
}

void GameScrollableTileBuffer_SetContent(
    GameScrollableTileBuffer *buffer,
    void *content)
{
    AppendEvent('B');
    assert(buffer == (GameScrollableTileBuffer *)(sViewer + 0x40370));
    assert(content == 0);
}

void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size)
{
    AppendEvent('A');
    assert(allocator == sViewer);
    assert(size == 8);
    return sAllocationShouldFail ? 0 : &sPageIndex;
}

GameManualPageIndex *GameManualPageIndex_Construct(
    GameManualPageIndex *page_index)
{
    AppendEvent('C');
    assert(page_index == &sPageIndex);
    page_index->records = 0;
    page_index->record_count = 0;
    page_index->reserved_06 = 0;
    return page_index;
}

bool GameManualPageIndex_Load_020b8c78(
    GameManualPageIndex *page_index,
    const char *resource_name,
    NtmvM2dArcResourceAccessor *accessor,
    void *allocator_context)
{
    AppendEvent('L');
    assert(page_index == &sPageIndex);
    assert(strcmp(resource_name, "manual") == 0);
    assert(accessor == (NtmvM2dArcResourceAccessor *)(sViewer + 0x40500));
    assert(*(void **)allocator_context == sViewer);
    page_index->records = sRecords;
    page_index->record_count = 2;
    return true;
}

void NtmvM2dManualTocPanel_SetTocData(
    NtmvM2dManualTocPanel *panel,
    const NtmvM2dItemsTocData *toc_data)
{
    AppendEvent('T');
    assert(panel == &sTocPanel);
    assert(toc_data == (const NtmvM2dItemsTocData *)&sPageIndex);
}

void NtmvM2dManualTocPanel_TriggerExitFeedback_020bb70c(
    NtmvM2dManualTocPanel *panel)
{
    assert(panel == &sTocPanel);
    assert(sViewer[0x40621] == 0);
    ++sExitFeedbackCount;
}

static void StorePointer(uint32_t offset, const void *pointer)
{
    uintptr_t value = (uintptr_t)pointer;
    uint32_t index;
    for (index = 0; index < sizeof(void *); ++index) {
        sViewer[offset + index] = (uint8_t)(value >> (index * 8));
    }
}

void GameManualViewer_LoadPage_020bdd60(
    GameManualViewerPageFlow *viewer,
    const char *page_name,
    NtmvM2dArcResourceAccessor *accessor,
    bool reset_scroll)
{
    AppendEvent('V');
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    assert(strcmp(page_name, sExpectedPageName) == 0);
    assert(
        accessor == (NtmvM2dArcResourceAccessor *)(sViewer + 0x403f8) ||
        accessor == (NtmvM2dArcResourceAccessor *)(sViewer + 0x40500));
    assert(reset_scroll == sExpectedReset);
    assert(sStep++ == 0);
}

void *GameScrollableTileBuffer_GetContentResource(
    const GameScrollableTileBuffer *buffer)
{
    AppendEvent('R');
    assert(buffer == (const GameScrollableTileBuffer *)(sViewer + 0x40370));
    assert(sStep++ == 1);
    return sTitle;
}

void NtmvM2dPageHeaderPanel_SetText(
    NtmvM2dPageHeaderPanel *panel,
    const uint16_t *text,
    uint16_t page_index)
{
    AppendEvent('H');
    assert(panel == &sHeader);
    assert(text == sExpectedTitle);
    assert(page_index == sExpectedPageIndex);
    assert(sStep++ == sExpectedHeaderStep);
}

int32_t NtmvM2dManualTocPanel_GetPrimarySelection_020bac20(
    const NtmvM2dManualTocPanel *panel)
{
    AppendEvent('P');
    assert(panel == &sTocPanel);
    return sPrimary;
}

int32_t NtmvM2dManualTocPanel_GetSecondarySelection_020bac58(
    const NtmvM2dManualTocPanel *panel)
{
    AppendEvent('S');
    assert(panel == &sTocPanel);
    assert(sSecondaryCallCount < 2);
    return sSecondaryValues[sSecondaryCallCount++];
}

int main(void)
{
    memset(sViewer, 0, sizeof(sViewer));
    StorePointer(0x354, &sHeader);
    *(uint32_t *)(sViewer + 0x40608) = 0xffffffffu;
    sStep = 0;
    GameManualViewer_ShowGuideFallback(
        (GameManualViewerPageFlow *)sViewer);
    assert(sStep == 3);
    assert(*(uint32_t *)(sViewer + 0x40608) == 0);

    memset(sRecords, 0, sizeof(sRecords));
    sPageIndex.records = sRecords;
    sRecords[1].page_name = "100root";
    sRecords[1].title = sTitle;
    {
        static const char *const subpages[] = {
            "100sub0", "100sub1", "100sub2"
        };
        sRecords[1].subpage_names = subpages;
    }
    StorePointer(0x348, &sPageIndex);
    StorePointer(0x358, &sTocPanel);

    /* A root selection calls the secondary getter only once. */
    sPrimary = 1;
    sSecondaryValues[0] = -1;
    sSecondaryCallCount = 0;
    sExpectedPageName = "100root";
    sExpectedReset = true;
    sExpectedTitle = sTitle;
    sExpectedPageIndex = 1;
    sExpectedHeaderStep = 1;
    *(uint32_t *)(sViewer + 0x40608) = 0x00030002u;
    sStep = 0;
    GameManualViewer_ShowSelectedPage(
        (GameManualViewerPageFlow *)sViewer);
    assert(sSecondaryCallCount == 1);
    assert(sStep == 2);
    assert(*(uint32_t *)(sViewer + 0x40608) == 0x00020000u);

    /* Page lookup re-reads the child, while packed state keeps the first. */
    sSecondaryValues[0] = 0;
    sSecondaryValues[1] = 2;
    sSecondaryCallCount = 0;
    sExpectedPageName = "100sub2";
    sExpectedReset = false;
    *(uint32_t *)(sViewer + 0x40608) = 0x00010000u;
    sStep = 0;
    GameManualViewer_ShowSelectedPage(
        (GameManualViewerPageFlow *)sViewer);
    assert(sSecondaryCallCount == 2);
    assert(sStep == 2);
    assert(*(uint32_t *)(sViewer + 0x40608) == 0x00020001u);

    /* Allocation preserves the target's fixed eight-byte request and null path. */
    {
        void *allocator_context = sViewer;
        sAllocationShouldFail = true;
        assert(GameManualPageIndex_Allocate(&allocator_context) == 0);
        sAllocationShouldFail = false;
    }

    /* Initialization loads "manual", installs the TOC and shows a selection. */
    sPrimary = 1;
    sSecondaryValues[0] = -1;
    sSecondaryCallCount = 0;
    sExpectedPageName = "100root";
    sExpectedReset = true;
    sExpectedTitle = sTitle;
    sExpectedPageIndex = 1;
    sExpectedHeaderStep = 1;
    *(uint32_t *)(sViewer + 0x40608) = 0x00030002u;
    sStep = 0;
    sEventCount = 0;
    sEvents[0] = '\0';
    sCaptureEvents = true;
    GameManualViewer_InitializePageIndex(
        (GameManualViewerPageFlow *)sViewer);
    sCaptureEvents = false;
    assert(strcmp(sEvents, "ACLTPPSVH") == 0);
    assert(sEmbeddedPageIndex == &sPageIndex);
    assert(sPageIndex.records == sRecords);
    assert(sPageIndex.record_count == 2);
    assert(sStep == 2);

    /* No primary selection takes the exact guide fallback branch. */
    sPrimary = -1;
    sExpectedPageName = "000guide";
    sExpectedReset = true;
    sExpectedTitle = sTitle;
    sExpectedPageIndex = UINT16_MAX;
    sExpectedHeaderStep = 2;
    sStep = 0;
    sEventCount = 0;
    sEvents[0] = '\0';
    sCaptureEvents = true;
    GameManualViewer_InitializePageIndex(
        (GameManualViewerPageFlow *)sViewer);
    sCaptureEvents = false;
    assert(strcmp(sEvents, "ACLTPVRH") == 0);
    assert(sStep == 3);
    assert(*(uint32_t *)(sViewer + 0x40608) == 0);

    /* Current-page cleanup is null-safe and clears the scroll source last. */
    sCurrentPage = 0;
    sEventCount = 0;
    sEvents[0] = '\0';
    sCaptureEvents = true;
    GameManualViewer_UnloadCurrentPage(
        (GameManualViewerPageFlow *)sViewer);
    sCaptureEvents = false;
    assert(strcmp(sEvents, "") == 0);

    sCurrentPage = &sCurrentPageStorage;
    sEventCount = 0;
    sEvents[0] = '\0';
    sCaptureEvents = true;
    GameManualViewer_UnloadCurrentPage(
        (GameManualViewerPageFlow *)sViewer);
    sCaptureEvents = false;
    assert(strcmp(sEvents, "DB") == 0);
    assert(sCurrentPage == 0);

    /* Page requests keep guide, unavailable and selected paths distinct. */
    sExpectedPageName = "000guide";
    sExpectedReset = true;
    sExpectedTitle = sTitle;
    sExpectedPageIndex = UINT16_MAX;
    sExpectedHeaderStep = 2;
    sStep = 0;
    GameManualViewer_ShowRequestedPage(
        (GameManualViewerPageFlow *)sViewer, true);
    assert(sStep == 3);

    sPrimary = -1;
    sStep = 0;
    GameManualViewer_ShowRequestedPage(
        (GameManualViewerPageFlow *)sViewer, false);
    assert(sStep == 0);

    sPrimary = 1;
    sSecondaryValues[0] = -1;
    sSecondaryCallCount = 0;
    sExpectedPageName = "100root";
    sExpectedReset = true;
    sExpectedTitle = sTitle;
    sExpectedPageIndex = 1;
    sExpectedHeaderStep = 1;
    *(uint32_t *)(sViewer + 0x40608) = 0x00030002u;
    sStep = 0;
    GameManualViewer_ShowRequestedPage(
        (GameManualViewerPageFlow *)sViewer, false);
    assert(sStep == 2);

    /* Exit feedback runs before the main-loop termination byte is raised. */
    sViewer[0x40621] = 0;
    sExitFeedbackCount = 0;
    GameManualViewer_RequestExit((GameManualViewerPageFlow *)sViewer);
    assert(sExitFeedbackCount == 1);
    assert(sViewer[0x40621] == 1);

    {
        static const uint32_t context_words[3] = {
            0x11223344u, 0x55667788u, 0x99aabbccu
        };
        const uint32_t *stored = (const uint32_t *)(sViewer + 0xf0);
        GameScrollableTileBuffer *buffer =
            (GameScrollableTileBuffer *)(sViewer + 0x40370);

        GameManualViewer_SetTilemapContext(
            (GameManualViewerPageFlow *)sViewer, context_words);
        assert(stored[0] == context_words[0]);
        assert(stored[1] == context_words[1]);
        assert(stored[2] == context_words[2]);
        assert(buffer->tilemap == stored);
    }
    return 0;
}
