#include "ntmv/m2d/blit.h"

/* 0x02126c4c */
static const NtmvM2dTransformDescriptor sTransformDescriptors[] = {
    {{0, 0}, {1, 1}, 0},
    {{1, 0}, {-1, 1}, 0},
    {{0, 1}, {1, -1}, 0},
    {{0, 1}, {1, -1}, 1},
    {{1, 1}, {-1, -1}, 0},
    {{1, 0}, {-1, 1}, 1},
};

/* 0x020b76bc */
const NtmvM2dTransformDescriptor *NtmvM2d_GetTransformDescriptor_020b76bc(
    uint8_t transform_code)
{
    return &sTransformDescriptors[transform_code];
}

static int16_t GetSizeAxis(const NtmvM2dSize *size, uint32_t axis)
{
    return axis != 0 ? size->height : size->width;
}

static int8_t GetOriginFactorAxis(
    const NtmvM2dTransformDescriptor *transform,
    uint32_t axis)
{
    return transform->origin_factor[axis];
}

static void SetPointAxis(NtmvM2dPoint *point, uint32_t axis, int16_t value)
{
    if (axis != 0) {
        point->y = value;
    } else {
        point->x = value;
    }
}

/* 0x020b76d0 */
void NtmvM2d_ComputeSourceOrigin_020b76d0(
    NtmvM2dPoint *origin,
    const NtmvM2dSize *bitmap_size,
    const NtmvM2dTransformDescriptor *transform)
{
    uint32_t axis = transform->transpose_axes != 0 ? 1u : 0u;
    uint32_t other = axis ^ 1u;
    int16_t axis_size = GetSizeAxis(bitmap_size, axis);
    int16_t other_size = GetSizeAxis(bitmap_size, other);
    int8_t axis_factor = GetOriginFactorAxis(transform, axis);
    int8_t other_factor = GetOriginFactorAxis(transform, other);

    SetPointAxis(
        origin,
        axis,
        (int16_t)(axis_factor * (int32_t)(axis_size - 1)));
    SetPointAxis(
        origin,
        other,
        (int16_t)(other_factor * (int32_t)(other_size - 1)));
}
