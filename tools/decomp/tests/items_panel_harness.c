#include "ntmv/m2d/items_panel.h"
#include "ntmv/m2d/ui_static.h"

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct TestChild {
    NtmvM2dUIElement base;
    int id;
} TestChild;

typedef struct HarnessCharCanvas HarnessCharCanvas;

typedef struct HarnessCharCanvasOps {
    void (*unknown_00)(HarnessCharCanvas *canvas);
    void (*clear)(HarnessCharCanvas *canvas);
} HarnessCharCanvasOps;

struct HarnessCharCanvas {
    uint8_t unknown_00[0x14];
    const HarnessCharCanvasOps *ops;
};

typedef struct HarnessTextCanvasState {
    HarnessCharCanvas *canvas;
    const NtmvM2dItemsFont *font;
    int32_t character_spacing;
    uint32_t reserved_0c;
} HarnessTextCanvasState;

typedef struct HarnessTextDirection {
    int8_t x;
    int8_t y;
} HarnessTextDirection;

typedef struct CanvasInitRecord {
    uintptr_t vram;
    int32_t width;
    int32_t height;
} CanvasInitRecord;

static void *sFreedPointers[4];
static unsigned int sFreeCount;
static unsigned int sChildDestroyCount;
static unsigned int sChildRenderCount;
static CanvasInitRecord sCanvasInits[6];
static unsigned int sCanvasInitCount;
static unsigned int sClearCount;
static unsigned int sFormatCount;
static uint32_t sFormattedValues[3];
static unsigned int sRectCount;
static uint16_t sRectFirstCharacters[3];
static HarnessTextDirection sRectDirections[3];
static unsigned int sTextCount;
static const uint16_t *sDrawnTexts[3];
static HarnessTextDirection sTextDirections[3];
static unsigned int sAlternateCallCount;
static unsigned int sProviderCallCount;
static int16_t sProviderIndices[3];

typedef struct ManualButtonConfigRecord {
    NtmvM2dManualButton *button;
    NtmvM2dPoint position;
    NtmvM2dSize size;
    int32_t animation_handle;
    int32_t alternate_animation_handle;
    int32_t overlay_animation_handle;
} ManualButtonConfigRecord;

typedef struct ObjButtonConfigRecord {
    NtmvM2dObjButton *button;
    NtmvM2dPoint position;
    NtmvM2dSize size;
    int32_t animation_handle;
} ObjButtonConfigRecord;

typedef struct StaticConfigRecord {
    NtmvM2dUIStatic *element;
    NtmvM2dPoint position;
    NtmvM2dSize size;
    int32_t animation_handle;
} StaticConfigRecord;

static ManualButtonConfigRecord sManualButtonConfigs[8];
static ObjButtonConfigRecord sObjButtonConfigs[2];
static StaticConfigRecord sStaticConfigs[2];
static uint16_t sAnimationSequences[32];
static unsigned int sManualButtonConfigCount;
static unsigned int sObjButtonConfigCount;
static unsigned int sStaticConfigCount;
static unsigned int sAnimationSequenceCount;

static const uint16_t sProviderTexts[4][2] = {
    {'0', 0}, {'1', 0}, {'2', 0}, {'3', 0},
};

static void ResetTextCalls(void)
{
    sCanvasInitCount = 0;
    sClearCount = 0;
    sFormatCount = 0;
    sRectCount = 0;
    sTextCount = 0;
    sAlternateCallCount = 0;
}

static void ProvideRow(
    NtmvM2dItemsRow *output, void *context, int16_t item_index)
{
    assert(context == (void *)(uintptr_t)0x3333);
    assert(0 <= item_index && item_index < 4);
    assert(sProviderCallCount < 3);
    sProviderIndices[sProviderCallCount++] = item_index;
    output->text = sProviderTexts[item_index];
    output->use_alternate_animation = (uint8_t)(item_index & 1);
    output->reserved_05[0] = 0;
    output->reserved_05[1] = 0;
    output->reserved_05[2] = 0;
}

void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size)
{
    assert(allocator == (void *)(uintptr_t)0x5678);
    return malloc(size);
}

void NNS_FndFreeToAllocator(void *allocator, void *memory)
{
    assert(allocator == (void *)(uintptr_t)0x5678);
    assert(sFreeCount < 4);
    sFreedPointers[sFreeCount++] = memory;
    free(memory);
}

static void HarnessCanvasClear(HarnessCharCanvas *canvas)
{
    assert(canvas->ops != NULL);
    ++sClearCount;
}

static const HarnessCharCanvasOps sCanvasOps = {
    NULL,
    HarnessCanvasClear,
};

void NNS_G2dCharCanvasInitForOBJ1D(
    HarnessCharCanvas *canvas,
    void *vram,
    int32_t width_tiles,
    int32_t height_tiles,
    uint32_t color_depth)
{
    assert(sCanvasInitCount < 6);
    assert(color_depth == 4);
    sCanvasInits[sCanvasInitCount].vram = (uintptr_t)vram;
    sCanvasInits[sCanvasInitCount].width = width_tiles;
    sCanvasInits[sCanvasInitCount].height = height_tiles;
    ++sCanvasInitCount;
    canvas->ops = &sCanvasOps;
}

int NtmvM2d_FormatWideText_0200a324(
    uint16_t *output, uint32_t capacity, const uint16_t *format, ...)
{
    va_list arguments;
    uint32_t value;

    assert(capacity == 10);
    assert(format[0] == '%' && format[1] == 'u' && format[2] == 0);
    assert(sFormatCount < 3);
    va_start(arguments, format);
    value = va_arg(arguments, uint32_t);
    va_end(arguments);
    sFormattedValues[sFormatCount++] = value;
    output[0] = (uint16_t)('0' + value);
    output[1] = 0;
    return 1;
}

void NNSi_G2dTextCanvasDrawTextRect_020c6b50(
    HarnessTextCanvasState *canvas,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height,
    uint32_t flags,
    uint32_t draw_value,
    const uint16_t *text,
    HarnessTextDirection direction)
{
    assert(canvas->canvas != NULL && canvas->font != NULL);
    assert(x == 0 && y == 0 && width == 0x20 && height == 0x20);
    assert(flags == 1 && draw_value == 0x480);
    assert(sRectCount < 3);
    sRectFirstCharacters[sRectCount] = text[0];
    sRectDirections[sRectCount] = direction;
    ++sRectCount;
}

void NNSi_G2dTextCanvasDrawText_020c6a2c(
    HarnessTextCanvasState *canvas,
    int32_t x,
    int32_t y,
    uint32_t flags,
    uint32_t draw_value,
    const uint16_t *text,
    HarnessTextDirection direction)
{
    assert(canvas->canvas != NULL && canvas->font != NULL);
    assert(x == 0 && y == 0x10);
    assert(flags == 1 && draw_value == 0x20a);
    assert(sTextCount < 3);
    sDrawnTexts[sTextCount] = text;
    sTextDirections[sTextCount] = direction;
    ++sTextCount;
}

