#include "ntmv/m2d/blit.h"
#include "ntmv/m2d/render_surface.h"

static int16_t GetPointAxis(const NtmvM2dPoint *point, uint32_t axis)
{
    return axis != 0 ? point->y : point->x;
}

static int16_t GetSizeAxis(const NtmvM2dSize *size, uint32_t axis)
{
    return axis != 0 ? size->height : size->width;
}

static void SetPointAxis(
    NtmvM2dPoint *point, uint32_t axis, int16_t value)
{
    if (axis != 0) {
        point->y = value;
    } else {
        point->x = value;
    }
}

static int32_t ClampSourceCoordinate(int32_t value, int32_t upper_bound)
{
    if (value <= 0) {
        value = 0;
    }
    if (value > upper_bound) {
        value = upper_bound;
    }
    return value;
}

/* 0x020b786c..0x020b7afb; literal 0x020b7afc excluded. */
void NtmvM2d_BlitTransformed_020b786c(
    void *context,
    const NtmvM2dPoint *destination,
    const NtmvM2dSize *draw_size,
    const NtmvM2dBitmap *const *const *bitmap_ref,
    const NtmvM2dPoint *source_origin,
    const NtmvM2dTransformDescriptor *transform)
{
    NtmvM2dRenderSurface *surface = context;
    const NtmvM2dBitmap *bitmap = **bitmap_ref;
    NtmvM2dPoint local_destination = *destination;
    NtmvM2dSize local_draw_size = *draw_size;
    NtmvM2dSize bitmap_size = {
        (int16_t)bitmap->width,
        (int16_t)bitmap->height,
    };
    NtmvM2dRect clipped;
    NtmvM2dTextureRef texture = {(const uint8_t *)bitmap};
    NtmvM2dTexelPicker picker = {
        &gNtmvM2dTexelPickerVTable,
        0,
        0,
        0,
        0,
        0,
        0,
        {0, 0, 0},
    };
    uint32_t axis;
    uint32_t other_axis;
    int32_t axis_limit;
    int32_t other_limit;
    int32_t initial_source_axis;
    int32_t source_other;
    int8_t step_axis;
    int8_t step_other;
    int32_t y;

    NtmvM2dTextBox_LineIntersectsClip(
        &clipped,
        &local_destination,
        &local_draw_size,
        &surface->clip);
    if ((int16_t)(clipped.right - clipped.left) == 0) {
        return;
    }

    axis = transform->transpose_axes != 0 ? 1u : 0u;
    other_axis = axis ^ 1u;
    step_axis = transform->step[axis];
    step_other = transform->step[other_axis];
    axis_limit = GetSizeAxis(&bitmap_size, axis) - 1;
    other_limit = GetSizeAxis(&bitmap_size, other_axis) - 1;
    initial_source_axis = (int16_t)(
        GetPointAxis(source_origin, axis) + clipped.left * step_axis);
    source_other = (int16_t)(
        GetPointAxis(source_origin, other_axis) + clipped.top * step_other);

    (void)NtmvM2dTexelPicker_Create(&picker, &texture);
    y = clipped.top;
    if (clipped.bottom <= y) {
        return;
    }
    do {
        uint16_t *row = NtmvM2d_RenderSurfaceRowAddress_020b7508(
            surface,
            (uint32_t)(int16_t)(destination->y + y));
        NtmvM2dPoint source = {0, 0};
        int32_t x = clipped.left;
        int32_t source_axis = initial_source_axis;

        SetPointAxis(
            &source,
            other_axis,
            (int16_t)ClampSourceCoordinate(source_other, other_limit));
        if (x < clipped.right) {
            do {
                uint16_t texel;

                SetPointAxis(
                    &source,
                    axis,
                    (int16_t)ClampSourceCoordinate(source_axis, axis_limit));
                texel = picker.vtable->pick(&picker, &source);
                if ((texel & 0x8000u) != 0) {
                    row[destination->x + x] = texel;
                }
                source_axis = (int16_t)(source_axis + step_axis);
                ++x;
            } while (x < clipped.right);
        }
        y = (int16_t)(y + 1);
        source_other = (int16_t)(source_other + step_other);
    } while (y < clipped.bottom);
}
