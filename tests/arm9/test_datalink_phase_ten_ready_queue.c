#include "game/datalink_phase_ten_ready_queue.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static Game_DatalinkFlowState *sExpectedState;
static uint32_t sQueuedAddress;
static size_t sQueueCount;

void Game_PushDatalinkTransition(
    Game_DatalinkFlowState *state,
    uint32_t transition_address
)
{
    assert(state == sExpectedState);
    assert(state->phase_41 == 10);
    sQueuedAddress = transition_address;
    ++sQueueCount;
    state->ready_transition_address_38 = 0xDEADBEEFU;
}

static void TestCapturesAddressAndQueuesExactlyOnce(void)
{
    Game_DatalinkFlowState state;

    memset(&state, 0xA5, sizeof(state));
    state.ready_transition_address_38 = 0x02123456U;
    state.phase_41 = 10;
    sExpectedState = &state;
    sQueuedAddress = 0;
    sQueueCount = 0;

    Game_QueueDatalinkPhaseTenReadyTransition(&state);

    assert(sQueueCount == 1);
    assert(sQueuedAddress == 0x02123456U);
    assert(state.ready_transition_address_38 == 0xDEADBEEFU);
    assert(state.phase_41 == 10);
    assert(state.selection_dirty_43 == 0xA5U);
    assert(state.phase_timer_45 == 0xA5U);
}

int main(void)
{
    TestCapturesAddressAndQueuesExactlyOnce();
    return 0;
}
