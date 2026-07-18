#include "ntmv/m2d/blit.h"
#include "ntmv/m2d/render_surface.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef union BitmapStorage {
    max_align_t alignment;
    uint8_t bytes[0x10 + 3 * 2 * sizeof(uint16_t)];
} BitmapStorage;

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

static NtmvM2dBitmap *MakeBitmap(BitmapStorage *storage)
{
    NtmvM2dBitmap *bitmap = (NtmvM2dBitmap *)(void *)storage->bytes;
    uint16_t *texels;

    memset(storage, 0, sizeof(*storage));
    bitmap->width = 3;
    bitmap->height = 2;
    bitmap->format = 7;
    texels = (uint16_t *)(void *)bitmap->payload;
    texels[0] = 0x8001;
    texels[1] = 0x0002;
    texels[2] = 0x8003;
    texels[3] = 0x8004;
    texels[4] = 0x8005;
    texels[5] = 0x8006;
    return bitmap;
}

static void ResetPixels(uint16_t *pixels, uint16_t value)
{
    unsigned int index;

    for (index = 0; index < 8 * 8; ++index) {
        pixels[index] = value;
    }
}

static void RunBlit(
    NtmvM2dRenderSurface *surface,
    const NtmvM2dPoint *destination,
    const NtmvM2dSize *draw_size,
    const NtmvM2dBitmap *bitmap,
    const NtmvM2dTransformDescriptor *transform)
{
    const NtmvM2dBitmap *bitmap_slot = bitmap;
    const NtmvM2dBitmap *const *bitmap_ref = &bitmap_slot;
    NtmvM2dSize bitmap_size = {
        (int16_t)bitmap->width,
        (int16_t)bitmap->height,
    };
    NtmvM2dPoint source_origin = {0, 0};

    NtmvM2d_ComputeSourceOrigin_020b76d0(
        &source_origin, &bitmap_size, transform);
    NtmvM2d_BlitTransformed_020b786c(
        surface,
        destination,
        draw_size,
        &bitmap_ref,
        &source_origin,
        transform);
}

static void TestIdentityAndAlpha(void)
{
    BitmapStorage storage;
    NtmvM2dBitmap *bitmap = MakeBitmap(&storage);
    uint16_t pixels[8 * 8];
    NtmvM2dRenderSurface surface;
    NtmvM2dPoint destination = {1, 1};
    NtmvM2dSize draw_size = {3, 2};

    memset(&surface, 0, sizeof(surface));
    surface.clip = (NtmvM2dRect){0, 0, 8, 8};
    surface.pixels = pixels;
    surface.row_stride = 8;
    surface.row_wrap = 8;
    ResetPixels(pixels, 0x1111);
    RunBlit(
        &surface,
        &destination,
        &draw_size,
        bitmap,
        NtmvM2d_GetTransformDescriptor_020b76bc(0));

    assert(pixels[1 * 8 + 1] == 0x8001);
    assert(pixels[1 * 8 + 2] == 0x1111);
    assert(pixels[1 * 8 + 3] == 0x8003);
    assert(pixels[2 * 8 + 1] == 0x8004);
    assert(pixels[2 * 8 + 2] == 0x8005);
    assert(pixels[2 * 8 + 3] == 0x8006);
    assert(pixels[0] == 0x1111 && pixels[3 * 8 + 1] == 0x1111);
}

static void TestFlips(void)
{
    BitmapStorage storage;
    NtmvM2dBitmap *bitmap = MakeBitmap(&storage);
    uint16_t pixels[8 * 8];
    NtmvM2dRenderSurface surface;
    NtmvM2dPoint destination = {1, 1};
    NtmvM2dSize draw_size = {3, 2};

    memset(&surface, 0, sizeof(surface));
    surface.clip = (NtmvM2dRect){0, 0, 8, 8};
    surface.pixels = pixels;
    surface.row_stride = 8;
    surface.row_wrap = 8;

    ResetPixels(pixels, 0x2222);
    RunBlit(
        &surface,
        &destination,
        &draw_size,
        bitmap,
        NtmvM2d_GetTransformDescriptor_020b76bc(1));
    assert(pixels[1 * 8 + 1] == 0x8003);
    assert(pixels[1 * 8 + 2] == 0x2222);
    assert(pixels[1 * 8 + 3] == 0x8001);
    assert(pixels[2 * 8 + 1] == 0x8006);
    assert(pixels[2 * 8 + 2] == 0x8005);
    assert(pixels[2 * 8 + 3] == 0x8004);

    ResetPixels(pixels, 0x3333);
    RunBlit(
        &surface,
        &destination,
        &draw_size,
        bitmap,
        NtmvM2d_GetTransformDescriptor_020b76bc(2));
    assert(pixels[1 * 8 + 1] == 0x8004);
    assert(pixels[1 * 8 + 2] == 0x8005);
    assert(pixels[1 * 8 + 3] == 0x8006);
    assert(pixels[2 * 8 + 1] == 0x8001);
    assert(pixels[2 * 8 + 2] == 0x3333);
    assert(pixels[2 * 8 + 3] == 0x8003);

    ResetPixels(pixels, 0x3a3a);
    RunBlit(
        &surface,
        &destination,
        &draw_size,
        bitmap,
        NtmvM2d_GetTransformDescriptor_020b76bc(4));
    assert(pixels[1 * 8 + 1] == 0x8006);
    assert(pixels[1 * 8 + 2] == 0x8005);
    assert(pixels[1 * 8 + 3] == 0x8004);
    assert(pixels[2 * 8 + 1] == 0x8003);
    assert(pixels[2 * 8 + 2] == 0x3a3a);
    assert(pixels[2 * 8 + 3] == 0x8001);
}

