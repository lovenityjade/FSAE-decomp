#include "game/manual_viewer_lifecycle.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static uint8_t sViewer[0x40628];
static uint32_t sSharedAllocation;
static _Alignas(32) uint8_t sLocalizedStorage[96];
static void *sStoredShared;
static void *sStoredLocalized;
static char sEvents[8];
static unsigned int sEventCount;
static const uint32_t sAllocatorTemplate[4] = {
    0x11111111,
    0x22222222,
    0x33333333,
    0x44444444,
};
static const uint32_t sTilemapContext[3] = {
    0x55555555,
    0x66666666,
    0x77777777,
};

static void Event(char event)
{
    sEvents[sEventCount++] = event;
    sEvents[sEventCount] = '\0';
}

void GameManualViewer_InitializeHardware_020bc8bc(
    GameManualViewerPageFlow *viewer,
    const uint32_t initial_context[4])
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    assert(initial_context == sAllocatorTemplate);
    Event('H');
}

void *Game_LoadBlzFileToAllocator_020b7b1c(
    const char *path,
    void *allocator,
    uint32_t alignment,
    uint32_t *uncompressed_size)
{
    assert(allocator == sViewer);
    assert(uncompressed_size == 0);
    if (alignment == 4) {
        assert(strcmp(path, "shared.blz") == 0);
        Event('A');
        return &sSharedAllocation;
    }
    assert(alignment == 0x20);
    assert(strcmp(path, "localized.blz") == 0);
    Event('B');
    return sLocalizedStorage + 3;
}

void GameManualViewer_StoreHostPointer(
    void *object,
    uint32_t offset,
    void *value)
{
    assert(object == sViewer);
    if (offset == 0x10) {
        sStoredShared = value;
        return;
    }
    assert(offset == 0x14);
    sStoredLocalized = value;
}

void GameManualViewer_Setup_020bc99c(
    GameManualViewerPageFlow *viewer,
    void *shared_archive,
    void *localized_archive,
    const uint32_t tilemap_context[3],
    uint16_t language)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    assert(shared_archive == &sSharedAllocation);
    assert(localized_archive == sLocalizedStorage + 0x20);
    assert(tilemap_context == sTilemapContext);
    assert(language == 7);
    Event('S');
}

int main(void)
{
    sStoredShared = 0;
    sStoredLocalized = 0;
    sEventCount = 0;
    sEvents[0] = '\0';

    GameManualViewer_Initialize_020bc84c(
        (GameManualViewerPageFlow *)sViewer,
        sAllocatorTemplate,
        "shared.blz",
        "localized.blz",
        sTilemapContext,
        7);

    assert(strcmp(sEvents, "HABS") == 0);
    assert(sStoredShared == &sSharedAllocation);
    assert(sStoredLocalized == sLocalizedStorage + 3);
    return 0;
}
