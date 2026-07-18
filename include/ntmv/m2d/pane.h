#ifndef FSAE_NTMV_M2D_PANE_H
#define FSAE_NTMV_M2D_PANE_H

#include <stddef.h>
#include <stdint.h>

typedef struct NtmvM2dPane NtmvM2dPane;
typedef struct NtmvM2dPaneVTable NtmvM2dPaneVTable;

typedef struct NtmvM2dPoint {
    int16_t x;
    int16_t y;
} NtmvM2dPoint;

typedef struct NtmvM2dSize {
    int16_t width;
    int16_t height;
} NtmvM2dSize;

/* Only the portion consumed by the recovered Pane constructors is named. */
typedef struct NtmvM2dPaneResource {
    uint8_t unknown_00[8];
    NtmvM2dPoint position;
    NtmvM2dSize size;
} NtmvM2dPaneResource;

typedef char NtmvM2dPaneResourceSizeCheck[
    sizeof(NtmvM2dPaneResource) == 0x10 ? 1 : -1];
typedef char NtmvM2dPaneResourcePositionOffsetCheck[
    offsetof(NtmvM2dPaneResource, position) == 0x08 ? 1 : -1];

/* The original allocator context stores its NNS allocator in its first word. */
typedef struct NtmvM2dAllocatorContext {
    void *nns_allocator;
} NtmvM2dAllocatorContext;

struct NtmvM2dPaneVTable {
    const void *(*get_runtime_type)(const NtmvM2dPane *pane);
    void (*dispose)(NtmvM2dPane *pane, NtmvM2dAllocatorContext *allocator);
    void (*update_position)(
        NtmvM2dPane *pane, void *context, const NtmvM2dPoint *parent_position);
    void (*draw)(NtmvM2dPane *pane, void *context);
};

/* Target ARM9 layout: 0x20 bytes. */
struct NtmvM2dPane {
    const NtmvM2dPaneVTable *vtable; /* +0x00 */
    NtmvM2dPane *parent;             /* +0x04 */
    NtmvM2dPane **children;          /* +0x08 */
    NtmvM2dPoint local_position;     /* +0x0c */
    NtmvM2dPoint world_position;     /* +0x10 */
    NtmvM2dSize size;                /* +0x14 */
    uint16_t child_count;            /* +0x18 */
    uint16_t child_capacity;         /* +0x1a */
    uint8_t state_1c;                /* +0x1c: purpose not yet established */
    uint8_t reserved_1d[3];
};

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dPaneTargetSizeCheck[sizeof(NtmvM2dPane) == 0x20 ? 1 : -1];
typedef char NtmvM2dPaneChildrenOffsetCheck[
    offsetof(NtmvM2dPane, children) == 0x08 ? 1 : -1];
typedef char NtmvM2dPaneLocalPositionOffsetCheck[
    offsetof(NtmvM2dPane, local_position) == 0x0c ? 1 : -1];
typedef char NtmvM2dPaneChildCountOffsetCheck[
    offsetof(NtmvM2dPane, child_count) == 0x18 ? 1 : -1];
#endif

/* Semantic vtable corresponding to address point 0x02126a88. */
extern const NtmvM2dPaneVTable gNtmvM2dPaneVTable;

/* 0x020b5578: base-object constructor variant, used by derived classes. */
NtmvM2dPane *NtmvM2dPane_ConstructBase(
    NtmvM2dPane *pane, const NtmvM2dPaneResource *resource);

/* 0x020b55d8: complete-object constructor variant, used for a plain Pane. */
NtmvM2dPane *NtmvM2dPane_ConstructComplete(
    NtmvM2dPane *pane, const NtmvM2dPaneResource *resource);

/* 0x020b5638: reset parent/children bookkeeping, without changing geometry. */
void NtmvM2dPane_ResetTreeState(NtmvM2dPane *pane);

/* 0x020b56a0: null-safe release of a Pane child-pointer array. */
void NtmvM2dPane_FreeChildArray_020b56a0(
    NtmvM2dAllocatorContext *allocator,
    NtmvM2dPane **children);

/* 0x020b56b8: installs a zeroed array and records capacity at +0x1a. */
void NtmvM2dPane_AllocateChildren_020b56b8(
    NtmvM2dPane *pane,
    NtmvM2dAllocatorContext *allocator,
    uint16_t child_capacity);

/* 0x020b56dc: count zero still allocates one pointer-sized slot. */
NtmvM2dPane **NtmvM2dPane_AllocateChildArray_020b56dc(
    NtmvM2dAllocatorContext *allocator,
    uint32_t child_count);

/* 0x020b5728: appends without a capacity check and writes child's parent. */
void NtmvM2dPane_AddChild_020b5728(
    NtmvM2dPane *parent,
    NtmvM2dPane *child);

/* 0x020b5744: virtual position update followed by recursive child updates. */
void NtmvM2dPane_UpdateTree_020b5744(
    NtmvM2dPane *pane,
    void *context,
    const NtmvM2dPoint *parent_position);

/* 0x020b57f4: virtual draw followed by recursive child draws. */
void NtmvM2dPane_DrawTree_020b57f4(
    NtmvM2dPane *pane,
    void *context);

/* Virtual slots at the 0x02126a88 address point. */
const void *NtmvM2dPane_GetRuntimeType(const NtmvM2dPane *pane); /* 0x020b5844 */
void NtmvM2dPane_Dispose(                                      /* 0x020b5654 */
    NtmvM2dPane *pane, NtmvM2dAllocatorContext *allocator);
void NtmvM2dPane_UpdatePosition(                               /* 0x020b57c8 */
    NtmvM2dPane *pane, void *context, const NtmvM2dPoint *parent_position);
void NtmvM2dPane_Draw(NtmvM2dPane *pane, void *context);       /* 0x020b5840 */

#endif
