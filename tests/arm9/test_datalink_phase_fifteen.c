#include "game/datalink_phase_fifteen.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

Game_DatalinkCompletionOwner gGameDatalinkCompletionOwner;
Game_DatalinkSceneOwner gGameDatalinkSceneOwner;

static bool sTransitionComplete;
static size_t sTransitionQueryCount;
static size_t sScreenRebuildCount;

bool Game_IsDatalinkPrimaryTransitionComplete(void)
{
    assert(gGameDatalinkCompletionOwner.completion_flag_d0 == 0xA5U);
    assert(gGameDatalinkCompletionOwner.completion_flag_d1 == 0xA5U);
    assert(gGameDatalinkSceneOwner.screen_rebuild_resource_53e == 0xA5U);
    assert(sScreenRebuildCount == 0);
    ++sTransitionQueryCount;
    return sTransitionComplete;
}

void Game_RebuildDatalinkScreen(void)
{
    assert(sTransitionQueryCount == 1);
    assert(gGameDatalinkCompletionOwner.completion_flag_d0 == 1U);
    assert(gGameDatalinkCompletionOwner.completion_flag_d1 == 1U);
    assert(gGameDatalinkSceneOwner.screen_rebuild_resource_53e == 0x83U);
    ++sScreenRebuildCount;
}

static void ResetHarness(bool transition_complete)
{
    memset(&gGameDatalinkCompletionOwner, 0xA5,
           sizeof(gGameDatalinkCompletionOwner));
    memset(&gGameDatalinkSceneOwner, 0xA5,
           sizeof(gGameDatalinkSceneOwner));
    sTransitionComplete = transition_complete;
    sTransitionQueryCount = 0;
    sScreenRebuildCount = 0;
}

static void TestIncompleteTransitionHasNoEffects(void)
{
    ResetHarness(false);

    Game_UpdateDatalinkPhaseFifteen();

    assert(sTransitionQueryCount == 1);
    assert(sScreenRebuildCount == 0);
    assert(gGameDatalinkCompletionOwner.completion_flag_d0 == 0xA5U);
    assert(gGameDatalinkCompletionOwner.completion_flag_d1 == 0xA5U);
    assert(gGameDatalinkSceneOwner.screen_rebuild_resource_53e == 0xA5U);
}

static void TestCompleteTransitionRebuildsAfterWrites(void)
{
    ResetHarness(true);

    Game_UpdateDatalinkPhaseFifteen();

    assert(sTransitionQueryCount == 1);
    assert(sScreenRebuildCount == 1);
    assert(gGameDatalinkCompletionOwner.unknown_000[0] == 0xA5U);
    assert(gGameDatalinkSceneOwner.unknown_000[0] == 0xA5U);
}

int main(void)
{
    TestIncompleteTransitionHasNoEffects();
    TestCompleteTransitionRebuildsAfterWrites();
    return 0;
}
