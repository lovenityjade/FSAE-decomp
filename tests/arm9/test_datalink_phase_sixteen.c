#include "game/datalink_phase_sixteen.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

Game_DatalinkSceneOwner gGameDatalinkSceneOwner;

static Game_DatalinkFlowState *sExpectedState;
static bool sTransitionComplete;
static size_t sTransitionQueryCount;
static size_t sTransitionPushCount;
static uint32_t sQueuedAddress;

bool Game_IsDatalinkPrimaryTransitionComplete(void)
{
    assert(sTransitionPushCount == 0);
    assert(sExpectedState->return_phase_depth_48 == 2U);
    assert(sExpectedState->return_phase_stack_4c[2] == 0xA5A5A5A5U);
    ++sTransitionQueryCount;
    return sTransitionComplete;
}

void Game_PushDatalinkSceneTransition(
    Game_DatalinkSceneOwner *scene,
    uint32_t transition_address
)
{
    assert(scene == &gGameDatalinkSceneOwner);
    assert(sTransitionQueryCount == 1);
    assert(sExpectedState->return_phase_depth_48 == 3U);
    assert(sExpectedState->return_phase_stack_4c[0] == 7U);
    assert(sExpectedState->return_phase_stack_4c[1] == 9U);
    assert(sExpectedState->return_phase_stack_4c[2] == 1U);
    assert(sExpectedState->return_phase_stack_4c[3] == 0xA5A5A5A5U);
    sQueuedAddress = transition_address;
    ++sTransitionPushCount;
}

static void ResetHarness(Game_DatalinkFlowState *state, bool complete)
{
    memset(state, 0xA5, sizeof(*state));
    memset(&gGameDatalinkSceneOwner, 0xA5,
           sizeof(gGameDatalinkSceneOwner));
    state->unavailable_transition_address_3c = 0x02123456U;
    state->return_phase_depth_48 = 2;
    state->return_phase_stack_4c[0] = 7;
    state->return_phase_stack_4c[1] = 9;
    sExpectedState = state;
    sTransitionComplete = complete;
    sTransitionQueryCount = 0;
    sTransitionPushCount = 0;
    sQueuedAddress = 0;
}

static void TestIncompleteTransitionHasNoEffects(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state, false);

    Game_UpdateDatalinkPhaseSixteen(&state);

    assert(sTransitionQueryCount == 1);
    assert(sTransitionPushCount == 0);
    assert(state.return_phase_depth_48 == 2U);
    assert(state.return_phase_stack_4c[2] == 0xA5A5A5A5U);
    assert(state.unavailable_transition_address_3c == 0x02123456U);
}

static void TestCompleteTransitionPushesAfterStackUpdate(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state, true);

    Game_UpdateDatalinkPhaseSixteen(&state);

    assert(sTransitionQueryCount == 1);
    assert(sTransitionPushCount == 1);
    assert(sQueuedAddress == 0x02123456U);
    assert(state.return_phase_depth_48 == 3U);
    assert(state.return_phase_stack_4c[2] == 1U);
    assert(state.selected_participant_40 == (int8_t)0xA5);
}

int main(void)
{
    TestIncompleteTransitionHasNoEffects();
    TestCompleteTransitionPushesAfterStackUpdate();
    return 0;
}
