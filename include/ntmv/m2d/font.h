#ifndef FSAE_NTMV_M2D_FONT_H
#define FSAE_NTMV_M2D_FONT_H

#include <stddef.h>
#include <stdint.h>

/* Shared view of the NNS G2D font resource used by m2d text consumers. */
typedef struct NtmvM2dFontGlyphBlock {
    uint8_t cell_width;       /* +0x00 */
    uint8_t cell_height;      /* +0x01 */
    uint16_t glyph_stride;    /* +0x02 */
    uint8_t reserved_04[2];
    uint8_t coverage_shift;   /* +0x06: two-bit coverage uses value 2 */
    uint8_t layout;           /* +0x07: renderer recognizes values 0..7 */
    uint8_t glyph_data[];     /* +0x08 */
} NtmvM2dFontGlyphBlock;

/* Header-only aliasing view used by PageHeaderPanel. */
typedef struct NtmvM2dFontMetadata {
    uint8_t unknown_00[7];
    uint8_t orientation;
} NtmvM2dFontMetadata;

typedef struct NtmvM2dFontResource {
    uint16_t reserved_00;
    uint16_t alternate_glyph_index; /* +0x02: fallback for lookup 0xffff */
    uint32_t reserved_04;
    union {
        const NtmvM2dFontGlyphBlock *glyphs; /* +0x08 */
        const NtmvM2dFontMetadata *metadata;
    };
} NtmvM2dFontResource;

typedef char NtmvM2dFontGlyphDataOffsetCheck[
    offsetof(NtmvM2dFontGlyphBlock, glyph_data) == 0x08 ? 1 : -1];
typedef char NtmvM2dFontResourceGlyphsOffsetCheck[
    offsetof(NtmvM2dFontResource, glyphs) == 0x08 ? 1 : -1];

#endif
