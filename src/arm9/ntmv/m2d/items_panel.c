#include "ntmv/m2d/items_panel.h"
#include "ntmv/m2d/obj_button.h"
#include "ntmv/m2d/ui_static.h"
#include "ntmv/ui_renderer.h"

#include <stddef.h>

typedef struct NtmvM2dItemsCharCanvas NtmvM2dItemsCharCanvas;

typedef struct NtmvM2dItemsCharCanvasOps {
    void (*unknown_00)(NtmvM2dItemsCharCanvas *canvas);
    void (*clear)(NtmvM2dItemsCharCanvas *canvas);
} NtmvM2dItemsCharCanvasOps;

struct NtmvM2dItemsCharCanvas {
    uint8_t unknown_00[0x14];
    const NtmvM2dItemsCharCanvasOps *ops;
};

typedef struct NtmvM2dItemsTextCanvasState {
    NtmvM2dItemsCharCanvas *canvas;
    const NtmvM2dItemsFont *font;
    int32_t character_spacing;
    uint32_t reserved_0c;
} NtmvM2dItemsTextCanvasState;

typedef struct NtmvM2dItemsObjTextCanvas {
    NtmvM2dItemsCharCanvas canvas;
    NtmvM2dItemsTextCanvasState text_canvas;
} NtmvM2dItemsObjTextCanvas;

typedef struct NtmvM2dItemsTextDirection {
    int8_t x;
    int8_t y;
} NtmvM2dItemsTextDirection;

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dItemsCharCanvasTargetSizeCheck[
    sizeof(NtmvM2dItemsCharCanvas) == 0x18 ? 1 : -1];
typedef char NtmvM2dItemsTextCanvasTargetSizeCheck[
    sizeof(NtmvM2dItemsTextCanvasState) == 0x10 ? 1 : -1];
typedef char NtmvM2dItemsObjTextCanvasTargetSizeCheck[
    sizeof(NtmvM2dItemsObjTextCanvas) == 0x28 ? 1 : -1];
#endif

extern void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size);
extern void NNS_FndFreeToAllocator(void *allocator, void *memory);
extern void NNS_G2dCharCanvasInitForOBJ1D(
    NtmvM2dItemsCharCanvas *canvas,
    void *vram,
    int32_t width_tiles,
    int32_t height_tiles,
    uint32_t color_depth);
extern int NtmvM2d_FormatWideText_0200a324(
    uint16_t *output, uint32_t capacity, const uint16_t *format, ...);
extern void NNSi_G2dTextCanvasDrawTextRect_020c6b50(
    NtmvM2dItemsTextCanvasState *canvas,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height,
    uint32_t flags,
    uint32_t draw_value,
    const uint16_t *text,
    NtmvM2dItemsTextDirection direction);
extern void NNSi_G2dTextCanvasDrawText_020c6a2c(
    NtmvM2dItemsTextCanvasState *canvas,
    int32_t x,
    int32_t y,
    uint32_t flags,
    uint32_t draw_value,
    const uint16_t *text,
    NtmvM2dItemsTextDirection direction);
const NtmvM2dUIPanelVTable gNtmvM2dItemsPanelVTable = {
    (void (*)(NtmvM2dUIPanel *, NtmvM2dAllocatorContext *))
        NtmvM2dItemsPanel_Destroy, /* slot 0: 0x020b931c */
    NtmvM2dUIPanel_Render,         /* slot 1: 0x020b81dc, inherited */
    (void (*)(NtmvM2dUIPanel *, const NtmvM2dPoint *))
        NtmvM2dUIElement_SetPosition, /* slot 2: 0x020b7e48 */
    (void (*)(NtmvM2dUIPanel *, const NtmvM2dSize *))
        NtmvM2dUIElement_SetSize, /* slot 3: 0x020b7e5c */
    (void (*)(NtmvM2dUIPanel *, bool))
        NtmvM2dUIElement_SetVisible, /* slot 4: 0x020b7e70 */
};

