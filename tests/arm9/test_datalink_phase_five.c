#include "game/datalink_phase_five.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_PROGRESS,
    EVENT_FIRST_TRANSITION,
    EVENT_SECOND_TRANSITION
} Event;

volatile uint32_t gGameDatalinkDisplayControl;
Game_DatalinkPositionObject gGameDatalinkPhaseFiveGateObject;
Game_DatalinkPositionObject gGameDatalinkPhaseFiveFirstObject;
Game_DatalinkPositionObject gGameDatalinkPhaseFiveSecondObject;
int32_t gGameDatalinkPhaseFiveTransitionDuration;

static Event sEvents[3];
static size_t sEventCount;
static int32_t sProgress;
static int32_t sExpectedDuration;
static Game_DatalinkFlowState *sExpectedState;

static void RecordEvent(Event event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

int32_t Game_GetDatalinkObjectTransitionProgressFx(
    const Game_DatalinkPositionObject *object
)
{
    assert(object == &gGameDatalinkPhaseFiveGateObject);
    assert(gGameDatalinkDisplayControl == 0xA5A5BFA5U);
    assert(sExpectedState->phase_41 == 5);
    RecordEvent(EVENT_PROGRESS);
    return sProgress;
}

void Game_StartDatalinkObjectTransition(
    Game_DatalinkPositionObject *object,
    int32_t target_x_fx,
    int32_t target_y_fx,
    int32_t duration,
    int mode
)
{
    assert(sExpectedState->phase_41 == 5);
    assert(duration == sExpectedDuration);
    assert(mode == 2);

    if (object == &gGameDatalinkPhaseFiveFirstObject) {
        assert(target_x_fx == 255 * 0x1000);
        assert(target_y_fx == 0);
        RecordEvent(EVENT_FIRST_TRANSITION);
        gGameDatalinkPhaseFiveTransitionDuration = 99;
        return;
    }

    assert(object == &gGameDatalinkPhaseFiveSecondObject);
    assert(target_x_fx == 24 * 0x1000);
    assert(target_y_fx == 180 * 0x1000);
    RecordEvent(EVENT_SECOND_TRANSITION);
}

static void ResetHarness(Game_DatalinkFlowState *state)
{
    memset(state, 0xA5, sizeof(*state));
    memset(&gGameDatalinkPhaseFiveGateObject, 0,
           sizeof(gGameDatalinkPhaseFiveGateObject));
    memset(&gGameDatalinkPhaseFiveFirstObject, 0,
           sizeof(gGameDatalinkPhaseFiveFirstObject));
    memset(&gGameDatalinkPhaseFiveSecondObject, 0,
           sizeof(gGameDatalinkPhaseFiveSecondObject));
    state->phase_41 = 5;
    sExpectedState = state;
    sEventCount = 0;
    sProgress = 0;
    gGameDatalinkDisplayControl = 0xA5A5A5A5U;
    sExpectedDuration = 7;
    gGameDatalinkPhaseFiveTransitionDuration = sExpectedDuration;
}

static void TestIncompleteProgressOnlyUpdatesDisplayField(void)
{
    static const int32_t incomplete_values[] = {0, 0x0FFF, 0x1001, -1};
    Game_DatalinkFlowState state;
    size_t index;

    for (index = 0;
         index < sizeof(incomplete_values) / sizeof(incomplete_values[0]);
         ++index) {
        ResetHarness(&state);
        sProgress = incomplete_values[index];

        Game_UpdateDatalinkPhaseFive(&state);

        assert(gGameDatalinkDisplayControl == 0xA5A5BFA5U);
        assert(state.phase_41 == 5);
        assert(sEventCount == 1);
        assert(sEvents[0] == EVENT_PROGRESS);
    }
}

static void TestCompletionStartsBothTransitionsBeforePhaseAdvance(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state);
    sProgress = 0x1000;

    Game_UpdateDatalinkPhaseFive(&state);

    assert(gGameDatalinkDisplayControl == 0xA5A5BFA5U);
    assert(state.phase_41 == 6);
    assert(state.unknown_00[0] == 0xA5U);
    assert(gGameDatalinkPhaseFiveTransitionDuration == 99);
    assert(sEventCount == 3);
    assert(sEvents[0] == EVENT_PROGRESS);
    assert(sEvents[1] == EVENT_FIRST_TRANSITION);
    assert(sEvents[2] == EVENT_SECOND_TRANSITION);
}

int main(void)
{
    TestIncompleteProgressOnlyUpdatesDisplayField();
    TestCompletionStartsBothTransitionsBeforePhaseAdvance();
    return 0;
}
