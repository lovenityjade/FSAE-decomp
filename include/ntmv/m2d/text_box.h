#ifndef FSAE_NTMV_M2D_TEXT_BOX_H
#define FSAE_NTMV_M2D_TEXT_BOX_H

#include "ntmv/m2d/font.h"
#include "ntmv/m2d/pane.h"

#include <stddef.h>
#include <stdint.h>

typedef struct NtmvM2dTextBox NtmvM2dTextBox;
typedef struct NtmvM2dGlyphRenderer NtmvM2dGlyphRenderer;
typedef struct NtmvM2dGlyphRendererOps NtmvM2dGlyphRendererOps;

typedef struct NtmvM2dRect {
    int16_t left;
    int16_t top;
    int16_t right;
    int16_t bottom;
} NtmvM2dRect;

/*
 * The object selected by 0x020b70c0 has a resource pointer followed by its
 * character decoder.  "font" is strongly suggested by the NNS_G2dFont calls
 * downstream, but the original C++ type name is not present in the ROM.
 */
typedef struct NtmvM2dInlineObject {
    int16_t width;
    int16_t height;
} NtmvM2dInlineObject;

typedef struct NtmvM2dTextFont {
    const NtmvM2dFontResource *resource;
    int32_t (*decode_next)(const uint8_t **cursor);
    uint8_t reserved_08[0x10];
    const NtmvM2dInlineObject *const *inline_objects; /* target +0x18 */
} NtmvM2dTextFont;

typedef struct NtmvM2dGlyphWidths {
    int8_t left;
    uint8_t glyph_width;
    int8_t advance;
} NtmvM2dGlyphWidths;

typedef struct NtmvM2dGlyphData {
    const NtmvM2dGlyphWidths *widths;
    const uint8_t *bitmap;
} NtmvM2dGlyphData;

/* Target drawing context fields consumed by 0x020b6d74/0x020b6f4c. */
typedef struct NtmvM2dTextDrawContext {
    uint8_t unknown_00[8];
    NtmvM2dRect clip;                 /* +0x08 */
    void *render_target;              /* +0x10 */
    int16_t render_row_stride;        /* +0x14, in 16-bit pixels */
    int16_t render_row_wrap;          /* +0x16, power-of-two wrap height */
    const NtmvM2dTextFont *fonts[3];  /* +0x18 */
} NtmvM2dTextDrawContext;

struct NtmvM2dGlyphRendererOps {
    void (*draw)(
        NtmvM2dGlyphRenderer *renderer,
        const NtmvM2dTextFont *font,
        int32_t x,
        int32_t y,
        uint32_t draw_value,
        const NtmvM2dGlyphData *glyph);
    void (*begin)(NtmvM2dGlyphRenderer *renderer);
    void (*end)(NtmvM2dGlyphRenderer *renderer);
};

/* Target layout written by 0x020b6870: 0x18 bytes. */
struct NtmvM2dGlyphRenderer {
    void *render_target;                    /* +0x00 */
    int32_t row_stride;                      /* +0x04, in 16-bit pixels */
    int32_t row_wrap;                        /* +0x08, power-of-two wrap height */
    uint8_t color_depth;                     /* +0x0c */
    uint8_t reserved_0d[3];
    const NtmvM2dRect *clip;                 /* +0x10 */
    const NtmvM2dGlyphRendererOps *operations; /* +0x14 */
};

/* One eight-byte line record in the resource's tail. */
typedef struct NtmvM2dTextBoxLine {
    int16_t width;
    int16_t height;
    int16_t baseline_offset;
    uint16_t text_length;
} NtmvM2dTextBoxLine;

typedef struct NtmvM2dTextBoxResource {
    NtmvM2dPaneResource pane;     /* +0x00 */
    uint16_t text_index;          /* +0x10 */
    uint16_t line_count;          /* +0x12 */
    int16_t unknown_metric_14;    /* +0x14: copied but unused by recovered slots */
    uint16_t font_size;           /* +0x16: provisional; selects one of three fonts */
    int16_t character_spacing;    /* +0x18 */
    int16_t line_spacing;         /* +0x1a */
    uint16_t draw_value;          /* +0x1c: passed to the glyph renderer */
    uint8_t alignment;            /* +0x1e: low nibble horizontal, high vertical */
    uint8_t line_alignment;       /* +0x1f: 0 inherits; 1/2/3 left/center/right */
    int16_t padding_left;         /* +0x20 */
    int16_t padding_right;        /* +0x22 */
    int16_t padding_top;          /* +0x24 */
    int16_t padding_bottom;       /* +0x26 */
    NtmvM2dTextBoxLine lines[];   /* +0x28 */
} NtmvM2dTextBoxResource;

