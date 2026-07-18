#include "ntmv/m2d/page.h"

#include <stddef.h>

typedef struct NtmvM2dPaneVTable {
    void (*slot_0)(void);
    void (*destroy)(
        NtmvM2dPane *pane,
        NtmvAllocatorRef *allocator,
        void *context,
        uint32_t flags
    );
} NtmvM2dPaneVTable;

struct NtmvM2dPane {
    const NtmvM2dPaneVTable *vtable;
};

/* Address point 0x02126a48, recovered from the Page RTTI object. */
extern const NtmvM2dPageVTable gNtmvM2dPageVTable;

extern void NNS_FndFreeToAllocator(void *allocator, void *allocation);

/* 0x020b4eb4 */
static void NtmvM2dPage_FreeOwnedBlock(NtmvAllocatorRef *allocator, void *block)
{
    if (block != NULL) {
        NNS_FndFreeToAllocator(allocator->handle, block);
    }
}

/*
 * 0x020b4ecc
 *
 * The virtual call is proven to use Pane vtable slot 1.  The semantic names
 * of the two trailing arguments remain provisional until Pane is recovered.
 */
static void NtmvM2dPage_DestroyOwnedPane(
    NtmvAllocatorRef *allocator,
    NtmvM2dPane *pane,
    void *context,
    uint32_t flags
)
{
    if (pane != NULL) {
        pane->vtable->destroy(pane, allocator, context, flags);
        NNS_FndFreeToAllocator(allocator->handle, pane);
    }
}

/* 0x020b4e60 */
void NtmvM2dPage_Init(NtmvM2dPage *page)
{
    page->vtable = &gNtmvM2dPageVTable;
    page->root_pane = NULL;
    page->content_width = 0;
    page->content_extent = 0;
    page->resource_count = 0;
    page->resource_files = NULL;
    page->viewport_extent = 0;
}

/* 0x020b4e8c, Page vtable slot 0. */
void NtmvM2dPage_Release(
    NtmvM2dPage *page,
    NtmvAllocatorRef *allocator,
    void *context,
    uint32_t flags
)
{
    NtmvM2dPage_DestroyOwnedPane(allocator, page->root_pane, context, flags);
    NtmvM2dPage_FreeOwnedBlock(allocator, page->resource_files);
}
