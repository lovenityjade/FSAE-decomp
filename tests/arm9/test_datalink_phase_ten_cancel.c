#include "game/datalink_phase_ten_cancel.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_MODE_THREE,
    EVENT_CANCEL_SOUND
} Event;

static Event sEvents[2];
static size_t sEventCount;
static Game_DatalinkFlowState *sExpectedState;

static void RecordEvent(Event event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

void Game_EnterDatalinkModeThree(void *state)
{
    assert(state == sExpectedState);
    assert(sExpectedState->phase_41 == 10);
    RecordEvent(EVENT_MODE_THREE);
}

void Game_PlayDatalinkSoundEffect(uint32_t sound_id)
{
    assert(sound_id == 0x44U);
    assert(sExpectedState->phase_41 == 10);
    RecordEvent(EVENT_CANCEL_SOUND);
}

static void TestCancelOrderAndPhaseAdvance(void)
{
    Game_DatalinkFlowState state;

    memset(&state, 0xA5, sizeof(state));
    state.phase_41 = 10;
    sExpectedState = &state;
    sEventCount = 0;

    Game_CancelDatalinkPhaseTen(&state);

    assert(state.phase_41 == 15);
    assert(state.unknown_00[0] == 0xA5U);
    assert(state.phase_timer_45 == 0xA5U);
    assert(sEventCount == 2);
    assert(sEvents[0] == EVENT_MODE_THREE);
    assert(sEvents[1] == EVENT_CANCEL_SOUND);
}

int main(void)
{
    TestCancelOrderAndPhaseAdvance();
    return 0;
}
