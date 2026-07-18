#ifndef FSAE_NTMV_M2D_PAGE_H
#define FSAE_NTMV_M2D_PAGE_H

#include <stddef.h>
#include <stdint.h>

typedef struct NtmvM2dPane NtmvM2dPane;
typedef struct NtmvM2dPage NtmvM2dPage;

/* The game passes a wrapper whose first word is an NNS Fnd allocator handle. */
typedef struct NtmvAllocatorRef {
    void *handle;
} NtmvAllocatorRef;

typedef struct NtmvM2dPageVTable {
    void (*release)(
        NtmvM2dPage *page,
        NtmvAllocatorRef *allocator,
        void *context,
        uint32_t flags
    );
} NtmvM2dPageVTable;

/*
 * RTTI name: ntmv::m2d::Page
 * Typeinfo: 0x02126a38; vtable address point: 0x02126a48.
 */
struct NtmvM2dPage {
    const NtmvM2dPageVTable *vtable; /* +0x00 */
    NtmvM2dPane *root_pane;         /* +0x04 */
    uint16_t content_width;         /* +0x08 */
    int16_t content_extent;         /* +0x0a */
    uint16_t palette_color;         /* +0x0c */
    uint16_t reserved_0e;           /* +0x0e */
    void *content_resource;         /* +0x10: selected txp1 pool entry */
    uint16_t resource_count;        /* +0x14 */
    uint16_t field_16;              /* +0x16 */
    void **resource_files;          /* +0x18: files named by the txl1 block */
    int16_t viewport_extent;        /* +0x1c */
    uint16_t field_1e;              /* +0x1e */
};

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dPageSizeCheck[sizeof(NtmvM2dPage) == 0x20 ? 1 : -1];
typedef char NtmvM2dPageRootOffsetCheck[
    offsetof(NtmvM2dPage, root_pane) == 0x04 ? 1 : -1];
typedef char NtmvM2dPageResourceOffsetCheck[
    offsetof(NtmvM2dPage, resource_files) == 0x18 ? 1 : -1];
typedef char NtmvM2dPageContentResourceOffsetCheck[
    offsetof(NtmvM2dPage, content_resource) == 0x10 ? 1 : -1];
typedef char NtmvM2dPageContentExtentOffsetCheck[
    offsetof(NtmvM2dPage, content_extent) == 0x0a ? 1 : -1];
typedef char NtmvM2dPageContentWidthOffsetCheck[
    offsetof(NtmvM2dPage, content_width) == 0x08 ? 1 : -1];
typedef char NtmvM2dPagePaletteOffsetCheck[
    offsetof(NtmvM2dPage, palette_color) == 0x0c ? 1 : -1];
#endif

/* 0x020b4e60 */
void NtmvM2dPage_Init(NtmvM2dPage *page);

/* Vtable slot 0, 0x020b4e8c. */
void NtmvM2dPage_Release(
    NtmvM2dPage *page,
    NtmvAllocatorRef *allocator,
    void *context,
    uint32_t flags
);

#endif /* FSAE_NTMV_M2D_PAGE_H */
