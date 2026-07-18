#include "game/manual_page_loader.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static uint32_t sAllocator;
static GameManualPage sPage;
static bool sFailAllocation;
static unsigned int sInitCount;

void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size)
{
    assert(allocator == &sAllocator);
    assert(size == 0x20);
    return sFailAllocation ? 0 : &sPage;
}

void NtmvM2dPage_Init(NtmvM2dPage *page)
{
    assert(page == &sPage);
    ++sInitCount;
}

int main(void)
{
    void *allocator_context = &sAllocator;

    memset(&sPage, 0xa5, sizeof(sPage));
    sFailAllocation = false;
    sInitCount = 0;
    assert(GameManualPage_Allocate_020bcd60(&allocator_context) == &sPage);
    assert(sInitCount == 1);

    sFailAllocation = true;
    sInitCount = 0;
    assert(GameManualPage_Allocate_020bcd60(&allocator_context) == 0);
    assert(sInitCount == 0);
    return 0;
}