/* 0x020b8f0c..0x020b8f63; address-point literal 0x020b8f64 excluded. */
NtmvM2dItemsPanel *NtmvM2dItemsPanel_Construct(NtmvM2dItemsPanel *panel)
{
    NtmvM2dUIPanel_ConstructBase(&panel->base);
    panel->base.base.vtable =
        (const NtmvM2dUIElementVTable *)&gNtmvM2dItemsPanelVTable;
    panel->text_context = NULL;
    panel->item_count = 0;
    panel->first_visible_item = 0;
    panel->visible_row_count_minus_one = 0;
    panel->page_index = 0;
    panel->page_count = 0;
    panel->selected_item = -1;
    panel->active_control_index = -1;
    panel->row_buttons = NULL;
    panel->scroll_y = 0;
    panel->target_scroll_y = 0;
    panel->interaction_enabled = true;
    return panel;
}

typedef struct NtmvM2dItemsFixedButtonDefinition {
    uint16_t animation_sequence;
    int16_t center_y;
} NtmvM2dItemsFixedButtonDefinition;

static const NtmvM2dItemsFixedButtonDefinition sFixedButtonDefinitions[2] = {
    {33, -23},
    {32, 115},
};

static const int16_t sSeparatorCenterY[2] = {-29, 121};

static NtmvM2dUIElement **AllocateChildren(
    NtmvM2dAllocatorContext *allocator, uint16_t child_count)
{
    uint32_t allocation_count = child_count != 0 ? child_count : 1;
    NtmvM2dUIElement **children = NNS_FndAllocFromAllocator(
        allocator->nns_allocator, allocation_count * sizeof(*children));
    uint16_t index;

    if (children == NULL) {
        return NULL;
    }
    for (index = 0; index < child_count; ++index) {
        children[index] = NULL;
    }
    return children;
}

/* 0x020b9258 */
static NtmvM2dManualButton *AllocateManualButton(
    NtmvM2dAllocatorContext *allocator)
{
    NtmvM2dManualButton *button = NNS_FndAllocFromAllocator(
        allocator->nns_allocator, (uint32_t)sizeof(*button));

    if (button != NULL) {
        NtmvM2dManualButton_Construct(button);
    }
    return button;
}

/* 0x020b9284 */
static int16_t *AllocateRowItemIndices(
    NtmvM2dAllocatorContext *allocator, uint16_t row_count)
{
    uint32_t allocation_count = row_count != 0 ? row_count : 1;
    int16_t *indices = NNS_FndAllocFromAllocator(
        allocator->nns_allocator, allocation_count * sizeof(*indices));
    uint16_t index;

    if (indices == NULL) {
        return NULL;
    }
    for (index = 0; index < row_count; ++index) {
        indices[index] = -1;
    }
    return indices;
}

/* 0x020b92d0 */
static NtmvM2dManualButton **AllocateRowButtons(
    NtmvM2dAllocatorContext *allocator, uint16_t row_count)
{
    uint32_t allocation_count = row_count != 0 ? row_count : 1;
    NtmvM2dManualButton **buttons = NNS_FndAllocFromAllocator(
        allocator->nns_allocator, allocation_count * sizeof(*buttons));
    uint16_t index;

    if (buttons == NULL) {
        return NULL;
    }
    for (index = 0; index < row_count; ++index) {
        buttons[index] = NULL;
    }
    return buttons;
}

static NtmvM2dObjButton *AllocateObjButton(
    NtmvM2dAllocatorContext *allocator)
{
    NtmvM2dObjButton *button = NNS_FndAllocFromAllocator(
        allocator->nns_allocator, (uint32_t)sizeof(*button));

    if (button != NULL) {
        NtmvM2dObjButton_ConstructComplete(button);
    }
    return button;
}

