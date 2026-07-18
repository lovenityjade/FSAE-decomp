#include "ntmv/m2d/scroll_controls.h"
#include "ntmv/ui_renderer.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void *sExpectedAllocator = (void *)(uintptr_t)0x1111;
static void *sExpectedRenderer = (void *)(uintptr_t)0x2222;
static unsigned int sAllocationCount;
static unsigned int sFreeCount;
static uint16_t sCreatedSequences[4];
static unsigned int sCreateCount;
static unsigned int sFrameCount;
static unsigned int sSubmitCount;
static unsigned int sScaledCount;

void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size)
{
    void *memory;

    assert(allocator == sExpectedAllocator);
    ++sAllocationCount;
    memory = malloc(size);
    assert(memory != NULL);
    memset(memory, 0xa5, size);
    return memory;
}

void NNS_FndFreeToAllocator(void *allocator, void *memory)
{
    assert(allocator == sExpectedAllocator);
    assert(memory != NULL);
    ++sFreeCount;
    free(memory);
}

int32_t NtmvUiRenderer_CreateAnimation(
    NtmvUiRenderer *animation_manager, uint16_t sequence_index)
{
    assert(animation_manager == (NtmvUiRenderer *)sExpectedRenderer);
    assert(sCreateCount < 4);
    sCreatedSequences[sCreateCount++] = sequence_index;
    return 100 + sequence_index;
}

bool NtmvUiRenderer_SetAnimationFrame(
    NtmvUiRenderer *renderer, int32_t animation_handle, uint32_t frame)
{
    assert(renderer == (NtmvUiRenderer *)sExpectedRenderer);
    if (animation_handle == 105 || animation_handle == 106) {
        assert(frame <= 2);
        ++sFrameCount;
    }
    return true;
}

bool NtmvUiRenderer_SubmitCell(
    NtmvUiRenderer *renderer,
    int32_t animation_handle,
    int32_t x_fx12,
    int32_t y_fx12)
{
    assert(renderer == (NtmvUiRenderer *)sExpectedRenderer);
    assert(animation_handle == 105 || animation_handle == 106 ||
        animation_handle == 120 || animation_handle == 121);
    if (animation_handle == 105 || animation_handle == 106) {
        assert(x_fx12 == 75 * 0x1000 || x_fx12 == 125 * 0x1000);
        assert(y_fx12 == 50 * 0x1000);
        ++sSubmitCount;
    }
    return true;
}

bool NtmvUiRenderer_TickAnimation(
    NtmvUiRenderer *renderer, int32_t resource_id, int32_t scale_fx12)
{
    assert(renderer == (NtmvUiRenderer *)sExpectedRenderer);
    assert(resource_id == 120 || resource_id == 121);
    assert(scale_fx12 == 0x1000);
    ++sScaledCount;
    return true;
}

static NtmvM2dObjButton *GetObjChild(
    NtmvM2dScrollButton *button, unsigned int index)
{
    return (NtmvM2dObjButton *)button->base.children[index];
}

static NtmvM2dUIStatic *GetStaticChild(
    NtmvM2dScrollIndicator *indicator, unsigned int index)
{
    return (NtmvM2dUIStatic *)indicator->base.children[index];
}

