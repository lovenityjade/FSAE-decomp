#include "game/manual_viewer_lifecycle.h"

#include "game/blz_file_loader.h"

#include <stddef.h>
#include <stdint.h>

enum {
    GAME_MANUAL_SHARED_ALLOCATION_OFFSET = 0x10,
    GAME_MANUAL_LOCALIZED_ALLOCATION_OFFSET = 0x14
};

#if UINTPTR_MAX > UINT32_MAX
extern void GameManualViewer_StoreHostPointer(
    void *object,
    uint32_t offset,
    void *value);
#endif

static void StoreEmbeddedPointer(void *object, uint32_t offset, void *value)
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

static void *AlignPointer(void *pointer, uint32_t alignment)
{
    uintptr_t value = (uintptr_t)pointer;
    uintptr_t mask = (uintptr_t)alignment - 1;

    return (void *)((value + mask) & ~mask);
}

/* 0x020bc84c */
void GameManualViewer_Initialize_020bc84c(
    GameManualViewerPageFlow *viewer,
    const uint32_t allocator_template[4],
    const char *shared_archive_path,
    const char *localized_archive_path,
    const uint32_t tilemap_context[3],
    uint16_t language)
{
    void *shared_allocation;
    void *localized_allocation;

    GameManualViewer_InitializeHardware_020bc8bc(
        viewer, allocator_template);

    shared_allocation = Game_LoadBlzFileToAllocator_020b7b1c(
        shared_archive_path, viewer, 4, NULL);
    StoreEmbeddedPointer(
        viewer,
        GAME_MANUAL_SHARED_ALLOCATION_OFFSET,
        shared_allocation);

    localized_allocation = Game_LoadBlzFileToAllocator_020b7b1c(
        localized_archive_path, viewer, 0x20, NULL);
    StoreEmbeddedPointer(
        viewer,
        GAME_MANUAL_LOCALIZED_ALLOCATION_OFFSET,
        localized_allocation);

    GameManualViewer_Setup_020bc99c(
        viewer,
        shared_allocation,
        AlignPointer(localized_allocation, 0x20),
        tilemap_context,
        language);
}
