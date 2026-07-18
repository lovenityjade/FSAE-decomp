#include "ntmv/m2d/text_box.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

typedef struct GlyphRecord {
    void *renderer;
    const NtmvM2dTextFont *font;
    int32_t x;
    int32_t y;
    uint32_t draw_value;
    uint16_t character;
} GlyphRecord;

static GlyphRecord sGlyphs[8];
static unsigned int sGlyphCount;
static unsigned int sControlCount;
static int32_t sLastControl;
static unsigned int sDecodeCount;
static unsigned int sRendererInitCount;
static void *sExpectedRenderTarget;
static NtmvM2dTextDrawContext *sExpectedContext;

void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size)
{
    (void)allocator;
    (void)size;
    assert(0);
    return NULL;
}

void NNS_FndFreeToAllocator(void *allocator, void *memory)
{
    (void)allocator;
    (void)memory;
}

void NtmvM2d_InitGlyphRenderer_020b6898(
    NtmvM2dGlyphRenderer *renderer,
    void *render_target,
    int32_t row_stride,
    int32_t row_wrap,
    uint32_t color_depth,
    const NtmvM2dRect *clip)
{
    assert(renderer != NULL);
    assert(render_target == sExpectedRenderTarget);
    assert(row_stride == -2);
    assert(row_wrap == 3);
    assert(color_depth == 0x10);
    assert(clip == &sExpectedContext->clip);
    ++sRendererInitCount;
}

int32_t NtmvM2d_DrawGlyph_020b68c8(
    NtmvM2dGlyphRenderer *renderer,
    const NtmvM2dTextFont *font,
    int32_t x,
    int32_t y,
    uint32_t draw_value,
    uint16_t character)
{
    GlyphRecord *record;

    assert(sGlyphCount < sizeof(sGlyphs) / sizeof(sGlyphs[0]));
    record = &sGlyphs[sGlyphCount++];
    record->renderer = renderer;
    record->font = font;
    record->x = x;
    record->y = y;
    record->draw_value = draw_value;
    record->character = character;
    return 2;
}

void NtmvM2d_HandleTextControl_020b69f4(
    NtmvM2dTextDrawContext *context,
    int32_t control_code,
    NtmvM2dTextBoxTextSession *session)
{
    assert(context == sExpectedContext);
    assert(session->font == context->fonts[1]);
    ++sControlCount;
    sLastControl = control_code;
}

static int32_t DecodeByte(const uint8_t **cursor)
{
    int32_t character = **cursor;

    ++sDecodeCount;
    ++*cursor;
    return character;
}

static void ResetRenderRecords(void)
{
    memset(sGlyphs, 0, sizeof(sGlyphs));
    sGlyphCount = 0;
    sControlCount = 0;
    sLastControl = 0;
    sDecodeCount = 0;
}

static void AssertGlyph(
    unsigned int index,
    const NtmvM2dTextFont *font,
    int32_t x,
    int32_t y,
    uint16_t character)
{
    const GlyphRecord *record = &sGlyphs[index];

    assert(record->renderer != NULL);
    assert(record->font == font);
    assert(record->x == x);
    assert(record->y == y);
    assert(record->draw_value == 0x1234);
    assert(record->character == character);
}

static void AssertRect(
    const NtmvM2dRect *rect,
    int16_t left,
    int16_t top,
    int16_t right,
    int16_t bottom)
{
    assert(rect->left == left);
    assert(rect->top == top);
    assert(rect->right == right);
    assert(rect->bottom == bottom);
}