static NtmvM2dUIStatic *AllocateUIStatic(
    NtmvM2dAllocatorContext *allocator)
{
    NtmvM2dUIStatic *element = NNS_FndAllocFromAllocator(
        allocator->nns_allocator, (uint32_t)sizeof(*element));

    if (element != NULL) {
        NtmvM2dUIStatic_ConstructComplete(element);
    }
    return element;
}

/* 0x020b8f68..0x020b924f; literal pool 0x020b9250..0x020b9257 excluded. */
void NtmvM2dItemsPanel_Configure(
    NtmvM2dItemsPanel *panel,
    NtmvM2dAllocatorContext *allocator,
    void *animation_manager,
    const NtmvM2dItemsTextContext *text_context,
    const NtmvM2dItemsPanelDefinition *definition)
{
    uint16_t row_count =
        (uint16_t)(definition->visible_row_count_minus_one + 1);
    uint16_t child_count = (uint16_t)(
        definition->visible_row_count_minus_one +
        definition->auxiliary_child_count + 5);
    uint32_t child_index;
    uint16_t index;

    panel->base.base.local_position = definition->position;
    panel->base.base.size.width = 0x100;
    panel->base.base.size.height = 0xc0;
    panel->text_context = text_context;
    panel->visible_row_count_minus_one =
        definition->visible_row_count_minus_one;
    panel->base.child_count = child_count;
    panel->base.children = AllocateChildren(allocator, child_count);
    panel->row_buttons = AllocateRowButtons(allocator, row_count);
    panel->row_item_indices = AllocateRowItemIndices(allocator, row_count);

    /* The original repeats the allocator's null initialization here, after
     * all three arrays have been published in the panel. */
    for (child_index = 0; child_index < panel->base.child_count; ++child_index) {
        panel->base.children[child_index] = NULL;
    }

    for (index = 0; index < row_count; ++index) {
        NtmvM2dManualButton *button = AllocateManualButton(allocator);
        NtmvM2dPoint position = {
            definition->row_center_x,
            (int16_t)(index * 0x17),
        };
        const NtmvM2dSize size = {definition->row_width, 0x14};
        int32_t alternate_animation_handle = -1;
        int32_t animation_handle;
        int32_t overlay_animation_handle;

        panel->row_buttons[index] = button;
        if (definition->row_alternate_animation_sequence != -1) {
            alternate_animation_handle = NtmvUiRenderer_CreateAnimation(
                animation_manager,
                (uint16_t)definition->row_alternate_animation_sequence);
        }
        animation_handle = NtmvUiRenderer_CreateAnimation(
            animation_manager, definition->row_animation_sequence);
        overlay_animation_handle = NtmvUiRenderer_CreateAnimation(
            animation_manager,
            (uint16_t)(definition->first_row_overlay_sequence + index));
        NtmvM2dManualButton_Configure(
            button,
            &position,
            &size,
            animation_handle,
            alternate_animation_handle,
            overlay_animation_handle);
        panel->base.children[index + 4] = &button->base.base;
        panel->row_item_indices[index] = -1;
    }

    for (index = 0; index < 2; ++index) {
        const NtmvM2dItemsFixedButtonDefinition *fixed_definition =
            &sFixedButtonDefinitions[index];
        NtmvM2dObjButton *button = AllocateObjButton(allocator);
        NtmvM2dPoint position = {
            definition->row_center_x,
            fixed_definition->center_y,
        };
        const NtmvM2dSize size = {0x33, 0x14};
        int32_t animation_handle = NtmvUiRenderer_CreateAnimation(
            animation_manager, fixed_definition->animation_sequence);

        NtmvM2dObjButton_Configure(
            button, &position, &size, animation_handle);
        panel->base.children[index] = &button->base;
    }

    for (index = 0; index < 2; ++index) {
        NtmvM2dUIStatic *separator = AllocateUIStatic(allocator);
        const NtmvM2dPoint position = {-27, sSeparatorCenterY[index]};
        const NtmvM2dSize size = {0xc0, 0x20};
        int32_t animation_handle = NtmvUiRenderer_CreateAnimation(
            animation_manager, 0x27);

        NtmvM2dUIStatic_Configure(
            separator, &position, &size, animation_handle);
        NtmvM2dUIElement_SetVisible(&separator->base, false);
        panel->base.children[index + 2] = &separator->base;
    }
}

