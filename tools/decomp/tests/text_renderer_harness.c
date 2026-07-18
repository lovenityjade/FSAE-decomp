#include "ntmv/m2d/text_box.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef union TestGlyphStorage {
    max_align_t alignment;
    uint8_t bytes[0x28];
} TestGlyphStorage;

static NtmvM2dGlyphWidths sWidths;
static uint16_t sLookupResult;
static uint16_t sExpectedGlyphIndex;
static uint16_t sExpectedCharacter;
static unsigned int sFindCount;
static unsigned int sWidthsCount;
static unsigned int sDrawCount;
static NtmvM2dGlyphRenderer *sDrawRenderer;
static const NtmvM2dTextFont *sDrawFont;
static int32_t sDrawX;
static int32_t sDrawY;
static uint32_t sDrawValue;
static const NtmvM2dGlyphWidths *sDrawWidths;
static const uint8_t *sDrawBitmap;
static unsigned int sInlineDrawCount;
static NtmvM2dPoint sInlinePosition;
static const NtmvM2dInlineObject *sInlineObject;

uint16_t NNS_G2dFontFindGlyphIndex(
    const NtmvM2dTextFont *font,
    uint16_t character)
{
    assert(font != NULL);
    assert(character == sExpectedCharacter);
    ++sFindCount;
    return sLookupResult;
}

const NtmvM2dGlyphWidths *NNS_G2dFontGetCharWidthsFromIndex(
    const NtmvM2dTextFont *font,
    uint16_t glyph_index)
{
    assert(font != NULL);
    assert(glyph_index == sExpectedGlyphIndex);
    ++sWidthsCount;
    return &sWidths;
}

static void CaptureGlyphRendererDraw(
    NtmvM2dGlyphRenderer *renderer,
    const NtmvM2dTextFont *font,
    int32_t x,
    int32_t y,
    uint32_t draw_value,
    const NtmvM2dGlyphData *glyph)
{
    ++sDrawCount;
    sDrawRenderer = renderer;
    sDrawFont = font;
    sDrawX = x;
    sDrawY = y;
    sDrawValue = draw_value;
    sDrawWidths = glyph->widths;
    sDrawBitmap = glyph->bitmap;
}

void NtmvM2dGlyphRenderer_BeginNoOp_020b6868(
    NtmvM2dGlyphRenderer *renderer)
{
    (void)renderer;
}

void NtmvM2dGlyphRenderer_EndNoOp_020b686c(
    NtmvM2dGlyphRenderer *renderer)
{
    (void)renderer;
}

void NtmvM2dTextBox_LineIntersectsClip(
    NtmvM2dRect *output,
    const NtmvM2dPoint *destination,
    const NtmvM2dSize *size,
    const NtmvM2dRect *clip)
{
    int32_t clipped_left = clip->left - destination->x;
    int32_t clipped_top = clip->top - destination->y;
    int32_t clipped_right = clip->right - destination->x;
    int32_t clipped_bottom = clip->bottom - destination->y;

    output->left = 0;
    output->top = 0;
    output->right = 0;
    output->bottom = 0;

    if (clipped_left < 0) {
        clipped_left = 0;
    }
    if (size->width < clipped_right) {
        clipped_right = size->width;
    }
    if (clipped_left < clipped_right) {
        if (clipped_top < 0) {
            clipped_top = 0;
        }
        if (size->height < clipped_bottom) {
            clipped_bottom = size->height;
        }
        if (clipped_top < clipped_bottom) {
            output->left = (int16_t)clipped_left;
            output->top = (int16_t)clipped_top;
            output->right = (int16_t)clipped_right;
            output->bottom = (int16_t)clipped_bottom;
        }
    }
}

void NtmvM2d_DrawInlineObject_020b771c(
    NtmvM2dTextDrawContext *context,
    const NtmvM2dPoint *position,
    const NtmvM2dInlineObject *const **inline_object_ref)
{
    assert(context != NULL);
    ++sInlineDrawCount;
    sInlinePosition = *position;
    sInlineObject = **inline_object_ref;
}

