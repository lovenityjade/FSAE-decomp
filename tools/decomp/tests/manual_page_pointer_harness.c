#include "game/manual_page_pointer.h"

#include "game/manual_display.h"
#include "game/scrollable_tile_buffer.h"
#include "ntmv/m2d/manual_toc_panel.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
    TEST_VIEWER_SIZE = 0x40628,
    TEST_SCROLL_BUFFER_OFFSET = 0x40370,
    TEST_GESTURE_OFFSET = 0x4060c
};

typedef struct TestGesture {
    int16_t drag_origin_y;
    int16_t previous_y;
    int16_t direction;
    int16_t scroll_origin;
    int16_t direction_anchor_y;
    int16_t delta_history[3];
    int16_t velocity;
    int16_t interaction_mode;
    uint8_t page_region_active;
} TestGesture;

static uint8_t sViewer[TEST_VIEWER_SIZE];
static NtmvM2dManualTocPanel sTocPanel;
static bool sCanScroll;
static int32_t sScrollExtent;
static int32_t sContentTop;
static unsigned int sCanScrollCount;
static unsigned int sExtentCount;
static unsigned int sResetHistoryCount;
static unsigned int sContentTopCount;

static GameScrollableTileBuffer *Buffer(void)
{
    return (GameScrollableTileBuffer *)(
        sViewer + TEST_SCROLL_BUFFER_OFFSET);
}

static TestGesture *Gesture(void)
{
    return (TestGesture *)(sViewer + TEST_GESTURE_OFFSET);
}

void *GameManualViewer_ResolveHostPointer(
    const void *object,
    uint32_t offset)
{
    assert(object == sViewer);
    assert(offset == 0x358);
    return &sTocPanel;
}

bool GameScrollableTileBuffer_CanScroll(
    const GameScrollableTileBuffer *buffer)
{
    assert(buffer == Buffer());
    ++sCanScrollCount;
    return sCanScroll;
}

int32_t GameScrollableTileBuffer_GetScrollExtent(
    const GameScrollableTileBuffer *buffer)
{
    assert(buffer == Buffer());
    ++sExtentCount;
    return sScrollExtent;
}

int32_t GameManualViewer_GetPointerContentTop_020bc574(
    const NtmvM2dManualTocPanel *toc_panel)
{
    assert(toc_panel == &sTocPanel);
    ++sContentTopCount;
    return sContentTop;
}

void GameManualViewer_ResetInputHistory(GameManualViewerDisplay *viewer)
{
    TestGesture *gesture = Gesture();

    assert(viewer == (GameManualViewerDisplay *)sViewer);
    memset(gesture->delta_history, 0, sizeof(gesture->delta_history));
    gesture->velocity = 0;
    ++sResetHistoryCount;
}

static void ResetFixture(void)
{
    memset(sViewer, 0, sizeof(sViewer));
    Buffer()->content = (void *)(uintptr_t)0x1234u;
    Buffer()->viewport_height = 0x142;
    Buffer()->visible_start = 100;
    Buffer()->target_start = 100;
    sCanScroll = true;
    sScrollExtent = 500;
    sContentTop = 148;
    sCanScrollCount = 0;
    sExtentCount = 0;
    sResetHistoryCount = 0;
    sContentTopCount = 0;
}

static bool RunPointer(
    int16_t y,
    uint8_t held,
    uint8_t pressed,
    uint8_t released)
{
    GameTouchState pointer;

    memset(&pointer, 0, sizeof(pointer));
    pointer.y = (uint16_t)y;
    pointer.held = held;
    pointer.pressed = pressed;
    pointer.released = released;
    return GameManualViewer_HandlePagePointer_020bd474(
        (GameManualViewerPageFlow *)sViewer, &pointer);
}

static void BeginDrag(int16_t y)
{
    assert(RunPointer(y, 1, 1, 0));
    assert(Gesture()->drag_origin_y == y);
    assert(Gesture()->previous_y == y);
    assert(Gesture()->direction == 0);
    assert(Gesture()->scroll_origin == Buffer()->visible_start);
    assert(Gesture()->direction_anchor_y == y);
    assert(Gesture()->interaction_mode == 3);
    assert(Gesture()->page_region_active == 1);
    assert(Buffer()->force_upload == 1);
    assert(sResetHistoryCount == 1);
}

static void TestAvailabilityAndCapture(void)
{
    ResetFixture();
    Buffer()->content = NULL;
    assert(!RunPointer(50, 1, 1, 0));
    assert(sCanScrollCount == 0);

    ResetFixture();
    sCanScroll = false;
    assert(!RunPointer(50, 1, 1, 0));
    assert(sCanScrollCount == 1);

    ResetFixture();
    assert(!RunPointer(148, 1, 1, 0));
    assert(Gesture()->page_region_active == 0);
    assert(Gesture()->interaction_mode == 0);
    assert(sContentTopCount == 1);

    ResetFixture();
    BeginDrag(-1); /* ROM uses a signed halfword comparison for pointer Y. */

    ResetFixture();
    Gesture()->delta_history[0] = 7;
    Gesture()->velocity = 9;
    BeginDrag(50);
    assert(Gesture()->delta_history[0] == 0);
    assert(Gesture()->velocity == 0);
}

