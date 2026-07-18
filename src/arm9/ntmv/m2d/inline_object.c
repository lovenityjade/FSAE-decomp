#include "ntmv/m2d/render_surface.h"
#include "ntmv/m2d/texel_picker.h"
#include "ntmv/m2d/text_box.h"

/* 0x020b771c */
void NtmvM2d_DrawInlineObject_020b771c(
    NtmvM2dTextDrawContext *context,
    const NtmvM2dPoint *destination,
    const NtmvM2dInlineObject *const **inline_object_ref)
{
    const NtmvM2dInlineObject *inline_object = **inline_object_ref;
    NtmvM2dRenderSurface *surface = (NtmvM2dRenderSurface *)context;
    NtmvM2dTextureRef texture;
    NtmvM2dTexelPicker picker = {
        &gNtmvM2dTexelPickerVTable,
        0,
        0,
        0,
        0,
        0,
        0,
        {0, 0, 0}
    };
    NtmvM2dSize size;
    NtmvM2dPoint local_destination = *destination;
    NtmvM2dRect clipped;
    int32_t y;

    size.width = inline_object->width;
    size.height = inline_object->height;
    NtmvM2dTextBox_LineIntersectsClip(
        &clipped,
        &local_destination,
        &size,
        &surface->clip);
    if ((int16_t)(clipped.right - clipped.left) == 0) {
        return;
    }

    texture.data = (const uint8_t *)inline_object;
    (void)NtmvM2dTexelPicker_Create(&picker, &texture);
    y = clipped.top;
    if (clipped.bottom <= y) {
        return;
    }
    do {
        uint16_t *row = NtmvM2d_RenderSurfaceRowAddress_020b7508(
            surface,
            (uint32_t)(int16_t)(destination->y + y));
        int32_t x = clipped.left;

        if (x < clipped.right) {
            do {
                NtmvM2dPoint source;
                uint16_t texel;

                source.x = (int16_t)x;
                source.y = (int16_t)y;
                texel = picker.vtable->pick(&picker, &source);
                if ((texel & 0x8000u) != 0) {
                    row[destination->x + x] = texel;
                }
                ++x;
            } while (x < clipped.right);
        }
        ++y;
    } while (y < clipped.bottom);
}
