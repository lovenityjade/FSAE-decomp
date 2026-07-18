#include "ntmv/m2d/text_box.h"

#include <stddef.h>

enum {
    NTMV_M2D_INVALID_GLYPH_INDEX = 0xffff
};

typedef struct NtmvM2dTextControlRecord {
    uint16_t byte_length;
    uint16_t value;
} NtmvM2dTextControlRecord;

extern uint16_t NNS_G2dFontFindGlyphIndex(
    const NtmvM2dTextFont *font,
    uint16_t character);
extern const NtmvM2dGlyphWidths *NNS_G2dFontGetCharWidthsFromIndex(
    const NtmvM2dTextFont *font,
    uint16_t glyph_index);

/* The two no-op operations are the BX LR bodies at 0x020b6868/0x020b686c. */
extern void NtmvM2dGlyphRenderer_BeginNoOp_020b6868(
    NtmvM2dGlyphRenderer *renderer);
extern void NtmvM2dGlyphRenderer_EndNoOp_020b686c(
    NtmvM2dGlyphRenderer *renderer);

/* ROM table 0x020deed4. */
static const NtmvM2dGlyphRendererOps sGlyphRendererOperations = {
    NtmvM2dGlyphRenderer_DrawBitmap_020b64fc,
    NtmvM2dGlyphRenderer_BeginNoOp_020b6868,
    NtmvM2dGlyphRenderer_EndNoOp_020b686c,
};

static uint32_t GlyphCoverageAt(
    const NtmvM2dFontGlyphBlock *glyphs,
    const NtmvM2dGlyphData *glyph,
    int32_t x,
    int32_t y)
{
    uint32_t shift = glyphs->coverage_shift;
    uint32_t mask = (1u << shift) - 1u;
    uint32_t index = (uint32_t)y * glyphs->cell_width + (uint32_t)x;
    uint32_t packed = glyph->bitmap[index >> shift];
    uint32_t bit_shift = shift * (mask - (index & mask));

    return (packed >> bit_shift) & mask;
}

static uint8_t ColorChannel(uint32_t color, uint32_t shift)
{
    return (uint8_t)((color >> shift) & 0x1fu);
}

static uint8_t ScaleSourceByCoverage(uint32_t channel, uint32_t coverage)
{
    return (uint8_t)((channel * coverage) / 3u);
}

static uint8_t BlendChannel(
    uint32_t target,
    uint32_t source,
    uint32_t coverage,
    uint32_t max_coverage)
{
    uint32_t blended =
        target * (max_coverage - coverage) / max_coverage +
        ScaleSourceByCoverage(source, coverage);

    if (blended > 0x1fu) {
        blended = 0x1fu;
    }
    return (uint8_t)blended;
}

static uint16_t BlendGlyphPixel(
    uint16_t target,
    uint32_t source,
    uint32_t coverage,
    uint32_t max_coverage)
{
    uint32_t red;
    uint32_t green;
    uint32_t blue;

    if (coverage == max_coverage) {
        return (uint16_t)(source | 0x8000u);
    }
    if (coverage == 0) {
        return target;
    }

    red = BlendChannel(
        ColorChannel(target, 0),
        ColorChannel(source, 0),
        coverage,
        max_coverage);
    green = BlendChannel(
        ColorChannel(target, 5),
        ColorChannel(source, 5),
        coverage,
        max_coverage);
    blue = BlendChannel(
        ColorChannel(target, 10),
        ColorChannel(source, 10),
        coverage,
        max_coverage);

    return (uint16_t)(red | (green << 5) | (blue << 10) | 0x8000u);
}

/* 0x020b64fc..0x020b6863; 0x55555556 literal at 0x020b6864 excluded. */
void NtmvM2dGlyphRenderer_DrawBitmap_020b64fc(
    NtmvM2dGlyphRenderer *renderer,
    const NtmvM2dTextFont *font,
    int32_t x,
    int32_t y,
    uint32_t draw_value,
    const NtmvM2dGlyphData *glyph)
{
    const NtmvM2dFontGlyphBlock *glyphs = font->resource->glyphs;
    NtmvM2dPoint destination = {(int16_t)x, (int16_t)y};
    NtmvM2dSize size = {
        (int16_t)glyph->widths->glyph_width,
        (int16_t)glyphs->cell_height,
    };
    NtmvM2dRect clipped;
    uint32_t max_coverage;
    int32_t row;

    NtmvM2dTextBox_LineIntersectsClip(
        &clipped,
        &destination,
        &size,
        renderer->clip);
    if ((int16_t)(clipped.right - clipped.left) == 0) {
        return;
    }

    max_coverage = (1u << glyphs->coverage_shift) - 1u;
    row = clipped.top;
    if (row < clipped.bottom) {
        do {
            uint16_t *target_row =
                (uint16_t *)renderer->render_target +
                (int16_t)renderer->row_stride *
                    ((uint32_t)((int16_t)renderer->row_wrap - 1) &
                        (uint32_t)(int16_t)(y + row));
            int32_t column = clipped.left;

            if (column < clipped.right) {
                do {
                    uint32_t coverage = GlyphCoverageAt(
                        glyphs,
                        glyph,
                        column,
                        row);

                    if (coverage != 0) {
                        uint16_t *target =
                            &target_row[(int16_t)x + column];

                        *target = BlendGlyphPixel(
                            *target,
                            draw_value,
                            coverage,
                            max_coverage);
                    }
                    ++column;
                } while (column < clipped.right);
            }
            ++row;
        } while (row < clipped.bottom);
    }
}

