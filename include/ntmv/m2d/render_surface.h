#ifndef FSAE_NTMV_M2D_RENDER_SURFACE_H
#define FSAE_NTMV_M2D_RENDER_SURFACE_H

#include "ntmv/m2d/text_box.h"

#include <stddef.h>
#include <stdint.h>

typedef struct NtmvM2dRenderSurface {
    uint8_t reserved_00[8];
    NtmvM2dRect clip;      /* +0x08 */
    uint16_t *pixels;      /* +0x10 on ARM9 */
    int16_t row_stride;    /* +0x14 on ARM9, in 16-bit pixels */
    int16_t row_wrap;      /* +0x16 on ARM9, power-of-two wrap height */
} NtmvM2dRenderSurface;

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dRenderSurfacePixelsOffsetCheck[
    offsetof(NtmvM2dRenderSurface, pixels) == 0x10 ? 1 : -1];
typedef char NtmvM2dRenderSurfaceStrideOffsetCheck[
    offsetof(NtmvM2dRenderSurface, row_stride) == 0x14 ? 1 : -1];
typedef char NtmvM2dRenderSurfaceWrapOffsetCheck[
    offsetof(NtmvM2dRenderSurface, row_wrap) == 0x16 ? 1 : -1];
#endif

uint16_t *NtmvM2d_RenderSurfaceRowAddress_020b7508(
    const NtmvM2dRenderSurface *surface,
    uint32_t y);

void NtmvM2d_FillRect_020b73b8(
    void *context,
    const NtmvM2dPoint *destination,
    const NtmvM2dSize *size,
    uint16_t value);

#endif
