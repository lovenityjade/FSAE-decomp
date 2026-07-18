#include "ntmv/m2d/picture.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static NtmvM2dTransformDescriptor sTransform;
static uint8_t sTransformCode;
static unsigned int sOriginCalls;
static unsigned int sBlitCalls;

void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size)
{
    (void)allocator;
    (void)size;
    assert(0);
    return NULL;
}

void NNS_FndFreeToAllocator(void *allocator, void *memory)
{
    (void)allocator;
    (void)memory;
}

const struct NtmvM2dTransformDescriptor *NtmvM2d_GetTransformDescriptor_020b76bc(
    uint8_t transform_code)
{
    sTransformCode = transform_code;
    return &sTransform;
}

void NtmvM2d_ComputeSourceOrigin_020b76d0(
    NtmvM2dPoint *origin,
    const NtmvM2dSize *bitmap_size,
    const struct NtmvM2dTransformDescriptor *transform)
{
    assert(bitmap_size->width == 13 && bitmap_size->height == 7);
    assert(transform == &sTransform && sTransformCode == 4);
    origin->x = 2;
    origin->y = 3;
    ++sOriginCalls;
}

void NtmvM2d_BlitTransformed_020b786c(
    void *context,
    const NtmvM2dPoint *destination,
    const NtmvM2dSize *bitmap_size,
    const NtmvM2dPictureBitmap *const *const *bitmap_ref,
    const NtmvM2dPoint *source_origin,
    const struct NtmvM2dTransformDescriptor *transform)
{
    assert(context == (void *)(uintptr_t)0x5678);
    assert(destination->x == 40 && destination->y == -5);
    assert(bitmap_size->width == 13 && bitmap_size->height == 7);
    assert((**bitmap_ref)->width == 13 && (**bitmap_ref)->height == 7);
    assert(source_origin->x == 2 && source_origin->y == 3);
    assert(transform == &sTransform);
    ++sBlitCalls;
}

int main(void)
{
    struct {
        NtmvM2dPictureBitmap bitmap;
        uint8_t pixels[4];
    } image;
    const NtmvM2dPictureBitmap *bitmaps[1];
    NtmvM2dPictureResourceSet resource_set;
    NtmvM2dPictureBuildContext build_context;
    NtmvM2dPictureResource resource;
    NtmvM2dPicture picture;

    memset(&image, 0, sizeof(image));
    image.bitmap.width = 13;
    image.bitmap.height = 7;
    bitmaps[0] = &image.bitmap;
    resource_set.unknown_00 = 0;
    resource_set.bitmaps = bitmaps;
    memset(&build_context, 0, sizeof(build_context));
    build_context.resource_set = &resource_set;
    memset(&resource, 0, sizeof(resource));
    resource.pane.position.x = 5;
    resource.pane.position.y = 6;
    resource.pane.size.width = 13;
    resource.pane.size.height = 7;
    resource.bitmap_index = 0;
    resource.transform_code = 4;

    assert(NtmvM2dPicture_Construct(&picture, &resource, &build_context) == &picture);
    assert(picture.pane.vtable == &gNtmvM2dPictureVTable);
    assert(picture.bitmap_ref == &bitmaps[0]);
    assert(picture.transform_code == 4);
    assert(strcmp(
               (const char *)picture.pane.vtable->get_runtime_type(&picture.pane),
               "N4ntmv3m2d7PictureE") == 0);

    picture.pane.world_position.x = 40;
    picture.pane.world_position.y = -5;
    picture.pane.vtable->draw(&picture.pane, (void *)(uintptr_t)0x5678);
    assert(sOriginCalls == 1);
    assert(sBlitCalls == 1);
    return 0;
}