/* 0x020b9350..0x020b9367 */
void NtmvM2dItemsPanel_FreeRowButtons(
    NtmvM2dAllocatorContext *allocator,
    NtmvM2dManualButton **row_buttons)
{
    if (row_buttons != NULL) {
        NNS_FndFreeToAllocator(allocator->nns_allocator, row_buttons);
    }
}

/* 0x020b9368..0x020b937f */
void NtmvM2dItemsPanel_FreeRowItemIndices(
    NtmvM2dAllocatorContext *allocator,
    int16_t *row_item_indices)
{
    if (row_item_indices != NULL) {
        NNS_FndFreeToAllocator(allocator->nns_allocator, row_item_indices);
    }
}

/* 0x020b931c */
void NtmvM2dItemsPanel_Destroy(
    NtmvM2dItemsPanel *panel, NtmvM2dAllocatorContext *allocator)
{
    NtmvM2dItemsPanel_FreeRowItemIndices(
        allocator, panel->row_item_indices);
    NtmvM2dItemsPanel_FreeRowButtons(allocator, panel->row_buttons);
    NtmvM2dUIPanel_Destroy(&panel->base, allocator);
}

/* 0x020b9688 */
void NtmvM2dItemsPanel_UpdatePageControlVisibility(
    NtmvM2dItemsPanel *panel)
{
    bool multiple_pages = panel->page_count >= 2;
    uint16_t index;

    for (index = 0; index < 2; ++index) {
        NtmvM2dUIElement *button = panel->base.children[index];

        button->vtable->set_visible(button, multiple_pages);
    }
    for (index = 0; index < 2; ++index) {
        NtmvM2dUIElement *separator = panel->base.children[index + 2];

        separator->vtable->set_visible(separator, multiple_pages);
        separator->flags |= NTMV_M2D_UI_HIT_STATE_LOCKED;
    }
}

/* 0x020b9720 */
void NtmvM2dItemsPanel_SetPageIndex(
    NtmvM2dItemsPanel *panel, int16_t page_index)
{
    panel->page_index = page_index;
    panel->first_visible_item = (int16_t)(
        page_index * panel->visible_row_count_minus_one);
    NtmvM2dItemsPanel_UpdatePageButtonLocks_020b97d4(panel);
}

static void SetControlLocked(NtmvM2dUIElement *control, bool locked)
{
    control->flags = (uint8_t)(
        (control->flags & (uint8_t)~NTMV_M2D_UI_HIT_STATE_LOCKED) |
        (locked ? NTMV_M2D_UI_HIT_STATE_LOCKED : 0));
}

/* 0x020b97d4 */
void NtmvM2dItemsPanel_UpdatePageButtonLocks_020b97d4(
    NtmvM2dItemsPanel *panel)
{
    if (panel->page_count <= 1) {
        return;
    }

    SetControlLocked(
        panel->base.children[0], panel->page_index <= 0);
    SetControlLocked(
        panel->base.children[1],
        (int32_t)panel->page_index + 1 >= panel->page_count);
}

/* 0x020b9380 */
void NtmvM2dItemsPanel_SetItemCount(
    NtmvM2dItemsPanel *panel, uint16_t item_count)
{
    int32_t page_step = panel->visible_row_count_minus_one;
    int32_t numerator = (int32_t)item_count + page_step - 1;
    uint16_t row_count = (uint16_t)(
        panel->visible_row_count_minus_one + 1);
    uint16_t index;

    panel->item_count = item_count;
    panel->page_count = page_step != 0
        ? (uint16_t)(numerator / page_step)
        : 0;
    NtmvM2dItemsPanel_UpdatePageControlVisibility(panel);
    NtmvM2dItemsPanel_SetPageIndex(panel, 0);
    panel->selected_item = -1;
    panel->active_control_index = -1;

    for (index = 0; index < row_count; ++index) {
        NtmvM2dManualButton *button = panel->row_buttons[index];

        button->selected = false;
        button->base.base.vtable->set_visible(&button->base.base, false);
    }
}

