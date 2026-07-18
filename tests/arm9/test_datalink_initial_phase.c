#include "game/datalink_initial_phase.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_MODE_ZERO,
    EVENT_MODE_ONE,
    EVENT_PARTICIPANT_TYPE,
    EVENT_LOAD_RESOURCE
} Event;

static Event sEvents[3];
static size_t sEventCount;
static Game_DatalinkFlowState *sExpectedState;
static int sParticipantType;
static int sQueriedParticipantIndex;

static void RecordEvent(Event event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

void Game_EnterDatalinkModeZero(void *state)
{
    assert(state == sExpectedState);
    assert(sExpectedState->phase_41 == 0);
    RecordEvent(EVENT_MODE_ZERO);
}

void Game_EnterDatalinkModeOne(void *state)
{
    assert(state == sExpectedState);
    assert(sExpectedState->phase_41 == 1);
    RecordEvent(EVENT_MODE_ONE);
}

int Game_GetDatalinkParticipantType(
    const Game_DatalinkFlowState *state,
    int participant_index
)
{
    assert(state == sExpectedState);
    assert(state->phase_41 == 5);
    RecordEvent(EVENT_PARTICIPANT_TYPE);
    sQueriedParticipantIndex = participant_index;
    return sParticipantType;
}

void Game_LoadDatalinkScreenResource(uint16_t resource_id)
{
    assert(sExpectedState->phase_41 == 5);
    assert(resource_id == 0x0834U);
    RecordEvent(EVENT_LOAD_RESOURCE);
}

static void ResetHarness(Game_DatalinkFlowState *state)
{
    memset(state, 0xA5, sizeof(*state));
    sExpectedState = state;
    sParticipantType = 0;
    sQueriedParticipantIndex = 0;
    sEventCount = 0;
}

static void TestPhaseZeroAlwaysLoadsResource(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state);
    state.phase_41 = 0;
    state.active_participant_index_2c = 2;

    Game_EnterDatalinkInitialPhase(&state);

    assert(state.phase_41 == 5);
    assert(state.unknown_00[0] == 0xA5U);
    assert(state.unknown_2d[0] == 0xA5U);
    assert(sEventCount == 2);
    assert(sEvents[0] == EVENT_MODE_ZERO);
    assert(sEvents[1] == EVENT_LOAD_RESOURCE);
}

static void TestPhaseOneSkipsReloadForParticipantTypeOne(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state);
    state.phase_41 = 1;
    state.active_participant_index_2c = -2;
    sParticipantType = 1;

    Game_EnterDatalinkInitialPhase(&state);

    assert(state.phase_41 == 5);
    assert(sQueriedParticipantIndex == -2);
    assert(sEventCount == 2);
    assert(sEvents[0] == EVENT_MODE_ONE);
    assert(sEvents[1] == EVENT_PARTICIPANT_TYPE);
}

static void TestPhaseOneReloadsForEveryOtherParticipantType(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state);
    state.phase_41 = 1;
    state.active_participant_index_2c = 1;
    sParticipantType = 2;

    Game_EnterDatalinkInitialPhase(&state);

    assert(state.phase_41 == 5);
    assert(sQueriedParticipantIndex == 1);
    assert(sEventCount == 3);
    assert(sEvents[0] == EVENT_MODE_ONE);
    assert(sEvents[1] == EVENT_PARTICIPANT_TYPE);
    assert(sEvents[2] == EVENT_LOAD_RESOURCE);
}

int main(void)
{
    TestPhaseZeroAlwaysLoadsResource();
    TestPhaseOneSkipsReloadForParticipantTypeOne();
    TestPhaseOneReloadsForEveryOtherParticipantType();
    return 0;
}
