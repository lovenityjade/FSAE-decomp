#include "game/datalink_mode_dispatch.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

static void *sDispatchedState;
static int sDispatchedMode;
static size_t sDispatchCount;

void Game_DispatchDatalinkMode(void *state, int mode)
{
    sDispatchedState = state;
    sDispatchedMode = mode;
    ++sDispatchCount;
}

static void ResetDispatch(void)
{
    sDispatchedState = (void *)(uintptr_t)UINTPTR_MAX;
    sDispatchedMode = -1;
    sDispatchCount = 0;
}

static void TestForwardsStateWithModeZero(void)
{
    uint8_t state[32];

    ResetDispatch();

    Game_EnterDatalinkModeZero(state);

    assert(sDispatchCount == 1);
    assert(sDispatchedState == state);
    assert(sDispatchedMode == 0);
}

static void TestForwardsNullStateUnchanged(void)
{
    ResetDispatch();

    Game_EnterDatalinkModeZero(NULL);

    assert(sDispatchCount == 1);
    assert(sDispatchedState == NULL);
    assert(sDispatchedMode == 0);
}

static void TestForwardsStateWithModeOne(void)
{
    uint8_t state[32];

    ResetDispatch();

    Game_EnterDatalinkModeOne(state);

    assert(sDispatchCount == 1);
    assert(sDispatchedState == state);
    assert(sDispatchedMode == 1);
}

static void TestForwardsStateWithModeThree(void)
{
    uint8_t state[32];

    ResetDispatch();

    Game_EnterDatalinkModeThree(state);

    assert(sDispatchCount == 1);
    assert(sDispatchedState == state);
    assert(sDispatchedMode == 3);
}

int main(void)
{
    TestForwardsStateWithModeZero();
    TestForwardsNullStateUnchanged();
    TestForwardsStateWithModeOne();
    TestForwardsStateWithModeThree();
    return 0;
}