/* 0x020b9774 */
void NtmvM2dItemsPanel_SetSelectedRowVisual(
    NtmvM2dItemsPanel *panel, bool selected)
{
    int32_t selected_item = panel->selected_item;
    int32_t first_visible_item = panel->first_visible_item;
    uint16_t visible_row_count_minus_one =
        (uint16_t)panel->visible_row_count_minus_one;
    uint16_t row_count = (uint16_t)(visible_row_count_minus_one + 1);
    uint16_t row_slot;

    if (selected_item == -1 || panel->interaction_enabled == 0) {
        return;
    }
    if (selected_item < first_visible_item) {
        return;
    }
    if (selected_item >=
        first_visible_item + (int32_t)visible_row_count_minus_one) {
        return;
    }

    row_slot = (uint16_t)(selected_item % row_count);
    panel->row_buttons[row_slot]->selected = selected;
}

/* 0x020b9424 */
bool NtmvM2dItemsPanel_SetSelectedItem(
    NtmvM2dItemsPanel *panel, int16_t selected_item)
{
    if (panel->selected_item == selected_item &&
        panel->interaction_enabled != 0) {
        return false;
    }

    panel->interaction_enabled = true;
    NtmvM2dItemsPanel_SetSelectedRowVisual(panel, false);
    panel->selected_item = selected_item;
    NtmvM2dItemsPanel_SetSelectedRowVisual(panel, true);
    return true;
}

/* 0x020b9740 */
bool NtmvM2dItemsPanel_SetPageIndexIfChanged(
    NtmvM2dItemsPanel *panel, int16_t page_index)
{
    if (panel->page_index == page_index) {
        return false;
    }

    NtmvM2dItemsPanel_SetPageIndex(panel, page_index);
    panel->target_scroll_y = (int16_t)(panel->first_visible_item * 0x17);
    return true;
}

/* 0x020b9478 */
bool NtmvM2dItemsPanel_SyncPageToSelection(
    NtmvM2dItemsPanel *panel)
{
    int16_t page_step;
    int16_t page_index;

    if (panel->selected_item == -1) {
        return false;
    }

    page_step = panel->visible_row_count_minus_one;
    page_index = page_step != 0
        ? (int16_t)(panel->selected_item / page_step)
        : 0;
    return NtmvM2dItemsPanel_SetPageIndexIfChanged(panel, page_index);
}

