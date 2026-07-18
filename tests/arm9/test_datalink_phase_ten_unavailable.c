#include "game/datalink_phase_ten_unavailable.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_SDK_MODE_TWO,
    EVENT_FIRST_TRANSITION,
    EVENT_SECOND_TRANSITION
} Event;

Game_DatalinkPositionObject gGameDatalinkPhaseFiveFirstObject;
Game_DatalinkPositionObject gGameDatalinkPhaseFiveSecondObject;
int32_t gGameDatalinkUnavailableTransitionDuration;
int32_t gGameDatalinkUnavailableFirstYOffset;
int32_t gGameDatalinkUnavailableSecondYOffset;

static Event sEvents[3];
static size_t sEventCount;
static Game_DatalinkFlowState *sExpectedState;

static void RecordEvent(Event event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

void Game_EnterOpaqueSdkDatalinkModeTwo(void *state)
{
    assert(state == sExpectedState);
    assert(sExpectedState->phase_41 == 16);
    RecordEvent(EVENT_SDK_MODE_TWO);
    gGameDatalinkUnavailableTransitionDuration = 7;
    gGameDatalinkUnavailableFirstYOffset = 11;
}

void Game_StartDatalinkObjectTransition(
    Game_DatalinkPositionObject *object,
    int32_t target_x_fx,
    int32_t target_y_fx,
    int32_t duration,
    int mode
)
{
    assert(sExpectedState->phase_41 == 16);
    assert(duration == 7);
    assert(mode == 1);

    if (object == &gGameDatalinkPhaseFiveFirstObject) {
        assert(target_x_fx == 255 * 0x1000);
        assert(target_y_fx == 11 * 0x1000);
        RecordEvent(EVENT_FIRST_TRANSITION);
        gGameDatalinkUnavailableTransitionDuration = 99;
        gGameDatalinkUnavailableSecondYOffset = -20;
        return;
    }

    assert(object == &gGameDatalinkPhaseFiveSecondObject);
    assert(target_x_fx == 24 * 0x1000);
    assert(target_y_fx == 160 * 0x1000);
    RecordEvent(EVENT_SECOND_TRANSITION);
}

static void TestHeaderAndBothTransitionsPreserveLoadOrder(void)
{
    Game_DatalinkFlowState state;

    memset(&state, 0xA5, sizeof(state));
    memset(&gGameDatalinkPhaseFiveFirstObject, 0,
           sizeof(gGameDatalinkPhaseFiveFirstObject));
    memset(&gGameDatalinkPhaseFiveSecondObject, 0,
           sizeof(gGameDatalinkPhaseFiveSecondObject));
    state.phase_41 = 10;
    gGameDatalinkUnavailableTransitionDuration = 5;
    gGameDatalinkUnavailableFirstYOffset = 3;
    gGameDatalinkUnavailableSecondYOffset = 4;
    sExpectedState = &state;
    sEventCount = 0;

    Game_HandleDatalinkPhaseTenUnavailable(&state);

    assert(state.phase_41 == 16);
    assert(state.selection_dirty_43 == 0xA5U);
    assert(state.phase_timer_45 == 0xA5U);
    assert(gGameDatalinkUnavailableTransitionDuration == 99);
    assert(gGameDatalinkUnavailableSecondYOffset == -20);
    assert(sEventCount == 3);
    assert(sEvents[0] == EVENT_SDK_MODE_TWO);
    assert(sEvents[1] == EVENT_FIRST_TRANSITION);
    assert(sEvents[2] == EVENT_SECOND_TRANSITION);
}

int main(void)
{
    TestHeaderAndBothTransitionsPreserveLoadOrder();
    return 0;
}
