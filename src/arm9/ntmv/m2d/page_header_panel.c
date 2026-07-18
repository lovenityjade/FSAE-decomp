#include "ntmv/m2d/page_header_panel.h"
#include "ntmv/ui_renderer.h"

#include <stdbool.h>

typedef struct NtmvM2dCharCanvas NtmvM2dCharCanvas;

typedef struct NtmvM2dCharCanvasOps {
    void (*unknown_00)(NtmvM2dCharCanvas *canvas);
    void (*clear)(NtmvM2dCharCanvas *canvas);
} NtmvM2dCharCanvasOps;

struct NtmvM2dCharCanvas {
    uint8_t unknown_00[0x14];
    const NtmvM2dCharCanvasOps *ops;
};

typedef struct NtmvM2dTextCanvasState {
    NtmvM2dCharCanvas *canvas;
    const NtmvM2dPageHeaderFont *font;
    int32_t character_spacing;
    uint32_t reserved_0c;
} NtmvM2dTextCanvasState;

typedef struct NtmvM2dObjTextCanvas {
    NtmvM2dCharCanvas canvas;
    NtmvM2dTextCanvasState text_canvas;
} NtmvM2dObjTextCanvas;

typedef struct NtmvM2dTextDirection {
    int8_t x;
    int8_t y;
} NtmvM2dTextDirection;

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dCharCanvasTargetSizeCheck[
    sizeof(NtmvM2dCharCanvas) == 0x18 ? 1 : -1];
typedef char NtmvM2dTextCanvasStateTargetSizeCheck[
    sizeof(NtmvM2dTextCanvasState) == 0x10 ? 1 : -1];
typedef char NtmvM2dObjTextCanvasTargetSizeCheck[
    sizeof(NtmvM2dObjTextCanvas) == 0x28 ? 1 : -1];
#endif

extern void NNS_G2dCharCanvasInitForOBJ1D(
    NtmvM2dCharCanvas *canvas,
    void *vram,
    int32_t width_tiles,
    int32_t height_tiles,
    uint32_t color_depth);
extern int NtmvM2d_FormatWideText_0200a324(
    uint16_t *output, uint32_t capacity, const uint16_t *format, ...);
extern void NNSi_G2dTextCanvasDrawTextRect_020c6b50(
    NtmvM2dTextCanvasState *canvas,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height,
    uint32_t flags,
    uint32_t draw_value,
    const uint16_t *text,
    NtmvM2dTextDirection direction);
extern void NNSi_G2dTextCanvasDrawText_020c6a2c(
    NtmvM2dTextCanvasState *canvas,
    int32_t x,
    int32_t y,
    uint32_t flags,
    uint32_t draw_value,
    const uint16_t *text,
    NtmvM2dTextDirection direction);

const NtmvM2dUIStaticVTable gNtmvM2dPageHeaderPanelVTable = {
    (void (*)(NtmvM2dUIStatic *, NtmvM2dAllocatorContext *))
        NtmvM2dUIElement_Destroy,
    NtmvM2dUIStatic_Render,
    (void (*)(NtmvM2dUIStatic *, const NtmvM2dPoint *))
        NtmvM2dUIElement_SetPosition,
    (void (*)(NtmvM2dUIStatic *, const NtmvM2dSize *))
        NtmvM2dUIElement_SetSize,
    (void (*)(NtmvM2dUIStatic *, bool))NtmvM2dUIElement_SetVisible,
};

static const uint16_t sPageNumberFormat[] = {'%', 'u', 0};

static NtmvM2dTextDirection GetTextDirection(
    const NtmvM2dPageHeaderFont *font)
{
    static const NtmvM2dTextDirection directions[8] = {
        {1, 0}, {0, 1}, {0, 1}, {-1, 0},
        {-1, 0}, {0, -1}, {0, -1}, {1, 0},
    };
    uint8_t orientation = font->resource->metadata->orientation;

    if (orientation >= 8) {
        return (NtmvM2dTextDirection){0, 0};
    }
    return directions[orientation];
}

/* 0x020bbdd8 */
static NtmvM2dObjTextCanvas *NtmvM2d_InitObjTextCanvas(
    NtmvM2dObjTextCanvas *output,
    bool use_sub_engine,
    const NtmvM2dPoint *tile_origin,
    const NtmvM2dSize *canvas_size,
    const NtmvM2dPageHeaderFont *font,
    int16_t character_spacing)
{
    uintptr_t vram_base = use_sub_engine ? 0x06600000u : 0x06400000u;
    void *vram = (void *)(vram_base +
        (uint32_t)(uint16_t)tile_origin->x * 0x20u +
        (uint32_t)(uint16_t)tile_origin->y * 0x400u);

    NNS_G2dCharCanvasInitForOBJ1D(
        &output->canvas,
        vram,
        canvas_size->width,
        canvas_size->height,
        4);
    output->text_canvas.canvas = &output->canvas;
    output->text_canvas.font = font;
    output->text_canvas.reserved_0c = 0;
    output->text_canvas.character_spacing = character_spacing;
    output->canvas.ops->clear(&output->canvas);
    return output;
}

/* 0x020bbb78 */
NtmvM2dPageHeaderPanel *NtmvM2dPageHeaderPanel_Construct(
    NtmvM2dPageHeaderPanel *panel)
{
    NtmvM2dUIStatic_ConstructBase(&panel->base);
    panel->base.base.vtable =
        (const NtmvM2dUIElementVTable *)&gNtmvM2dPageHeaderPanelVTable;
    panel->text = NULL;
    return panel;
}

/* 0x020bbba0 */
void NtmvM2dPageHeaderPanel_Configure(
    NtmvM2dPageHeaderPanel *panel,
    void *animation_manager,
    const NtmvM2dPageHeaderContext *context)
{
    const NtmvM2dPoint position = {0x80, 9};
    const NtmvM2dSize size = {0x100, 0x12};
    int32_t animation_handle =
        NtmvUiRenderer_CreateAnimation(animation_manager, 0);

    NtmvM2dUIStatic_Configure(
        &panel->base, &position, &size, animation_handle);
    panel->context = context;
}

/*
 * 0x020bbbfc.  A page number is drawn into the leading 0x12x0x10 area unless
 * UINT16_MAX selects a title-only header.  Font orientations 0..7 map to the
 * exact signed text direction pair used by both SDK drawing calls.
 */
void NtmvM2dPageHeaderPanel_SetText(
    NtmvM2dPageHeaderPanel *panel,
    const uint16_t *text,
    uint16_t page_index)
{
    NtmvM2dObjTextCanvas canvas;
    const NtmvM2dPoint tile_origin = {0, 0};
    const NtmvM2dSize canvas_size = {0x20, 2};
    NtmvM2dTextDirection direction;
    uint16_t page_number[10];
    int32_t text_x = 0;

    panel->text = text;
    NtmvM2d_InitObjTextCanvas(
        &canvas,
        true,
        &tile_origin,
        &canvas_size,
        panel->context->font,
        0);
    direction = GetTextDirection(canvas.text_canvas.font);

    if (page_index != UINT16_MAX) {
        NtmvM2d_FormatWideText_0200a324(
            page_number, 10, sPageNumberFormat, (uint32_t)page_index + 1u);
        NNSi_G2dTextCanvasDrawTextRect_020c6b50(
            &canvas.text_canvas,
            0,
            0,
            0x12,
            0x10,
            1,
            0x480,
            page_number,
            direction);
        text_x = 0x16;
    }
    NNSi_G2dTextCanvasDrawText_020c6a2c(
        &canvas.text_canvas,
        text_x,
        8,
        1,
        0x20a,
        panel->text,
        direction);
}
