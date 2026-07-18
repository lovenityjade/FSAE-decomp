#include "game/manual_viewer_lifecycle.h"

#include "game/scrollable_tile_buffer.h"
#include "game/sound_manager.h"
#include "ntmv/m2d/resource_accessor.h"
#include "ntmv/ui_renderer.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

typedef void *(*OwnedDestroy)(void *object, void *const *allocator_context);

typedef struct OwnedVTable {
    OwnedDestroy destroy;
} OwnedVTable;

typedef struct OwnedObject {
    const OwnedVTable *vtable;
    char identity;
} OwnedObject;

static uint8_t sViewer[0x40628];
static NtmvUiRenderer sRenderer0;
static NtmvUiRenderer sRenderer1;
static GameManualPageIndex sPageIndex;
static OwnedObject sPanel0;
static OwnedObject sPanel1;
static OwnedObject sPage;
static uint32_t sAllocation0;
static uint32_t sAllocation1;
static char sEvents[32];
static unsigned int sEventCount;
static void *sFreed[8];
static unsigned int sFreeCount;

static void AppendEvent(char event)
{
    assert(sEventCount + 1 < sizeof(sEvents));
    sEvents[sEventCount++] = event;
    sEvents[sEventCount] = '\0';
}

static void *DestroyOwned(
    void *raw_object,
    void *const *allocator_context)
{
    OwnedObject *object = raw_object;

    assert(*allocator_context == sViewer);
    assert(object == &sPanel0 || object == &sPanel1 || object == &sPage);
    AppendEvent(object->identity);
    return object;
}

static const OwnedVTable sOwnedVTable = {DestroyOwned};

void *GameManualViewer_ResolveHostPointer(
    const void *object,
    uint32_t offset)
{
    assert(object == sViewer);
    switch (offset) {
    case 0x10:
        return &sAllocation0;
    case 0x14:
        return &sAllocation1;
    case 0x340:
        return &sRenderer0;
    case 0x344:
        return &sRenderer1;
    case 0x348:
        return &sPageIndex;
    case 0x34c:
        return &sPanel0;
    case 0x350:
        return &sPanel1;
    case 0x35c:
        return &sPage;
    default:
        assert(0);
        return 0;
    }
}

void GameManualViewer_UnloadCurrentPage(GameManualViewerPageFlow *viewer)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    AppendEvent('U');
}

void NtmvUiRenderer_Finalize(NtmvUiRenderer *renderer)
{
    assert(renderer == &sRenderer0 || renderer == &sRenderer1);
    AppendEvent(renderer == &sRenderer0 ? '0' : '1');
}

void GameManualPageIndex_Destroy_020b8c10(
    GameManualPageIndex *page_index,
    void *const *allocator_context)
{
    assert(page_index == &sPageIndex);
    assert(*allocator_context == sViewer);
    AppendEvent('I');
}

void NNS_FndFreeToAllocator(void *allocator, void *allocation)
{
    assert(allocator == sViewer);
    assert(sFreeCount < sizeof(sFreed) / sizeof(sFreed[0]));
    sFreed[sFreeCount++] = allocation;
    AppendEvent('F');
}

NtmvM2dResourceAccessor *
NtmvM2dResourceAccessor_DestroySubobject_020b70fc(
    NtmvM2dResourceAccessor *accessor)
{
    if (accessor == (NtmvM2dResourceAccessor *)(sViewer + 0x40500)) {
        AppendEvent('L');
    } else {
        assert(accessor ==
            (NtmvM2dResourceAccessor *)(sViewer + 0x403f8));
        AppendEvent('S');
    }
    return accessor;
}

void GameScrollableTileBuffer_Destroy(GameScrollableTileBuffer *buffer)
{
    assert(buffer == (GameScrollableTileBuffer *)(sViewer + 0x40370));
    AppendEvent('B');
}

void GameSoundManager_Destroy(GameSoundManager *manager)
{
    assert(manager == (GameSoundManager *)(sViewer + 0xfc));
    AppendEvent('M');
}

static void ResetEvents(void)
{
    sEventCount = 0;
    sEvents[0] = '\0';
    sFreeCount = 0;
    memset(sFreed, 0, sizeof(sFreed));
}

int main(void)
{
    static void *const expected_freed[] = {
        &sPanel0,
        &sPanel1,
        &sPageIndex,
        &sPage,
        &sRenderer1,
        &sRenderer0,
        &sAllocation1,
        &sAllocation0,
    };
    void *allocator_context = sViewer;
    unsigned int index;

    sPanel0.vtable = &sOwnedVTable;
    sPanel0.identity = 'A';
    sPanel1.vtable = &sOwnedVTable;
    sPanel1.identity = 'B';
    sPage.vtable = &sOwnedVTable;
    sPage.identity = 'P';

    ResetEvents();
    GameManualRenderer_DestroyAndFree_020bc78c(&allocator_context, 0);
    GameManualPage_DestroyAndFree_020bc7b8(&allocator_context, 0);
    GameManualPageIndex_DestroyAndFree_020bc7ec(&allocator_context, 0);
    GameManualPanel_DestroyAndFree_020bc818(&allocator_context, 0);
    assert(sEventCount == 0);
    assert(sFreeCount == 0);

    ResetEvents();
    assert(GameManualViewer_Destroy_020bc6c4(
        (GameManualViewerPageFlow *)sViewer) ==
        (GameManualViewerPageFlow *)sViewer);
    assert(strcmp(sEvents, "UAFBFIFPF1F0FFFLSBM") == 0);
    assert(sFreeCount == sizeof(expected_freed) / sizeof(expected_freed[0]));
    for (index = 0;
         index < sizeof(expected_freed) / sizeof(expected_freed[0]);
         ++index) {
        assert(sFreed[index] == expected_freed[index]);
    }
    return 0;
}