/*
 * text_offsets are relative to their own table at archive offset +0x0c, not
 * to the beginning of this header.  Only that constructor-visible prefix is
 * named here.
 */
typedef struct NtmvM2dTextArchive {
    uint8_t unknown_00[0x0c];
    uint32_t text_offsets[];
} NtmvM2dTextArchive;

typedef struct NtmvM2dTextBoxBuildContext {
    uint8_t unknown_00[8];
    const NtmvM2dTextArchive *text_archive; /* +0x08 on ARM9 */
} NtmvM2dTextBoxBuildContext;

/*
 * Runtime cursor assembled on the stack by 0x020b6d74/0x020b6f4c and consumed
 * by 0x020b6b3c.  Names remain semantic because no debug symbols survive.
 */
typedef struct NtmvM2dTextBoxTextSession {
    NtmvM2dGlyphRenderer *renderer;    /* +0x00 */
    const NtmvM2dTextFont *font;       /* +0x04 */
    int32_t character_spacing;         /* +0x08 */
    int32_t line_spacing;              /* +0x0c */
    const uint8_t *cursor;             /* +0x10 */
    int32_t x;                         /* +0x14 */
    int32_t y;                         /* +0x18 */
    uint32_t draw_value;               /* +0x1c */
    uint8_t reserved_20[4];
    uint8_t has_previous_glyph;        /* +0x24 */
    uint8_t reserved_25[3];
} NtmvM2dTextBoxTextSession;

/* Target ARM9 layout: 0x48 bytes. */
struct NtmvM2dTextBox {
    NtmvM2dPane pane;                  /* +0x00 */
    const uint8_t *text;               /* +0x20 */
    int16_t measured_width;            /* +0x24 */
    int16_t measured_height;           /* +0x26 */
    uint16_t draw_value;               /* +0x28 */
    int16_t unknown_metric_2a;         /* +0x2a */
    uint16_t font_size;                /* +0x2c, provisional name */
    int16_t line_spacing;              /* +0x2e */
    int16_t character_spacing;         /* +0x30 */
    uint8_t reserved_32[2];
    const NtmvM2dTextBoxLine *lines;   /* +0x34 */
    uint16_t line_count;               /* +0x38 */
    uint8_t alignment;                 /* +0x3a */
    uint8_t reserved_3b;
    int16_t padding_left;              /* +0x3c */
    int16_t padding_right;             /* +0x3e */
    int16_t padding_top;               /* +0x40 */
    int16_t padding_bottom;            /* +0x42 */
    uint8_t line_alignment;            /* +0x44 */
    uint8_t reserved_45[3];
};

typedef char NtmvM2dTextBoxLineSizeCheck[
    sizeof(NtmvM2dTextBoxLine) == 0x08 ? 1 : -1];
typedef char NtmvM2dTextBoxResourceLinesOffsetCheck[
    offsetof(NtmvM2dTextBoxResource, lines) == 0x28 ? 1 : -1];
typedef char NtmvM2dTextArchiveTableOffsetCheck[
    offsetof(NtmvM2dTextArchive, text_offsets) == 0x0c ? 1 : -1];
#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dTextFontTargetSizeCheck[
    sizeof(NtmvM2dTextFont) == 0x1c ? 1 : -1];
typedef char NtmvM2dTextFontInlineObjectsOffsetCheck[
    offsetof(NtmvM2dTextFont, inline_objects) == 0x18 ? 1 : -1];
typedef char NtmvM2dGlyphDataTargetSizeCheck[
    sizeof(NtmvM2dGlyphData) == 0x08 ? 1 : -1];
typedef char NtmvM2dGlyphRendererTargetSizeCheck[
    sizeof(NtmvM2dGlyphRenderer) == 0x18 ? 1 : -1];
typedef char NtmvM2dTextDrawTargetOffsetCheck[
    offsetof(NtmvM2dTextDrawContext, fonts) == 0x18 ? 1 : -1];