/* 0x020b94ac */
bool NtmvM2dItemsPanel_HandlePointer(
    NtmvM2dItemsPanel *panel,
    NtmvM2dItemsPointerAction *action,
    const NtmvM2dItemsPointerState *pointer)
{
    bool handled = false;

    action->page_changed = 0;
    action->selection_changed = 0;
    action->row_activated = 0;
    action->control_hit = 0;
    action->control_pressed = 0;

    if ((panel->base.base.flags & NTMV_M2D_UI_HIDDEN) != 0 ||
        panel->scroll_y != panel->target_scroll_y) {
        return false;
    }

    if (pointer->active != 0) {
        if (pointer->pressed != 0) {
            int32_t control_index = NtmvM2dItemsPanel_FindControlAtPoint(
                panel, &pointer->position);

            if (control_index != -1) {
                NtmvM2dUIElement *control =
                    panel->base.children[control_index];

                handled = true;
                if ((control->flags & NTMV_M2D_UI_HIT_STATE_LOCKED) == 0) {
                    panel->active_control_index = (int16_t)control_index;
                    action->control_pressed = 1;
                    action->control_hit = 1;
                } else if (control_index < 2 || control_index > 3) {
                    action->control_hit = 1;
                }
            }
        }

        if (panel->active_control_index != -1) {
            NtmvM2dUIElement *control =
                panel->base.children[panel->active_control_index];

            NtmvM2dUIElement_UpdateHitState(control, &pointer->position);
            handled = true;
        }
        return handled;
    }

    if (pointer->released != 0 && panel->active_control_index != -1) {
        int16_t control_index = panel->active_control_index;
        NtmvM2dUIElement *control = panel->base.children[control_index];

        panel->active_control_index = -1;
        handled = true;
        if ((control->flags & NTMV_M2D_UI_HOVERED) != 0) {
            control->flags &= (uint8_t)~NTMV_M2D_UI_HOVERED;

            if (control_index < 2) {
                int16_t page_index = (int16_t)(
                    panel->page_index + (control_index == 0 ? -1 : 1));

                action->page_changed =
                    NtmvM2dItemsPanel_SetPageIndexIfChanged(
                        panel, page_index);
            } else if (control_index > 3) {
                uint16_t row_count = (uint16_t)(
                    panel->visible_row_count_minus_one + 1);
                uint16_t first_row_slot = (uint16_t)(
                    panel->first_visible_item % row_count);
                uint16_t control_row_slot = (uint16_t)(control_index - 4);
                uint16_t item_offset = (uint16_t)(
                    (control_row_slot + row_count - first_row_slot) %
                    row_count);
                int16_t item_index = (int16_t)(
                    panel->first_visible_item + item_offset);

                action->row_activated = 1;
                action->selection_changed =
                    NtmvM2dItemsPanel_SetSelectedItem(panel, item_index);
            }
        }
    }
    return handled;
}

/* 0x020b9d30 */
void NtmvM2dItemsPanel_SetAuxiliaryChild(
    NtmvM2dItemsPanel *panel,
    int32_t auxiliary_index,
    NtmvM2dUIElement *child)
{
    int16_t first_auxiliary_index = (int16_t)(
        panel->visible_row_count_minus_one + 5);

    panel->base.children[first_auxiliary_index + auxiliary_index] = child;
}

/* 0x020b9d58 */
int32_t NtmvM2dItemsPanel_FindControlAtPoint(
    const NtmvM2dItemsPanel *panel, const NtmvM2dPoint *point)
{
    int16_t control_count = (int16_t)(
        panel->visible_row_count_minus_one + 5);
    int16_t index;

    for (index = 0; index < control_count; ++index) {
        if (NtmvM2dUIElement_ContainsPoint(
                panel->base.children[index], point)) {
            return index;
        }
    }
    return -1;
}

static NtmvM2dItemsObjTextCanvas *InitTextCanvas(
    NtmvM2dItemsObjTextCanvas *output,
    const NtmvM2dPoint *tile_origin,
    const NtmvM2dSize *canvas_size,
    const NtmvM2dItemsFont *font)
{
    void *vram = (void *)(uintptr_t)(
        UINT32_C(0x06400000) +
        (uint32_t)(uint16_t)tile_origin->x * UINT32_C(0x20) +
        (uint32_t)(uint16_t)tile_origin->y * UINT32_C(0x400));

    NNS_G2dCharCanvasInitForOBJ1D(
        &output->canvas,
        vram,
        canvas_size->width,
        canvas_size->height,
        4);
    output->text_canvas.canvas = &output->canvas;
    output->text_canvas.font = font;
    output->text_canvas.character_spacing = 0;
    output->text_canvas.reserved_0c = 0;
    output->canvas.ops->clear(&output->canvas);
    return output;
}

static NtmvM2dItemsTextDirection GetTextDirection(
    const NtmvM2dItemsFont *font)
{
    static const NtmvM2dItemsTextDirection directions[8] = {
        {1, 0}, {0, 1}, {0, 1}, {-1, 0},
        {-1, 0}, {0, -1}, {0, -1}, {1, 0},
    };
    uint8_t orientation = font->resource->metadata->orientation;

    if (orientation >= 8) {
        return (NtmvM2dItemsTextDirection){0, 0};
    }
    return directions[orientation];
}

