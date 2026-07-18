#include "game/datalink_phase_six.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_PROGRESS,
    EVENT_PROCESS_OUTCOME
} Event;

Game_DatalinkPositionObject gGameDatalinkPhaseFiveFirstObject;

static Event sEvents[2];
static size_t sEventCount;
static int32_t sProgress;
static int sOutcomeResult;
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
    assert(object == &gGameDatalinkPhaseFiveFirstObject);
    assert(sExpectedState->phase_41 == 6);
    RecordEvent(EVENT_PROGRESS);
    return sProgress;
}

int Game_ProcessDatalinkSlotOutcome(void)
{
    assert(sExpectedState->phase_41 == 10);
    assert(sExpectedState->phase_timer_45 == 0xA5U);
    RecordEvent(EVENT_PROCESS_OUTCOME);
    return sOutcomeResult;
}

static void ResetHarness(Game_DatalinkFlowState *state)
{
    memset(state, 0xA5, sizeof(*state));
    memset(&gGameDatalinkPhaseFiveFirstObject, 0,
           sizeof(gGameDatalinkPhaseFiveFirstObject));
    state->phase_41 = 6;
    sExpectedState = state;
    sEventCount = 0;
    sProgress = 0;
    sOutcomeResult = 0;
}

static void TestIncompleteProgressHasNoOtherEffects(void)
{
    static const int32_t incomplete_values[] = {0, 0x0FFF, 0x1001, -1};
    Game_DatalinkFlowState state;
    size_t index;

    for (index = 0;
         index < sizeof(incomplete_values) / sizeof(incomplete_values[0]);
         ++index) {
        ResetHarness(&state);
        sProgress = incomplete_values[index];

        Game_UpdateDatalinkPhaseSix(&state);

        assert(state.phase_41 == 6);
        assert(state.phase_timer_45 == 0xA5U);
        assert(sEventCount == 1);
        assert(sEvents[0] == EVENT_PROGRESS);
    }
}

static void TestCompletionWithoutOutcomeEntersPhaseTen(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state);
    sProgress = 0x1000;

    Game_UpdateDatalinkPhaseSix(&state);

    assert(state.phase_41 == 10);
    assert(state.phase_timer_45 == 0xA5U);
    assert(sEventCount == 2);
    assert(sEvents[0] == EVENT_PROGRESS);
    assert(sEvents[1] == EVENT_PROCESS_OUTCOME);
}

static void TestNonzeroOutcomeDivertsToTimedPhaseThirty(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state);
    sProgress = 0x1000;
    sOutcomeResult = -1;

    Game_UpdateDatalinkPhaseSix(&state);

    assert(state.phase_41 == 30);
    assert(state.phase_timer_45 == 180U);
    assert(state.unknown_00[0] == 0xA5U);
    assert(sEventCount == 2);
    assert(sEvents[0] == EVENT_PROGRESS);
    assert(sEvents[1] == EVENT_PROCESS_OUTCOME);
}

int main(void)
{
    TestIncompleteProgressHasNoOtherEffects();
    TestCompletionWithoutOutcomeEntersPhaseTen();
    TestNonzeroOutcomeDivertsToTimedPhaseThirty();
    return 0;
}