static void ResetGlyphCalls(void)
{
    sFindCount = 0;
    sWidthsCount = 0;
    sDrawCount = 0;
    sDrawRenderer = NULL;
    sDrawFont = NULL;
    sDrawX = 0;
    sDrawY = 0;
    sDrawValue = 0;
    sDrawWidths = NULL;
    sDrawBitmap = NULL;
}

static void TestRendererInitialization(void)
{
    NtmvM2dGlyphRenderer renderer;
    NtmvM2dRect clip = {1, 2, 30, 40};
    NtmvM2dGlyphRendererOps custom_operations = {
        CaptureGlyphRendererDraw,
        NtmvM2dGlyphRenderer_BeginNoOp_020b6868,
        NtmvM2dGlyphRenderer_EndNoOp_020b686c,
    };
    uint32_t target;

    memset(&renderer, 0xa5, sizeof(renderer));
    assert(NtmvM2dGlyphRenderer_Init_020b6870(
        &renderer, &target, -7, 12, 0x123, &custom_operations, &clip) ==
        &renderer);
    assert(renderer.render_target == &target);
    assert(renderer.row_stride == -7);
    assert(renderer.row_wrap == 12);
    assert(renderer.color_depth == 0x23);
    assert(renderer.operations == &custom_operations);
    assert(renderer.clip == &clip);

    NtmvM2d_InitGlyphRenderer_020b6898(
        &renderer, &target, 4, -9, 0x10, &clip);
    assert(renderer.render_target == &target);
    assert(renderer.row_stride == 4);
    assert(renderer.row_wrap == -9);
    assert(renderer.color_depth == 0x10);
    assert(renderer.clip == &clip);
    assert(renderer.operations->draw ==
        NtmvM2dGlyphRenderer_DrawBitmap_020b64fc);
    assert(renderer.operations->begin ==
        NtmvM2dGlyphRenderer_BeginNoOp_020b6868);
    assert(renderer.operations->end ==
        NtmvM2dGlyphRenderer_EndNoOp_020b686c);
}

static void TestEveryGlyphLayout(void)
{
    static const int32_t expected_x[8] = {
        98, 95, 95, 95, 95, 100, 100, 98
    };
    static const int32_t expected_y[8] = {
        200, 198, 198, 191, 191, 193, 193, 200
    };
    TestGlyphStorage storage;
    NtmvM2dFontGlyphBlock *glyphs =
        (NtmvM2dFontGlyphBlock *)(void *)storage.bytes;
    NtmvM2dFontResource resource;
    NtmvM2dTextFont font;
    NtmvM2dGlyphRenderer renderer;
    NtmvM2dRect clip = {0, 0, 256, 192};
    NtmvM2dGlyphRendererOps capture_operations = {
        CaptureGlyphRendererDraw,
        NtmvM2dGlyphRenderer_BeginNoOp_020b6868,
        NtmvM2dGlyphRenderer_EndNoOp_020b686c,
    };
    uint32_t target;
    uint32_t layout;

    memset(&storage, 0, sizeof(storage));
    memset(&font, 0, sizeof(font));
    glyphs->cell_width = 5;
    glyphs->cell_height = 9;
    glyphs->glyph_stride = 4;
    memset(&resource, 0, sizeof(resource));
    resource.alternate_glyph_index = 2;
    resource.glyphs = glyphs;
    font.resource = &resource;
    sWidths.left = -2;
    sWidths.glyph_width = 7;
    sWidths.advance = -3;
    sExpectedCharacter = 0x41;
    sLookupResult = 1;
    sExpectedGlyphIndex = 1;
    NtmvM2dGlyphRenderer_Init_020b6870(
        &renderer, &target, 0, 0, 0x10, &capture_operations, &clip);

    for (layout = 0; layout < 8; ++layout) {
        int32_t advance;

        glyphs->layout = (uint8_t)layout;
        ResetGlyphCalls();
        advance = NtmvM2d_DrawGlyph_020b68c8(
            &renderer, &font, 100, 200, 0x1234, 0x41);
        assert(advance == -3);
        assert(sFindCount == 1 && sWidthsCount == 1 && sDrawCount == 1);
        assert(sDrawRenderer == &renderer && sDrawFont == &font);
        assert(sDrawX == expected_x[layout]);
        assert(sDrawY == expected_y[layout]);
        assert(sDrawValue == 0x1234);
        assert(sDrawWidths == &sWidths);
        assert(sDrawBitmap == glyphs->glyph_data + 4);
    }

    sLookupResult = 0xffff;
    sExpectedGlyphIndex = 2;
    glyphs->layout = 0;
    ResetGlyphCalls();
    assert(NtmvM2d_DrawGlyph_020b68c8(
        &renderer, &font, 100, 200, 0x55, 0x41) == -3);
    assert(sDrawBitmap == glyphs->glyph_data + 8);
}

