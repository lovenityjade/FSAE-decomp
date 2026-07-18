#include "game/datalink_phase_thirty.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_SCENE_COMMAND,
    EVENT_SCREEN_COMMIT,
    EVENT_STATE_CHANGE
} Event;

Game_DatalinkSceneOwner gGameDatalinkSceneOwner;
Game_DatalinkPhaseThirtyObjectOwner
    gGameDatalinkPhaseThirtyObjectOwner;
volatile uint16_t gGameDatalinkPhaseThirtyAdvanceFlags;

static Game_DatalinkFlowState *sExpectedState;
static Event sEvents[3];
static size_t sEventCount;
static uint16_t sAdvanceFlagsAfterSceneCommand;

static void RecordEvent(Event event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

static void AssertFixedPosition(void)
{
    assert(gGameDatalinkPhaseThirtyObjectOwner
               .position_transition_duration_69c == 0);
    assert(gGameDatalinkPhaseThirtyObjectOwner.position_x_fx_6a4 ==
           224 * 0x1000);
    assert(gGameDatalinkPhaseThirtyObjectOwner.position_y_fx_6a8 ==
           192 * 0x1000);
}

void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command
)
{
    assert(scene == &gGameDatalinkSceneOwner);
    assert(command == 0x35);
    assert(sExpectedState->phase_41 == 30U);
    assert(sExpectedState->phase_timer_45 == 0U);
    AssertFixedPosition();
    RecordEvent(EVENT_SCENE_COMMAND);
    gGameDatalinkPhaseThirtyAdvanceFlags =
        sAdvanceFlagsAfterSceneCommand;
}

void Game_CommitDatalinkScreenUpdate(void)
{
    assert(sEventCount == 1);
    assert(sEvents[0] == EVENT_SCENE_COMMAND);
    assert(sExpectedState->phase_41 == 30U);
    assert(sExpectedState->phase_timer_45 == 0U);
    RecordEvent(EVENT_SCREEN_COMMIT);
}

void Game_ChangeDatalinkState(
    int state,
    int participant_count,
    int argument_2,
    int argument_3
)
{
    assert(sEventCount == 2);
    assert(sEvents[1] == EVENT_SCREEN_COMMIT);
    assert(state == 15);
    assert(participant_count == 0);
    assert(argument_2 == 0);
    assert(argument_3 == 0);
    assert(sExpectedState->phase_41 == 30U);
    assert(sExpectedState->phase_timer_45 == 0U);
    RecordEvent(EVENT_STATE_CHANGE);
}

static void ResetHarness(
    Game_DatalinkFlowState *state,
    uint8_t timer,
    uint16_t advance_flags_after_command
)
{
    memset(state, 0xA5, sizeof(*state));
    memset(&gGameDatalinkSceneOwner, 0xA5,
           sizeof(gGameDatalinkSceneOwner));
    memset(&gGameDatalinkPhaseThirtyObjectOwner, 0xA5,
           sizeof(gGameDatalinkPhaseThirtyObjectOwner));
    state->phase_41 = 30;
    state->phase_timer_45 = timer;
    gGameDatalinkPhaseThirtyAdvanceFlags = 0xFFFFU;
    sExpectedState = state;
    sEventCount = 0;
    sAdvanceFlagsAfterSceneCommand = advance_flags_after_command;
}

static void TestCountdownRepositionsWithoutCommands(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state, 2, 1);

    Game_UpdateDatalinkPhaseThirty(&state);

    AssertFixedPosition();
    assert(state.phase_timer_45 == 1U);
    assert(state.phase_41 == 30U);
    assert(sEventCount == 0);
    assert(gGameDatalinkPhaseThirtyAdvanceFlags == 0xFFFFU);
}

static void TestClosedAdvanceGateStopsAfterSceneCommand(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state, 0, 0);

    Game_UpdateDatalinkPhaseThirty(&state);

    assert(sEventCount == 1);
    assert(sEvents[0] == EVENT_SCENE_COMMAND);
    assert(state.phase_timer_45 == 0U);
    assert(state.phase_41 == 30U);
}

static void TestOpenAdvanceGatePreservesOrderedEffects(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state, 0, 1);

    Game_UpdateDatalinkPhaseThirty(&state);

    assert(sEventCount == 3);
    assert(sEvents[0] == EVENT_SCENE_COMMAND);
    assert(sEvents[1] == EVENT_SCREEN_COMMIT);
    assert(sEvents[2] == EVENT_STATE_CHANGE);
    assert(state.phase_timer_45 == 60U);
    assert(state.phase_41 == 31U);
}

int main(void)
{
    TestCountdownRepositionsWithoutCommands();
    TestClosedAdvanceGateStopsAfterSceneCommand();
    TestOpenAdvanceGatePreservesOrderedEffects();
    return 0;
}
