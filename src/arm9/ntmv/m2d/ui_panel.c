#include "ntmv/m2d/ui_panel.h"

#include <stddef.h>

extern void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size);
extern void NNS_FndFreeToAllocator(void *allocator, void *memory);

const NtmvM2dUIPanelVTable gNtmvM2dUIPanelVTable = {
    NtmvM2dUIPanel_Destroy,
    NtmvM2dUIPanel_Render,
    (void (*)(NtmvM2dUIPanel *, const NtmvM2dPoint *))NtmvM2dUIElement_SetPosition,
    (void (*)(NtmvM2dUIPanel *, const NtmvM2dSize *))NtmvM2dUIElement_SetSize,
    (void (*)(NtmvM2dUIPanel *, bool))NtmvM2dUIElement_SetVisible,
};

static NtmvM2dUIPanel *ConstructCommon(NtmvM2dUIPanel *panel)
{
    NtmvM2dUIElement_Init(&panel->base);
    panel->base.vtable = (const NtmvM2dUIElementVTable *)&gNtmvM2dUIPanelVTable;
    panel->children = NULL;
    panel->child_count = 0;
    return panel;
}

/* 0x020b8020: base-object constructor variant. */
NtmvM2dUIPanel *NtmvM2dUIPanel_ConstructBase(NtmvM2dUIPanel *panel)
{
    return ConstructCommon(panel);
}

/* 0x020b804c: instruction-identical complete-object constructor variant. */
NtmvM2dUIPanel *NtmvM2dUIPanel_ConstructComplete(NtmvM2dUIPanel *panel)
{
    return ConstructCommon(panel);
}

/* 0x020b80ec */
static NtmvM2dUIElement **AllocateChildren(
    NtmvM2dAllocatorContext *allocator, uint16_t child_count)
{
    uint32_t allocation_count = child_count != 0 ? child_count : 1;
    NtmvM2dUIElement **children = NNS_FndAllocFromAllocator(
        allocator->nns_allocator, allocation_count * sizeof(*children));
    uint16_t index;

    if (children == NULL) {
        return NULL;
    }
    for (index = 0; index < child_count; ++index) {
        children[index] = NULL;
    }
    return children;
}

/* 0x020b8078 */
void NtmvM2dUIPanel_Configure(
    NtmvM2dUIPanel *panel,
    NtmvM2dAllocatorContext *allocator,
    NtmvM2dUIElement *const *children,
    uint16_t child_count,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size)
{
    uint16_t index;

    panel->children = AllocateChildren(allocator, child_count);
    panel->child_count = child_count;
    for (index = 0; index < child_count; ++index) {
        panel->children[index] = children[index];
    }
    panel->base.local_position = *position;
    panel->base.size = *size;
}

/* 0x020b8190 */
static void FreeChildren(
    NtmvM2dAllocatorContext *allocator, NtmvM2dUIElement **children)
{
    if (children != NULL) {
        NNS_FndFreeToAllocator(allocator->nns_allocator, children);
    }
}

/* 0x020b81a8 */
static void DeleteChild(
    NtmvM2dAllocatorContext *allocator, NtmvM2dUIElement *child)
{
    if (child != NULL) {
        child->vtable->destroy(child, allocator);
        NNS_FndFreeToAllocator(allocator->nns_allocator, child);
    }
}

/* 0x020b8138, UIPanel vtable slot 0. */
void NtmvM2dUIPanel_Destroy(
    NtmvM2dUIPanel *panel, NtmvM2dAllocatorContext *allocator)
{
    uint16_t index;

    for (index = 0; index < panel->child_count; ++index) {
        DeleteChild(allocator, panel->children[index]);
    }
    FreeChildren(allocator, panel->children);
    NtmvM2dUIElement_Destroy(&panel->base, allocator);
}

/* 0x020b81dc, UIPanel vtable slot 1. */
void NtmvM2dUIPanel_Render(
    NtmvM2dUIPanel *panel,
    void *render_context,
    const NtmvM2dPoint *parent_position)
{
    uint16_t index;

    if ((panel->base.flags & NTMV_M2D_UI_HIDDEN) != 0) {
        return;
    }
    panel->base.world_center.x =
        (int16_t)(parent_position->x + panel->base.local_position.x);
    panel->base.world_center.y =
        (int16_t)(parent_position->y + panel->base.local_position.y);
    for (index = 0; index < panel->child_count; ++index) {
        NtmvM2dUIElement *child = panel->children[index];
        child->vtable->render(child, render_context, &panel->base.world_center);
    }
}
