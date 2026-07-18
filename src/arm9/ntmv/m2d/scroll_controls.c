#include "ntmv/m2d/scroll_controls.h"
#include "ntmv/ui_renderer.h"

extern void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size);

const NtmvM2dUIPanelVTable gNtmvM2dScrollButtonVTable = {
    NtmvM2dUIPanel_Destroy, /* slot 0: 0x020b8138 */
    NtmvM2dUIPanel_Render,  /* slot 1: 0x020b81dc */
    (void (*)(NtmvM2dUIPanel *, const NtmvM2dPoint *))
        NtmvM2dUIElement_SetPosition, /* slot 2: 0x020b7e48 */
    (void (*)(NtmvM2dUIPanel *, const NtmvM2dSize *))
        NtmvM2dUIElement_SetSize, /* slot 3: 0x020b7e5c */
    (void (*)(NtmvM2dUIPanel *, bool))
        NtmvM2dUIElement_SetVisible, /* slot 4: 0x020b7e70 */
};

const NtmvM2dUIPanelVTable gNtmvM2dScrollIndicatorVTable = {
    NtmvM2dUIPanel_Destroy, /* slot 0: 0x020b8138 */
    NtmvM2dUIPanel_Render,  /* slot 1: 0x020b81dc */
    (void (*)(NtmvM2dUIPanel *, const NtmvM2dPoint *))
        NtmvM2dUIElement_SetPosition, /* slot 2: 0x020b7e48 */
    (void (*)(NtmvM2dUIPanel *, const NtmvM2dSize *))
        NtmvM2dUIElement_SetSize, /* slot 3: 0x020b7e5c */
    (void (*)(NtmvM2dUIPanel *, bool))
        NtmvM2dUIElement_SetVisible, /* slot 4: 0x020b7e70 */
};

static int16_t GetSelectedChild(const NtmvM2dScrollButton *button)
{
    return (int16_t)button->base.reserved_1a;
}

static void SetSelectedChild(NtmvM2dScrollButton *button, int16_t index)
{
    button->base.reserved_1a = (uint16_t)index;
}

static int16_t GetIndicatorItemCount(const NtmvM2dScrollIndicator *indicator)
{
    return (int16_t)indicator->base.reserved_1a;
}

/* 0x020b8408 */
NtmvM2dScrollButton *NtmvM2dScrollButton_Construct(
    NtmvM2dScrollButton *button)
{
    NtmvM2dUIPanel_ConstructBase(&button->base);
    button->base.base.vtable =
        (const NtmvM2dUIElementVTable *)&gNtmvM2dScrollButtonVTable;
    SetSelectedChild(button, -1);
    button->item_count = 0;
    button->page_size = 0;
    button->scroll_offset = 0;
    return button;
}

/* 0x020b8528: target allocation is exactly the ObjButton size, 0x1c. */
static NtmvM2dObjButton *AllocateObjButton(NtmvM2dAllocatorContext *allocator)
{
    NtmvM2dObjButton *button = NNS_FndAllocFromAllocator(
        allocator->nns_allocator, (uint32_t)sizeof(*button));

    if (button != NULL) {
        NtmvM2dObjButton_ConstructComplete(button);
    }
    return button;
}

/* 0x020b8440 */
void NtmvM2dScrollButton_Configure(
    NtmvM2dScrollButton *button,
    NtmvM2dAllocatorContext *allocator,
    void *animation_manager,
    const NtmvM2dPoint *position,
    int16_t page_size)
{
    static const NtmvM2dPoint sChildPositions[2] = {{25, 0}, {-25, 0}};
    static const uint16_t sAnimationSequences[2] = {6, 5};
    NtmvM2dUIElement *children[2];
    NtmvM2dSize child_size = {0x33, 0x14};
    NtmvM2dSize panel_size = {0x65, 0x14};
    uint32_t index;

    for (index = 0; index < 2; ++index) {
        NtmvM2dObjButton *child = AllocateObjButton(allocator);
        int32_t handle = NtmvUiRenderer_CreateAnimation(
            animation_manager, sAnimationSequences[index]);
        NtmvM2dObjButton_Configure(
            child, &sChildPositions[index], &child_size, handle);
        children[index] = &child->base;
    }
    NtmvM2dUIPanel_Configure(
        &button->base, allocator, children, 2, position, &panel_size);
    button->page_size = page_size;
}

