#include "ntmv/m2d/texel_picker.h"

#include <stddef.h>

const NtmvM2dTexelPickerVTable gNtmvM2dTexelPickerVTable = {
    NtmvM2dTexelPicker_Pick,
};

const NtmvM2dTexelPickerVTable gNtmvM2dPalette256TexelPickerVTable = {
    NtmvM2dPalette256TexelPicker_Pick,
};

const NtmvM2dTexelPickerVTable gNtmvM2dDirectTexelPickerVTable = {
    NtmvM2dDirectTexelPicker_Pick,
};

static void ResetPicker(NtmvM2dTexelPicker *picker)
{
    picker->texels = NULL;
    picker->palette = NULL;
    picker->width = 0;
    picker->height = 0;
    picker->row_stride = 0;
    picker->index_zero_transparent = 0;
}

/* 0x020b7570, abstract/base fallback vtable slot. */
uint16_t NtmvM2dTexelPicker_Pick(
    const NtmvM2dTexelPicker *picker, const NtmvM2dPoint *position)
{
    (void)picker;
    (void)position;
    return 0;
}

/* 0x020b7578 */
void NtmvM2dDirectTexelPicker_Init(
    NtmvM2dTexelPicker *picker, const NtmvM2dTextureRef *texture)
{
    const NtmvM2dTextureHeader *header =
        (const NtmvM2dTextureHeader *)texture->data;

    ResetPicker(picker);
    picker->vtable = &gNtmvM2dDirectTexelPickerVTable;
    picker->width = (int16_t)header->width;
    picker->height = (int16_t)header->height;
    picker->texels = header->payload;
    picker->row_stride = picker->width * 2;
}

/* 0x020b75d0, DirectTexelPicker vtable slot 0. */
uint16_t NtmvM2dDirectTexelPicker_Pick(
    const NtmvM2dTexelPicker *picker, const NtmvM2dPoint *position)
{
    const uint8_t *row = picker->texels + position->y * picker->row_stride;
    const uint16_t *pixels = (const uint16_t *)row;
    return pixels[position->x];
}

/* 0x020b75f0 */
void NtmvM2dPalette256TexelPicker_Init(
    NtmvM2dTexelPicker *picker, const NtmvM2dTextureRef *texture)
{
    const NtmvM2dTextureHeader *header =
        (const NtmvM2dTextureHeader *)texture->data;

    ResetPicker(picker);
    picker->vtable = &gNtmvM2dPalette256TexelPickerVTable;
    picker->width = (int16_t)header->width;
    picker->height = (int16_t)header->height;
    picker->palette = (const uint16_t *)header->payload;
    picker->texels = header->payload + header->palette_entries * sizeof(uint16_t);
    picker->row_stride = picker->width;
    picker->index_zero_transparent = header->index_zero_transparent != 0;
}

/* 0x020b766c, Palette256TexelPicker vtable slot 0. */
uint16_t NtmvM2dPalette256TexelPicker_Pick(
    const NtmvM2dTexelPicker *picker, const NtmvM2dPoint *position)
{
    uint8_t index = picker->texels[position->y * picker->row_stride + position->x];
    uint16_t alpha = 0x8000;

    if (index == 0 && picker->index_zero_transparent != 0) {
        alpha = 0;
    }
    return (uint16_t)(alpha | picker->palette[index]);
}

/* 0x020b7528 */
NtmvM2dTexelPicker *NtmvM2dTexelPicker_Create(
    NtmvM2dTexelPicker *picker, const NtmvM2dTextureRef *texture)
{
    const NtmvM2dTextureHeader *header =
        (const NtmvM2dTextureHeader *)texture->data;

    if (header->format == 4) {
        NtmvM2dPalette256TexelPicker_Init(picker, texture);
    } else if (header->format == 7) {
        NtmvM2dDirectTexelPicker_Init(picker, texture);
    }
    return picker;
}