void NtmvM2dManualButton_SetAlternateAnimation(
    NtmvM2dManualButton *button, bool use_alternate_animation)
{
    button->use_alternate_animation = use_alternate_animation;
    ++sAlternateCallCount;
}

NtmvM2dManualButton *NtmvM2dManualButton_Construct(
    NtmvM2dManualButton *button)
{
    NtmvM2dUIElement_Init(&button->base.base);
    button->base.animation_handle = -1;
    button->alternate_animation_handle = -1;
    button->overlay_animation_handle = -1;
    button->use_alternate_animation = 0;
    button->selected = 0;
    return button;
}

void NtmvM2dManualButton_Configure(
    NtmvM2dManualButton *button,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size,
    int32_t animation_handle,
    int32_t alternate_animation_handle,
    int32_t overlay_animation_handle)
{
    ManualButtonConfigRecord *record;

    assert(sManualButtonConfigCount < 8);
    record = &sManualButtonConfigs[sManualButtonConfigCount++];
    record->button = button;
    record->position = *position;
    record->size = *size;
    record->animation_handle = animation_handle;
    record->alternate_animation_handle = alternate_animation_handle;
    record->overlay_animation_handle = overlay_animation_handle;
    button->base.base.local_position = *position;
    button->base.base.size = *size;
    button->base.animation_handle = animation_handle;
    button->alternate_animation_handle = alternate_animation_handle;
    button->overlay_animation_handle = overlay_animation_handle;
}

NtmvM2dObjButton *NtmvM2dObjButton_ConstructComplete(
    NtmvM2dObjButton *button)
{
    NtmvM2dUIElement_Init(&button->base);
    button->animation_handle = -1;
    return button;
}

void NtmvM2dObjButton_Configure(
    NtmvM2dObjButton *button,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size,
    int32_t animation_handle)
{
    ObjButtonConfigRecord *record;

    assert(sObjButtonConfigCount < 2);
    record = &sObjButtonConfigs[sObjButtonConfigCount++];
    record->button = button;
    record->position = *position;
    record->size = *size;
    record->animation_handle = animation_handle;
    button->base.local_position = *position;
    button->base.size = *size;
    button->animation_handle = animation_handle;
}

NtmvM2dUIStatic *NtmvM2dUIStatic_ConstructComplete(
    NtmvM2dUIStatic *element)
{
    NtmvM2dUIElement_Init(&element->base);
    element->resource_id = -1;
    return element;
}

void NtmvM2dUIStatic_Configure(
    NtmvM2dUIStatic *element,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size,
    int32_t resource_id)
{
    StaticConfigRecord *record;

    assert(sStaticConfigCount < 2);
    record = &sStaticConfigs[sStaticConfigCount++];
    record->element = element;
    record->position = *position;
    record->size = *size;
    record->animation_handle = resource_id;
    element->base.local_position = *position;
    element->base.size = *size;
    element->resource_id = resource_id;
}

int32_t NtmvUiRenderer_CreateAnimation(
    void *renderer, uint16_t sequence_index)
{
    assert(renderer == (void *)(uintptr_t)0x3333);
    assert(sAnimationSequenceCount < 32);
    sAnimationSequences[sAnimationSequenceCount++] = sequence_index;
    return 1000 + sequence_index;
}

static void TestChildDestroy(
    NtmvM2dUIElement *element, NtmvM2dAllocatorContext *allocator)
{
    TestChild *child = (TestChild *)element;

    assert(allocator->nns_allocator == (void *)(uintptr_t)0x5678);
    assert(child->id == 7);
    ++sChildDestroyCount;
}

static void TestChildRender(
    NtmvM2dUIElement *element,
    void *render_context,
    const NtmvM2dPoint *parent_position)
{
    assert(render_context == (void *)(uintptr_t)0x2222);
    assert(parent_position->x == 112 && parent_position->y == 75);
    element->world_center.x =
        (int16_t)(parent_position->x + element->local_position.x);
    element->world_center.y =
        (int16_t)(parent_position->y + element->local_position.y);
    ++sChildRenderCount;
}

static const NtmvM2dUIElementVTable sChildVTable = {
    TestChildDestroy,
    TestChildRender,
    NtmvM2dUIElement_SetPosition,
    NtmvM2dUIElement_SetSize,
    NtmvM2dUIElement_SetVisible,
};

static TestChild *AllocateChild(void)
{
    TestChild *child = malloc(sizeof(*child));

    assert(child != NULL);
    NtmvM2dUIElement_Init(&child->base);
    child->base.vtable = &sChildVTable;
    child->base.local_position.x = 1;
    child->base.local_position.y = 2;
    child->id = 7;
    return child;
}

static void TestArrayFreeHelpers(void)
{
    NtmvM2dAllocatorContext allocator = {(void *)(uintptr_t)0x5678};
    NtmvM2dManualButton **row_buttons = malloc(sizeof(*row_buttons));
    int16_t *row_item_indices = malloc(sizeof(*row_item_indices));

    assert(row_buttons != NULL && row_item_indices != NULL);
    sFreeCount = 0;

    NtmvM2dItemsPanel_FreeRowButtons(&allocator, NULL);
    NtmvM2dItemsPanel_FreeRowItemIndices(&allocator, NULL);
    assert(sFreeCount == 0);

    NtmvM2dItemsPanel_FreeRowButtons(&allocator, row_buttons);
    NtmvM2dItemsPanel_FreeRowItemIndices(&allocator, row_item_indices);
    assert(sFreeCount == 2);
    assert(sFreedPointers[0] == row_buttons);
    assert(sFreedPointers[1] == row_item_indices);

    sFreeCount = 0;
}