typedef char NtmvM2dTextBoxBuildArchiveOffsetCheck[
    offsetof(NtmvM2dTextBoxBuildContext, text_archive) == 0x08 ? 1 : -1];
typedef char NtmvM2dTextBoxSessionTargetSizeCheck[
    sizeof(NtmvM2dTextBoxTextSession) == 0x28 ? 1 : -1];
typedef char NtmvM2dTextBoxTargetSizeCheck[
    sizeof(NtmvM2dTextBox) == 0x48 ? 1 : -1];
typedef char NtmvM2dTextBoxTextOffsetCheck[
    offsetof(NtmvM2dTextBox, text) == 0x20 ? 1 : -1];
typedef char NtmvM2dTextBoxLinesOffsetCheck[
    offsetof(NtmvM2dTextBox, lines) == 0x34 ? 1 : -1];
typedef char NtmvM2dTextBoxLineCountOffsetCheck[
    offsetof(NtmvM2dTextBox, line_count) == 0x38 ? 1 : -1];
#endif

/* Header 0x02126b40; typeinfo word 0x02126b44; address point 0x02126b48. */
extern const NtmvM2dPaneVTable gNtmvM2dTextBoxVTable;

/* Lower-level text renderer cluster. */
NtmvM2dGlyphRenderer *NtmvM2dGlyphRenderer_Init_020b6870(
    NtmvM2dGlyphRenderer *renderer,
    void *render_target,
    int32_t row_stride,
    int32_t row_wrap,
    uint32_t color_depth,
    const NtmvM2dGlyphRendererOps *operations,
    const NtmvM2dRect *clip);
void NtmvM2dGlyphRenderer_DrawBitmap_020b64fc(
    NtmvM2dGlyphRenderer *renderer,
    const NtmvM2dTextFont *font,
    int32_t x,
    int32_t y,
    uint32_t draw_value,
    const NtmvM2dGlyphData *glyph);
void NtmvM2d_InitGlyphRenderer_020b6898(
    NtmvM2dGlyphRenderer *renderer,
    void *render_target,
    int32_t row_stride,
    int32_t row_wrap,
    uint32_t color_depth,
    const NtmvM2dRect *clip);
int32_t NtmvM2d_DrawGlyph_020b68c8(
    NtmvM2dGlyphRenderer *renderer,
    const NtmvM2dTextFont *font,
    int32_t x,
    int32_t y,
    uint32_t draw_value,
    uint16_t character);
void NtmvM2d_HandleTextControl_020b69f4(
    NtmvM2dTextDrawContext *context,
    int32_t control_code,
    NtmvM2dTextBoxTextSession *session);
void NtmvM2d_DrawInlineObject_020b771c(
    NtmvM2dTextDrawContext *context,
    const NtmvM2dPoint *destination,
    const NtmvM2dInlineObject *const **inline_object_ref);
void NtmvM2dTextBox_DrawTextUntil_020b6b3c(
    NtmvM2dTextBoxTextSession *session,
    const uint8_t *end,
    NtmvM2dTextDrawContext *context);

/* 0x020b6bf8 */
NtmvM2dTextBox *NtmvM2dTextBox_Construct(
    NtmvM2dTextBox *text_box,
    const NtmvM2dTextBoxResource *resource,
    const NtmvM2dTextBoxBuildContext *build_context);

/* Four slots at the 0x02126b48 address point. */
const void *NtmvM2dTextBox_GetRuntimeType(const NtmvM2dPane *pane); /* slot 0, 0x020b7074 */
/* Slot 1 is inherited NtmvM2dPane_Dispose at 0x020b5654. */
void NtmvM2dTextBox_UpdatePosition(                              /* slot 2, 0x020b6cc4 */
    NtmvM2dPane *pane, void *context, const NtmvM2dPoint *parent_position);
void NtmvM2dTextBox_Draw(NtmvM2dPane *pane, void *context);      /* slot 3, 0x020b6d74 */
void NtmvM2dTextBox_LineIntersectsClip(                          /* 0x020b7488 */
    NtmvM2dRect *output,
    const NtmvM2dPoint *destination,
    const NtmvM2dSize *size,
    const NtmvM2dRect *clip);

#endif
