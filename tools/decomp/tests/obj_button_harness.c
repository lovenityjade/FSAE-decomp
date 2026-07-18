#include "ntmv/m2d/obj_button.h"
#include "ntmv/ui_renderer.h"

#include <assert.h>
#include <stdint.h>

static void *sExpectedRenderer = (void *)(uintptr_t)0x1234;
static unsigned int sFrameCallCount;
static unsigned int sSubmitCallCount;
static unsigned int sCallSequence;
static int32_t sAnimationHandle;
static uint32_t sFrame;
static int32_t sX;
static int32_t sY;

bool NtmvUiRenderer_SetAnimationFrame(
    NtmvUiRenderer *renderer, int32_t animation_handle, uint32_t frame)
{
    assert(renderer == (NtmvUiRenderer *)sExpectedRenderer);
    assert(sCallSequence == 0);
    sCallSequence = 1;
    ++sFrameCallCount;
    sAnimationHandle = animation_handle;
    sFrame = frame;
    return true;
}

bool NtmvUiRenderer_SubmitCell(
    NtmvUiRenderer *renderer,
    int32_t animation_handle,
    int32_t x_fx12,
    int32_t y_fx12)
{
    assert(renderer == (NtmvUiRenderer *)sExpectedRenderer);
    assert(sCallSequence == 1);
    sCallSequence = 2;
    ++sSubmitCallCount;
    assert(animation_handle == sAnimationHandle);
    sX = x_fx12;
    sY = y_fx12;
    return true;
}

static void ResetCalls(void)
{
    sFrameCallCount = 0;
    sSubmitCallCount = 0;
    sCallSequence = 0;
    sAnimationHandle = -1;
    sFrame = 99;
    sX = 0;
    sY = 0;
}

static void RenderAndAssertState(
    NtmvM2dObjButton *button,
    const NtmvM2dPoint *parent,
    uint32_t expected_state)
{
    void *render_context = sExpectedRenderer;

    ResetCalls();
    button->base.vtable->render(&button->base, &render_context, parent);
    assert(sFrameCallCount == 1);
    assert(sSubmitCallCount == 1);
    assert(sCallSequence == 2);
    assert(sAnimationHandle == 7);
    assert(sFrame == expected_state);
    assert(sX == 112 * 0x1000);
    assert(sY == 75 * 0x1000);
}

int main(void)
{
    NtmvM2dObjButton base_button;
    NtmvM2dObjButton button;
    NtmvM2dPoint position = {12, -5};
    NtmvM2dSize size = {32, 16};
    NtmvM2dPoint parent = {100, 80};
    NtmvM2dPoint changed_position = {1, 2};
    NtmvM2dSize changed_size = {3, 4};
    void *render_context = sExpectedRenderer;

    assert(NtmvM2dObjButton_ConstructBase(&base_button) == &base_button);
    assert(base_button.base.vtable ==
           (const NtmvM2dUIElementVTable *)&gNtmvM2dObjButtonVTable);
    assert(base_button.animation_handle == -1);
    assert(base_button.active_timer == 0);

    assert(NtmvM2dObjButton_ConstructComplete(&button) == &button);
    NtmvM2dObjButton_Configure(&button, &position, &size, 7);
    assert(button.base.local_position.x == 12);
    assert(button.base.local_position.y == -5);
    assert(button.base.size.width == 32);
    assert(button.base.size.height == 16);
    assert(button.animation_handle == 7);

    RenderAndAssertState(&button, &parent, NTMV_M2D_OBJ_BUTTON_NORMAL);
    assert(button.base.world_center.x == 112 && button.base.world_center.y == 75);

    button.base.flags = NTMV_M2D_UI_HOVERED;
    RenderAndAssertState(&button, &parent, NTMV_M2D_OBJ_BUTTON_ACTIVE);

    button.base.flags = NTMV_M2D_UI_HOVERED | NTMV_M2D_UI_HIT_STATE_LOCKED;
    RenderAndAssertState(&button, &parent, NTMV_M2D_OBJ_BUTTON_LOCKED);

    /* The timer's active state has priority over the locked flag. */
    button.active_timer = 2;
    RenderAndAssertState(&button, &parent, NTMV_M2D_OBJ_BUTTON_ACTIVE);
    assert(button.active_timer == 2); /* rendering itself does not tick it */
    NtmvM2dObjButton_TickActiveState(&button);
    assert(button.active_timer == 1);
    NtmvM2dObjButton_TickActiveState(&button);
    assert(button.active_timer == 0);
    NtmvM2dObjButton_TickActiveState(&button);
    assert(button.active_timer == 0);

    /* Hidden and off-screen elements invoke neither backend callback. */
    button.base.flags = NTMV_M2D_UI_HIDDEN;
    ResetCalls();
    button.base.vtable->render(&button.base, &render_context, &parent);
    assert(sFrameCallCount == 0 && sSubmitCallCount == 0);
    button.base.flags = 0;
    button.base.local_position.x = 500;
    ResetCalls();
    button.base.vtable->render(&button.base, &render_context, &parent);
    assert(sFrameCallCount == 0 && sSubmitCallCount == 0);

    /* Remaining virtual callbacks are inherited from UIElement. */
    button.base.vtable->set_position(&button.base, &changed_position);
    button.base.vtable->set_size(&button.base, &changed_size);
    button.base.vtable->set_visible(&button.base, false);
    assert(button.base.local_position.x == 1 && button.base.local_position.y == 2);
    assert(button.base.size.width == 3 && button.base.size.height == 4);
    assert((button.base.flags & NTMV_M2D_UI_HIDDEN) != 0);
    return 0;
}