static void TestConstructorRenderAndDestroy(void)
{
    NtmvM2dItemsPanel panel;
    NtmvM2dAllocatorContext allocator = {(void *)(uintptr_t)0x5678};
    TestChild *child = AllocateChild();
    NtmvM2dUIElement *children[1] = {&child->base};
    NtmvM2dPoint position = {12, -5};
    NtmvM2dSize size = {32, 16};
    NtmvM2dPoint parent = {100, 80};
    void *row_buttons;
    void *row_indices;
    void *owned_children;

    memset(&panel, 0xa5, sizeof(panel));
    assert(NtmvM2dItemsPanel_Construct(&panel) == &panel);
    assert(panel.base.base.vtable ==
           (const NtmvM2dUIElementVTable *)&gNtmvM2dItemsPanelVTable);
    assert(panel.base.children == NULL && panel.base.child_count == 0);
    assert(panel.text_context == NULL);
    assert(panel.item_count == 0);
    assert(panel.first_visible_item == 0);
    assert(panel.visible_row_count_minus_one == 0);
    assert(panel.page_index == 0 && panel.page_count == 0);
    assert(panel.selected_item == -1);
    assert(panel.active_control_index == -1);
    assert(panel.row_buttons == NULL);
    assert(panel.scroll_y == 0 && panel.target_scroll_y == 0);
    assert(panel.interaction_enabled == 1);
    assert(panel.reserved_2e == 0xa5a5);
    assert(panel.reserved_3d[0] == 0xa5 && panel.reserved_3d[1] == 0xa5 &&
           panel.reserved_3d[2] == 0xa5);
    for (size_t byte_index = 0; byte_index < sizeof(panel.row_item_indices);
         byte_index++) {
        assert(((const unsigned char *)&panel)
                   [offsetof(NtmvM2dItemsPanel, row_item_indices) + byte_index] ==
               0xa5);
    }

    NtmvM2dUIPanel_Configure(
        &panel.base, &allocator, children, 1, &position, &size);
    owned_children = panel.base.children;
    row_buttons = malloc(sizeof(void *));
    row_indices = malloc(sizeof(int16_t));
    assert(row_buttons != NULL && row_indices != NULL);
    panel.row_buttons = row_buttons;
    panel.row_item_indices = row_indices;

    panel.base.base.vtable->render(
        &panel.base.base, (void *)(uintptr_t)0x2222, &parent);
    assert(sChildRenderCount == 1);
    assert(panel.base.base.world_center.x == 112);
    assert(panel.base.base.world_center.y == 75);
    assert(child->base.world_center.x == 113);
    assert(child->base.world_center.y == 77);

    panel.base.base.vtable->destroy(&panel.base.base, &allocator);
    assert(sChildDestroyCount == 1);
    assert(sFreeCount == 4);
    assert(sFreedPointers[0] == row_indices);
    assert(sFreedPointers[1] == row_buttons);
    assert(sFreedPointers[2] == child);
    assert(sFreedPointers[3] == owned_children);
}

static void TestBoundedHelpers(void)
{
    NtmvM2dItemsPanel panel;
    NtmvM2dUIElement elements[6];
    NtmvM2dUIElement auxiliary;
    NtmvM2dUIElement *children[10] = {0};
    NtmvM2dPoint point = {10, 10};
    unsigned int index;

    NtmvM2dItemsPanel_Construct(&panel);
    panel.base.children = children;
    panel.visible_row_count_minus_one = 1; /* two rows plus four controls */
    for (index = 0; index < 6; ++index) {
        NtmvM2dUIElement_Init(&elements[index]);
        elements[index].world_center.x = 1000;
        elements[index].world_center.y = 1000;
        elements[index].size.width = 10;
        elements[index].size.height = 10;
        children[index] = &elements[index];
    }
    elements[4].world_center = point;
    assert(NtmvM2dItemsPanel_FindControlAtPoint(&panel, &point) == 4);
    elements[4].flags = NTMV_M2D_UI_HIDDEN;
    assert(NtmvM2dItemsPanel_FindControlAtPoint(&panel, &point) == -1);

    NtmvM2dUIElement_Init(&auxiliary);
    NtmvM2dItemsPanel_SetAuxiliaryChild(&panel, 2, &auxiliary);
    assert(children[8] == &auxiliary); /* (visible rows 2 + controls 4) + 2 */

    panel.visible_row_count_minus_one = -5;
    assert(NtmvM2dItemsPanel_FindControlAtPoint(&panel, &point) == -1);
}

static void ResetConfigurationCalls(void)
{
    sManualButtonConfigCount = 0;
    sObjButtonConfigCount = 0;
    sStaticConfigCount = 0;
    sAnimationSequenceCount = 0;
}

