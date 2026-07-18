#include "game/datalink_phase_thirty_one.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_SLOT_CLEANUP,
    EVENT_SCREEN_COMMIT
} Event;

static Game_DatalinkFlowState *sExpectedState;
static Event sEvents[2];
static size_t sEventCount;
static bool sMutateTimerDuringCleanup;
static uint8_t sTimerAfterCleanup;

static void RecordEvent(Event event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

void Game_CleanupDatalinkPlayerSlots(void)
{
    assert(sEventCount == 0);
    assert(sExpectedState->phase_41 == 31U);
    assert(sExpectedState->phase_timer_45 == 15U);
    RecordEvent(EVENT_SLOT_CLEANUP);
    if (sMutateTimerDuringCleanup) {
        sExpectedState->phase_timer_45 = sTimerAfterCleanup;
    }
}

void Game_CommitDatalinkScreenUpdate(void)
{
    assert(sExpectedState->phase_41 == 6U);
    assert(sExpectedState->phase_timer_45 == 0U);
    RecordEvent(EVENT_SCREEN_COMMIT);
}

static void ResetHarness(Game_DatalinkFlowState *state, uint8_t timer)
{
    memset(state, 0xA5, sizeof(*state));
    state->phase_41 = 31;
    state->phase_timer_45 = timer;
    sExpectedState = state;
    sEventCount = 0;
    sMutateTimerDuringCleanup = false;
    sTimerAfterCleanup = 0;
}

static void TestLastCountdownFrameDoesNotFinishPhase(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state, 1);

    Game_UpdateDatalinkPhaseThirtyOne(&state);

    assert(sEventCount == 0);
    assert(state.phase_timer_45 == 0U);
    assert(state.phase_41 == 31U);
}

static void TestCleanupIsFollowedByFreshTimerRead(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state, 15);
    sMutateTimerDuringCleanup = true;
    sTimerAfterCleanup = 7;

    Game_UpdateDatalinkPhaseThirtyOne(&state);

    assert(sEventCount == 1);
    assert(sEvents[0] == EVENT_SLOT_CLEANUP);
    assert(state.phase_timer_45 == 6U);
    assert(state.phase_41 == 31U);
}

static void TestCleanupCanExposeZeroToCompletionPath(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state, 15);
    sMutateTimerDuringCleanup = true;
    sTimerAfterCleanup = 0;

    Game_UpdateDatalinkPhaseThirtyOne(&state);

    assert(sEventCount == 2);
    assert(sEvents[0] == EVENT_SLOT_CLEANUP);
    assert(sEvents[1] == EVENT_SCREEN_COMMIT);
    assert(state.phase_timer_45 == 0U);
    assert(state.phase_41 == 6U);
}

static void TestZeroTimerFinishesWithoutCleanup(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state, 0);

    Game_UpdateDatalinkPhaseThirtyOne(&state);

    assert(sEventCount == 1);
    assert(sEvents[0] == EVENT_SCREEN_COMMIT);
    assert(state.phase_timer_45 == 0U);
    assert(state.phase_41 == 6U);
}

int main(void)
{
    TestLastCountdownFrameDoesNotFinishPhase();
    TestCleanupIsFollowedByFreshTimerRead();
    TestCleanupCanExposeZeroToCompletionPath();
    TestZeroTimerFinishesWithoutCleanup();
    return 0;
}
