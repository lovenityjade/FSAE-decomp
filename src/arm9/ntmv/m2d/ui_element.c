#include "ntmv/m2d/ui_element.h"

enum {
    NTMV_M2D_SCREEN_WIDTH = 256,
    NTMV_M2D_SCREEN_HEIGHT = 192,
};

const NtmvM2dUIElementVTable gNtmvM2dUIElementVTable = {
    NtmvM2dUIElement_Destroy,
    NtmvM2dUIElement_Render,
    NtmvM2dUIElement_SetPosition,
    NtmvM2dUIElement_SetSize,
    NtmvM2dUIElement_SetVisible,
};

/* 0x020b7c50 */
void NtmvM2dUIElement_Init(NtmvM2dUIElement *element)
{
    element->vtable = &gNtmvM2dUIElementVTable;
    element->local_position.x = 0;
    element->local_position.y = 0;
    element->size.width = 0;
    element->size.height = 0;
    element->world_center.x = 0;
    element->world_center.y = 0;
    element->flags = 0;
}

/* 0x020b7c80 */
void NtmvM2dUIElement_InitWithGeometry(
    NtmvM2dUIElement *element,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size)
{
    NtmvM2dUIElement_Init(element);
    element->local_position = *position;
    element->size = *size;
}

/* 0x020b7cd0, UIElement vtable slot 0. */
void NtmvM2dUIElement_Destroy(
    NtmvM2dUIElement *element, NtmvM2dAllocatorContext *allocator)
{
    (void)element;
    (void)allocator;
}

/*
 * 0x020b7cd4, UIElement vtable slot 1.  This routine is present in the ROM
 * vtable even though the initial Ghidra pass did not create a function record
 * at its exact entry address.
 */
void NtmvM2dUIElement_Render(
    NtmvM2dUIElement *element,
    void *render_context,
    const NtmvM2dPoint *parent_position)
{
    /* Derived UIStatic/UIPanel implementations use this slot to draw. */
    (void)render_context;
    if ((element->flags & NTMV_M2D_UI_HIDDEN) != 0) {
        return;
    }
    element->world_center.x = (int16_t)(parent_position->x + element->local_position.x);
    element->world_center.y = (int16_t)(parent_position->y + element->local_position.y);
}

/* 0x020b7ef8 */
void NtmvM2dUIElement_GetTopLeft(
    NtmvM2dPoint *output, const NtmvM2dUIElement *element)
{
    output->x = (int16_t)(element->world_center.x - element->size.width / 2);
    output->y = (int16_t)(element->world_center.y - element->size.height / 2);
}

/* 0x020b7d08 */
bool NtmvM2dUIElement_IsOnScreen(const NtmvM2dUIElement *element)
{
    NtmvM2dPoint top_left;

    NtmvM2dUIElement_GetTopLeft(&top_left, element);
    return top_left.x < NTMV_M2D_SCREEN_WIDTH &&
           top_left.y < NTMV_M2D_SCREEN_HEIGHT &&
           top_left.x + element->size.width > 0 &&
           top_left.y + element->size.height > 0;
}

/* 0x020b7d74 */
bool NtmvM2dUIElement_UpdateWorldCenter(
    NtmvM2dUIElement *element, const NtmvM2dPoint *parent_position)
{
    if ((element->flags & NTMV_M2D_UI_HIDDEN) != 0) {
        return false;
    }
    element->world_center.x = (int16_t)(parent_position->x + element->local_position.x);
    element->world_center.y = (int16_t)(parent_position->y + element->local_position.y);
    return NtmvM2dUIElement_IsOnScreen(element);
}

/* 0x020b7db4 */
bool NtmvM2dUIElement_ContainsPoint(
    const NtmvM2dUIElement *element, const NtmvM2dPoint *point)
{
    NtmvM2dPoint top_left;

    if ((element->flags & NTMV_M2D_UI_HIDDEN) != 0) {
        return false;
    }
    NtmvM2dUIElement_GetTopLeft(&top_left, element);
    return top_left.x <= point->x &&
           point->x <= top_left.x + element->size.width &&
           top_left.y <= point->y &&
           point->y <= top_left.y + element->size.height;
}

/* 0x020b7e48, UIElement vtable slot 2. */
void NtmvM2dUIElement_SetPosition(
    NtmvM2dUIElement *element, const NtmvM2dPoint *position)
{
    element->local_position = *position;
}

/* 0x020b7e5c, UIElement vtable slot 3. */
void NtmvM2dUIElement_SetSize(
    NtmvM2dUIElement *element, const NtmvM2dSize *size)
{
    element->size = *size;
}

/* 0x020b7e70, UIElement vtable slot 4.  Bit zero stores the inverse state. */
void NtmvM2dUIElement_SetVisible(NtmvM2dUIElement *element, bool visible)
{
    if (visible) {
        element->flags &= (uint8_t)~NTMV_M2D_UI_HIDDEN;
    } else {
        element->flags |= NTMV_M2D_UI_HIDDEN;
    }
}

/* 0x020b7e94 */
void NtmvM2dUIElement_UpdateHitState(
    NtmvM2dUIElement *element, const NtmvM2dPoint *point)
{
    bool hovered;

    if ((element->flags & (NTMV_M2D_UI_HIDDEN | NTMV_M2D_UI_HIT_STATE_LOCKED)) != 0) {
        return;
    }
    hovered = NtmvM2dUIElement_ContainsPoint(element, point);
    element->flags = (uint8_t)(
        (element->flags & (uint8_t)~NTMV_M2D_UI_HOVERED) |
        (hovered ? NTMV_M2D_UI_HOVERED : 0));
}
