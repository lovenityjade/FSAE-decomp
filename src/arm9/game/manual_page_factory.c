#include "game/manual_page_loader.h"

extern void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size);

/* 0x020bcd60 */
GameManualPage *GameManualPage_Allocate_020bcd60(
    void *const *allocator_context)
{
    GameManualPage *page = NNS_FndAllocFromAllocator(
        *allocator_context, 0x20);

    if (page != 0) {
        NtmvM2dPage_Init(page);
        return page;
    }
    return 0;
}
