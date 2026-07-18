#include "ntmv/m2d/manual_button.h"
#include "ntmv/ui_renderer.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

typedef struct RenderRecord {
    int32_t animation_handle;
    uint32_t frame;
    int32_t x_fx12;
    int32_t y_fx12;
} RenderRecord;

static void *sExpectedRenderer = (void *)(uintptr_t)0x1234;
static RenderRecord sRecords[2];
static unsigned int sRecordCount;
static int32_t sPendingHandle;
static uint32_t sPendingFrame;
static bool sFramePending;

bool NtmvUiRenderer_SetAnimationFrame(
    NtmvUiRenderer *renderer, int32_t animation_handle, uint32_t frame)
{
    assert(renderer == (NtmvUiRenderer *)sExpectedRenderer);
    assert(!sFramePending);
    assert(sRecordCount < 2);
    sPendingHandle = animation_handle;
    sPendingFrame = frame;
    sFramePending = true;
    return true;
}

bool NtmvUiRenderer_SubmitCell(
    NtmvUiRenderer *renderer,
    int32_t animation_handle,
    int32_t x_fx12,
    int32_t y_fx12)
{
    RenderRecord *record;

    assert(renderer == (NtmvUiRenderer *)sExpectedRenderer);
    assert(sFramePending);
    assert(animation_handle == sPendingHandle);
    assert(sRecordCount < 2);
    record = &sRecords[sRecordCount++];
    record->animation_handle = animation_handle;
    record->frame = sPendingFrame;
    record->x_fx12 = x_fx12;
    record->y_fx12 = y_fx12;
    sFramePending = false;
    return true;
}

static void ResetCalls(void)
{
    memset(sRecords, 0, sizeof(sRecords));
    sRecordCount = 0;
    sPendingHandle = -1;
    sPendingFrame = 99;
    sFramePending = false;
}

static void AssertRecord(
    unsigned int index, int32_t handle, uint32_t frame)
{
    assert(index < sRecordCount);
    assert(sRecords[index].animation_handle == handle);
    assert(sRecords[index].frame == frame);
    assert(sRecords[index].x_fx12 == 112 * 0x1000);
    assert(sRecords[index].y_fx12 == 75 * 0x1000);
}

static void RenderAndAssert(
    NtmvM2dManualButton *button,
    const NtmvM2dPoint *parent,
    int32_t main_handle,
    uint32_t expected_state,
    bool expect_overlay)
{
    void *render_context = sExpectedRenderer;

    ResetCalls();
    button->base.base.vtable->render(
        &button->base.base, &render_context, parent);
    assert(!sFramePending);
    assert(sRecordCount == (expect_overlay ? 2u : 1u));
    if (expect_overlay) {
        AssertRecord(0, 13, expected_state);
        AssertRecord(1, main_handle, expected_state);
    } else {
        AssertRecord(0, main_handle, expected_state);
    }
}

int main(void)
{
    NtmvM2dManualButton button;
    NtmvM2dPoint position = {12, -5};
    NtmvM2dSize size = {32, 16};
    NtmvM2dPoint parent = {100, 80};
    NtmvM2dPoint changed_position = {1, 2};
    NtmvM2dSize changed_size = {3, 4};
    void *render_context = sExpectedRenderer;

    memset(&button, 0xa5, sizeof(button));
    assert(NtmvM2dManualButton_Construct(&button) == &button);
    assert(button.base.base.vtable ==
           (const NtmvM2dUIElementVTable *)&gNtmvM2dManualButtonVTable);
    assert(button.base.animation_handle == -1);
    assert(button.base.active_timer == 0);
    assert(button.alternate_animation_handle == -1);
    assert(button.overlay_animation_handle == -1);
    assert(button.use_alternate_animation == 0);
    assert(button.selected == 0);
    /* The recovered constructor deliberately leaves these bytes untouched. */
    assert(button.reserved_1c == UINT32_C(0xa5a5a5a5));
    assert(button.reserved_2a[0] == 0xa5 && button.reserved_2a[1] == 0xa5);

    NtmvM2dManualButton_Configure(
        &button, &position, &size, 7, 11, 13);
    assert(button.base.base.local_position.x == 12);
    assert(button.base.base.local_position.y == -5);
    assert(button.base.base.size.width == 32);
    assert(button.base.base.size.height == 16);
    assert(button.base.animation_handle == 7);
    assert(button.alternate_animation_handle == 11);
    assert(button.overlay_animation_handle == 13);

    RenderAndAssert(
        &button, &parent, 7, NTMV_M2D_MANUAL_BUTTON_NORMAL, true);
    assert(button.base.base.world_center.x == 112);
    assert(button.base.base.world_center.y == 75);

    button.selected = true;
    RenderAndAssert(
        &button, &parent, 7, NTMV_M2D_MANUAL_BUTTON_SELECTED, true);

    /* Hover has priority over the selected state. */
    button.base.base.flags = NTMV_M2D_UI_HOVERED;
    RenderAndAssert(
        &button, &parent, 7, NTMV_M2D_MANUAL_BUTTON_ACTIVE, true);

    /* Locked has priority over both hover and selection. */
    button.base.base.flags =
        NTMV_M2D_UI_HOVERED | NTMV_M2D_UI_HIT_STATE_LOCKED;
    RenderAndAssert(
        &button, &parent, 7, NTMV_M2D_MANUAL_BUTTON_LOCKED, true);

    /* The inherited active timer has the highest priority. */
    button.base.active_timer = 2;
    RenderAndAssert(
        &button, &parent, 7, NTMV_M2D_MANUAL_BUTTON_ACTIVE, true);
    assert(button.base.active_timer == 2);

    button.base.active_timer = 0;
    button.base.base.flags = 0;
    button.selected = false;
    NtmvM2dManualButton_SetAlternateAnimation(&button, true);
    assert(button.use_alternate_animation == 1);
    RenderAndAssert(
        &button, &parent, 11, NTMV_M2D_MANUAL_BUTTON_NORMAL, true);

    button.overlay_animation_handle = -1;
    RenderAndAssert(
        &button, &parent, 11, NTMV_M2D_MANUAL_BUTTON_NORMAL, false);

    /* Hidden and off-screen elements invoke neither renderer callback. */
    button.base.base.flags = NTMV_M2D_UI_HIDDEN;
    ResetCalls();
    button.base.base.vtable->render(
        &button.base.base, &render_context, &parent);
    assert(sRecordCount == 0 && !sFramePending);
    button.base.base.flags = 0;
    button.base.base.local_position.x = 500;
    ResetCalls();
    button.base.base.vtable->render(
        &button.base.base, &render_context, &parent);
    assert(sRecordCount == 0 && !sFramePending);

    /* Slots 0 and 2-4 remain inherited from UIElement through ObjButton. */
    button.base.base.vtable->set_position(
        &button.base.base, &changed_position);
    button.base.base.vtable->set_size(&button.base.base, &changed_size);
    button.base.base.vtable->set_visible(&button.base.base, false);
    assert(button.base.base.local_position.x == 1);
    assert(button.base.base.local_position.y == 2);
    assert(button.base.base.size.width == 3);
    assert(button.base.base.size.height == 4);
    assert((button.base.base.flags & NTMV_M2D_UI_HIDDEN) != 0);
    return 0;
}