/* 0x020b98f8 */
void NtmvM2dItemsPanel_UpdateTextRow(
    NtmvM2dItemsPanel *panel,
    const NtmvM2dItemsRow *row,
    int16_t item_index)
{
    static const uint16_t row_number_format[] = {'%', 'u', 0};
    int16_t row_count = (int16_t)(panel->visible_row_count_minus_one + 1);
    int16_t row_slot = (int16_t)(item_index % row_count);
    NtmvM2dItemsObjTextCanvas number_canvas;
    NtmvM2dItemsObjTextCanvas text_canvas;
    NtmvM2dPoint tile_origin = {0, (int16_t)(row_slot * 3)};
    const NtmvM2dSize number_size = {4, 4};
    const NtmvM2dSize text_size = {0x14, 4};
    NtmvM2dItemsTextDirection direction;
    uint16_t row_number[10];
    NtmvM2dManualButton *button;

    InitTextCanvas(
        &number_canvas,
        &tile_origin,
        &number_size,
        panel->text_context->font);
    NtmvM2d_FormatWideText_0200a324(
        row_number,
        10,
        row_number_format,
        (uint32_t)((int32_t)item_index + 1));
    direction = GetTextDirection(number_canvas.text_canvas.font);
    NNSi_G2dTextCanvasDrawTextRect_020c6b50(
        &number_canvas.text_canvas,
        0,
        0,
        0x20,
        0x20,
        1,
        0x480,
        row_number,
        direction);

    tile_origin.x = 0x10;
    InitTextCanvas(
        &text_canvas,
        &tile_origin,
        &text_size,
        panel->text_context->font);
    direction = GetTextDirection(text_canvas.text_canvas.font);
    NNSi_G2dTextCanvasDrawText_020c6a2c(
        &text_canvas.text_canvas,
        0,
        0x10,
        1,
        0x20a,
        row->text,
        direction);

    button = panel->row_buttons[row_slot];
    NtmvM2dManualButton_SetAlternateAnimation(
        button, row->use_alternate_animation != 0);
    button->base.base.vtable->set_visible(&button->base.base, true);
    panel->row_item_indices[row_slot] = item_index;
}

/* 0x020b9844 */
void NtmvM2dItemsPanel_UpdateRows(
    NtmvM2dItemsPanel *panel,
    const NtmvM2dItemsRow *rows,
    uint16_t populated_count,
    int16_t first_item_index)
{
    int16_t row_count = (int16_t)(panel->visible_row_count_minus_one + 1);
    int16_t row;

    if ((uint16_t)panel->visible_row_count_minus_one <= populated_count) {
        populated_count = (uint16_t)panel->visible_row_count_minus_one;
    }
    for (row = 0; row < row_count; ++row) {
        int16_t item_index = (int16_t)(first_item_index + row);
        int16_t row_slot = (int16_t)(item_index % row_count);

        if ((uint16_t)row < populated_count) {
            NtmvM2dItemsPanel_UpdateTextRow(
                panel, &rows[row], item_index);
        } else {
            NtmvM2dManualButton *button = panel->row_buttons[row_slot];
            button->base.base.vtable->set_visible(&button->base.base, false);
        }
    }
}

