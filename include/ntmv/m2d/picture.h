#ifndef FSAE_NTMV_M2D_PICTURE_H
#define FSAE_NTMV_M2D_PICTURE_H

#include "ntmv/m2d/blit.h"
#include "ntmv/m2d/pane.h"

#include <stddef.h>
#include <stdint.h>

typedef NtmvM2dBitmap NtmvM2dPictureBitmap;

typedef struct NtmvM2dPictureResource {
    NtmvM2dPaneResource pane; /* +0x00 */
    uint16_t bitmap_index;    /* +0x10 */
    uint8_t transform_code;   /* +0x12 */
    uint8_t unknown_13;
} NtmvM2dPictureResource;

/* The constructor uses only the bitmap table pointer at +0x04. */
typedef struct NtmvM2dPictureResourceSet {
    uint32_t unknown_00;
    const NtmvM2dPictureBitmap *const *bitmaps;
} NtmvM2dPictureResourceSet;

/* The resource-set pointer is read at +0x0c on the 32-bit target. */
typedef struct NtmvM2dPictureBuildContext {
    uint8_t unknown_00[0x0c];
    const NtmvM2dPictureResourceSet *resource_set;
} NtmvM2dPictureBuildContext;

typedef struct NtmvM2dPicture {
    NtmvM2dPane pane;                              /* +0x00 */
    const NtmvM2dPictureBitmap *const *bitmap_ref; /* +0x20 */
    uint8_t transform_code;                        /* +0x24 */
    uint8_t reserved_25[3];
} NtmvM2dPicture;

typedef char NtmvM2dPictureResourceSizeCheck[
    sizeof(NtmvM2dPictureResource) == 0x14 ? 1 : -1];
typedef char NtmvM2dPictureBitmapIndexOffsetCheck[
    offsetof(NtmvM2dPictureResource, bitmap_index) == 0x10 ? 1 : -1];

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dPictureResourceSetSizeCheck[
    sizeof(NtmvM2dPictureResourceSet) == 0x08 ? 1 : -1];
typedef char NtmvM2dPictureBitmapsOffsetCheck[
    offsetof(NtmvM2dPictureResourceSet, bitmaps) == 0x04 ? 1 : -1];
typedef char NtmvM2dPictureTargetSizeCheck[
    sizeof(NtmvM2dPicture) == 0x28 ? 1 : -1];
typedef char NtmvM2dPictureBitmapOffsetCheck[
    offsetof(NtmvM2dPicture, bitmap_ref) == 0x20 ? 1 : -1];
typedef char NtmvM2dPictureTransformOffsetCheck[
    offsetof(NtmvM2dPicture, transform_code) == 0x24 ? 1 : -1];
typedef char NtmvM2dPictureResourceSetOffsetCheck[
    offsetof(NtmvM2dPictureBuildContext, resource_set) == 0x0c ? 1 : -1];
#endif

/* Header typeinfo word 0x02126ac0; callable address point 0x02126ac4. */
extern const NtmvM2dPaneVTable gNtmvM2dPictureVTable;

/* 0x020b5850 */
NtmvM2dPicture *NtmvM2dPicture_Construct(
    NtmvM2dPicture *picture,
    const NtmvM2dPictureResource *resource,
    const NtmvM2dPictureBuildContext *build_context);

/* Vtable slots 0 and 3; slots 1 and 2 are inherited from Pane. */
const void *NtmvM2dPicture_GetRuntimeType(const NtmvM2dPane *pane); /* 0x020b5944 */
void NtmvM2dPicture_Draw(NtmvM2dPane *pane, void *context);         /* 0x020b589c */

#endif
