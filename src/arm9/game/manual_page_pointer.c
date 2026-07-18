#include "game/manual_page_pointer.h"

#include "game/manual_display.h"
#include "game/scrollable_tile_buffer.h"

#include <stddef.h>
#include <stdint.h>

enum {
    GAME_MANUAL_TOC_PANEL_OFFSET = 0x358,
    GAME_MANUAL_SCROLL_BUFFER_OFFSET = 0x40370,
    GAME_MANUAL_GESTURE_OFFSET = 0x4060c,
    GAME_MANUAL_PAGE_VIEWPORT_HEIGHT = 0x142,
    GAME_MANUAL_PAGE_DRAG_MODE = 3,
    GAME_MANUAL_REVERSAL_THRESHOLD = 3,
    GAME_MANUAL_MAX_RELEASE_STEP = 10,
    GAME_MANUAL_VELOCITY_SCALE = 16,
    GAME_MANUAL_VELOCITY_DECAY = 7
};

typedef struct GameManualPageGesture {
    int16_t drag_origin_y;       /* target +0x4060c */
    int16_t previous_y;          /* target +0x4060e */
    int16_t direction;           /* target +0x40610 */
    int16_t scroll_origin;       /* target +0x40612 */
    int16_t direction_anchor_y;  /* target +0x40614 */
    int16_t delta_history[3];    /* target +0x40616 */
    int16_t velocity;            /* target +0x4061c, 1/16 pixel */
    int16_t interaction_mode;    /* target +0x4061e */
    uint8_t page_region_active;  /* target +0x40620 */
} GameManualPageGesture;

typedef char GameManualPageGestureModeOffsetCheck[
    offsetof(GameManualPageGesture, interaction_mode) == 0x12 ? 1 : -1];
typedef char GameManualPageGestureFlagOffsetCheck[
    offsetof(GameManualPageGesture, page_region_active) == 0x14 ? 1 : -1];

#if UINTPTR_MAX > UINT32_MAX
extern void *GameManualViewer_ResolveHostPointer(
    const void *object,
    uint32_t offset);
#endif

static void *ReadViewerPointer(const void *viewer, uint32_t offset)
{
#if UINTPTR_MAX > UINT32_MAX
    return GameManualViewer_ResolveHostPointer(viewer, offset);
#else
    const uint8_t *bytes = (const uint8_t *)viewer + offset;
    uintptr_t value = 0;
    uint32_t index;

    for (index = 0; index < sizeof(void *); ++index) {
        value |= (uintptr_t)bytes[index] << (index * 8);
    }
    return (void *)value;
#endif
}

static int32_t AbsoluteValue(int32_t value)
{
    return value < 0 ? -value : value;
}

static int16_t DeltaDirection(int32_t delta)
{
    return (int16_t)(delta < 0 ? -1 : 1);
}

static int16_t ClampTarget(
    const GameScrollableTileBuffer *buffer,
    int32_t target)
{
    int32_t maximum =
        GameScrollableTileBuffer_GetScrollExtent(buffer) -
        GAME_MANUAL_PAGE_VIEWPORT_HEIGHT;

    if (target < 0) {
        target = 0;
    }
    if (target > maximum) {
        target = maximum;
    }
    return (int16_t)target;
}

static void BeginPageDrag(
    GameManualViewerPageFlow *viewer,
    GameScrollableTileBuffer *buffer,
    GameManualPageGesture *gesture,
    int16_t pointer_y)
{
    gesture->drag_origin_y = pointer_y;
    gesture->scroll_origin = buffer->visible_start;
    gesture->previous_y = pointer_y;
    gesture->direction = 0;
    gesture->direction_anchor_y = pointer_y;
    GameManualViewer_ResetInputHistory((GameManualViewerDisplay *)viewer);
    gesture->interaction_mode = GAME_MANUAL_PAGE_DRAG_MODE;
    buffer->force_upload = 1;
}

static void UpdateDirection(
    GameScrollableTileBuffer *buffer,
    GameManualPageGesture *gesture,
    int16_t pointer_y,
    int16_t delta,
    int16_t *effective_y)
{
    int16_t direction = DeltaDirection(delta);

    if (gesture->direction == 0) {
        gesture->direction = direction;
        gesture->direction_anchor_y = pointer_y;
        return;
    }
    if (gesture->direction == direction) {
        gesture->direction_anchor_y = pointer_y;
        return;
    }
    if (AbsoluteValue(
            (int32_t)pointer_y - gesture->direction_anchor_y) <=
        GAME_MANUAL_REVERSAL_THRESHOLD) {
        *effective_y = gesture->previous_y;
        return;
    }

    gesture->direction = direction;
    gesture->drag_origin_y = (int16_t)(
        gesture->direction_anchor_y +
        (delta < 0 ? -GAME_MANUAL_REVERSAL_THRESHOLD
                   : GAME_MANUAL_REVERSAL_THRESHOLD));
    gesture->scroll_origin = buffer->visible_start;
    gesture->direction_anchor_y = pointer_y;
    buffer->force_upload = 1;
}