static void SetControlRecord(
    uint16_t record[3],
    uint16_t byte_length,
    uint16_t value)
{
    record[0] = byte_length;
    record[1] = value;
    record[2] = 0;
}

static void TestTextControls(void)
{
    TestGlyphStorage storage;
    NtmvM2dFontGlyphBlock *glyphs =
        (NtmvM2dFontGlyphBlock *)(void *)storage.bytes;
    NtmvM2dFontResource resource;
    NtmvM2dTextFont session_font;
    NtmvM2dTextFont normal_font;
    NtmvM2dTextDrawContext context;
    NtmvM2dTextBoxTextSession session;
    NtmvM2dInlineObject inline_object = {6, 4};
    const NtmvM2dInlineObject *inline_objects[1] = {&inline_object};
    uint16_t record[3];
    const uint8_t *start;

    memset(&storage, 0, sizeof(storage));
    memset(&resource, 0, sizeof(resource));
    memset(&session_font, 0, sizeof(session_font));
    memset(&normal_font, 0, sizeof(normal_font));
    memset(&context, 0, sizeof(context));
    glyphs->cell_height = 9;
    resource.glyphs = glyphs;
    session_font.resource = &resource;
    normal_font.inline_objects = inline_objects;
    context.fonts[1] = &normal_font;

    memset(&session, 0, sizeof(session));
    SetControlRecord(record, 4, 1);
    start = (const uint8_t *)record;
    session.font = &session_font;
    session.cursor = start;
    session.x = 10;
    session.y = 20;
    session.character_spacing = 2;
    session.has_previous_glyph = 1;
    sInlineDrawCount = 0;
    NtmvM2d_HandleTextControl_020b69f4(&context, 1, &session);
    assert(session.cursor == start + 4);
    assert(session.x == 18);
    assert(session.has_previous_glyph == 1);
    assert(sInlineDrawCount == 1);
    assert(sInlinePosition.x == 12 && sInlinePosition.y == 25);
    assert(sInlineObject == &inline_object);

    SetControlRecord(record, 6, 1);
    start = (const uint8_t *)record;
    session.cursor = start;
    session.x = 7;
    session.has_previous_glyph = 1;
    sInlineDrawCount = 0;
    NtmvM2d_HandleTextControl_020b69f4(&context, 1, &session);
    assert(session.cursor == start + 6);
    assert(session.x == 7 && session.has_previous_glyph == 1);
    assert(sInlineDrawCount == 0);

    SetControlRecord(record, 4, 0xabcd);
    start = (const uint8_t *)record;
    session.cursor = start;
    session.draw_value = 0;
    NtmvM2d_HandleTextControl_020b69f4(&context, 2, &session);
    assert(session.cursor == start + 4);
    assert(session.draw_value == 0xabcd);

    SetControlRecord(record, 6, 0x1111);
    start = (const uint8_t *)record;
    session.cursor = start;
    session.draw_value = 0x2222;
    NtmvM2d_HandleTextControl_020b69f4(&context, 2, &session);
    assert(session.cursor == start + 6);
    assert(session.draw_value == 0x2222);

    SetControlRecord(record, 4, 5);
    start = (const uint8_t *)record;
    session.cursor = start;
    session.x = 30;
    session.has_previous_glyph = 1;
    NtmvM2d_HandleTextControl_020b69f4(&context, 3, &session);
    assert(session.cursor == start + 4);
    assert(session.x == 35);
    assert(session.has_previous_glyph == 0);

    session.cursor = start;
    session.x = 99;
    NtmvM2d_HandleTextControl_020b69f4(&context, 9, &session);
    assert(session.cursor == start && session.x == 99);
}

