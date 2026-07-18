#ifndef FSAE_NTMV_M2D_WINDOW_H
#define FSAE_NTMV_M2D_WINDOW_H

#include "ntmv/m2d/blit.h"
#include "ntmv/m2d/pane.h"

#include <stddef.h>
#include <stdint.h>

typedef struct NtmvM2dWindow NtmvM2dWindow;
typedef struct NtmvM2dWindowVTable NtmvM2dWindowVTable;

/* Bitmap header consumed by Window's border renderer. */
typedef NtmvM2dBitmap NtmvM2dWindowBitmap;

/* One four-byte border record beginning at resource offset +0x14. */
typedef struct NtmvM2dWindowResourceEntry {
    uint16_t bitmap_index;
    uint8_t transform_code;
    uint8_t unknown_03;
} NtmvM2dWindowResourceEntry;

typedef struct NtmvM2dWindowResource {
    NtmvM2dPaneResource pane; /* +0x00 */
    uint16_t fill_value;      /* +0x10: exact purpose below bit 15 is uncertain */
    uint8_t border_count;     /* +0x12: renderer recognizes 1 and 4 */
    uint8_t unknown_13;
    NtmvM2dWindowResourceEntry borders[]; /* +0x14 */
} NtmvM2dWindowResource;

/* Constructor-visible portion of the shared resource set. */
typedef struct NtmvM2dWindowResourceSet {
    uint32_t unknown_00;
    const NtmvM2dWindowBitmap *const *bitmaps; /* +0x04 on ARM9 */
} NtmvM2dWindowResourceSet;

typedef struct NtmvM2dWindowBuildContext {
    uint8_t unknown_00[0x0c];
    const NtmvM2dWindowResourceSet *resource_set; /* +0x0c on ARM9 */
} NtmvM2dWindowBuildContext;

/* Runtime form of one resource border. Target size: eight bytes. */
typedef struct NtmvM2dWindowBorder {
    const NtmvM2dWindowBitmap *const *bitmap_ref;
    uint8_t transform_code;
    uint8_t reserved_05[3];
} NtmvM2dWindowBorder;

typedef struct NtmvM2dWindowMargins {
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;
} NtmvM2dWindowMargins;

/*
 * Target ARM9 layout: 0x28 bytes.  The fill value itself is stored at +0x1e,
 * inside Pane's currently unnamed reserved tail, rather than after the base.
 */
struct NtmvM2dWindow {
    NtmvM2dPane pane;              /* +0x00 */
    NtmvM2dWindowBorder *borders;  /* +0x20 */
    uint8_t border_count;          /* +0x24 */
    uint8_t reserved_25[3];
};

struct NtmvM2dWindowVTable {
    const void *(*get_runtime_type)(const NtmvM2dPane *pane);
    void (*dispose)(NtmvM2dPane *pane, NtmvM2dAllocatorContext *allocator);
    void (*update_position)(
        NtmvM2dPane *pane, void *context, const NtmvM2dPoint *parent_position);
    void (*draw)(NtmvM2dPane *pane, void *context);
    void (*draw_interior)(
        NtmvM2dWindow *window,
        void *context,
        const NtmvM2dPoint *position,
        const NtmvM2dWindowMargins *margins);
    void (*draw_single_border)(
        NtmvM2dWindow *window,
        void *context,
        const NtmvM2dPoint *position,
        const NtmvM2dWindowBorder *border,
        const NtmvM2dWindowMargins *margins);
    void (*draw_four_borders)(
        NtmvM2dWindow *window,
        void *context,
        const NtmvM2dPoint *position,
        const NtmvM2dWindowBorder *borders,
        const NtmvM2dWindowMargins *margins);
};

typedef char NtmvM2dWindowResourceEntrySizeCheck[
    sizeof(NtmvM2dWindowResourceEntry) == 0x04 ? 1 : -1];
typedef char NtmvM2dWindowResourceHeaderSizeCheck[
    offsetof(NtmvM2dWindowResource, borders) == 0x14 ? 1 : -1];
typedef char NtmvM2dWindowFillOffsetCheck[
    offsetof(NtmvM2dWindowResource, fill_value) == 0x10 ? 1 : -1];

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dWindowResourceSetSizeCheck[
    sizeof(NtmvM2dWindowResourceSet) == 0x08 ? 1 : -1];
typedef char NtmvM2dWindowResourceSetOffsetCheck[
    offsetof(NtmvM2dWindowBuildContext, resource_set) == 0x0c ? 1 : -1];
typedef char NtmvM2dWindowBorderSizeCheck[
    sizeof(NtmvM2dWindowBorder) == 0x08 ? 1 : -1];
typedef char NtmvM2dWindowTargetSizeCheck[
    sizeof(NtmvM2dWindow) == 0x28 ? 1 : -1];
typedef char NtmvM2dWindowBordersOffsetCheck[
    offsetof(NtmvM2dWindow, borders) == 0x20 ? 1 : -1];
typedef char NtmvM2dWindowBorderCountOffsetCheck[
    offsetof(NtmvM2dWindow, border_count) == 0x24 ? 1 : -1];
#endif

/* Header 0x02126af8; typeinfo word 0x02126afc; address point 0x02126b00. */
extern const NtmvM2dWindowVTable gNtmvM2dWindowVTable;

/* 0x020b5ba8 */
NtmvM2dWindow *NtmvM2dWindow_Construct(
    NtmvM2dWindow *window,
    NtmvM2dAllocatorContext *allocator,
    const NtmvM2dWindowResource *resource,
    const NtmvM2dWindowBuildContext *build_context);

/* Seven virtual slots at the 0x02126b00 address point. */
const void *NtmvM2dWindow_GetRuntimeType(const NtmvM2dPane *pane); /* slot 0, 0x020b64f0 */
void NtmvM2dWindow_Dispose(                                      /* slot 1, 0x020b5cd4 */
    NtmvM2dPane *pane, NtmvM2dAllocatorContext *allocator);
/* Slot 2 is inherited NtmvM2dPane_UpdatePosition at 0x020b57c8. */
void NtmvM2dWindow_Draw(NtmvM2dPane *pane, void *context);       /* slot 3, 0x020b5d48 */
void NtmvM2dWindow_DrawInterior(                                /* slot 4, 0x020b5e48 */
    NtmvM2dWindow *window,
    void *context,
    const NtmvM2dPoint *position,
    const NtmvM2dWindowMargins *margins);
void NtmvM2dWindow_DrawSingleBorder(                            /* slot 5, 0x020b5ec4 */
    NtmvM2dWindow *window,
    void *context,
    const NtmvM2dPoint *position,
    const NtmvM2dWindowBorder *border,
    const NtmvM2dWindowMargins *margins);
void NtmvM2dWindow_DrawFourBorders(                             /* slot 6, 0x020b6158 */
    NtmvM2dWindow *window,
    void *context,
    const NtmvM2dPoint *position,
    const NtmvM2dWindowBorder *borders,
    const NtmvM2dWindowMargins *margins);

#endif
