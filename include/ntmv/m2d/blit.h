#ifndef FSAE_NTMV_M2D_BLIT_H
#define FSAE_NTMV_M2D_BLIT_H

#include "ntmv/m2d/texel_picker.h"

#include <stdint.h>

/* Five-byte records selected from the ROM table at 0x02126c4c. */
typedef struct NtmvM2dTransformDescriptor {
    int8_t origin_factor[2];
    int8_t step[2];
    uint8_t transpose_axes;
} NtmvM2dTransformDescriptor;

/* Pictures, window borders and texel pickers share this texture header. */
typedef NtmvM2dTextureHeader NtmvM2dBitmap;

typedef char NtmvM2dTransformDescriptorSizeCheck[
    sizeof(NtmvM2dTransformDescriptor) == 0x05 ? 1 : -1];

const NtmvM2dTransformDescriptor *NtmvM2d_GetTransformDescriptor_020b76bc(
    uint8_t transform_code);
void NtmvM2d_ComputeSourceOrigin_020b76d0(
    NtmvM2dPoint *origin,
    const NtmvM2dSize *bitmap_size,
    const NtmvM2dTransformDescriptor *transform);

void NtmvM2d_BlitTransformed_020b786c(
    void *context,
    const NtmvM2dPoint *destination,
    const NtmvM2dSize *draw_size,
    const NtmvM2dBitmap *const *const *bitmap_ref,
    const NtmvM2dPoint *source_origin,
    const NtmvM2dTransformDescriptor *transform);

#endif