/* 0x020b8398 */
static int16_t NtmvM2dUIElementArray_FindContainingPoint_020b8398(
    NtmvM2dUIElement *const *children,
    int16_t child_count,
    const NtmvM2dPoint *point)
{
    int16_t index;

    for (index = 0; index < child_count; ++index) {
        NtmvM2dPoint local_point = *point;

        if (NtmvM2dUIElement_ContainsPoint(children[index], &local_point)) {
            return index;
        }
    }
    return -1;
}

/* 0x020b8554; the three input-event field names remain provisional. */
bool NtmvM2dScrollButton_HandlePointer(
    NtmvM2dScrollButton *button,
    NtmvM2dScrollButtonAction *action,
    const NtmvM2dScrollPointerState *pointer)
{
    bool handled = false;
    int16_t selected;

    if ((button->base.base.flags & NTMV_M2D_UI_HIDDEN) != 0) {
        return false;
    }
    action->activated = 0;

    if (pointer->active == 0) {
        if (pointer->released != 0) {
            selected = GetSelectedChild(button);
            if (selected != -1) {
                NtmvM2dUIElement *child = button->base.children[selected];
                handled = true;
                child->flags &= (uint8_t)~NTMV_M2D_UI_HOVERED;
                SetSelectedChild(button, -1);
            }
        }
        return handled;
    }

    if (pointer->pressed != 0) {
        int16_t hit = NtmvM2dUIElementArray_FindContainingPoint_020b8398(
            button->base.children,
            (int16_t)button->base.child_count,
            &pointer->position);
        if (hit != -1) {
            NtmvM2dUIElement *child = button->base.children[hit];
            handled = true;
            if ((child->flags & NTMV_M2D_UI_HIT_STATE_LOCKED) == 0) {
                SetSelectedChild(button, hit);
            }
        }
    }

    selected = GetSelectedChild(button);
    if (selected != -1) {
        NtmvM2dUIElement *child = button->base.children[selected];
        handled = true;
        if ((child->flags & NTMV_M2D_UI_HIT_STATE_LOCKED) == 0) {
            NtmvM2dUIElement_UpdateHitState(child, &pointer->position);
            if ((child->flags & NTMV_M2D_UI_HOVERED) != 0) {
                action->activated = 1;
                action->decrement = selected == 1;
                action->increment = selected == 0;
            }
        }
    }
    return handled;
}

/* 0x020b86b8 */
void NtmvM2dScrollButton_SetDirectionalHover(
    NtmvM2dScrollButton *button, bool decrement, bool increment)
{
    NtmvM2dUIElement *decrement_child = button->base.children[1];
    NtmvM2dUIElement *increment_child = button->base.children[0];

    decrement_child->flags = (uint8_t)(
        (decrement_child->flags & (uint8_t)~NTMV_M2D_UI_HOVERED) |
        (decrement ? NTMV_M2D_UI_HOVERED : 0));
    increment_child->flags = (uint8_t)(
        (increment_child->flags & (uint8_t)~NTMV_M2D_UI_HOVERED) |
        (increment ? NTMV_M2D_UI_HOVERED : 0));
}

/* 0x020b86f4 */
void NtmvM2dScrollButton_SetScrollOffset(
    NtmvM2dScrollButton *button, int16_t scroll_offset)
{
    NtmvM2dUIElement *increment_child = button->base.children[0];
    NtmvM2dUIElement *decrement_child = button->base.children[1];
    bool decrement_locked;
    bool increment_locked;

    button->scroll_offset = scroll_offset;
    decrement_locked = scroll_offset < 1;
    increment_locked =
        scroll_offset + button->page_size >= button->item_count;
    decrement_child->flags = (uint8_t)(
        (decrement_child->flags & (uint8_t)~NTMV_M2D_UI_HIT_STATE_LOCKED) |
        (decrement_locked ? NTMV_M2D_UI_HIT_STATE_LOCKED : 0));
    increment_child->flags = (uint8_t)(
        (increment_child->flags & (uint8_t)~NTMV_M2D_UI_HIT_STATE_LOCKED) |
        (increment_locked ? NTMV_M2D_UI_HIT_STATE_LOCKED : 0));
}