static void TestConfigure(void)
{
    static const uint16_t compact_sequences[22] = {
        25, 26, 7,
        25, 26, 8,
        25, 26, 9,
        25, 26, 10,
        25, 26, 11,
        25, 26, 12,
        33, 32, 39, 39,
    };
    NtmvM2dItemsFontMetadata metadata = {{0}, 0};
    NtmvM2dItemsFontResource resource = {{0}, &metadata};
    NtmvM2dItemsFont font = {&resource};
    NtmvM2dItemsTextContext text_context = {NULL, &font};
    NtmvM2dAllocatorContext allocator = {(void *)(uintptr_t)0x5678};
    NtmvM2dItemsPanelDefinition compact_definition = {
        {0, -36}, 26, 25, 7, -30, 186, 5, 1,
    };
    NtmvM2dItemsPanelDefinition secondary_definition = {
        {137, -59}, 34, -1, 13, 31, 186, 7, 2,
    };
    NtmvM2dItemsPanelDefinition zero_row_definition = {
        {-4, 9}, 6, -1, 0, 12, 88, -1, 0,
    };
    NtmvM2dItemsPanel panel;
    unsigned int index;

    ResetConfigurationCalls();
    NtmvM2dItemsPanel_Construct(&panel);
    NtmvM2dItemsPanel_Configure(
        &panel,
        &allocator,
        (void *)(uintptr_t)0x3333,
        &text_context,
        &compact_definition);

    assert(panel.base.base.local_position.x == 0);
    assert(panel.base.base.local_position.y == -36);
    assert(panel.base.base.size.width == 0x100);
    assert(panel.base.base.size.height == 0xc0);
    assert(panel.text_context == &text_context);
    assert(panel.visible_row_count_minus_one == 5);
    assert(panel.base.child_count == 11);
    assert(panel.base.children != NULL);
    assert(panel.row_buttons != NULL);
    assert(panel.row_item_indices != NULL);
    assert(sManualButtonConfigCount == 6);
    for (index = 0; index < 6; ++index) {
        ManualButtonConfigRecord *record = &sManualButtonConfigs[index];

        assert(panel.row_buttons[index] == record->button);
        assert(panel.base.children[index + 4] == &record->button->base.base);
        assert(panel.row_item_indices[index] == -1);
        assert(record->position.x == -30);
        assert(record->position.y == (int16_t)(index * 0x17));
        assert(record->size.width == 186);
        assert(record->size.height == 0x14);
        assert(record->animation_handle == 1026);
        assert(record->alternate_animation_handle == 1025);
        assert(record->overlay_animation_handle == (int32_t)(1007 + index));
    }
    assert(sObjButtonConfigCount == 2);
    assert(sObjButtonConfigs[0].position.x == -30);
    assert(sObjButtonConfigs[0].position.y == -23);
    assert(sObjButtonConfigs[0].size.width == 0x33);
    assert(sObjButtonConfigs[0].size.height == 0x14);
    assert(sObjButtonConfigs[0].animation_handle == 1033);
    assert(sObjButtonConfigs[1].position.x == -30);
    assert(sObjButtonConfigs[1].position.y == 115);
    assert(sObjButtonConfigs[1].animation_handle == 1032);
    assert(panel.base.children[0] == &sObjButtonConfigs[0].button->base);
    assert(panel.base.children[1] == &sObjButtonConfigs[1].button->base);

    assert(sStaticConfigCount == 2);
    assert(sStaticConfigs[0].position.x == -27);
    assert(sStaticConfigs[0].position.y == -29);
    assert(sStaticConfigs[0].size.width == 0xc0);
    assert(sStaticConfigs[0].size.height == 0x20);
    assert(sStaticConfigs[0].animation_handle == 1039);
    assert(sStaticConfigs[1].position.x == -27);
    assert(sStaticConfigs[1].position.y == 121);
    assert(sStaticConfigs[1].animation_handle == 1039);
    assert(panel.base.children[2] == &sStaticConfigs[0].element->base);
    assert(panel.base.children[3] == &sStaticConfigs[1].element->base);
    assert((panel.base.children[2]->flags & NTMV_M2D_UI_HIDDEN) != 0);
    assert((panel.base.children[3]->flags & NTMV_M2D_UI_HIDDEN) != 0);
    assert(panel.base.children[10] == NULL); /* one auxiliary owner slot */
    assert(sAnimationSequenceCount == 22);
    assert(memcmp(
               sAnimationSequences,
               compact_sequences,
               sizeof(compact_sequences)) == 0);

    /* 0x020b9380 computes ceil(item_count / page step), resets page zero,
     * clears selection/active control and hides every circular row. */
    panel.selected_item = 4;
    panel.active_control_index = 5;
    for (index = 0; index < 6; ++index) {
        panel.row_buttons[index]->selected = 1;
        panel.row_buttons[index]->base.base.flags &=
            (uint8_t)~NTMV_M2D_UI_HIDDEN;
    }
    NtmvM2dItemsPanel_SetItemCount(&panel, 13);
    assert(panel.item_count == 13);
    assert(panel.page_count == 3);
    assert(panel.page_index == 0);
    assert(panel.first_visible_item == 0);
    assert(panel.selected_item == -1);
    assert(panel.active_control_index == -1);
    for (index = 0; index < 4; ++index) {
        assert((panel.base.children[index]->flags & NTMV_M2D_UI_HIDDEN) == 0);
    }
    assert((panel.base.children[2]->flags &
            NTMV_M2D_UI_HIT_STATE_LOCKED) != 0);
    assert((panel.base.children[3]->flags &
            NTMV_M2D_UI_HIT_STATE_LOCKED) != 0);
    for (index = 0; index < 6; ++index) {
        assert(panel.row_buttons[index]->selected == 0);
        assert((panel.row_buttons[index]->base.base.flags &
                NTMV_M2D_UI_HIDDEN) != 0);
    }

    NtmvM2dItemsPanel_SetPageIndex(&panel, 2);
    assert(panel.page_index == 2);
    assert(panel.first_visible_item == 10);

    NtmvM2dItemsPanel_SetItemCount(&panel, 5);
    assert(panel.page_count == 1);
    assert(panel.page_index == 0);
    assert(panel.first_visible_item == 0);
    for (index = 0; index < 4; ++index) {
        assert((panel.base.children[index]->flags & NTMV_M2D_UI_HIDDEN) != 0);
    }

    /* The secondary record proves the -1 alternate path and eight rows. */
    ResetConfigurationCalls();
    NtmvM2dItemsPanel_Construct(&panel);
    NtmvM2dItemsPanel_Configure(
        &panel,
        &allocator,
        (void *)(uintptr_t)0x3333,
        &text_context,
        &secondary_definition);
    assert(panel.base.base.local_position.x == 137);
    assert(panel.base.base.local_position.y == -59);
    assert(panel.visible_row_count_minus_one == 7);
    assert(panel.base.child_count == 14);
    assert(sManualButtonConfigCount == 8);
    assert(sAnimationSequenceCount == 20);
    for (index = 0; index < 8; ++index) {
        assert(sManualButtonConfigs[index].position.x == 31);
        assert(sManualButtonConfigs[index].position.y ==
               (int16_t)(index * 0x17));
        assert(sManualButtonConfigs[index].animation_handle == 1034);
        assert(sManualButtonConfigs[index].alternate_animation_handle == -1);
        assert(sManualButtonConfigs[index].overlay_animation_handle ==
               (int32_t)(1013 + index));
        assert(panel.row_item_indices[index] == -1);
    }
    assert(sObjButtonConfigs[0].position.x == 31);
    assert(sObjButtonConfigs[1].position.x == 31);
    assert(panel.base.children[12] == NULL);
    assert(panel.base.children[13] == NULL);

    /* visible_row_count_minus_one == -1 is the exact zero-row branch. */
    ResetConfigurationCalls();
    NtmvM2dItemsPanel_Construct(&panel);
    NtmvM2dItemsPanel_Configure(
        &panel,
        &allocator,
        (void *)(uintptr_t)0x3333,
        &text_context,
        &zero_row_definition);
    assert(panel.base.base.local_position.x == -4);
    assert(panel.base.base.local_position.y == 9);
    assert(panel.visible_row_count_minus_one == -1);
    assert(panel.base.child_count == 4);
    assert(panel.row_buttons != NULL);
    assert(panel.row_item_indices != NULL);
    assert(sManualButtonConfigCount == 0);
    assert(sObjButtonConfigCount == 2);
    assert(sStaticConfigCount == 2);
    assert(sAnimationSequenceCount == 4);
    assert(sAnimationSequences[0] == 33 && sAnimationSequences[1] == 32 &&
           sAnimationSequences[2] == 39 && sAnimationSequences[3] == 39);
}

static void TestSelectionSetter(void)
{
    NtmvM2dItemsPanel panel;
    NtmvM2dManualButton buttons[3];
    NtmvM2dManualButton *row_buttons[3];
    unsigned int index;

    NtmvM2dItemsPanel_Construct(&panel);
    panel.visible_row_count_minus_one = 2;
    panel.first_visible_item = 3;
    panel.row_buttons = row_buttons;
    for (index = 0; index < 3; ++index) {
        memset(&buttons[index], 0, sizeof(buttons[index]));
        NtmvM2dUIElement_Init(&buttons[index].base.base);
        row_buttons[index] = &buttons[index];
    }

    /* Old item 4 maps to slot 1; new item 3 maps to slot 0. */
    panel.selected_item = 4;
    panel.interaction_enabled = 1;
    buttons[1].selected = 1;
    assert(NtmvM2dItemsPanel_SetSelectedItem(&panel, 3));
    assert(panel.selected_item == 3);
    assert(panel.interaction_enabled == 1);
    assert(buttons[0].selected == 1);
    assert(buttons[1].selected == 0);
    assert(buttons[2].selected == 0);

    /* Identical active selection is the sole no-change path. */
    assert(!NtmvM2dItemsPanel_SetSelectedItem(&panel, 3));
    assert(panel.selected_item == 3);
    assert(buttons[0].selected == 1);

    /* The upper bound is strict: first 3 + (+0x24) 2 excludes item 5. */
    assert(NtmvM2dItemsPanel_SetSelectedItem(&panel, 5));
    assert(panel.selected_item == 5);
    assert(buttons[0].selected == 0);
    assert(buttons[1].selected == 0);
    assert(buttons[2].selected == 0);

    /* -1 clears the logical selection; an out-of-range old item has no row. */
    assert(NtmvM2dItemsPanel_SetSelectedItem(&panel, -1));
    assert(panel.selected_item == -1);
    assert(buttons[0].selected == 0);
    assert(buttons[1].selected == 0);
    assert(buttons[2].selected == 0);

    /* No old selection still permits selecting a visible row. */
    assert(NtmvM2dItemsPanel_SetSelectedItem(&panel, 4));
    assert(panel.selected_item == 4);
    assert(buttons[1].selected == 1);

    /* Same item while disabled re-enables interaction and performs refresh. */
    panel.interaction_enabled = 0;
    assert(NtmvM2dItemsPanel_SetSelectedItem(&panel, 4));
    assert(panel.interaction_enabled == 1);
    assert(panel.selected_item == 4);
    assert(buttons[1].selected == 1);

    /* Below the current page clears the old row but sets no new visual. */
    assert(NtmvM2dItemsPanel_SetSelectedItem(&panel, 2));
    assert(panel.selected_item == 2);
    assert(buttons[0].selected == 0);
    assert(buttons[1].selected == 0);
    assert(buttons[2].selected == 0);
}