int main(void)
{
    NtmvM2dAllocatorContext allocator = {sExpectedAllocator};
    NtmvM2dScrollButton button;
    NtmvM2dScrollIndicator indicator;
    NtmvM2dPoint button_position = {100, 50};
    NtmvM2dPoint indicator_position = {200, 60};
    NtmvM2dPoint zero = {0, 0};
    NtmvM2dScrollPointerState pointer;
    NtmvM2dScrollButtonAction action;
    uint16_t indicator_sequences[2] = {20, 21};
    NtmvM2dObjButton *increment;
    NtmvM2dObjButton *decrement;
    NtmvM2dUIStatic *previous_indicator;
    NtmvM2dUIStatic *next_indicator;
    void *render_context = sExpectedRenderer;

    assert(NtmvM2dScrollButton_Construct(&button) == &button);
    assert(button.base.base.vtable ==
           (const NtmvM2dUIElementVTable *)&gNtmvM2dScrollButtonVTable);
    assert((int16_t)button.base.reserved_1a == -1);
    assert(button.item_count == 0 && button.page_size == 0);
    NtmvM2dScrollButton_Configure(
        &button, &allocator, sExpectedRenderer, &button_position, 3);
    assert(button.base.child_count == 2);
    assert(button.base.base.local_position.x == 100);
    assert(button.base.base.local_position.y == 50);
    assert(button.base.base.size.width == 0x65);
    assert(button.base.base.size.height == 0x14);
    assert(button.page_size == 3);
    increment = GetObjChild(&button, 0);
    decrement = GetObjChild(&button, 1);
    assert(increment->base.local_position.x == 25);
    assert(decrement->base.local_position.x == -25);
    assert(increment->base.size.width == 0x33);
    assert(increment->base.size.height == 0x14);
    assert(increment->animation_handle == 106);
    assert(decrement->animation_handle == 105);

    button.item_count = 10;
    NtmvM2dScrollButton_SetScrollOffset(&button, 0);
    assert((decrement->base.flags & NTMV_M2D_UI_HIT_STATE_LOCKED) != 0);
    assert((increment->base.flags & NTMV_M2D_UI_HIT_STATE_LOCKED) == 0);
    NtmvM2dScrollButton_SetScrollOffset(&button, 7);
    assert((decrement->base.flags & NTMV_M2D_UI_HIT_STATE_LOCKED) == 0);
    assert((increment->base.flags & NTMV_M2D_UI_HIT_STATE_LOCKED) != 0);

    NtmvM2dScrollButton_SetDirectionalHover(&button, true, false);
    assert((decrement->base.flags & NTMV_M2D_UI_HOVERED) != 0);
    assert((increment->base.flags & NTMV_M2D_UI_HOVERED) == 0);
    NtmvM2dScrollButton_SetDirectionalHover(&button, false, true);
    assert((decrement->base.flags & NTMV_M2D_UI_HOVERED) == 0);
    assert((increment->base.flags & NTMV_M2D_UI_HOVERED) != 0);

    increment->active_timer = 2;
    decrement->active_timer = 1;
    NtmvM2dScrollButton_TickChildren(&button);
    assert(increment->active_timer == 1);
    assert(decrement->active_timer == 0);

    /* Panel rendering establishes the children's world-space hit rectangles. */
    increment->base.flags = 0;
    decrement->base.flags = 0;
    NtmvM2dUIPanel_Render(&button.base, &render_context, &zero);
    assert(sFrameCount == 2 && sSubmitCount == 2);
    NtmvM2dScrollButton_SetScrollOffset(&button, 0);

    memset(&pointer, 0, sizeof(pointer));
    memset(&action, 0, sizeof(action));
    pointer.position.x = 12;
    pointer.position.y = 12;
    pointer.active = 1;
    pointer.pressed = 1;
    assert(!NtmvM2dScrollButton_HandlePointer(&button, &action, &pointer));
    assert(action.activated == 0);
    assert((int16_t)button.base.reserved_1a == -1);

    memset(&pointer, 0, sizeof(pointer));
    memset(&action, 0, sizeof(action));
    pointer.position.x = 125;
    pointer.position.y = 50;
    pointer.active = 1;
    pointer.pressed = 1;
    assert(NtmvM2dScrollButton_HandlePointer(&button, &action, &pointer));
    assert(action.activated == 1);
    assert(action.decrement == 0 && action.increment == 1);
    assert((int16_t)button.base.reserved_1a == 0);

    memset(&pointer, 0, sizeof(pointer));
    memset(&action, 0, sizeof(action));
    pointer.position.x = 125;
    pointer.position.y = 50;
    pointer.released = 1;
    assert(NtmvM2dScrollButton_HandlePointer(&button, &action, &pointer));
    assert(action.activated == 0);
    assert((int16_t)button.base.reserved_1a == -1);
    assert((increment->base.flags & NTMV_M2D_UI_HOVERED) == 0);

    /* A locked hit is consumed but cannot become the selected child. */
    NtmvM2dScrollButton_SetScrollOffset(&button, 7);
    memset(&pointer, 0, sizeof(pointer));
    memset(&action, 0, sizeof(action));
    pointer.position.x = 125;
    pointer.position.y = 50;
    pointer.active = 1;
    pointer.pressed = 1;
    assert(NtmvM2dScrollButton_HandlePointer(&button, &action, &pointer));
    assert(action.activated == 0);
    assert((int16_t)button.base.reserved_1a == -1);

    assert(NtmvM2dScrollIndicator_Construct(&indicator) == &indicator);
    assert(indicator.base.base.vtable ==
           (const NtmvM2dUIElementVTable *)&gNtmvM2dScrollIndicatorVTable);
    NtmvM2dScrollIndicator_Configure(
        &indicator,
        &allocator,
        sExpectedRenderer,
        &indicator_position,
        50,
        3,
        indicator_sequences);
    assert(indicator.base.child_count == 2);
    assert(indicator.base.base.size.width == 13);
    assert(indicator.base.base.size.height == 50);
    assert(indicator.page_size == 3);
    assert(indicator.child_extent[0] == 8 && indicator.child_extent[1] == 8);
    previous_indicator = GetStaticChild(&indicator, 0);
    next_indicator = GetStaticChild(&indicator, 1);
    assert(previous_indicator->base.local_position.y == -19);
    assert(next_indicator->base.local_position.y == 19);
    assert(previous_indicator->base.size.width == 13);
    assert(previous_indicator->base.size.height == 8);
    assert(previous_indicator->resource_id == 120);
    assert(next_indicator->resource_id == 121);

    indicator.base.reserved_1a = 10; /* total item count at inherited +0x1a */
    NtmvM2dScrollIndicator_SetScrollOffset(&indicator, 0);
    sScaledCount = 0;
    NtmvM2dScrollIndicator_UpdateVisibility(&indicator, sExpectedRenderer);
    assert(sScaledCount == 2);
    assert((previous_indicator->base.flags & NTMV_M2D_UI_HIDDEN) != 0);
    assert((next_indicator->base.flags & NTMV_M2D_UI_HIDDEN) == 0);
    NtmvM2dScrollIndicator_SetScrollOffset(&indicator, 7);
    NtmvM2dScrollIndicator_UpdateVisibility(&indicator, sExpectedRenderer);
    assert((previous_indicator->base.flags & NTMV_M2D_UI_HIDDEN) == 0);
    assert((next_indicator->base.flags & NTMV_M2D_UI_HIDDEN) != 0);

    NtmvM2dScrollIndicator_RepositionChildren(&indicator, 40);
    assert(previous_indicator->base.local_position.y == -14);
    assert(next_indicator->base.local_position.y == 14);

    assert(sCreateCount == 4);
    assert(sCreatedSequences[0] == 6 && sCreatedSequences[1] == 5);
    assert(sCreatedSequences[2] == 20 && sCreatedSequences[3] == 21);
    assert(sAllocationCount == 6);
    NtmvM2dUIPanel_Destroy(&button.base, &allocator);
    NtmvM2dUIPanel_Destroy(&indicator.base, &allocator);
    assert(sFreeCount == 6);
    return 0;
}
