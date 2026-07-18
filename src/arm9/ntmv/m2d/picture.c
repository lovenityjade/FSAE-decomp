#include "ntmv/m2d/picture.h"

/*
 * 0x02126a98 precedes the ABI typeinfo at 0x02126a9c.  That typeinfo names
 * N4ntmv3m2d7PictureE and records Pane's 0x02126a64 typeinfo as its base.
 */
static const char sNtmvM2dPictureRuntimeType[] = "N4ntmv3m2d7PictureE";

/*
 * The ROM header begins at 0x02126abc.  Its typeinfo word is 0x02126ac0,
 * while objects contain the callable address point 0x02126ac4.
 */
const NtmvM2dPaneVTable gNtmvM2dPictureVTable = {
    NtmvM2dPicture_GetRuntimeType, /* 0x020b5944 */
    NtmvM2dPane_Dispose,           /* 0x020b5654 */
    NtmvM2dPane_UpdatePosition,    /* 0x020b57c8 */
    NtmvM2dPicture_Draw,           /* 0x020b589c */
};

/* 0x020b5850 */
NtmvM2dPicture *NtmvM2dPicture_Construct(
    NtmvM2dPicture *picture,
    const NtmvM2dPictureResource *resource,
    const NtmvM2dPictureBuildContext *build_context)
{
    NtmvM2dPane_ConstructBase(&picture->pane, &resource->pane);
    picture->pane.vtable = &gNtmvM2dPictureVTable;
    picture->bitmap_ref = NULL;
    picture->bitmap_ref =
        &build_context->resource_set->bitmaps[resource->bitmap_index];
    picture->transform_code = resource->transform_code;
    return picture;
}

/* 0x020b5944, vtable slot 0. */
const void *NtmvM2dPicture_GetRuntimeType(const NtmvM2dPane *pane)
{
    (void)pane;
    return sNtmvM2dPictureRuntimeType;
}

/*
 * 0x020b589c, vtable slot 3.  Picture keeps a pointer to one entry of the
 * resource bitmap table, so the bitmap remains an intentional double
 * indirection throughout this routine.
 */
void NtmvM2dPicture_Draw(NtmvM2dPane *pane, void *context)
{
    NtmvM2dPicture *picture = (NtmvM2dPicture *)pane;
    const NtmvM2dPictureBitmap *bitmap = *picture->bitmap_ref;
    const NtmvM2dTransformDescriptor *transform;
    NtmvM2dPoint source_origin;
    NtmvM2dPoint adjusted_origin;
    NtmvM2dSize bitmap_size;
    NtmvM2dSize adjusted_size;
    NtmvM2dPoint destination;

    bitmap_size.width = (int16_t)bitmap->width;
    bitmap_size.height = (int16_t)bitmap->height;
    transform = NtmvM2d_GetTransformDescriptor_020b76bc(picture->transform_code);
    source_origin.x = 0;
    source_origin.y = 0;
    NtmvM2d_ComputeSourceOrigin_020b76d0(
        &source_origin, &bitmap_size, transform);

    adjusted_origin = source_origin;
    adjusted_size = bitmap_size;
    destination = picture->pane.world_position;
    NtmvM2d_BlitTransformed_020b786c(
        context,
        &destination,
        &adjusted_size,
        &picture->bitmap_ref,
        &adjusted_origin,
        transform);
}