static void TestTransposedAxes(void)
{
    BitmapStorage storage;
    NtmvM2dBitmap *bitmap = MakeBitmap(&storage);
    uint16_t pixels[8 * 8];
    NtmvM2dRenderSurface surface;
    NtmvM2dPoint destination = {2, 1};
    NtmvM2dSize draw_size = {2, 3};

    memset(&surface, 0, sizeof(surface));
    surface.clip = (NtmvM2dRect){0, 0, 8, 8};
    surface.pixels = pixels;
    surface.row_stride = 8;
    surface.row_wrap = 8;
    ResetPixels(pixels, 0x4444);
    RunBlit(
        &surface,
        &destination,
        &draw_size,
        bitmap,
        NtmvM2d_GetTransformDescriptor_020b76bc(3));

    assert(pixels[1 * 8 + 2] == 0x8004);
    assert(pixels[1 * 8 + 3] == 0x8001);
    assert(pixels[2 * 8 + 2] == 0x8005);
    assert(pixels[2 * 8 + 3] == 0x4444);
    assert(pixels[3 * 8 + 2] == 0x8006);
    assert(pixels[3 * 8 + 3] == 0x8003);

    ResetPixels(pixels, 0x4a4a);
    RunBlit(
        &surface,
        &destination,
        &draw_size,
        bitmap,
        NtmvM2d_GetTransformDescriptor_020b76bc(5));
    assert(pixels[1 * 8 + 2] == 0x8003);
    assert(pixels[1 * 8 + 3] == 0x8006);
    assert(pixels[2 * 8 + 2] == 0x4a4a);
    assert(pixels[2 * 8 + 3] == 0x8005);
    assert(pixels[3 * 8 + 2] == 0x8001);
    assert(pixels[3 * 8 + 3] == 0x8004);
}

static void TestClipOffsetsAndSourceSaturation(void)
{
    BitmapStorage storage;
    NtmvM2dBitmap *bitmap = MakeBitmap(&storage);
    uint16_t pixels[8 * 8];
    NtmvM2dRenderSurface surface;
    NtmvM2dPoint destination = {-1, -1};
    NtmvM2dSize draw_size = {5, 4};
    unsigned int row;

    memset(&surface, 0, sizeof(surface));
    surface.clip = (NtmvM2dRect){0, 0, 4, 4};
    surface.pixels = pixels;
    surface.row_stride = 8;
    surface.row_wrap = 8;
    ResetPixels(pixels, 0x5555);
    RunBlit(
        &surface,
        &destination,
        &draw_size,
        bitmap,
        NtmvM2d_GetTransformDescriptor_020b76bc(0));

    for (row = 0; row < 3; ++row) {
        assert(pixels[row * 8 + 0] == 0x8005);
        assert(pixels[row * 8 + 1] == 0x8006);
        assert(pixels[row * 8 + 2] == 0x8006);
        assert(pixels[row * 8 + 3] == 0x8006);
    }

    surface.clip = (NtmvM2dRect){10, 10, 12, 12};
    ResetPixels(pixels, 0x6666);
    RunBlit(
        &surface,
        &destination,
        &draw_size,
        bitmap,
        NtmvM2d_GetTransformDescriptor_020b76bc(0));
    for (row = 0; row < 8 * 8; ++row) {
        assert(pixels[row] == 0x6666);
    }
}

int main(void)
{
    TestIdentityAndAlpha();
    TestFlips();
    TestTransposedAxes();
    TestClipOffsetsAndSourceSaturation();
    return 0;
}