static void TestPageButtonLocks(void)
{
    NtmvM2dItemsPanel panel;
    NtmvM2dUIElement previous;
    NtmvM2dUIElement next;
    NtmvM2dUIElement *children[2] = {&previous, &next};

    NtmvM2dItemsPanel_Construct(&panel);
    NtmvM2dUIElement_Init(&previous);
    NtmvM2dUIElement_Init(&next);
    panel.base.children = children;

    /* A unique page returns before touching either control. */
    panel.page_count = 1;
    panel.page_index = 0;
    previous.flags = NTMV_M2D_UI_HIDDEN |
        NTMV_M2D_UI_HIT_STATE_LOCKED | NTMV_M2D_UI_HOVERED;
    next.flags = NTMV_M2D_UI_HOVERED;
    NtmvM2dItemsPanel_UpdatePageButtonLocks_020b97d4(&panel);
    assert(previous.flags ==
           (NTMV_M2D_UI_HIDDEN | NTMV_M2D_UI_HIT_STATE_LOCKED |
            NTMV_M2D_UI_HOVERED));
    assert(next.flags == NTMV_M2D_UI_HOVERED);

    /* First page locks previous and unlocks next, preserving other bits. */
    panel.page_count = 4;
    panel.page_index = 0;
    previous.flags = NTMV_M2D_UI_HIDDEN | NTMV_M2D_UI_HOVERED;
    next.flags = NTMV_M2D_UI_HIDDEN |
        NTMV_M2D_UI_HIT_STATE_LOCKED | NTMV_M2D_UI_HOVERED;
    NtmvM2dItemsPanel_UpdatePageButtonLocks_020b97d4(&panel);
    assert(previous.flags ==
           (NTMV_M2D_UI_HIDDEN | NTMV_M2D_UI_HIT_STATE_LOCKED |
            NTMV_M2D_UI_HOVERED));
    assert(next.flags == (NTMV_M2D_UI_HIDDEN | NTMV_M2D_UI_HOVERED));

    /* Intermediate page unlocks both controls. */
    panel.page_index = 2;
    previous.flags |= NTMV_M2D_UI_HIT_STATE_LOCKED;
    next.flags |= NTMV_M2D_UI_HIT_STATE_LOCKED;
    NtmvM2dItemsPanel_UpdatePageButtonLocks_020b97d4(&panel);
    assert((previous.flags & NTMV_M2D_UI_HIT_STATE_LOCKED) == 0);
    assert((next.flags & NTMV_M2D_UI_HIT_STATE_LOCKED) == 0);
    assert((previous.flags &
            (NTMV_M2D_UI_HIDDEN | NTMV_M2D_UI_HOVERED)) ==
           (NTMV_M2D_UI_HIDDEN | NTMV_M2D_UI_HOVERED));

    /* Last page unlocks previous and locks next. */
    panel.page_index = 3;
    NtmvM2dItemsPanel_UpdatePageButtonLocks_020b97d4(&panel);
    assert((previous.flags & NTMV_M2D_UI_HIT_STATE_LOCKED) == 0);
    assert((next.flags & NTMV_M2D_UI_HIT_STATE_LOCKED) != 0);
}

static void TestPageSynchronization(void)
{
    NtmvM2dItemsPanel panel;

    NtmvM2dItemsPanel_Construct(&panel);
    panel.visible_row_count_minus_one = 5;
    panel.page_index = 0;
    panel.target_scroll_y = -123;
    panel.selected_item = 11;

    /* 11 / page step 5 selects page 2; pages overlap one visible row. */
    assert(NtmvM2dItemsPanel_SyncPageToSelection(&panel));
    assert(panel.page_index == 2);
    assert(panel.first_visible_item == 10);
    assert(panel.target_scroll_y == 230);

    /* Identical page leaves both target scroll and lock state untouched. */
    panel.target_scroll_y = 777;
    assert(!NtmvM2dItemsPanel_SyncPageToSelection(&panel));
    assert(panel.page_index == 2);
    assert(panel.target_scroll_y == 777);

    /* Selection immediately below/at the step proves quotient boundaries. */
    panel.selected_item = 4;
    assert(NtmvM2dItemsPanel_SyncPageToSelection(&panel));
    assert(panel.page_index == 0);
    assert(panel.first_visible_item == 0);
    assert(panel.target_scroll_y == 0);

    panel.target_scroll_y = 99;
    assert(!NtmvM2dItemsPanel_SyncPageToSelection(&panel));
    assert(panel.target_scroll_y == 99);

    panel.selected_item = 5;
    assert(NtmvM2dItemsPanel_SyncPageToSelection(&panel));
    assert(panel.page_index == 1);
    assert(panel.first_visible_item == 5);
    assert(panel.target_scroll_y == 115);

    panel.selected_item = 9;
    panel.target_scroll_y = 321;
    assert(!NtmvM2dItemsPanel_SyncPageToSelection(&panel));
    assert(panel.page_index == 1);
    assert(panel.target_scroll_y == 321);

    /* No selection is another strict no-change path. */
    panel.selected_item = -1;
    assert(!NtmvM2dItemsPanel_SyncPageToSelection(&panel));
    assert(panel.page_index == 1);
    assert(panel.target_scroll_y == 321);

    /* Direct helper has the same identical/changed target-scroll contract. */
    assert(!NtmvM2dItemsPanel_SetPageIndexIfChanged(&panel, 1));
    assert(panel.target_scroll_y == 321);
    assert(NtmvM2dItemsPanel_SetPageIndexIfChanged(&panel, 3));
    assert(panel.page_index == 3);
    assert(panel.first_visible_item == 15);
    assert(panel.target_scroll_y == 345);
}