/* 0x020b8760 */
void NtmvM2dScrollButton_TickChildren(NtmvM2dScrollButton *button)
{
    NtmvM2dObjButton_TickActiveState(
        (NtmvM2dObjButton *)button->base.children[1]);
    NtmvM2dObjButton_TickActiveState(
        (NtmvM2dObjButton *)button->base.children[0]);
}

/* 0x020b8784 */
NtmvM2dScrollIndicator *NtmvM2dScrollIndicator_Construct(
    NtmvM2dScrollIndicator *indicator)
{
    NtmvM2dUIPanel_ConstructBase(&indicator->base);
    indicator->base.base.vtable =
        (const NtmvM2dUIElementVTable *)&gNtmvM2dScrollIndicatorVTable;
    indicator->base.reserved_1a = 0;
    indicator->page_size = 0;
    indicator->scroll_offset = 0;
    return indicator;
}

/* 0x020b88b0: target allocation is exactly the UIStatic size, 0x1c. */
static NtmvM2dUIStatic *AllocateUIStatic(NtmvM2dAllocatorContext *allocator)
{
    NtmvM2dUIStatic *element = NNS_FndAllocFromAllocator(
        allocator->nns_allocator, (uint32_t)sizeof(*element));

    if (element != NULL) {
        NtmvM2dUIStatic_ConstructComplete(element);
    }
    return element;
}

/* 0x020b88dc */
void NtmvM2dScrollIndicator_RepositionChildren(
    NtmvM2dScrollIndicator *indicator, int16_t height)
{
    int16_t y_positions[2];
    uint32_t index;

    y_positions[0] = (int16_t)(-(height / 2) + 6);
    y_positions[1] = (int16_t)(height - height / 2 - 6);
    for (index = 0; index < 2; ++index) {
        NtmvM2dUIElement *child = indicator->base.children[index];
        NtmvM2dPoint position = child->local_position;
        position.y = y_positions[index];
        child->vtable->set_position(child, &position);
    }
}

/* 0x020b87b4 */
void NtmvM2dScrollIndicator_Configure(
    NtmvM2dScrollIndicator *indicator,
    NtmvM2dAllocatorContext *allocator,
    void *animation_manager,
    const NtmvM2dPoint *position,
    int16_t height,
    int16_t page_size,
    const uint16_t animation_sequences[2])
{
    NtmvM2dUIElement *children[2];
    NtmvM2dPoint child_position = {0, 0};
    NtmvM2dSize child_size = {0x0d, 8};
    NtmvM2dSize panel_size = {0x0d, height};
    uint32_t index;

    for (index = 0; index < 2; ++index) {
        NtmvM2dUIStatic *child = AllocateUIStatic(allocator);
        int32_t handle = NtmvUiRenderer_CreateAnimation(
            animation_manager, animation_sequences[index]);
        NtmvM2dUIStatic_Configure(child, &child_position, &child_size, handle);
        children[index] = &child->base;
    }
    NtmvM2dUIPanel_Configure(
        &indicator->base, allocator, children, 2, position, &panel_size);
    NtmvM2dScrollIndicator_RepositionChildren(indicator, height);
    indicator->page_size = page_size;
    indicator->child_extent[0] = 8;
    indicator->child_extent[1] = 8;
}

/* 0x020b895c */
void NtmvM2dScrollIndicator_SetScrollOffset(
    NtmvM2dScrollIndicator *indicator, int16_t scroll_offset)
{
    indicator->scroll_offset = scroll_offset;
}

/* 0x020b8964 */
void NtmvM2dScrollIndicator_UpdateVisibility(
    NtmvM2dScrollIndicator *indicator, void *render_context)
{
    NtmvM2dUIElement *decrement_child = indicator->base.children[0];
    NtmvM2dUIElement *increment_child = indicator->base.children[1];
    bool can_decrement;
    bool can_increment;

    NtmvM2dUIStatic_TickAnimation(
        (NtmvM2dUIStatic *)decrement_child, render_context);
    NtmvM2dUIStatic_TickAnimation(
        (NtmvM2dUIStatic *)increment_child, render_context);
    can_decrement = indicator->scroll_offset > 0;
    can_increment =
        indicator->scroll_offset + indicator->page_size <
        GetIndicatorItemCount(indicator);
    decrement_child->vtable->set_visible(decrement_child, can_decrement);
    increment_child->vtable->set_visible(increment_child, can_increment);
}