static void TestDrawTextUntil(
    NtmvM2dTextDrawContext *context,
    NtmvM2dTextFont *font)
{
    static const uint8_t text[] = {'A', 'B', 1, 'C', '\n', 'D', 0};
    NtmvM2dTextBoxTextSession session;
    NtmvM2dGlyphRenderer renderer;

    memset(&session, 0, sizeof(session));
    memset(&renderer, 0, sizeof(renderer));
    session.renderer = &renderer;
    session.font = font;
    session.character_spacing = 3;
    session.cursor = text;
    session.x = 10;
    session.y = 77;
    session.draw_value = 0x1234;

    ResetRenderRecords();
    NtmvM2dTextBox_DrawTextUntil_020b6b3c(
        &session, text + 4, context);
    assert(session.cursor == text + 4);
    assert(session.x == 22);
    assert(session.has_previous_glyph == 1);
    assert(sDecodeCount == 4);
    assert(sGlyphCount == 3);
    assert(sControlCount == 1 && sLastControl == 1);
    AssertGlyph(0, font, 10, 77, 'A');
    AssertGlyph(1, font, 15, 77, 'B');
    AssertGlyph(2, font, 20, 77, 'C');
    assert(sGlyphs[0].renderer == &renderer);

    ResetRenderRecords();
    session.cursor = text + 4;
    session.x = 30;
    NtmvM2dTextBox_DrawTextUntil_020b6b3c(
        &session, text + sizeof(text), context);
    assert(session.cursor == text + 5);
    assert(session.x == 30);
    assert(sDecodeCount == 1 && sGlyphCount == 0 && sControlCount == 0);

    ResetRenderRecords();
    session.cursor = text + 6;
    NtmvM2dTextBox_DrawTextUntil_020b6b3c(
        &session, text + sizeof(text), context);
    assert(session.cursor == text + sizeof(text));
    assert(sDecodeCount == 1 && sGlyphCount == 0 && sControlCount == 0);

    ResetRenderRecords();
    session.cursor = text + 2;
    NtmvM2dTextBox_DrawTextUntil_020b6b3c(
        &session, text + 2, context);
    assert(session.cursor == text + 2);
    assert(sDecodeCount == 0 && sGlyphCount == 0 && sControlCount == 0);
}