static void UpdateHeldDrag(
    GameScrollableTileBuffer *buffer,
    GameManualPageGesture *gesture,
    int16_t pointer_y)
{
    int16_t effective_y = pointer_y;
    int16_t delta;
    int32_t target;

    gesture->delta_history[2] = gesture->delta_history[1];
    gesture->delta_history[1] = gesture->delta_history[0];
    delta = (int16_t)(pointer_y - gesture->previous_y);
    gesture->delta_history[0] = delta;
    if (delta != 0) {
        UpdateDirection(
            buffer, gesture, pointer_y, delta, &effective_y);
    }

    target =
        (int32_t)gesture->scroll_origin + gesture->drag_origin_y -
        effective_y;
    buffer->target_start = ClampTarget(buffer, target);
    gesture->previous_y = pointer_y;
}

static void BeginInertia(GameManualPageGesture *gesture)
{
    int32_t average;

    if (gesture->delta_history[0] == 0) {
        gesture->velocity = 0;
        return;
    }

    average =
        ((int32_t)gesture->delta_history[0] +
         gesture->delta_history[1] +
         gesture->delta_history[2]) /
        3;
    if (average > GAME_MANUAL_MAX_RELEASE_STEP) {
        average = GAME_MANUAL_MAX_RELEASE_STEP;
    } else if (average < -GAME_MANUAL_MAX_RELEASE_STEP) {
        average = -GAME_MANUAL_MAX_RELEASE_STEP;
    }
    gesture->velocity = (int16_t)(average * GAME_MANUAL_VELOCITY_SCALE);
}

static void AdvanceInertia(
    GameScrollableTileBuffer *buffer,
    GameManualPageGesture *gesture)
{
    int32_t velocity = gesture->velocity;

    if (velocity != 0) {
        int32_t target =
            buffer->visible_start - velocity / GAME_MANUAL_VELOCITY_SCALE;
        int32_t magnitude = AbsoluteValue(velocity) -
            GAME_MANUAL_VELOCITY_DECAY;

        buffer->target_start = ClampTarget(buffer, target);
        if (magnitude < 0) {
            magnitude = 0;
        }
        gesture->velocity = (int16_t)(
            velocity < 0 ? -magnitude : magnitude);
        if (buffer->visible_start == buffer->target_start) {
            gesture->velocity = 0;
        }
    }

    if (gesture->velocity == 0) {
        gesture->interaction_mode = 0;
    }
}

/* 0x020bd474..0x020bd807 */
bool GameManualViewer_HandlePagePointer_020bd474(
    GameManualViewerPageFlow *viewer,
    const GameTouchState *pointer)
{
    uint8_t *bytes = (uint8_t *)viewer;
    GameScrollableTileBuffer *buffer =
        (GameScrollableTileBuffer *)(
            bytes + GAME_MANUAL_SCROLL_BUFFER_OFFSET);
    GameManualPageGesture *gesture =
        (GameManualPageGesture *)(bytes + GAME_MANUAL_GESTURE_OFFSET);
    int16_t pointer_y = (int16_t)pointer->y;

    if (buffer->content == NULL ||
        !GameScrollableTileBuffer_CanScroll(buffer)) {
        return false;
    }

    if (pointer->pressed != 0) {
        const NtmvM2dManualTocPanel *toc_panel =
            ReadViewerPointer(viewer, GAME_MANUAL_TOC_PANEL_OFFSET);

        gesture->page_region_active =
            pointer_y <
            GameManualViewer_GetPointerContentTop_020bc574(toc_panel);
        if (gesture->page_region_active != 0) {
            BeginPageDrag(
                viewer, buffer, gesture, pointer_y);
            return true;
        }
    }

    if (gesture->interaction_mode != GAME_MANUAL_PAGE_DRAG_MODE) {
        return false;
    }

    if (gesture->page_region_active != 0 && pointer->held != 0) {
        UpdateHeldDrag(buffer, gesture, pointer_y);
        return true;
    }
    if (gesture->page_region_active != 0 && pointer->released != 0) {
        BeginInertia(gesture);
    }
    AdvanceInertia(buffer, gesture);
    return true;
}