static uint16_t PackBgr555(uint32_t red, uint32_t green, uint32_t blue)
{
    return (uint16_t)(red | (green << 5) | (blue << 10) | 0x8000u);
}

static uint8_t PixelChannel(uint16_t color, uint32_t shift)
{
    return (uint8_t)((color >> shift) & 0x1fu);
}

static uint8_t ExpectedBlendChannel(
    uint32_t target,
    uint32_t source,
    uint32_t coverage)
{
    uint32_t blended = target * (3u - coverage) / 3u +
        source * coverage / 3u;

    if (blended > 0x1fu) {
        blended = 0x1fu;
    }
    return (uint8_t)blended;
}

static uint16_t ExpectedBlend(
    uint16_t target,
    uint16_t source,
    uint32_t coverage)
{
    if (coverage == 0) {
        return target;
    }
    if (coverage == 3) {
        return (uint16_t)(source | 0x8000u);
    }
    return PackBgr555(
        ExpectedBlendChannel(
            PixelChannel(target, 0),
            PixelChannel(source, 0),
            coverage),
        ExpectedBlendChannel(
            PixelChannel(target, 5),
            PixelChannel(source, 5),
            coverage),
        ExpectedBlendChannel(
            PixelChannel(target, 10),
            PixelChannel(source, 10),
            coverage));
}

static uint8_t PackCoverages(
    uint32_t first,
    uint32_t second,
    uint32_t third,
    uint32_t fourth)
{
    return (uint8_t)((first << 6) | (second << 4) | (third << 2) | fourth);
}

static void PrepareGlyphBitmapFixture(
    TestGlyphStorage *storage,
    NtmvM2dFontResource *resource,
    NtmvM2dTextFont *font,
    NtmvM2dGlyphWidths *widths,
    NtmvM2dGlyphData *glyph,
    uint8_t coverage_byte)
{
    NtmvM2dFontGlyphBlock *glyphs =
        (NtmvM2dFontGlyphBlock *)(void *)storage->bytes;

    memset(storage, 0, sizeof(*storage));
    memset(resource, 0, sizeof(*resource));
    memset(font, 0, sizeof(*font));
    glyphs->cell_width = 4;
    glyphs->cell_height = 1;
    glyphs->glyph_stride = 1;
    glyphs->coverage_shift = 2;
    glyphs->glyph_data[0] = coverage_byte;
    resource->glyphs = glyphs;
    font->resource = resource;
    widths->left = 0;
    widths->glyph_width = 4;
    widths->advance = 4;
    glyph->widths = widths;
    glyph->bitmap = glyphs->glyph_data;
}

