#include "ntmv/m2d/page_header_panel.h"
#include "ntmv/ui_renderer.h"

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

typedef struct TestDirection {
    int8_t x;
    int8_t y;
} TestDirection;

typedef struct TestCharCanvas TestCharCanvas;
typedef struct TestCharCanvasOps {
    void (*unknown_00)(TestCharCanvas *canvas);
    void (*clear)(TestCharCanvas *canvas);
} TestCharCanvasOps;
struct TestCharCanvas {
    uint8_t unknown_00[0x14];
    const TestCharCanvasOps *ops;
};

static unsigned int sClearCalls;
static unsigned int sFormatCalls;
static unsigned int sRectCalls;
static unsigned int sTextCalls;
static uintptr_t sVram;
static int32_t sCanvasWidth;
static int32_t sCanvasHeight;
static uint32_t sFormattedPage;
static int32_t sTextX;
static TestDirection sRectDirection;
static TestDirection sTextDirection;
static const uint16_t *sExpectedText;

static void Clear(TestCharCanvas *canvas)
{
    (void)canvas;
    ++sClearCalls;
}

static const TestCharCanvasOps sOps = {NULL, Clear};

void NNS_G2dCharCanvasInitForOBJ1D(
    TestCharCanvas *canvas,
    void *vram,
    int32_t width_tiles,
    int32_t height_tiles,
    uint32_t color_depth)
{
    assert(color_depth == 4);
    canvas->ops = &sOps;
    sVram = (uintptr_t)vram;
    sCanvasWidth = width_tiles;
    sCanvasHeight = height_tiles;
}

int NtmvM2d_FormatWideText_0200a324(
    uint16_t *output, uint32_t capacity, const uint16_t *format, ...)
{
    va_list arguments;
    assert(capacity == 10);
    assert(format[0] == '%' && format[1] == 'u' && format[2] == 0);
    va_start(arguments, format);
    sFormattedPage = va_arg(arguments, uint32_t);
    va_end(arguments);
    output[0] = (uint16_t)('0' + sFormattedPage);
    output[1] = 0;
    ++sFormatCalls;
    return 1;
}

void NNSi_G2dTextCanvasDrawTextRect_020c6b50(
    void *canvas,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height,
    uint32_t flags,
    uint32_t draw_value,
    const uint16_t *text,
    TestDirection direction)
{
    (void)canvas;
    assert(x == 0 && y == 0 && width == 0x12 && height == 0x10);
    assert(flags == 1 && draw_value == 0x480);
    assert(text[0] == (uint16_t)('0' + sFormattedPage));
    sRectDirection = direction;
    ++sRectCalls;
}

void NNSi_G2dTextCanvasDrawText_020c6a2c(
    void *canvas,
    int32_t x,
    int32_t y,
    uint32_t flags,
    uint32_t draw_value,
    const uint16_t *text,
    TestDirection direction)
{
    (void)canvas;
    assert(y == 8 && flags == 1 && draw_value == 0x20a);
    assert(text == sExpectedText);
    sTextX = x;
    sTextDirection = direction;
    ++sTextCalls;
}

int32_t NtmvUiRenderer_CreateAnimation(
    NtmvUiRenderer *animation_manager, uint16_t sequence_index)
{
    assert(animation_manager == (NtmvUiRenderer *)(uintptr_t)0x1234);
    assert(sequence_index == 0);
    return 77;
}

bool NtmvUiRenderer_SubmitCell(
    NtmvUiRenderer *renderer,
    int32_t resource_id,
    int32_t x_fx12,
    int32_t y_fx12)
{
    (void)renderer;
    (void)resource_id;
    (void)x_fx12;
    (void)y_fx12;
    return true;
}

bool NtmvUiRenderer_SetAnimationFrame(
    NtmvUiRenderer *renderer, int32_t resource_id, uint32_t frame)
{
    (void)renderer;
    (void)resource_id;
    (void)frame;
    return true;
}

bool NtmvUiRenderer_TickAnimation(
    NtmvUiRenderer *renderer, int32_t resource_id, int32_t scale_fx12)
{
    (void)renderer;
    (void)resource_id;
    (void)scale_fx12;
    return true;
}

static void ResetDrawCalls(void)
{
    sClearCalls = 0;
    sFormatCalls = 0;
    sRectCalls = 0;
    sTextCalls = 0;
    sTextX = -1;
}

int main(void)
{
    static const TestDirection expected[8] = {
        {1, 0}, {0, 1}, {0, 1}, {-1, 0},
        {-1, 0}, {0, -1}, {0, -1}, {1, 0},
    };
    static const uint16_t text[] = {'T', 'i', 't', 'l', 'e', 0};
    NtmvM2dFontMetadata metadata = {{0}, 0};
    NtmvM2dFontResource resource = {0};
    NtmvM2dPageHeaderFont font = {&resource};
    NtmvM2dPageHeaderContext context = {NULL, &font};
    NtmvM2dPageHeaderPanel panel;
    unsigned int orientation;

    resource.metadata = &metadata;
    memset(&panel, 0xa5, sizeof(panel));
    assert(NtmvM2dPageHeaderPanel_Construct(&panel) == &panel);
    assert(panel.base.base.vtable ==
        (const NtmvM2dUIElementVTable *)&gNtmvM2dPageHeaderPanelVTable);
    assert(panel.text == NULL);
    assert(panel.reserved_18 == UINT32_C(0xa5a5a5a5));

    NtmvM2dPageHeaderPanel_Configure(
        &panel, (void *)(uintptr_t)0x1234, &context);
    assert(panel.base.base.local_position.x == 0x80);
    assert(panel.base.base.local_position.y == 9);
    assert(panel.base.base.size.width == 0x100);
    assert(panel.base.base.size.height == 0x12);
    assert(panel.base.resource_id == 77);
    assert(panel.context == &context);

    sExpectedText = text;
    for (orientation = 0; orientation < 8; ++orientation) {
        metadata.orientation = (uint8_t)orientation;
        ResetDrawCalls();
        NtmvM2dPageHeaderPanel_SetText(&panel, text, 3);
        assert(panel.text == text);
        assert(sVram == UINT32_C(0x06600000));
        assert(sCanvasWidth == 0x20 && sCanvasHeight == 2);
        assert(sClearCalls == 1 && sFormatCalls == 1);
        assert(sRectCalls == 1 && sTextCalls == 1);
        assert(sFormattedPage == 4 && sTextX == 0x16);
        assert(sRectDirection.x == expected[orientation].x);
        assert(sRectDirection.y == expected[orientation].y);
        assert(sTextDirection.x == expected[orientation].x);
        assert(sTextDirection.y == expected[orientation].y);
    }

    ResetDrawCalls();
    NtmvM2dPageHeaderPanel_SetText(&panel, text, UINT16_MAX);
    assert(sClearCalls == 1 && sFormatCalls == 0 && sRectCalls == 0);
    assert(sTextCalls == 1 && sTextX == 0);
    return 0;
}
