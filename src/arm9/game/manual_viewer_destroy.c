#include "game/manual_viewer_lifecycle.h"

#include "game/scrollable_tile_buffer.h"
#include "game/sound_manager.h"
#include "ntmv/m2d/resource_accessor.h"
#include "ntmv/ui_renderer.h"

#include <stdint.h>

enum {
    GAME_MANUAL_ALLOCATION_0_OFFSET = 0x10,
    GAME_MANUAL_ALLOCATION_1_OFFSET = 0x14,
    GAME_MANUAL_SOUND_MANAGER_OFFSET = 0x0fc,
    GAME_MANUAL_RENDERER_0_OFFSET = 0x340,
    GAME_MANUAL_RENDERER_1_OFFSET = 0x344,
    GAME_MANUAL_PAGE_INDEX_OFFSET = 0x348,
    GAME_MANUAL_ROOT_PANEL_0_OFFSET = 0x34c,
    GAME_MANUAL_ROOT_PANEL_1_OFFSET = 0x350,
    GAME_MANUAL_PAGE_RESOURCE_OFFSET = 0x35c,
    GAME_MANUAL_SCROLL_BUFFER_OFFSET = 0x40370,
    GAME_MANUAL_SHARED_ACCESSOR_OFFSET = 0x403f8,
    GAME_MANUAL_LOCALIZED_ACCESSOR_OFFSET = 0x40500
};

typedef void *(*GameManualOwnedDestroy)(
    void *object,
    void *const *allocator_context);

typedef struct GameManualOwnedVTable {
    GameManualOwnedDestroy destroy;
} GameManualOwnedVTable;

typedef struct GameManualOwnedObject {
    const GameManualOwnedVTable *vtable;
} GameManualOwnedObject;

extern void NNS_FndFreeToAllocator(void *allocator, void *allocation);

#if UINTPTR_MAX > UINT32_MAX
extern void *GameManualViewer_ResolveHostPointer(
    const void *object,
    uint32_t offset);
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

/* 0x020bc78c */
void GameManualRenderer_DestroyAndFree_020bc78c(
    void *const *allocator_context,
    NtmvUiRenderer *renderer)
{
    if (renderer != 0) {
        NtmvUiRenderer_Finalize(renderer);
        NNS_FndFreeToAllocator(*allocator_context, renderer);
    }
}

/* 0x020bc7b8 */
void GameManualPage_DestroyAndFree_020bc7b8(
    void *const *allocator_context,
    void *page)
{
    GameManualOwnedObject *object = (GameManualOwnedObject *)page;

    if (object != 0) {
        object->vtable->destroy(object, allocator_context);
        NNS_FndFreeToAllocator(*allocator_context, object);
    }
}

/* 0x020bc7ec */
void GameManualPageIndex_DestroyAndFree_020bc7ec(
    void *const *allocator_context,
    GameManualPageIndex *page_index)
{
    if (page_index != 0) {
        GameManualPageIndex_Destroy_020b8c10(
            page_index, allocator_context);
        NNS_FndFreeToAllocator(*allocator_context, page_index);
    }
}

/* 0x020bc818 */
void GameManualPanel_DestroyAndFree_020bc818(
    void *const *allocator_context,
    void *panel)
{
    GameManualOwnedObject *object = (GameManualOwnedObject *)panel;

    if (object != 0) {
        object->vtable->destroy(object, allocator_context);
        NNS_FndFreeToAllocator(*allocator_context, object);
    }
}

/* 0x020bc6c4 */
GameManualViewerPageFlow *GameManualViewer_Destroy_020bc6c4(
    GameManualViewerPageFlow *viewer)
{
    uint8_t *bytes = (uint8_t *)viewer;
    void *allocator_context = viewer;
    uint32_t index;
    void *allocation;

    GameManualViewer_UnloadCurrentPage(viewer);
    for (index = 0; index < 2; ++index) {
        GameManualPanel_DestroyAndFree_020bc818(
            &allocator_context,
            ReadEmbeddedPointer(
                viewer,
                GAME_MANUAL_ROOT_PANEL_0_OFFSET + index * 4));
    }
    GameManualPageIndex_DestroyAndFree_020bc7ec(
        &allocator_context,
        (GameManualPageIndex *)ReadEmbeddedPointer(
            viewer, GAME_MANUAL_PAGE_INDEX_OFFSET));
    GameManualPage_DestroyAndFree_020bc7b8(
        &allocator_context,
        ReadEmbeddedPointer(viewer, GAME_MANUAL_PAGE_RESOURCE_OFFSET));
    GameManualRenderer_DestroyAndFree_020bc78c(
        &allocator_context,
        (NtmvUiRenderer *)ReadEmbeddedPointer(
            viewer, GAME_MANUAL_RENDERER_1_OFFSET));
    GameManualRenderer_DestroyAndFree_020bc78c(
        &allocator_context,
        (NtmvUiRenderer *)ReadEmbeddedPointer(
            viewer, GAME_MANUAL_RENDERER_0_OFFSET));

    allocation = ReadEmbeddedPointer(viewer, GAME_MANUAL_ALLOCATION_1_OFFSET);
    if (allocation != 0) {
        NNS_FndFreeToAllocator(viewer, allocation);
    }
    allocation = ReadEmbeddedPointer(viewer, GAME_MANUAL_ALLOCATION_0_OFFSET);
    if (allocation != 0) {
        NNS_FndFreeToAllocator(viewer, allocation);
    }

    NtmvM2dResourceAccessor_DestroySubobject_020b70fc(
        (NtmvM2dResourceAccessor *)(
            bytes + GAME_MANUAL_LOCALIZED_ACCESSOR_OFFSET));
    NtmvM2dResourceAccessor_DestroySubobject_020b70fc(
        (NtmvM2dResourceAccessor *)(
            bytes + GAME_MANUAL_SHARED_ACCESSOR_OFFSET));
    GameScrollableTileBuffer_Destroy(
        (GameScrollableTileBuffer *)(
            bytes + GAME_MANUAL_SCROLL_BUFFER_OFFSET));
    GameSoundManager_Destroy(
        (GameSoundManager *)(bytes + GAME_MANUAL_SOUND_MANAGER_OFFSET));
    return viewer;
}
