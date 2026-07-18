#include "ntmv/m2d/ui_panel.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

static int sRenderCount;
static int sDestroyCount;
static int sFreeCount;

void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size)
{
    (void)allocator;
    return malloc(size);
}

void NNS_FndFreeToAllocator(void *allocator, void *memory)
{
    (void)allocator;
    ++sFreeCount;
    free(memory);
}

static void DestroyChild(
    NtmvM2dUIElement *element, NtmvM2dAllocatorContext *allocator)
{
    (void)element;
    (void)allocator;
    ++sDestroyCount;
}

static void RenderChild(
    NtmvM2dUIElement *element,
    void *render_context,
    const NtmvM2dPoint *parent_position)
{
    (void)element;
    (void)render_context;
    assert(parent_position->x == 13 && parent_position->y == 24);
    ++sRenderCount;
}

static const NtmvM2dUIElementVTable sChildVTable = {
    DestroyChild,
    RenderChild,
    NtmvM2dUIElement_SetPosition,
    NtmvM2dUIElement_SetSize,
    NtmvM2dUIElement_SetVisible,
};

int main(void)
{
    NtmvM2dUIPanel panel;
    NtmvM2dAllocatorContext allocator = {(void *)(uintptr_t)0x1234};
    NtmvM2dUIElement **source = malloc(2 * sizeof(*source));
    NtmvM2dPoint position = {3, 4};
    NtmvM2dSize size = {100, 80};
    NtmvM2dPoint parent = {10, 20};
    unsigned int index;

    assert(source != NULL);
    for (index = 0; index < 2; ++index) {
        source[index] = malloc(sizeof(*source[index]));
        assert(source[index] != NULL);
        source[index]->vtable = &sChildVTable;
    }
    assert(NtmvM2dUIPanel_ConstructComplete(&panel) == &panel);
    NtmvM2dUIPanel_Configure(&panel, &allocator, source, 2, &position, &size);
    free(source);
    NtmvM2dUIPanel_Render(&panel, NULL, &parent);
    assert(sRenderCount == 2);
    NtmvM2dUIPanel_Destroy(&panel, &allocator);
    assert(sDestroyCount == 2);
    assert(sFreeCount == 3);
    return 0;
}
