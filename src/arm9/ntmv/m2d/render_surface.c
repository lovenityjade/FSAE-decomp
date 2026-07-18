#include "ntmv/m2d/render_surface.h"

/* 0x020b7508 */
uint16_t *NtmvM2d_RenderSurfaceRowAddress_020b7508(
    const NtmvM2dRenderSurface *surface,
    uint32_t y)
{
    uint32_t wrapped_y = y & (uint32_t)(surface->row_wrap - 1);

    return surface->pixels + surface->row_stride * wrapped_y;
}

/* 0x020b73b8 */
void NtmvM2d_FillRect_020b73b8(
    void *context,
    const NtmvM2dPoint *destination,
    const NtmvM2dSize *size,
    uint16_t value)
{
    NtmvM2dRenderSurface *surface = context;
    NtmvM2dRect clipped;

    NtmvM2dTextBox_LineIntersectsClip(
        &clipped,
        destination,
        size,
        &surface->clip);

    if (clipped.left < clipped.right) {
        int32_t y = clipped.top;

        if (y < clipped.bottom) {
            do {
                uint16_t *row = NtmvM2d_RenderSurfaceRowAddress_020b7508(
                    surface,
                    (uint32_t)(int16_t)(destination->y + y));
                int32_t x = clipped.left;

                if (x < clipped.right) {
                    do {
                        row[destination->x + x] = value;
                        ++x;
                    } while (x < clipped.right);
                }
                ++y;
            } while (y < clipped.bottom);
        }
    }
}