static void TestGlyphBitmapCoverageBlending(void)
{
    TestGlyphStorage storage;
    NtmvM2dFontResource resource;
    NtmvM2dTextFont font;
    NtmvM2dGlyphWidths widths;
    NtmvM2dGlyphData glyph;
    NtmvM2dGlyphRenderer renderer;
    NtmvM2dRect clip = {0, 0, 8, 8};
    uint16_t pixels[64];
    uint16_t source = PackBgr555(30, 15, 6);
    uint16_t target0 = PackBgr555(4, 7, 10);
    uint16_t target1 = PackBgr555(12, 18, 24);
    uint16_t target2 = PackBgr555(2, 3, 4);

    memset(pixels, 0, sizeof(pixels));
    PrepareGlyphBitmapFixture(
        &storage,
        &resource,
        &font,
        &widths,
        &glyph,
        PackCoverages(3, 0, 1, 2));
    pixels[2 * 8 + 1] = target0;
    pixels[2 * 8 + 2] = target1;
    pixels[2 * 8 + 3] = target2;

    NtmvM2dGlyphRenderer_Init_020b6870(
        &renderer,
        pixels,
        8,
        8,
        0x10,
        &(NtmvM2dGlyphRendererOps){
            NtmvM2dGlyphRenderer_DrawBitmap_020b64fc,
            NtmvM2dGlyphRenderer_BeginNoOp_020b6868,
            NtmvM2dGlyphRenderer_EndNoOp_020b686c,
        },
        &clip);
    NtmvM2dGlyphRenderer_DrawBitmap_020b64fc(
        &renderer, &font, 1, 2, source, &glyph);

    assert(pixels[2 * 8 + 1] == (uint16_t)(source | 0x8000u));
    assert(pixels[2 * 8 + 2] == target1);
    assert(pixels[2 * 8 + 3] == ExpectedBlend(target2, source, 1));
    assert(pixels[2 * 8 + 4] == ExpectedBlend(0, source, 2));
}

static void TestGlyphBitmapClippingAndRowWrap(void)
{
    TestGlyphStorage storage;
    NtmvM2dFontResource resource;
    NtmvM2dTextFont font;
    NtmvM2dGlyphWidths widths;
    NtmvM2dGlyphData glyph;
    NtmvM2dGlyphRenderer renderer;
    NtmvM2dRect clip = {0, 0, 8, 16};
    uint16_t pixels[64];
    uint16_t source = PackBgr555(9, 21, 30);
    uint16_t target0 = PackBgr555(30, 3, 1);
    uint16_t target1 = PackBgr555(6, 9, 12);

    memset(pixels, 0, sizeof(pixels));
    PrepareGlyphBitmapFixture(
        &storage,
        &resource,
        &font,
        &widths,
        &glyph,
        PackCoverages(3, 1, 2, 3));
    pixels[2 * 8 + 0] = target0;
    pixels[2 * 8 + 1] = target1;

    NtmvM2dGlyphRenderer_Init_020b6870(
        &renderer,
        pixels,
        8,
        8,
        0x10,
        &(NtmvM2dGlyphRendererOps){
            NtmvM2dGlyphRenderer_DrawBitmap_020b64fc,
            NtmvM2dGlyphRenderer_BeginNoOp_020b6868,
            NtmvM2dGlyphRenderer_EndNoOp_020b686c,
        },
        &clip);
    NtmvM2dGlyphRenderer_DrawBitmap_020b64fc(
        &renderer, &font, -1, 10, source, &glyph);

    assert(pixels[2 * 8 + 0] == ExpectedBlend(target0, source, 1));
    assert(pixels[2 * 8 + 1] == ExpectedBlend(target1, source, 2));
    assert(pixels[2 * 8 + 2] == (uint16_t)(source | 0x8000u));
    assert(pixels[2 * 8 + 3] == 0);

    clip.left = 20;
    clip.top = 20;
    clip.right = 30;
    clip.bottom = 30;
    NtmvM2dGlyphRenderer_DrawBitmap_020b64fc(
        &renderer, &font, 0, 0, source, &glyph);
    assert(pixels[2 * 8 + 0] == ExpectedBlend(target0, source, 1));
    assert(pixels[2 * 8 + 1] == ExpectedBlend(target1, source, 2));
    assert(pixels[2 * 8 + 2] == (uint16_t)(source | 0x8000u));
}

int main(void)
{
    TestRendererInitialization();
    TestGlyphBitmapCoverageBlending();
    TestGlyphBitmapClippingAndRowWrap();
    TestEveryGlyphLayout();
    TestTextControls();
    return 0;
}
