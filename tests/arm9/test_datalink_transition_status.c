#include "game/datalink_transition_status.h"

#include "game/datalink_pair_position.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

Game_DatalinkPositionObject
    gGameDatalinkPositionObjects[GAME_DATALINK_POSITION_OBJECT_COUNT];

static const Game_DatalinkPositionObject *sQueriedObject;
static int32_t sTransitionProgress;
static size_t sQueryCount;

int32_t Game_GetDatalinkObjectTransitionProgressFx(
    const Game_DatalinkPositionObject *object
)
{
    sQueriedObject = object;
    ++sQueryCount;
    return sTransitionProgress;
}

static void ExpectCompletionResult(int32_t progress, bool expected)
{
    sQueriedObject = NULL;
    sTransitionProgress = progress;
    sQueryCount = 0;

    assert(Game_IsDatalinkPrimaryTransitionComplete() == expected);
    assert(sQueryCount == 1);
    assert(sQueriedObject == &gGameDatalinkPositionObjects[0]);
}

static void TestOnlyExactFxOneIsComplete(void)
{
    ExpectCompletionResult(0, false);
    ExpectCompletionResult(0x0FFF, false);
    ExpectCompletionResult(0x1000, true);
    ExpectCompletionResult(0x1001, false);
    ExpectCompletionResult(-1, false);
}

int main(void)
{
    TestOnlyExactFxOneIsComplete();
    return 0;
}
