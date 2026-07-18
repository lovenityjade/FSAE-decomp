#include "ntmv/m2d/blit.h"

#include <assert.h>
#include <stdint.h>

static void AssertDescriptor(
    uint8_t transform_code,
    int8_t origin_x,
    int8_t origin_y,
    int8_t step_x,
    int8_t step_y,
    uint8_t transpose_axes)
{
    const NtmvM2dTransformDescriptor *transform =
        NtmvM2d_GetTransformDescriptor_020b76bc(transform_code);

    assert(transform->origin_factor[0] == origin_x);
    assert(transform->origin_factor[1] == origin_y);
    assert(transform->step[0] == step_x);
    assert(transform->step[1] == step_y);
    assert(transform->transpose_axes == transpose_axes);
}

static void AssertOrigin(
    const NtmvM2dTransformDescriptor *transform,
    int16_t width,
    int16_t height,
    int16_t expected_x,
    int16_t expected_y)
{
    NtmvM2dSize size;
    NtmvM2dPoint origin;

    size.width = width;
    size.height = height;
    origin.x = 0x1234;
    origin.y = 0x5678;
    NtmvM2d_ComputeSourceOrigin_020b76d0(&origin, &size, transform);
    assert(origin.x == expected_x);
    assert(origin.y == expected_y);
}

int main(void)
{
    const NtmvM2dTransformDescriptor identity = {{0, 0}, {1, 1}, 0};
    const NtmvM2dTransformDescriptor bottom = {{0, 1}, {1, -1}, 0};
    const NtmvM2dTransformDescriptor right_transposed = {{1, 0}, {-1, 1}, 1};
    const NtmvM2dTransformDescriptor negative = {{-1, -1}, {-1, -1}, 1};

    AssertDescriptor(0, 0, 0, 1, 1, 0);
    AssertDescriptor(1, 1, 0, -1, 1, 0);
    AssertDescriptor(2, 0, 1, 1, -1, 0);
    AssertDescriptor(3, 0, 1, 1, -1, 1);
    AssertDescriptor(4, 1, 1, -1, -1, 0);
    AssertDescriptor(5, 1, 0, -1, 1, 1);

    AssertOrigin(&identity, 10, 6, 0, 0);
    AssertOrigin(&bottom, 10, 6, 0, 5);
    AssertOrigin(&right_transposed, 10, 6, 9, 0);
    AssertOrigin(&negative, 10, 6, -9, -5);
    return 0;
}
