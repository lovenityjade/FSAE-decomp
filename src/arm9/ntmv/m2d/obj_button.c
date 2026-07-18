#include "ntmv/m2d/obj_button.h"
#include "ntmv/ui_renderer.h"

const NtmvM2dObjButtonVTable gNtmvM2dObjButtonVTable = {
    (void (*)(NtmvM2dObjButton *, NtmvM2dAllocatorContext *))
        NtmvM2dUIElement_Destroy, /* slot 0: 0x020b7cd0 */
    NtmvM2dObjButton_Render,      /* slot 1: 0x020b82f4 */
    (void (*)(NtmvM2dObjButton *, const NtmvM2dPoint *))
        NtmvM2dUIElement_SetPosition, /* slot 2: 0x020b7e48 */
    (void (*)(NtmvM2dObjButton *, const NtmvM2dSize *))
        NtmvM2dUIElement_SetSize, /* slot 3: 0x020b7e5c */
    (void (*)(NtmvM2dObjButton *, bool))
        NtmvM2dUIElement_SetVisible, /* slot 4: 0x020b7e70 */
};

static NtmvM2dObjButton *ConstructCommon(NtmvM2dObjButton *button)
{
    NtmvM2dUIElement_Init(&button->base);
    button->base.vtable =
        (const NtmvM2dUIElementVTable *)&gNtmvM2dObjButtonVTable;
    button->animation_handle = -1;
    button->active_timer = 0;
    return button;
}

/* 0x020b826c: base-object constructor used by the ManualButton subclass. */
NtmvM2dObjButton *NtmvM2dObjButton_ConstructBase(NtmvM2dObjButton *button)
{
    return ConstructCommon(button);
}

/* 0x020b829c: instruction-identical complete-object constructor variant. */
NtmvM2dObjButton *NtmvM2dObjButton_ConstructComplete(NtmvM2dObjButton *button)
{
    return ConstructCommon(button);
}

/* 0x020b82cc */
void NtmvM2dObjButton_Configure(
    NtmvM2dObjButton *button,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size,
    int32_t animation_handle)
{
    button->base.local_position = *position;
    button->base.size = *size;
    button->animation_handle = animation_handle;
}

/*
 * Inlined by 0x020b82f4.  A nonzero timer has priority and forces frame 1;
 * otherwise UIElement bit 1 selects locked frame 2 and bit 2 selects frame 1.
 */
static uint32_t GetVisualState(const NtmvM2dObjButton *button)
{
    if (button->active_timer != 0) {
        return NTMV_M2D_OBJ_BUTTON_ACTIVE;
    }
    if ((button->base.flags & NTMV_M2D_UI_HIT_STATE_LOCKED) != 0) {
        return NTMV_M2D_OBJ_BUTTON_LOCKED;
    }
    if ((button->base.flags & NTMV_M2D_UI_HOVERED) != 0) {
        return NTMV_M2D_OBJ_BUTTON_ACTIVE;
    }
    return NTMV_M2D_OBJ_BUTTON_NORMAL;
}

/* 0x020b82f4, vtable slot 1. */
void NtmvM2dObjButton_Render(
    NtmvM2dObjButton *button,
    void *render_context,
    const NtmvM2dPoint *parent_position)
{
    NtmvUiRenderer *renderer;
    uint32_t visual_state;

    if (!NtmvM2dUIElement_UpdateWorldCenter(&button->base, parent_position)) {
        return;
    }

    renderer = *(NtmvUiRenderer **)render_context;
    visual_state = GetVisualState(button);
    (void)NtmvUiRenderer_SetAnimationFrame(
        renderer, button->animation_handle, visual_state);
    (void)NtmvUiRenderer_SubmitCell(
        renderer,
        button->animation_handle,
        (int32_t)button->base.world_center.x * 0x1000,
        (int32_t)button->base.world_center.y * 0x1000);
}

/* 0x020b8384. The timer saturates at zero rather than underflowing. */
void NtmvM2dObjButton_TickActiveState(NtmvM2dObjButton *button)
{
    if (button->active_timer != 0) {
        --button->active_timer;
    }
}