static void TestHeldDragAndHysteresis(void)
{
    ResetFixture();
    BeginDrag(100);
    Buffer()->force_upload = 0;

    assert(RunPointer(90, 1, 0, 0));
    assert(Gesture()->delta_history[0] == -10);
    assert(Gesture()->delta_history[1] == 0);
    assert(Gesture()->direction == -1);
    assert(Gesture()->direction_anchor_y == 90);
    assert(Gesture()->previous_y == 90);
    assert(Buffer()->target_start == 110);
    assert(Buffer()->force_upload == 0);

    assert(RunPointer(80, 1, 0, 0));
    assert(Gesture()->delta_history[0] == -10);
    assert(Gesture()->delta_history[1] == -10);
    assert(Gesture()->direction_anchor_y == 80);
    assert(Buffer()->target_start == 120);

    /* A reversal of at most three pixels freezes the effective pointer Y. */
    assert(RunPointer(82, 1, 0, 0));
    assert(Gesture()->delta_history[0] == 2);
    assert(Gesture()->direction == -1);
    assert(Gesture()->direction_anchor_y == 80);
    assert(Gesture()->previous_y == 82);
    assert(Buffer()->target_start == 120);

    /* Crossing the hysteresis threshold rebases drag and scroll origins. */
    Buffer()->visible_start = 110;
    Buffer()->force_upload = 0;
    assert(RunPointer(86, 1, 0, 0));
    assert(Gesture()->direction == 1);
    assert(Gesture()->drag_origin_y == 83);
    assert(Gesture()->scroll_origin == 110);
    assert(Gesture()->direction_anchor_y == 86);
    assert(Gesture()->previous_y == 86);
    assert(Buffer()->target_start == 107);
    assert(Buffer()->force_upload == 1);
}

static void TestHeldDragClamping(void)
{
    ResetFixture();
    BeginDrag(50);
    Gesture()->scroll_origin = 0;
    Gesture()->drag_origin_y = 0;
    assert(RunPointer(100, 1, 0, 0));
    assert(Buffer()->target_start == 0);

    Gesture()->previous_y = 100;
    Gesture()->direction = 0;
    Gesture()->scroll_origin = 400;
    Gesture()->drag_origin_y = 100;
    assert(RunPointer(90, 1, 0, 0));
    assert(Buffer()->target_start == 178); /* extent 500 - viewport 322 */
}

static void SeedRelease(
    int16_t delta0,
    int16_t delta1,
    int16_t delta2)
{
    Gesture()->interaction_mode = 3;
    Gesture()->page_region_active = 1;
    Gesture()->delta_history[0] = delta0;
    Gesture()->delta_history[1] = delta1;
    Gesture()->delta_history[2] = delta2;
    Buffer()->visible_start = 100;
    Buffer()->target_start = 100;
}

static void TestReleaseAndInertia(void)
{
    ResetFixture();
    SeedRelease(12, 9, 6);
    assert(RunPointer(90, 0, 0, 1));
    assert(Buffer()->target_start == 91); /* average 9 */
    assert(Gesture()->velocity == 137);  /* 9*16, then decay by 7 */
    assert(Gesture()->interaction_mode == 3);

    ResetFixture();
    SeedRelease(30, 30, 30);
    assert(RunPointer(90, 0, 0, 1));
    assert(Buffer()->target_start == 90);
    assert(Gesture()->velocity == 153); /* capped average 10 */

    ResetFixture();
    SeedRelease(-30, -30, -30);
    assert(RunPointer(90, 0, 0, 1));
    assert(Buffer()->target_start == 110);
    assert(Gesture()->velocity == -153);

    ResetFixture();
    SeedRelease(0, 20, 20);
    assert(RunPointer(90, 0, 0, 1));
    assert(Gesture()->velocity == 0);
    assert(Gesture()->interaction_mode == 0);

    /* Inertia continues without touch, using signed /16 and decay by seven. */
    ResetFixture();
    Gesture()->interaction_mode = 3;
    Gesture()->velocity = 20;
    assert(RunPointer(0, 0, 0, 0));
    assert(Buffer()->target_start == 99);
    assert(Gesture()->velocity == 13);
    assert(Gesture()->interaction_mode == 3);

    Gesture()->velocity = 5;
    assert(RunPointer(0, 0, 0, 0));
    assert(Buffer()->target_start == 100);
    assert(Gesture()->velocity == 0);
    assert(Gesture()->interaction_mode == 0);

    /* Arrival at a clamped edge cancels velocity immediately. */
    ResetFixture();
    Gesture()->interaction_mode = 3;
    Gesture()->velocity = 160;
    Buffer()->visible_start = 0;
    assert(RunPointer(0, 0, 0, 0));
    assert(Buffer()->target_start == 0);
    assert(Gesture()->velocity == 0);
    assert(Gesture()->interaction_mode == 0);
}

static void TestCapturedStateBranches(void)
{
    ResetFixture();
    Gesture()->interaction_mode = 2;
    Gesture()->page_region_active = 1;
    assert(!RunPointer(90, 1, 0, 0));

    /* A new press outside the page clears capture but lets mode-3 inertia run. */
    ResetFixture();
    Gesture()->interaction_mode = 3;
    Gesture()->page_region_active = 1;
    Gesture()->velocity = 20;
    assert(RunPointer(180, 1, 1, 0));
    assert(Gesture()->page_region_active == 0);
    assert(Buffer()->target_start == 99);
    assert(Gesture()->velocity == 13);
}

int main(void)
{
    TestAvailabilityAndCapture();
    TestHeldDragAndHysteresis();
    TestHeldDragClamping();
    TestReleaseAndInertia();
    TestCapturedStateBranches();
    return 0;
}