/* 0x020b6870..0x020b6897 */
NtmvM2dGlyphRenderer *NtmvM2dGlyphRenderer_Init_020b6870(
    NtmvM2dGlyphRenderer *renderer,
    void *render_target,
    int32_t row_stride,
    int32_t row_wrap,
    uint32_t color_depth,
    const NtmvM2dGlyphRendererOps *operations,
    const NtmvM2dRect *clip)
{
    renderer->render_target = render_target;
    renderer->row_stride = row_stride;
    renderer->row_wrap = row_wrap;
    renderer->color_depth = (uint8_t)color_depth;
    renderer->clip = clip;
    renderer->operations = operations;
    return renderer;
}

/* 0x020b6898..0x020b68c3; literal 0x020b68c4 excluded. */
void NtmvM2d_InitGlyphRenderer_020b6898(
    NtmvM2dGlyphRenderer *renderer,
    void *render_target,
    int32_t row_stride,
    int32_t row_wrap,
    uint32_t color_depth,
    const NtmvM2dRect *clip)
{
    (void)NtmvM2dGlyphRenderer_Init_020b6870(
        renderer,
        render_target,
        row_stride,
        row_wrap,
        color_depth,
        &sGlyphRendererOperations,
        clip);
}

static void AdjustGlyphPosition(
    int32_t *x,
    int32_t *y,
    const NtmvM2dGlyphWidths *widths,
    const NtmvM2dFontGlyphBlock *glyphs)
{
    switch (glyphs->layout) {
    case 0:
    case 7:
        *x += widths->left;
        break;
    case 1:
    case 2:
        *x -= glyphs->cell_width;
        *y += widths->left;
        break;
    case 3:
    case 4:
        *x -= widths->left + widths->glyph_width;
        *y -= glyphs->cell_height;
        break;
    case 5:
    case 6:
        *y -= widths->left + glyphs->cell_height;
        break;
    default:
        break;
    }
}

/* 0x020b68c8..0x020b69df; 0xffff literal at 0x020b69e0 excluded. */
int32_t NtmvM2d_DrawGlyph_020b68c8(
    NtmvM2dGlyphRenderer *renderer,
    const NtmvM2dTextFont *font,
    int32_t x,
    int32_t y,
    uint32_t draw_value,
    uint16_t character)
{
    uint16_t glyph_index = NNS_G2dFontFindGlyphIndex(font, character);
    const NtmvM2dFontGlyphBlock *glyphs = font->resource->glyphs;
    const NtmvM2dGlyphWidths *widths;
    NtmvM2dGlyphData glyph;

    if (glyph_index == NTMV_M2D_INVALID_GLYPH_INDEX) {
        glyph_index = font->resource->alternate_glyph_index;
    }
    widths = NNS_G2dFontGetCharWidthsFromIndex(font, glyph_index);
    glyph.widths = widths;
    glyph.bitmap = glyphs->glyph_data + glyph_index * glyphs->glyph_stride;
    AdjustGlyphPosition(&x, &y, widths, glyphs);
    renderer->operations->draw(
        renderer, font, x, y, draw_value, &glyph);
    return widths->advance;
}

static const NtmvM2dTextControlRecord *ConsumeControlRecord(
    NtmvM2dTextBoxTextSession *session)
{
    const NtmvM2dTextControlRecord *record =
        (const NtmvM2dTextControlRecord *)session->cursor;

    session->cursor += record->byte_length;
    return record;
}

static void DrawInlineControl(
    NtmvM2dTextDrawContext *context,
    NtmvM2dTextBoxTextSession *session)
{
    const NtmvM2dTextControlRecord *record = ConsumeControlRecord(session);
    const NtmvM2dFontGlyphBlock *glyphs = session->font->resource->glyphs;
    const NtmvM2dInlineObject *const *inline_object_ref;
    const NtmvM2dInlineObject *inline_object;
    NtmvM2dPoint position;

    if (record->byte_length != sizeof(*record)) {
        return;
    }
    inline_object_ref =
        &context->fonts[1]->inline_objects[record->value - 1u];
    inline_object = *inline_object_ref;
    if (session->has_previous_glyph != 0) {
        session->has_previous_glyph = 0;
        session->x += session->character_spacing;
    }
    position.x = (int16_t)session->x;
    position.y = (int16_t)(
        session->y + glyphs->cell_height - inline_object->height);
    NtmvM2d_DrawInlineObject_020b771c(
        context, &position, &inline_object_ref);
    session->x += inline_object->width;
    session->has_previous_glyph = 1;
}

/* 0x020b69f4..0x020b6b3b */
void NtmvM2d_HandleTextControl_020b69f4(
    NtmvM2dTextDrawContext *context,
    int32_t control_code,
    NtmvM2dTextBoxTextSession *session)
{
    const NtmvM2dTextControlRecord *record;

    switch (control_code) {
    case 1:
        DrawInlineControl(context, session);
        break;
    case 2:
        record = ConsumeControlRecord(session);
        if (record->byte_length == sizeof(*record)) {
            session->draw_value = record->value;
        }
        break;
    case 3:
        record = ConsumeControlRecord(session);
        session->x += record->value;
        session->has_previous_glyph = 0;
        break;
    default:
        break;
    }
}
