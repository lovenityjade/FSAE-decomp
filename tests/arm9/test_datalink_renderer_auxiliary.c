#include "game/datalink_renderer_auxiliary.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static Game_DatalinkFlowState *sExpectedState;
static int sParticipantType;
static size_t sTypeQueryCount;
static int sRefreshedSlot;
static size_t sRefreshCount;

int Game_GetDatalinkParticipantType(
    const Game_DatalinkFlowState *state,
    int participant_index
)
{
    assert(state == sExpectedState);
    assert(participant_index == 2);
    ++sTypeQueryCount;
    return sParticipantType;
}

void Game_RefreshDatalinkRowAuxiliary(int slot)
{
    sRefreshedSlot = slot;
    ++sRefreshCount;
}

static void ResetHarness(
    Game_DatalinkFlowState *state,
    int8_t selected,
    int participant_type
)
{
    memset(state, 0xA5, sizeof(*state));
    state->active_participant_index_2c = 2;
    state->selected_participant_40 = selected;
    sExpectedState = state;
    sParticipantType = participant_type;
    sTypeQueryCount = 0;
    sRefreshedSlot = -1;
    sRefreshCount = 0;
}

static void TestSelectedRowRefreshesWithoutTypeLookup(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state, 1, 1);

    Game_RefreshDatalinkRowAuxiliaryIfNeeded(&state, 1);

    assert(sTypeQueryCount == 0);
    assert(sRefreshCount == 1);
    assert(sRefreshedSlot == 1);
}

static void TestTypeOneSkipsNonSelectedRow(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state, -1, 1);

    Game_RefreshDatalinkRowAuxiliaryIfNeeded(&state, 1);

    assert(sTypeQueryCount == 1);
    assert(sRefreshCount == 0);
}

static void TestOtherTypeRefreshesNonSelectedRow(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state, 0, 2);

    Game_RefreshDatalinkRowAuxiliaryIfNeeded(&state, 1);

    assert(sTypeQueryCount == 1);
    assert(sRefreshCount == 1);
    assert(sRefreshedSlot == 1);
}

int main(void)
{
    TestSelectedRowRefreshesWithoutTypeLookup();
    TestTypeOneSkipsNonSelectedRow();
    TestOtherTypeRefreshesNonSelectedRow();
    return 0;
}
