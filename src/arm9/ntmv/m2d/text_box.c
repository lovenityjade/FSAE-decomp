#include "ntmv/m2d/text_box.h"

/* 0x02126b1c precedes typeinfo 0x02126b20 for N4ntmv3m2d7TextBoxE. */
static const char sNtmvM2dTextBoxRuntimeType[] = "N4ntmv3m2d7TextBoxE";

const NtmvM2dPaneVTable gNtmvM2dTextBoxVTable = {
    NtmvM2dTextBox_GetRuntimeType, /* slot 0: 0x020b7074 */
    NtmvM2dPane_Dispose,           /* slot 1: 0x020b5654 */
    NtmvM2dTextBox_UpdatePosition, /* slot 2: 0x020b6cc4 */
    NtmvM2dTextBox_Draw,           /* slot 3: 0x020b6d74 */
};

/* 0x020b6bf8 */
NtmvM2dTextBox *NtmvM2dTextBox_Construct(
    NtmvM2dTextBox *text_box,
    const NtmvM2dTextBoxResource *resource,
    const NtmvM2dTextBoxBuildContext *build_context)
{
    const uint8_t *offset_table =
        (const uint8_t *)build_context->text_archive + 0x0c;
    uint32_t text_offset =
        ((const uint32_t *)(const void *)offset_table)[resource->text_index];

    NtmvM2dPane_ConstructBase(&text_box->pane, &resource->pane);
    text_box->pane.vtable = &gNtmvM2dTextBoxVTable;
    text_box->measured_width = 0;
    text_box->measured_height = 0;
    text_box->unknown_metric_2a = 0;
    text_box->font_size = 0;
    text_box->line_alignment = 0;

    text_box->text = offset_table + text_offset;
    text_box->line_count = resource->line_count;
    text_box->unknown_metric_2a = resource->unknown_metric_14;
    text_box->font_size = resource->font_size;
    text_box->character_spacing = resource->character_spacing;
    text_box->line_spacing = resource->line_spacing;
    text_box->draw_value = resource->draw_value;
    text_box->lines = resource->lines;
    text_box->alignment = resource->alignment;
    text_box->line_alignment = (uint8_t)(resource->line_alignment & 3u);
    text_box->padding_left = resource->padding_left;
    text_box->padding_right = resource->padding_right;
    text_box->padding_top = resource->padding_top;
    text_box->padding_bottom = resource->padding_bottom;
    return text_box;
}

/* 0x020b7074, vtable slot 0. */
const void *NtmvM2dTextBox_GetRuntimeType(const NtmvM2dPane *pane)
{
    (void)pane;
    return sNtmvM2dTextBoxRuntimeType;
}

/* 0x020b6cc4, vtable slot 2. */
void NtmvM2dTextBox_UpdatePosition(
    NtmvM2dPane *pane, void *context, const NtmvM2dPoint *parent_position)
{
    NtmvM2dTextBox *text_box = (NtmvM2dTextBox *)pane;
    uint32_t line_index;

    /* The ROM returns before even updating Pane position when text is null. */
    if (text_box->text == NULL) {
        return;
    }

    NtmvM2dPane_UpdatePosition(pane, context, parent_position);
    text_box->measured_width = 0;
    text_box->measured_height = 0;
    for (line_index = 0; line_index < text_box->line_count; ++line_index) {
        const NtmvM2dTextBoxLine *line = &text_box->lines[line_index];

        if (text_box->measured_width < line->width) {
            text_box->measured_width = line->width;
        }
        if (line_index != 0) {
            text_box->measured_height = (int16_t)(
                text_box->measured_height + text_box->line_spacing);
        }
        text_box->measured_height = (int16_t)(
            text_box->measured_height + line->height);
    }
}

/* 0x020b70c0: choose the large, normal or small font around size 16. */
static const NtmvM2dTextFont *SelectFont(
    const NtmvM2dTextDrawContext *context, uint16_t font_size)
{
    uint32_t index = 1;

    if (font_size < 16) {
        index = 2;
    } else if (font_size > 16) {
        index = 0;
    }
    return context->fonts[index];
}

/* 0x020b6f4c: initialize the renderer and the sequential text cursor. */
static void InitializeTextSession(
    NtmvM2dTextBoxTextSession *session,
    NtmvM2dGlyphRenderer *renderer_storage,
    const NtmvM2dTextBox *text_box,
    NtmvM2dTextDrawContext *context)
{
    NtmvM2d_InitGlyphRenderer_020b6898(
        renderer_storage,
        context->render_target,
        context->render_row_stride,
        context->render_row_wrap,
        0x10,
        &context->clip);
    session->renderer = renderer_storage;
    session->font = SelectFont(context, text_box->font_size);
    session->character_spacing = text_box->character_spacing;
    session->line_spacing = text_box->line_spacing;
    session->cursor = text_box->text;
    session->x = 0;
    session->y = 0;
    session->draw_value = text_box->draw_value;
    session->reserved_20[0] = 0;
    session->reserved_20[1] = 0;
    session->reserved_20[2] = 0;
    session->reserved_20[3] = 0;
    session->has_previous_glyph = 0;
    session->reserved_25[0] = 0;
    session->reserved_25[1] = 0;
    session->reserved_25[2] = 0;
}

