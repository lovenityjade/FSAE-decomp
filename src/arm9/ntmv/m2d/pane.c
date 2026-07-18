#include "ntmv/m2d/pane.h"

/* External NNS allocator API used by the original Pane ownership helpers. */
extern void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size);
extern void NNS_FndFreeToAllocator(void *allocator, void *memory);

/*
 * The original slot at 0x020b5844 returns the opaque object at 0x02126a60.
 * Its ABI typeinfo begins at 0x02126a64 and names N4ntmv3m2d4PaneE.  Until the
 * CodeWarrior RTTI object layout is rebuilt, this stable local tag preserves
 * the observable identity/name without pretending to reproduce that ABI data.
 */
static const char sNtmvM2dPaneRuntimeType[] = "N4ntmv3m2d4PaneE";

const NtmvM2dPaneVTable gNtmvM2dPaneVTable = {
    NtmvM2dPane_GetRuntimeType,
    NtmvM2dPane_Dispose,
    NtmvM2dPane_UpdatePosition,
    NtmvM2dPane_Draw,
};

static NtmvM2dPane *ConstructCommon(
    NtmvM2dPane *pane, const NtmvM2dPaneResource *resource)
{
    pane->vtable = &gNtmvM2dPaneVTable;
    pane->local_position.x = 0;
    pane->local_position.y = 0;
    pane->world_position.x = 0;
    pane->world_position.y = 0;
    pane->size.width = 0;
    pane->size.height = 0;
    NtmvM2dPane_ResetTreeState(pane);

    pane->local_position = resource->position;
    pane->size = resource->size;
    return pane;
}

/*
 * 0x020b5578.  Its use by Picture and other derived constructors identifies
 * it as the base-object constructor variant; the original symbol is absent.
 */
NtmvM2dPane *NtmvM2dPane_ConstructBase(
    NtmvM2dPane *pane, const NtmvM2dPaneResource *resource)
{
    return ConstructCommon(pane, resource);
}

/*
 * 0x020b55d8.  Called after allocating exactly 0x20 bytes for a plain Pane;
 * this is the complete-object variant.  Its instructions match 0x020b5578.
 */
NtmvM2dPane *NtmvM2dPane_ConstructComplete(
    NtmvM2dPane *pane, const NtmvM2dPaneResource *resource)
{
    return ConstructCommon(pane, resource);
}

/* 0x020b5638 */
void NtmvM2dPane_ResetTreeState(NtmvM2dPane *pane)
{
    pane->state_1c = 0;
    pane->parent = NULL;
    pane->children = NULL;
    pane->child_count = 0;
    pane->child_capacity = 0;
}

/* 0x020b5844, vtable slot 0. */
const void *NtmvM2dPane_GetRuntimeType(const NtmvM2dPane *pane)
{
    (void)pane;
    return sNtmvM2dPaneRuntimeType;
}

static void DeleteChild(
    NtmvM2dPane *child, NtmvM2dAllocatorContext *allocator)
{
    if (child == NULL) {
        return;
    }
    child->vtable->dispose(child, allocator);
    NNS_FndFreeToAllocator(allocator->nns_allocator, child);
}

/*
 * 0x020b5654, vtable slot 1.  The caller at 0x020b4ecc frees the Pane object
 * itself after this virtual cleanup; this routine owns only descendants and
 * the child-pointer array.
 */
void NtmvM2dPane_Dispose(
    NtmvM2dPane *pane, NtmvM2dAllocatorContext *allocator)
{
    uint32_t index;

    for (index = 0; index < pane->child_count; ++index) {
        DeleteChild(pane->children[index], allocator);
    }
    NtmvM2dPane_FreeChildArray_020b56a0(allocator, pane->children);
}

/* 0x020b56a0 */
void NtmvM2dPane_FreeChildArray_020b56a0(
    NtmvM2dAllocatorContext *allocator,
    NtmvM2dPane **children)
{
    if (children != NULL) {
        NNS_FndFreeToAllocator(allocator->nns_allocator, children);
    }
}

/* 0x020b56dc */
NtmvM2dPane **NtmvM2dPane_AllocateChildArray_020b56dc(
    NtmvM2dAllocatorContext *allocator,
    uint32_t child_count)
{
    uint32_t allocation_count = child_count == 0 ? 1 : child_count;
    NtmvM2dPane **children = NNS_FndAllocFromAllocator(
        allocator->nns_allocator,
        allocation_count * sizeof(*children));
    uint32_t index;

    if (children == NULL) {
        return NULL;
    }
    for (index = 0; index < child_count; ++index) {
        children[index] = NULL;
    }
    return children;
}

/* 0x020b56b8 */
void NtmvM2dPane_AllocateChildren_020b56b8(
    NtmvM2dPane *pane,
    NtmvM2dAllocatorContext *allocator,
    uint16_t child_capacity)
{
    pane->children = NtmvM2dPane_AllocateChildArray_020b56dc(
        allocator, child_capacity);
    pane->child_capacity = child_capacity;
}

/* 0x020b5728 */
void NtmvM2dPane_AddChild_020b5728(
    NtmvM2dPane *parent,
    NtmvM2dPane *child)
{
    uint16_t child_index = parent->child_count;

    ++parent->child_count;
    parent->children[child_index] = child;
    child->parent = parent;
}

/* 0x020b5744 */
void NtmvM2dPane_UpdateTree_020b5744(
    NtmvM2dPane *pane,
    void *context,
    const NtmvM2dPoint *parent_position)
{
    NtmvM2dPoint inherited_position = *parent_position;
    uint32_t index;

    pane->vtable->update_position(pane, context, &inherited_position);
    for (index = 0; index < pane->child_count; ++index) {
        NtmvM2dPoint child_parent_position = pane->world_position;

        NtmvM2dPane_UpdateTree_020b5744(
            pane->children[index], context, &child_parent_position);
    }
}

/* 0x020b57f4..0x020b583f */
void NtmvM2dPane_DrawTree_020b57f4(
    NtmvM2dPane *pane,
    void *context)
{
    uint32_t index;

    pane->vtable->draw(pane, context);
    for (index = 0; index < pane->child_count; ++index) {
        NtmvM2dPane_DrawTree_020b57f4(
            pane->children[index], context);
    }
}

/*
 * 0x020b57c8, vtable slot 2.  The middle parameter is propagated by the tree
 * walker but unused by the base Pane implementation.
 */
void NtmvM2dPane_UpdatePosition(
    NtmvM2dPane *pane, void *context, const NtmvM2dPoint *parent_position)
{
    (void)context;
    pane->world_position.x = (int16_t)(pane->local_position.x + parent_position->x);
    pane->world_position.y = (int16_t)(pane->local_position.y + parent_position->y);
}

/* 0x020b5840, vtable slot 3. Derived pane types override this drawing hook. */
void NtmvM2dPane_Draw(NtmvM2dPane *pane, void *context)
{
    (void)pane;
    (void)context;
}