static void AssertPointerAction(
    const NtmvM2dItemsPointerAction *action,
    uint8_t page_changed,
    uint8_t selection_changed,
    uint8_t row_activated,
    uint8_t control_hit,
    uint8_t control_pressed)
{
    assert(action->page_changed == page_changed);
    assert(action->selection_changed == selection_changed);
    assert(action->row_activated == row_activated);
    assert(action->control_hit == control_hit);
    assert(action->control_pressed == control_pressed);
}

static void TestPointerHandler(void)
{
    NtmvM2dItemsPanel panel;
    NtmvM2dUIElement fixed_controls[4];
    NtmvM2dManualButton row_controls[3];
    NtmvM2dManualButton *row_buttons[3];
    NtmvM2dUIElement *children[7];
    NtmvM2dItemsPointerState pointer = {{0, 0}, 0, 0, 0, 0};
    NtmvM2dItemsPointerAction action;
    unsigned int index;

    NtmvM2dItemsPanel_Construct(&panel);
    panel.base.children = children;
    panel.base.child_count = 7;
    panel.visible_row_count_minus_one = 2;
    panel.row_buttons = row_buttons;
    panel.active_control_index = -1;
    panel.scroll_y = 0;
    panel.target_scroll_y = 0;
    panel.page_index = 1;
    panel.first_visible_item = 2;
    panel.interaction_enabled = 1;
    panel.selected_item = -1;
    for (index = 0; index < 4; ++index) {
        NtmvM2dUIElement_Init(&fixed_controls[index]);
        fixed_controls[index].world_center.x = (int16_t)(index * 20);
        fixed_controls[index].world_center.y = 0;
        fixed_controls[index].size.width = 10;
        fixed_controls[index].size.height = 10;
        children[index] = &fixed_controls[index];
    }
    for (index = 0; index < 3; ++index) {
        memset(&row_controls[index], 0, sizeof(row_controls[index]));
        NtmvM2dUIElement_Init(&row_controls[index].base.base);
        row_controls[index].base.base.world_center.x =
            (int16_t)((index + 4) * 20);
        row_controls[index].base.base.world_center.y = 0;
        row_controls[index].base.base.size.width = 10;
        row_controls[index].base.base.size.height = 10;
        row_buttons[index] = &row_controls[index];
        children[index + 4] = &row_controls[index].base.base;
    }

    /* Hidden or moving panels reject input after clearing all five bytes. */
    memset(&action, 0xa5, sizeof(action));
    panel.base.base.flags = NTMV_M2D_UI_HIDDEN;
    pointer.active = 1;
    pointer.pressed = 1;
    assert(!NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    AssertPointerAction(&action, 0, 0, 0, 0, 0);
    panel.base.base.flags = 0;

    memset(&action, 0xa5, sizeof(action));
    panel.target_scroll_y = 1;
    assert(!NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    AssertPointerAction(&action, 0, 0, 0, 0, 0);
    panel.target_scroll_y = 0;

    /* A miss is unhandled; an unlocked press captures and hovers child 0. */
    pointer.position.x = 500;
    pointer.position.y = 500;
    memset(&action, 0xa5, sizeof(action));
    assert(!NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    AssertPointerAction(&action, 0, 0, 0, 0, 0);

    pointer.position = fixed_controls[0].world_center;
    memset(&action, 0xa5, sizeof(action));
    assert(NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    AssertPointerAction(&action, 0, 0, 0, 1, 1);
    assert(panel.active_control_index == 0);
    assert((fixed_controls[0].flags & NTMV_M2D_UI_HOVERED) != 0);

    /* Release on hovered previous-page changes page 1 -> 0. */
    pointer.active = 0;
    pointer.pressed = 0;
    pointer.released = 1;
    memset(&action, 0xa5, sizeof(action));
    assert(NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    AssertPointerAction(&action, 1, 0, 0, 0, 0);
    assert(panel.active_control_index == -1);
    assert((fixed_controls[0].flags & NTMV_M2D_UI_HOVERED) == 0);
    assert(panel.page_index == 0);
    assert(panel.first_visible_item == 0);
    assert(panel.target_scroll_y == 0);

    /* Child 1 increments page 0 -> 1 and sets the 23-pixel scroll target. */
    pointer.position = fixed_controls[1].world_center;
    pointer.active = 1;
    pointer.pressed = 1;
    pointer.released = 0;
    assert(NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    pointer.active = 0;
    pointer.pressed = 0;
    pointer.released = 1;
    assert(NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    AssertPointerAction(&action, 1, 0, 0, 0, 0);
    assert(panel.page_index == 1);
    assert(panel.first_visible_item == 2);
    assert(panel.target_scroll_y == 46);

    /* Locked navigation/rows report a hit but never capture; separators
     * 2/3 are deliberately silent even though the function returns true. */
    panel.scroll_y = panel.target_scroll_y;
    fixed_controls[0].flags = NTMV_M2D_UI_HIT_STATE_LOCKED;
    pointer.position = fixed_controls[0].world_center;
    pointer.active = 1;
    pointer.pressed = 1;
    pointer.released = 0;
    assert(NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    AssertPointerAction(&action, 0, 0, 0, 1, 0);
    assert(panel.active_control_index == -1);
    fixed_controls[0].flags = 0;

    fixed_controls[2].flags = NTMV_M2D_UI_HIT_STATE_LOCKED;
    pointer.position = fixed_controls[2].world_center;
    assert(NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    AssertPointerAction(&action, 0, 0, 0, 0, 0);
    assert(panel.active_control_index == -1);

    row_controls[0].base.base.flags = NTMV_M2D_UI_HIT_STATE_LOCKED;
    pointer.position = row_controls[0].base.base.world_center;
    assert(NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    AssertPointerAction(&action, 0, 0, 0, 1, 0);
    row_controls[0].base.base.flags = 0;

    /* Circular mapping: first_visible 4 has slot 1, so physical child 5
     * resolves back to item 4 and marks both row/result selection state. */
    panel.scroll_y = 0;
    panel.target_scroll_y = 0;
    panel.first_visible_item = 4;
    panel.selected_item = -1;
    panel.interaction_enabled = 1;
    pointer.position = row_controls[1].base.base.world_center;
    pointer.active = 1;
    pointer.pressed = 1;
    assert(NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    AssertPointerAction(&action, 0, 0, 0, 1, 1);
    pointer.active = 0;
    pointer.pressed = 0;
    pointer.released = 1;
    assert(NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    AssertPointerAction(&action, 0, 1, 1, 0, 0);
    assert(panel.selected_item == 4);
    assert(row_controls[1].selected == 1);

    /* Releasing the same row is activated but does not change selection. */
    pointer.active = 1;
    pointer.pressed = 1;
    pointer.released = 0;
    assert(NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    pointer.active = 0;
    pointer.pressed = 0;
    pointer.released = 1;
    assert(NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    AssertPointerAction(&action, 0, 0, 1, 0, 0);
    assert(panel.selected_item == 4);

    /* Moving a captured row outside clears hover; release has no action. */
    pointer.position = row_controls[2].base.base.world_center;
    pointer.active = 1;
    pointer.pressed = 1;
    pointer.released = 0;
    assert(NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    assert(panel.active_control_index == 6);
    pointer.position.x = 500;
    pointer.position.y = 500;
    pointer.pressed = 0;
    memset(&action, 0xa5, sizeof(action));
    assert(NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    AssertPointerAction(&action, 0, 0, 0, 0, 0);
    assert((row_controls[2].base.base.flags & NTMV_M2D_UI_HOVERED) == 0);
    pointer.active = 0;
    pointer.released = 1;
    assert(NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    AssertPointerAction(&action, 0, 0, 0, 0, 0);
    assert(panel.active_control_index == -1);

    /* Release without capture remains unhandled. */
    memset(&action, 0xa5, sizeof(action));
    assert(!NtmvM2dItemsPanel_HandlePointer(&panel, &action, &pointer));
    AssertPointerAction(&action, 0, 0, 0, 0, 0);
}

static void TestRowUpdateCluster(void)
{
    static const uint16_t first_text[] = {'A', 0};
    static const uint16_t second_text[] = {'B', 0};
    static const uint16_t third_text[] = {'C', 0};
    NtmvM2dItemsFontMetadata metadata = {{0}, 3};
    NtmvM2dItemsFontResource resource = {{0}, &metadata};
    NtmvM2dItemsFont font = {&resource};
    NtmvM2dItemsTextContext context = {NULL, &font};
    NtmvM2dItemsPanel panel;
    NtmvM2dManualButton buttons[3];
    NtmvM2dManualButton *row_buttons[3];
    int16_t row_indices[3] = {-1, -1, -1};
    NtmvM2dItemsRow rows[2] = {
        {first_text, 1, {0, 0, 0}},
        {second_text, 0, {0, 0, 0}},
    };
    NtmvM2dItemsRow third_row = {third_text, 1, {0, 0, 0}};
    unsigned int index;

    NtmvM2dItemsPanel_Construct(&panel);
    panel.visible_row_count_minus_one = 2;
    panel.text_context = &context;
    panel.row_buttons = row_buttons;
    panel.row_item_indices = row_indices;
    for (index = 0; index < 3; ++index) {
        memset(&buttons[index], 0, sizeof(buttons[index]));
        NtmvM2dUIElement_Init(&buttons[index].base.base);
        buttons[index].base.base.flags = NTMV_M2D_UI_HIDDEN;
        row_buttons[index] = &buttons[index];
    }

    /* The ROM clamps 99 to +0x24 (2), then rotates slots by item modulo 3. */
    NtmvM2dItemsPanel_UpdateRows(&panel, rows, 99, 4);
    assert(sCanvasInitCount == 4 && sClearCount == 4);
    assert(sFormatCount == 2 && sRectCount == 2 && sTextCount == 2);
    assert(sAlternateCallCount == 2);
    assert(sCanvasInits[0].vram == UINT32_C(0x06400c00));
    assert(sCanvasInits[0].width == 4 && sCanvasInits[0].height == 4);
    assert(sCanvasInits[1].vram == UINT32_C(0x06400e00));
    assert(sCanvasInits[1].width == 0x14 && sCanvasInits[1].height == 4);
    assert(sCanvasInits[2].vram == UINT32_C(0x06401800));
    assert(sCanvasInits[3].vram == UINT32_C(0x06401a00));
    assert(sFormattedValues[0] == 5 && sFormattedValues[1] == 6);
    assert(sRectFirstCharacters[0] == '5');
    assert(sRectFirstCharacters[1] == '6');
    assert(sDrawnTexts[0] == first_text && sDrawnTexts[1] == second_text);
    assert(sRectDirections[0].x == -1 && sRectDirections[0].y == 0);
    assert(sTextDirections[1].x == -1 && sTextDirections[1].y == 0);
    assert(row_indices[0] == -1 && row_indices[1] == 4 && row_indices[2] == 5);
    assert(buttons[1].use_alternate_animation == 1);
    assert(buttons[2].use_alternate_animation == 0);
    assert((buttons[0].base.base.flags & NTMV_M2D_UI_HIDDEN) != 0);
    assert((buttons[1].base.base.flags & NTMV_M2D_UI_HIDDEN) == 0);
    assert((buttons[2].base.base.flags & NTMV_M2D_UI_HIDDEN) == 0);

    /* Out-of-range font orientation falls back to direction (0, 0). */
    metadata.orientation = 9;
    NtmvM2dItemsPanel_UpdateTextRow(&panel, &third_row, 6);
    assert(sCanvasInitCount == 6 && sClearCount == 6);
    assert(sFormatCount == 3 && sFormattedValues[2] == 7);
    assert(sRectDirections[2].x == 0 && sRectDirections[2].y == 0);
    assert(sTextDirections[2].x == 0 && sTextDirections[2].y == 0);
    assert(sDrawnTexts[2] == third_text);
    assert(row_indices[0] == 6);
    assert(buttons[0].use_alternate_animation == 1);
    assert((buttons[0].base.base.flags & NTMV_M2D_UI_HIDDEN) == 0);

    /* 0x020b9b58 uses the lower tile area and omits number formatting. */
    ResetTextCalls();
    metadata.orientation = 0;
    row_indices[0] = -1;
    row_indices[1] = -1;
    row_indices[2] = -1;
    buttons[0].base.base.flags = 0;
    buttons[1].base.base.flags = NTMV_M2D_UI_HIDDEN;
    buttons[2].base.base.flags = NTMV_M2D_UI_HIDDEN;
    NtmvM2dItemsPanel_UpdateSimpleRows(&panel, rows, 99, 7);
    assert(sCanvasInitCount == 2 && sClearCount == 2);
    assert(sFormatCount == 0 && sRectCount == 0 && sTextCount == 2);
    assert(sAlternateCallCount == 2);
    assert(sCanvasInits[0].vram == UINT32_C(0x06405400));
    assert(sCanvasInits[1].vram == UINT32_C(0x06406000));
    assert(sCanvasInits[0].width == 0x18 && sCanvasInits[0].height == 4);
    assert(sDrawnTexts[0] == first_text && sDrawnTexts[1] == second_text);
    assert(sTextDirections[0].x == 1 && sTextDirections[0].y == 0);
    assert(sTextDirections[1].x == 1 && sTextDirections[1].y == 0);
    assert(row_indices[0] == -1 && row_indices[1] == 7 && row_indices[2] == 8);
    assert((buttons[0].base.base.flags & NTMV_M2D_UI_HIDDEN) != 0);
    assert((buttons[1].base.base.flags & NTMV_M2D_UI_HIDDEN) == 0);
    assert((buttons[2].base.base.flags & NTMV_M2D_UI_HIDDEN) == 0);
}

static void TestScrollAnimation(void)
{
    NtmvM2dItemsFontMetadata metadata = {{0}, 7};
    NtmvM2dItemsFontResource resource = {{0}, &metadata};
    NtmvM2dItemsFont font = {&resource};
    NtmvM2dItemsTextContext context = {NULL, &font};
    NtmvM2dItemsPanel panel;
    NtmvM2dManualButton buttons[3];
    NtmvM2dManualButton *row_buttons[3];
    int16_t row_indices[3] = {-1, -1, -1};
    unsigned int index;

    NtmvM2dItemsPanel_Construct(&panel);
    panel.visible_row_count_minus_one = 2;
    panel.item_count = 4;
    panel.selected_item = 1;
    panel.interaction_enabled = 1;
    panel.scroll_y = 0;
    panel.target_scroll_y = 35;
    panel.text_context = &context;
    panel.row_buttons = row_buttons;
    panel.row_item_indices = row_indices;
    for (index = 0; index < 3; ++index) {
        memset(&buttons[index], 0, sizeof(buttons[index]));
        NtmvM2dUIElement_Init(&buttons[index].base.base);
        buttons[index].base.base.local_position.x = (int16_t)(20 + index);
        buttons[index].base.base.local_position.y = 99;
        buttons[index].selected = 1;
        row_buttons[index] = &buttons[index];
    }

    ResetTextCalls();
    sProviderCallCount = 0;
    assert(NtmvM2dItemsPanel_AnimateScroll(
        &panel, ProvideRow, (void *)(uintptr_t)0x3333));
    assert(panel.scroll_y == 12);
    assert(sProviderCallCount == 3);
    assert(sProviderIndices[0] == 0);
    assert(sProviderIndices[1] == 1);
    assert(sProviderIndices[2] == 2);
    assert(sCanvasInitCount == 6 && sFormatCount == 3 && sTextCount == 3);
    assert(row_indices[0] == 0 && row_indices[1] == 1 && row_indices[2] == 2);
    assert(buttons[0].base.base.local_position.x == 20);
    assert(buttons[1].base.base.local_position.x == 21);
    assert(buttons[2].base.base.local_position.x == 22);
    assert(buttons[0].base.base.local_position.y == -12);
    assert(buttons[1].base.base.local_position.y == 11);
    assert(buttons[2].base.base.local_position.y == 34);
    assert(buttons[0].selected == 0);
    assert(buttons[1].selected == 1);
    assert(buttons[2].selected == 0);
    assert((buttons[0].base.base.flags & NTMV_M2D_UI_HIDDEN) == 0);
    assert((buttons[1].base.base.flags & NTMV_M2D_UI_HIDDEN) == 0);
    assert((buttons[2].base.base.flags & NTMV_M2D_UI_HIDDEN) == 0);

    /* Crossing one 23-pixel row rotates only the newly mapped slot. */
    ResetTextCalls();
    sProviderCallCount = 0;
    assert(NtmvM2dItemsPanel_AnimateScroll(
        &panel, ProvideRow, (void *)(uintptr_t)0x3333));
    assert(panel.scroll_y == 24);
    assert(sProviderCallCount == 1 && sProviderIndices[0] == 3);
    assert(sCanvasInitCount == 2 && sFormatCount == 1 && sTextCount == 1);
    assert(row_indices[0] == 3 && row_indices[1] == 1 && row_indices[2] == 2);
    assert(buttons[1].base.base.local_position.y == -1);
    assert(buttons[2].base.base.local_position.y == 22);
    assert(buttons[0].base.base.local_position.y == 45);

    panel.target_scroll_y = 24;
    sProviderCallCount = 0;
    assert(!NtmvM2dItemsPanel_AnimateScroll(
        &panel, ProvideRow, (void *)(uintptr_t)0x3333));
    assert(sProviderCallCount == 0);

    /* Both directions saturate at the target instead of overshooting. */
    panel.target_scroll_y = 30;
    assert(NtmvM2dItemsPanel_AnimateScroll(
        &panel, ProvideRow, (void *)(uintptr_t)0x3333));
    assert(panel.scroll_y == 30);
    panel.target_scroll_y = 25;
    assert(NtmvM2dItemsPanel_AnimateScroll(
        &panel, ProvideRow, (void *)(uintptr_t)0x3333));
    assert(panel.scroll_y == 25);

    /* Out-of-range items are hidden and do not invoke the provider. */
    panel.item_count = 2;
    panel.target_scroll_y = 24;
    sProviderCallCount = 0;
    assert(NtmvM2dItemsPanel_AnimateScroll(
        &panel, ProvideRow, (void *)(uintptr_t)0x3333));
    assert(panel.scroll_y == 24 && sProviderCallCount == 0);
    assert((buttons[1].base.base.flags & NTMV_M2D_UI_HIDDEN) == 0);
    assert((buttons[2].base.base.flags & NTMV_M2D_UI_HIDDEN) != 0);
    assert((buttons[0].base.base.flags & NTMV_M2D_UI_HIDDEN) != 0);

    panel.item_count = 4;
    panel.interaction_enabled = 0;
    panel.target_scroll_y = 23;
    assert(NtmvM2dItemsPanel_AnimateScroll(
        &panel, ProvideRow, (void *)(uintptr_t)0x3333));
    assert(buttons[1].selected == 0);
}

static void TestTocRowProvider(void)
{
    static const uint16_t title[] = {'T', 'i', 't', 'l', 'e', 0};
    static const uint16_t empty[] = {0};
    NtmvM2dItemsTocRecord records[2];
    NtmvM2dItemsTocData toc_data;
    NtmvM2dItemsRow row;

    memset(records, 0, sizeof(records));
    records[0].text = title;
    records[0].child_count = 0;
    records[1].text = empty;
    records[1].child_count = 3;
    toc_data.records = records;
    toc_data.record_count = 2;
    toc_data.reserved_06 = 0;

    memset(&row, 0xa5, sizeof(row));
    NtmvM2dItemsPanel_BuildTocRow(&row, &toc_data, 0);
    assert(row.text == title);
    assert(row.use_alternate_animation == 0);
    assert(row.reserved_05[0] == 0xa5);
    assert(row.reserved_05[1] == 0xa5);
    assert(row.reserved_05[2] == 0xa5);

    memset(&row, 0xa5, sizeof(row));
    NtmvM2dItemsPanel_BuildTocRow(&row, &toc_data, 1);
    assert(row.text != empty);
    assert(row.text[0] == '(');
    assert(row.text[1] == '-');
    assert(row.text[2] == '-');
    assert(row.text[3] == '-');
    assert(row.text[4] == ')');
    assert(row.text[5] == 0);
    assert(row.use_alternate_animation == 1);
    assert(row.reserved_05[0] == 0xa5);
}

int main(void)
{
    TestArrayFreeHelpers();
    TestConstructorRenderAndDestroy();
    TestBoundedHelpers();
    TestConfigure();
    TestSelectionSetter();
    TestPageButtonLocks();
    TestPageSynchronization();
    TestPointerHandler();
    TestRowUpdateCluster();
    TestScrollAnimation();
    TestTocRowProvider();
    return 0;
}
