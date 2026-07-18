#include "ntmv/m2d/render_surface.h"
#include "ntmv/m2d/texel_picker.h"
#include "ntmv/m2d/text_box.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef union InlineObjectStorage {
    max_align_t alignment;
    uint8_t bytes[0x10 + 4 * 3 * sizeof(uint16_t)];
} InlineObjectStorage;

void NtmvM2dTextBox_LineIntersectsClip(
    NtmvM2dRect *output,
    const NtmvM2dPoint *destination,
    const NtmvM2dSize *size,
    const NtmvM2dRect *clip)
{
    int32_t clipped_left = clip->left - destination->x;
    int32_t clipped_top = clip->top - destination->y;
    int32_t clipped_right = clip->right - destination->x;
    int32_t clipped_bottom = clip->bottom - destination->y;

    output->left = 0;
    output->top = 0;
    output->right = 0;
    output->bottom = 0;
    if (clipped_left < 0) {
        clipped_left = 0;
    }
    if (size->width < clipped_right) {
        clipped_right = size->width;
    }
    if (clipped_left < clipped_right) {
        if (clipped_top < 0) {
            clipped_top = 0;
        }
        if (size->height < clipped_bottom) {
            clipped_bottom = size->height;
        }
        if (clipped_top < clipped_bottom) {
            output->left = (int16_t)clipped_left;
            output->top = (int16_t)clipped_top;
            output->right = (int16_t)clipped_right;
            output->bottom = (int16_t)clipped_bottom;
        }
    }
}

static void ResetPixels(uint16_t *pixels, uint16_t value, unsigned int count)
{
    unsigned int index;

    for (index = 0; index < count; ++index) {
        pixels[index] = value;
    }
}

static const NtmvM2dInlineObject *MakeDirectInlineObject(
    InlineObjectStorage *storage)
{
    NtmvM2dTextureHeader *header =
        (NtmvM2dTextureHeader *)(void *)storage->bytes;
    uint16_t *texels;

    (void)memset(storage, 0, sizeof(*storage));
    header->width = 4;
    header->height = 3;
    header->format = 7;
    texels = (uint16_t *)(void *)header->payload;
    texels[0] = 0x8001;
    texels[1] = 0x8002;
    texels[2] = 0x0003;
    texels[3] = 0x8004;
    texels[4] = 0x8005;
    texels[5] = 0x0006;
    texels[6] = 0x8007;
    texels[7] = 0x8008;
    texels[8] = 0x0009;
    texels[9] = 0x800a;
    texels[10] = 0x800b;
    texels[11] = 0x000c;
    return (const NtmvM2dInlineObject *)(const void *)storage->bytes;
}

static void DrawInlineObject(
    NtmvM2dRenderSurface *surface,
    const NtmvM2dPoint *destination,
    const NtmvM2dInlineObject *inline_object)
{
    const NtmvM2dInlineObject *const *slot = &inline_object;

    NtmvM2d_DrawInlineObject_020b771c(
        (NtmvM2dTextDrawContext *)(void *)surface,
        destination,
        &slot);
}

static void TestDirectInlineObjectCopiesOnlyAlphaTexels(void)
{
    InlineObjectStorage storage;
    const NtmvM2dInlineObject *inline_object =
        MakeDirectInlineObject(&storage);
    uint16_t pixels[6 * 8];
    NtmvM2dRenderSurface surface;
    NtmvM2dPoint destination = {1, 1};

    (void)memset(&surface, 0, sizeof(surface));
    surface.clip.left = 2;
    surface.clip.top = 1;
    surface.clip.right = 5;
    surface.clip.bottom = 4;
    surface.pixels = pixels;
    surface.row_stride = 6;
    surface.row_wrap = 8;

    ResetPixels(pixels, 0x1111, 6 * 8);
    DrawInlineObject(&surface, &destination, inline_object);

    assert(pixels[1 * 6 + 2] == 0x8002);
    assert(pixels[1 * 6 + 3] == 0x1111);
    assert(pixels[1 * 6 + 4] == 0x8004);
    assert(pixels[2 * 6 + 2] == 0x1111);
    assert(pixels[2 * 6 + 3] == 0x8007);
    assert(pixels[2 * 6 + 4] == 0x8008);
    assert(pixels[3 * 6 + 2] == 0x800a);
    assert(pixels[3 * 6 + 3] == 0x800b);
    assert(pixels[3 * 6 + 4] == 0x1111);

    assert(pixels[0] == 0x1111);
    assert(pixels[1 * 6 + 1] == 0x1111);
    assert(pixels[4 * 6 + 4] == 0x1111);
}

static void TestNoIntersectionDoesNotTouchPixels(void)
{
    InlineObjectStorage storage;
    const NtmvM2dInlineObject *inline_object =
        MakeDirectInlineObject(&storage);
    uint16_t pixels[6 * 8];
    NtmvM2dRenderSurface surface;
    NtmvM2dPoint destination = {0, 0};
    unsigned int index;

    (void)memset(&surface, 0, sizeof(surface));
    surface.clip.left = 10;
    surface.clip.top = 10;
    surface.clip.right = 12;
    surface.clip.bottom = 12;
    surface.pixels = pixels;
    surface.row_stride = 6;
    surface.row_wrap = 8;

    ResetPixels(pixels, 0x2222, 6 * 8);
    DrawInlineObject(&surface, &destination, inline_object);

    for (index = 0; index < 6 * 8; ++index) {
        assert(pixels[index] == 0x2222);
    }
}

int main(void)
{
    TestDirectInlineObjectCopiesOnlyAlphaTexels();
    TestNoIntersectionDoesNotTouchPixels();
    return 0;
}
