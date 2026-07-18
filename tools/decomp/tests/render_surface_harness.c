#include "ntmv/m2d/render_surface.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static void ResetPixels(uint16_t *pixels, uint16_t value, unsigned int count)
{
    unsigned int index;

    for (index = 0; index < count; ++index) {
        pixels[index] = value;
    }
}

static void AssertOnlyFilled(
    const uint16_t *pixels,
    unsigned int width,
    unsigned int height,
    uint16_t background,
    uint16_t value)
{
    unsigned int y;
    unsigned int x;

    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            uint16_t expected = background;

            if (y >= 1 && y < 4 && x >= 2 && x < 5) {
                expected = value;
            }
            assert(pixels[y * width + x] == expected);
        }
    }
}

static void AssertAllPixels(
    const uint16_t *pixels,
    uint16_t expected,
    unsigned int count)
{
    unsigned int index;

    for (index = 0; index < count; ++index) {
        assert(pixels[index] == expected);
    }
}

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

int main(void)
{
    uint16_t pixels[8 * 8];
    NtmvM2dRenderSurface surface;
    NtmvM2dPoint destination;
    NtmvM2dSize size;

    memset(&surface, 0, sizeof(surface));
    surface.clip.left = 2;
    surface.clip.top = 1;
    surface.clip.right = 5;
    surface.clip.bottom = 4;
    surface.pixels = pixels;
    surface.row_stride = 8;
    surface.row_wrap = 8;

    assert(NtmvM2d_RenderSurfaceRowAddress_020b7508(
               &surface, 9) == &pixels[8]);

    ResetPixels(pixels, 0x1111, 8 * 8);
    destination.x = 0;
    destination.y = 0;
    size.width = 8;
    size.height = 8;
    NtmvM2d_FillRect_020b73b8(&surface, &destination, &size, 0x7fff);
    AssertOnlyFilled(pixels, 8, 8, 0x1111, 0x7fff);

    ResetPixels(pixels, 0x2222, 8 * 8);
    destination.x = 6;
    destination.y = 6;
    size.width = 2;
    size.height = 2;
    NtmvM2d_FillRect_020b73b8(&surface, &destination, &size, 0x1234);
    AssertAllPixels(pixels, 0x2222, 8 * 8);
    return 0;
}