int main(void)
{
    union ArchiveStorage {
        uint32_t alignment;
        uint8_t bytes[0x0c + 4 + 6];
    } archive_storage;
    NtmvM2dTextArchive *archive =
        (NtmvM2dTextArchive *)(void *)archive_storage.bytes;
    union ResourceStorage {
        uint32_t alignment;
        uint8_t bytes[0x28 + 2 * sizeof(NtmvM2dTextBoxLine)];
    } resource_storage;
    NtmvM2dTextBoxResource *resource =
        (NtmvM2dTextBoxResource *)(void *)resource_storage.bytes;
    NtmvM2dTextBoxBuildContext build_context;
    NtmvM2dTextBox text_box;
    NtmvM2dTextFont fonts[3];
    NtmvM2dTextDrawContext draw_context;
    NtmvM2dPoint parent_position;
    NtmvM2dPoint clip_destination;
    NtmvM2dSize clip_size;
    NtmvM2dRect clip_rect;
    NtmvM2dRect clipped_rect;
    NtmvM2dTextBox null_text_box;
    static int render_target_token;
    uint8_t *text;
    unsigned int index;

    memset(&archive_storage, 0, sizeof(archive_storage));
    archive->text_offsets[0] = 4;
    text = archive_storage.bytes + 0x10;
    text[0] = 'A';
    text[1] = 'B';
    text[2] = 'C';
    text[3] = 'D';
    text[4] = 1;
    text[5] = 'E';
    memset(&build_context, 0, sizeof(build_context));
    build_context.text_archive = archive;

    memset(&resource_storage, 0, sizeof(resource_storage));
    resource->pane.position.x = 10;
    resource->pane.position.y = 20;
    resource->pane.size.width = 20;
    resource->pane.size.height = 20;
    resource->text_index = 0;
    resource->line_count = 2;
    resource->unknown_metric_14 = -7;
    resource->font_size = 16;
    resource->character_spacing = 1;
    resource->line_spacing = 1;
    resource->draw_value = 0x1234;
    resource->alignment = 0x21; /* horizontal center, vertical bottom */
    resource->line_alignment = 0xff; /* masked to explicit right (3) */
    resource->padding_left = 1;
    resource->padding_right = 2;
    resource->padding_top = 3;
    resource->padding_bottom = 4;
    resource->lines[0].width = 6;
    resource->lines[0].height = 4;
    resource->lines[0].baseline_offset = 1;
    resource->lines[0].text_length = 3;
    resource->lines[1].width = 4;
    resource->lines[1].height = 5;
    resource->lines[1].baseline_offset = 2;
    resource->lines[1].text_length = 3;

    assert(NtmvM2dTextBox_Construct(
               &text_box, resource, &build_context) == &text_box);
    assert(text_box.pane.vtable == &gNtmvM2dTextBoxVTable);
    assert(text_box.text == text);
    assert(text_box.lines == resource->lines);
    assert(text_box.line_count == 2);
    assert(text_box.unknown_metric_2a == -7);
    assert(text_box.font_size == 16);
    assert(text_box.line_alignment == 3);
    assert(strcmp(
               (const char *)text_box.pane.vtable->get_runtime_type(&text_box.pane),
               "N4ntmv3m2d7TextBoxE") == 0);

    clip_destination.x = 10;
    clip_destination.y = 20;
    clip_size.width = 8;
    clip_size.height = 6;
    clip_rect.left = 12;
    clip_rect.top = 18;
    clip_rect.right = 17;
    clip_rect.bottom = 25;
    NtmvM2dTextBox_LineIntersectsClip(
        &clipped_rect, &clip_destination, &clip_size, &clip_rect);
    AssertRect(&clipped_rect, 2, 0, 7, 5);

    clip_rect.left = 30;
    clip_rect.top = 18;
    clip_rect.right = 35;
    clip_rect.bottom = 25;
    NtmvM2dTextBox_LineIntersectsClip(
        &clipped_rect, &clip_destination, &clip_size, &clip_rect);
    AssertRect(&clipped_rect, 0, 0, 0, 0);

    parent_position.x = 3;
    parent_position.y = 4;
    text_box.pane.vtable->update_position(
        &text_box.pane, NULL, &parent_position);
    assert(text_box.pane.world_position.x == 13);
    assert(text_box.pane.world_position.y == 24);
    assert(text_box.measured_width == 6);
    assert(text_box.measured_height == 10);

    null_text_box = text_box;
    null_text_box.text = NULL;
    null_text_box.pane.world_position.x = -100;
    null_text_box.pane.world_position.y = -101;
    null_text_box.pane.vtable->update_position(
        &null_text_box.pane, NULL, &parent_position);
    assert(null_text_box.pane.world_position.x == -100);
    assert(null_text_box.pane.world_position.y == -101);

    for (index = 0; index < 3; ++index) {
        fonts[index].resource = NULL;
        fonts[index].decode_next = DecodeByte;
    }
    memset(&draw_context, 0, sizeof(draw_context));
    draw_context.clip.left = 0;
    draw_context.clip.top = 0;
    draw_context.clip.right = 100;
    draw_context.clip.bottom = 100;
    draw_context.render_target = &render_target_token;
    draw_context.render_row_stride = -2;
    draw_context.render_row_wrap = 3;
    draw_context.fonts[0] = &fonts[0];
    draw_context.fonts[1] = &fonts[1];
    draw_context.fonts[2] = &fonts[2];
    sExpectedRenderTarget = &render_target_token;
    sExpectedContext = &draw_context;

    TestDrawTextUntil(&draw_context, &fonts[1]);

    ResetRenderRecords();
    text_box.pane.vtable->draw(&text_box.pane, &draw_context);
    assert(sRendererInitCount == 1);
    assert(sGlyphCount == 5);
    AssertGlyph(0, &fonts[1], 19, 31, 'A');
    AssertGlyph(1, &fonts[1], 22, 31, 'B');
    AssertGlyph(2, &fonts[1], 25, 31, 'C');
    AssertGlyph(3, &fonts[1], 21, 37, 'D');
    AssertGlyph(4, &fonts[1], 24, 37, 'E');
    assert(sControlCount == 1 && sLastControl == 1);

    /* First line clipped out: cursor still advances to the second line. */
    draw_context.clip.top = 35;
    ResetRenderRecords();
    text_box.pane.vtable->draw(&text_box.pane, &draw_context);
    assert(sRendererInitCount == 2);
    assert(sGlyphCount == 2);
    AssertGlyph(0, &fonts[1], 21, 37, 'D');
    AssertGlyph(1, &fonts[1], 24, 37, 'E');
    assert(sControlCount == 1 && sLastControl == 1);
    return 0;
}