/* 0x020b9b58 */
void NtmvM2dItemsPanel_UpdateSimpleRows(
    NtmvM2dItemsPanel *panel,
    const NtmvM2dItemsRow *rows,
    uint16_t populated_count,
    int16_t first_item_index)
{
    int16_t row_count = (int16_t)(panel->visible_row_count_minus_one + 1);
    int16_t row;

    if ((uint16_t)panel->visible_row_count_minus_one <= populated_count) {
        populated_count = (uint16_t)panel->visible_row_count_minus_one;
    }
    for (row = 0; row < row_count; ++row) {
        int16_t item_index = (int16_t)(first_item_index + row);
        int16_t row_slot = (int16_t)(item_index % row_count);
        NtmvM2dManualButton *button = panel->row_buttons[row_slot];

        if ((uint16_t)row < populated_count) {
            NtmvM2dItemsObjTextCanvas text_canvas;
            NtmvM2dPoint tile_origin = {
                0, (int16_t)(row_slot * 3 + 0x12),
            };
            const NtmvM2dSize canvas_size = {0x18, 4};
            NtmvM2dItemsTextDirection direction;

            InitTextCanvas(
                &text_canvas,
                &tile_origin,
                &canvas_size,
                panel->text_context->font);
            direction = GetTextDirection(text_canvas.text_canvas.font);
            NNSi_G2dTextCanvasDrawText_020c6a2c(
                &text_canvas.text_canvas,
                0,
                0x10,
                1,
                0x20a,
                rows[row].text,
                direction);
            NtmvM2dManualButton_SetAlternateAnimation(
                button, rows[row].use_alternate_animation != 0);
            button->base.base.vtable->set_visible(&button->base.base, true);
            panel->row_item_indices[row_slot] = item_index;
        } else {
            button->base.base.vtable->set_visible(&button->base.base, false);
        }
    }
}

/* 0x020b9df0 */
bool NtmvM2dItemsPanel_AnimateScroll(
    NtmvM2dItemsPanel *panel,
    NtmvM2dItemsRowProvider row_provider,
    void *provider_context)
{
    int32_t current_scroll = panel->scroll_y;
    int32_t target_scroll = panel->target_scroll_y;
    int16_t row_count;
    int16_t first_item;
    int16_t row;

    if (current_scroll == target_scroll) {
        return false;
    }
    if (current_scroll < target_scroll) {
        current_scroll += 12;
        if (target_scroll < current_scroll) {
            current_scroll = target_scroll;
        }
    } else {
        current_scroll -= 12;
        if (current_scroll < target_scroll) {
            current_scroll = target_scroll;
        }
    }
    panel->scroll_y = (int16_t)current_scroll;

    row_count = (int16_t)(panel->visible_row_count_minus_one + 1);
    first_item = (int16_t)(panel->scroll_y / 0x17);
    for (row = 0; row < row_count; ++row) {
        int16_t item_index = (int16_t)(first_item + row);
        bool visible = item_index < (int32_t)panel->item_count;
        int16_t row_slot = (int16_t)(item_index % row_count);
        NtmvM2dManualButton *button = panel->row_buttons[row_slot];

        button->base.base.vtable->set_visible(&button->base.base, visible);
        if (visible) {
            NtmvM2dPoint position = {
                button->base.base.local_position.x,
                (int16_t)(item_index * 0x17 - panel->scroll_y),
            };
            bool selected;

            button->base.base.vtable->set_position(
                &button->base.base, &position);
            selected = panel->selected_item != -1 &&
                panel->interaction_enabled != 0 &&
                item_index == panel->selected_item;
            button->selected = selected;

            if (item_index != panel->row_item_indices[row_slot]) {
                NtmvM2dItemsRow row_data;

                row_provider(&row_data, provider_context, item_index);
                NtmvM2dItemsPanel_UpdateTextRow(
                    panel, &row_data, item_index);
            }
        }
    }
    return true;
}

/* 0x020b9fac; referenced as a callback literal at 0x020baef0. */
void NtmvM2dItemsPanel_BuildTocRow(
    NtmvM2dItemsRow *output, void *context, int16_t item_index)
{
    static const uint16_t empty_text[] = {'(', '-', '-', '-', ')', 0};
    const NtmvM2dItemsTocData *toc_data = context;
    const NtmvM2dItemsTocRecord *record = &toc_data->records[item_index];
    const uint16_t *text = record->text;

    if (text[0] == 0) {
        text = empty_text;
    }
    output->text = text;
    output->use_alternate_animation = record->child_count != 0;
}
