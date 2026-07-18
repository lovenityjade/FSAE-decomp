#ifndef FSAE_NTMV_M2D_TEXEL_PICKER_H
#define FSAE_NTMV_M2D_TEXEL_PICKER_H

#include "ntmv/m2d/pane.h"

#include <stdint.h>

typedef struct NtmvM2dTexelPicker NtmvM2dTexelPicker;

typedef struct NtmvM2dTexelPickerVTable {
    uint16_t (*pick)(const NtmvM2dTexelPicker *picker, const NtmvM2dPoint *position);
} NtmvM2dTexelPickerVTable;

typedef struct NtmvM2dTextureRef {
    const uint8_t *data;
} NtmvM2dTextureRef;

/* Header prefix consumed by the two recovered picker constructors. */
typedef struct NtmvM2dTextureHeader {
    uint16_t width;
    uint16_t height;
    uint8_t format;
    uint8_t index_zero_transparent;
    uint16_t palette_entries;
    uint8_t reserved_08[8];
    uint8_t payload[];
} NtmvM2dTextureHeader;

/*
 * RTTI/vtable address points:
 *   detail::TexelPicker           0x02126c18
 *   detail::Palette256TexelPicker 0x02126c30
 *   detail::DirectTexelPicker     0x02126c3c
 */
struct NtmvM2dTexelPicker {
    const NtmvM2dTexelPickerVTable *vtable; /* +0x00 */
    const uint8_t *texels;                  /* +0x04 */
    const uint16_t *palette;                /* +0x08 */
    int16_t width;                          /* +0x0c */
    int16_t height;                         /* +0x0e */
    int32_t row_stride;                     /* +0x10 */
    uint8_t index_zero_transparent;         /* +0x14 */
    uint8_t reserved_15[3];
};

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dTexelPickerSizeCheck[sizeof(NtmvM2dTexelPicker) == 0x18 ? 1 : -1];
#endif

extern const NtmvM2dTexelPickerVTable gNtmvM2dTexelPickerVTable;
extern const NtmvM2dTexelPickerVTable gNtmvM2dPalette256TexelPickerVTable;
extern const NtmvM2dTexelPickerVTable gNtmvM2dDirectTexelPickerVTable;

NtmvM2dTexelPicker *NtmvM2dTexelPicker_Create( /* 0x020b7528 */
    NtmvM2dTexelPicker *picker, const NtmvM2dTextureRef *texture);
void NtmvM2dDirectTexelPicker_Init( /* 0x020b7578 */
    NtmvM2dTexelPicker *picker, const NtmvM2dTextureRef *texture);
void NtmvM2dPalette256TexelPicker_Init( /* 0x020b75f0 */
    NtmvM2dTexelPicker *picker, const NtmvM2dTextureRef *texture);
uint16_t NtmvM2dTexelPicker_Pick( /* 0x020b7570 */
    const NtmvM2dTexelPicker *picker, const NtmvM2dPoint *position);
uint16_t NtmvM2dDirectTexelPicker_Pick( /* 0x020b75d0 */
    const NtmvM2dTexelPicker *picker, const NtmvM2dPoint *position);
uint16_t NtmvM2dPalette256TexelPicker_Pick( /* 0x020b766c */
    const NtmvM2dTexelPicker *picker, const NtmvM2dPoint *position);

#endif