/* 0x020b6fb8: 0/1/2 mean left/center/right; other values fall back left. */
static int32_t GetHorizontalAlignment(const NtmvM2dTextBox *text_box)
{
    uint8_t alignment = (uint8_t)(text_box->alignment & 0x0fu);
    return alignment <= 2 ? alignment : 0;
}

/* 0x020b6ff0: 0/1/2 mean top/center/bottom; other values fall back top. */
static int32_t GetVerticalAlignment(const NtmvM2dTextBox *text_box)
{
    uint8_t alignment = (uint8_t)(text_box->alignment >> 4);
    return alignment <= 2 ? alignment : 0;
}

/* 0x020b7028: zero inherits the block alignment; 1/2/3 are explicit. */
static int32_t GetLineAlignment(const NtmvM2dTextBox *text_box)
{
    switch (text_box->line_alignment & 3u) {
    case 1:
        return 0;
    case 2:
        return 1;
    case 3:
        return 2;
    default:
        return GetHorizontalAlignment(text_box);
    }
}

/* 0x020b7488 */
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

/* 0x020b6b3c: decode and render until the current line's byte boundary. */
void NtmvM2dTextBox_DrawTextUntil_020b6b3c(
    NtmvM2dTextBoxTextSession *session,
    const uint8_t *end,
    NtmvM2dTextDrawContext *context)
{
    if (end <= session->cursor) {
        return;
    }

    do {
        int32_t character = session->font->decode_next(&session->cursor);

        if (character == 0 || character == '\n') {
            return;
        }
        if (character < 0x20) {
            NtmvM2d_HandleTextControl_020b69f4(context, character, session);
        } else {
            int32_t advance;

            if (session->has_previous_glyph != 0) {
                session->has_previous_glyph = 0;
                session->x += session->character_spacing;
            }
            advance = NtmvM2d_DrawGlyph_020b68c8(
                session->renderer,
                session->font,
                session->x,
                session->y,
                session->draw_value,
                (uint16_t)character);
            session->has_previous_glyph = 1;
            session->x += advance;
        }
    } while (session->cursor < end);
}

/* 0x020b6d74, vtable slot 3. */
void NtmvM2dTextBox_Draw(NtmvM2dPane *pane, void *opaque_context)
{
    NtmvM2dTextBox *text_box = (NtmvM2dTextBox *)pane;
    NtmvM2dTextDrawContext *context = opaque_context;
    NtmvM2dTextBoxTextSession session;
    NtmvM2dGlyphRenderer renderer_storage; /* target 0x18-byte stack object */
    const uint8_t *line_start;
    int32_t available_width;
    int32_t available_height;
    int32_t block_x;
    int32_t line_y;
    int32_t line_alignment;
    uint32_t line_index;

    if (text_box->text == NULL) {
        return;
    }

    InitializeTextSession(&session, &renderer_storage, text_box, context);
    available_width = (int16_t)(
        text_box->pane.size.width - text_box->padding_left - text_box->padding_right);
    available_height = (int16_t)(
        text_box->pane.size.height - text_box->padding_top - text_box->padding_bottom);
    block_x = text_box->pane.world_position.x + text_box->padding_left +
        ((available_width - text_box->measured_width) *
         GetHorizontalAlignment(text_box)) / 2;
    line_y = text_box->pane.world_position.y + text_box->padding_top +
        ((available_height - text_box->measured_height) *
         GetVerticalAlignment(text_box)) / 2;
    line_alignment = GetLineAlignment(text_box);
    line_start = text_box->text;

    for (line_index = 0; line_index < text_box->line_count; ++line_index) {
        const NtmvM2dTextBoxLine *line = &text_box->lines[line_index];
        const uint8_t *line_end = line_start + line->text_length;
        NtmvM2dPoint clip_destination;
        NtmvM2dSize clip_size;
        NtmvM2dRect clipped_rect;

        clip_destination.x = (int16_t)block_x;
        clip_destination.y = (int16_t)line_y;
        clip_size.width = line->width;
        clip_size.height = line->height;
        NtmvM2dTextBox_LineIntersectsClip(
            &clipped_rect, &clip_destination, &clip_size, &context->clip);
        if (clipped_rect.left < clipped_rect.right &&
            clipped_rect.top < clipped_rect.bottom) {
            session.x = block_x +
                (line_alignment * (text_box->measured_width - line->width)) / 2;
            session.y = line_y + line->baseline_offset;
            session.has_previous_glyph = 0;
            NtmvM2dTextBox_DrawTextUntil_020b6b3c(
                &session, line_end, context);
        }

        session.cursor = line_end;
        line_start = line_end;
        line_y += (int16_t)(text_box->line_spacing + line->height);
    }
}
