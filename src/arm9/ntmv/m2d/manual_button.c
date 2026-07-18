#include "ntmv/m2d/manual_button.h"
#include "ntmv/ui_renderer.h"

const NtmvM2dManualButtonVTable gNtmvM2dManualButtonVTable = {
    (void (*)(NtmvM2dManualButton *, NtmvM2dAllocatorContext *))
        NtmvM2dUIElement_Destroy, /* slot 0: 0x020b7cd0 */
    NtmvM2dManualButton_Render,   /* slot 1: 0x020bba88 */
    (void (*)(NtmvM2dManualButton *, const NtmvM2dPoint *))
        NtmvM2dUIElement_SetPosition, /* slot 2: 0x020b7e48 */
    (void (*)(NtmvM2dManualButton *, const NtmvM2dSize *))
        NtmvM2dUIElement_SetSize, /* slot 3: 0x020b7e5c */
    (void (*)(NtmvM2dManualButton *, bool))
        NtmvM2dUIElement_SetVisible, /* slot 4: 0x020b7e70 */
};

/* 0x020bba00 */
NtmvM2dManualButton *NtmvM2dManualButton_Construct(
    NtmvM2dManualButton *button)
{
    NtmvM2dObjButton_ConstructBase(&button->base);
    button->base.base.vtable =
        (const NtmvM2dUIElementVTable *)&gNtmvM2dManualButtonVTable;
    button->alternate_animation_handle = -1;
    button->overlay_animation_handle = -1;
    button->use_alternate_animation = false;
    button->selected = false;
    return button;
}

/* 0x020bba38 */
void NtmvM2dManualButton_Configure(
    NtmvM2dManualButton *button,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size,
    int32_t animation_handle,
    int32_t alternate_animation_handle,
    int32_t overlay_animation_handle)
{
    NtmvM2dObjButton_Configure(
        &button->base, position, size, animation_handle);
    button->alternate_animation_handle = alternate_animation_handle;
    button->overlay_animation_handle = overlay_animation_handle;
}

/*
 * 0x020bba88.  active_timer wins over all other states; otherwise locked,
 * hovered and selected are tested in that order.  An optional overlay is
 * submitted first and the selected main animation is always submitted last.
 */
static uint32_t GetVisualState(const NtmvM2dManualButton *button)
{
    if (button->base.active_timer != 0) {
        return NTMV_M2D_MANUAL_BUTTON_ACTIVE;
    }
    if ((button->base.base.flags & NTMV_M2D_UI_HIT_STATE_LOCKED) != 0) {
        return NTMV_M2D_MANUAL_BUTTON_LOCKED;
    }
    if ((button->base.base.flags & NTMV_M2D_UI_HOVERED) != 0) {
        return NTMV_M2D_MANUAL_BUTTON_ACTIVE;
    }
    if (button->selected != 0) {
        return NTMV_M2D_MANUAL_BUTTON_SELECTED;
    }
    return NTMV_M2D_MANUAL_BUTTON_NORMAL;
}

static void SubmitAnimation(
    NtmvUiRenderer *renderer,
    int32_t animation_handle,
    uint32_t visual_state,
    const NtmvM2dPoint *world_center)
{
    (void)NtmvUiRenderer_SetAnimationFrame(
        renderer, animation_handle, visual_state);
    (void)NtmvUiRenderer_SubmitCell(
        renderer,
        animation_handle,
        (int32_t)world_center->x * 0x1000,
        (int32_t)world_center->y * 0x1000);
}

void NtmvM2dManualButton_Render(
    NtmvM2dManualButton *button,
    void *render_context,
    const NtmvM2dPoint *parent_position)
{
    NtmvUiRenderer *renderer;
    int32_t animation_handle;
    uint32_t visual_state;

    if (!NtmvM2dUIElement_UpdateWorldCenter(
            &button->base.base, parent_position)) {
        return;
    }

    renderer = *(NtmvUiRenderer **)render_context;
    visual_state = GetVisualState(button);
    animation_handle = button->use_alternate_animation != 0
        ? button->alternate_animation_handle
        : button->base.animation_handle;

    if (button->overlay_animation_handle != -1) {
        SubmitAnimation(
            renderer,
            button->overlay_animation_handle,
            visual_state,
            &button->base.base.world_center);
    }
    SubmitAnimation(
        renderer,
        animation_handle,
        visual_state,
        &button->base.base.world_center);
}

/* 0x020bbb70 */
void NtmvM2dManualButton_SetAlternateAnimation(
    NtmvM2dManualButton *button, bool use_alternate_animation)
{
    button->use_alternate_animation = use_alternate_animation;
}
