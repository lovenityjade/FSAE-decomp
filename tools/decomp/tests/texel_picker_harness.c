#include "ntmv/m2d/texel_picker.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

int main(void)
{
    uint8_t direct_data[0x18];
    uint8_t palette_data[0x10 + 8 + 4];
    NtmvM2dTextureHeader *direct = (NtmvM2dTextureHeader *)direct_data;
    NtmvM2dTextureHeader *palette = (NtmvM2dTextureHeader *)palette_data;
    NtmvM2dTextureRef reference;
    NtmvM2dTexelPicker picker;
    NtmvM2dPoint point = {1, 0};
    uint16_t *pixels;
    uint16_t *colors;

    (void)memset(direct_data, 0, sizeof(direct_data));
    direct->width = 2;
    direct->height = 2;
    direct->format = 7;
    pixels = (uint16_t *)direct->payload;
    pixels[0] = 0x8001;
    pixels[1] = 0x9234;
    reference.data = direct_data;
    assert(NtmvM2dTexelPicker_Create(&picker, &reference) == &picker);
    assert(picker.vtable->pick(&picker, &point) == 0x9234);

    (void)memset(palette_data, 0, sizeof(palette_data));
    palette->width = 2;
    palette->height = 2;
    palette->format = 4;
    palette->index_zero_transparent = 1;
    palette->palette_entries = 4;
    colors = (uint16_t *)palette->payload;
    colors[0] = 0x001f;
    colors[1] = 0x03e0;
    palette->payload[8] = 0;
    palette->payload[9] = 1;
    reference.data = palette_data;
    point.x = 0;
    assert(NtmvM2dTexelPicker_Create(&picker, &reference) == &picker);
    assert(picker.vtable->pick(&picker, &point) == 0x001f);
    point.x = 1;
    assert(picker.vtable->pick(&picker, &point) == 0x83e0);
    return 0;
}
