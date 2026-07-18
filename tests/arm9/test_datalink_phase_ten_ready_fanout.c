#include "game/datalink_phase_ten_ready_fanout.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct ExpectedTransition {
    int object_index;
    int32_t x_fx;
    int32_t y_fx;
} ExpectedTransition;

Game_DatalinkPositionObject
    gGameDatalinkPositionObjects[GAME_DATALINK_POSITION_OBJECT_COUNT];

static const ExpectedTransition sExpectedTransitions[] = {
    {0, 44 * 0x1000, 0},
    {5, 24 * 0x1000, -18 * 0x1000},
    {6, 24 * 0x1000, -18 * 0x1000},
    {1, 44 * 0x1000, 48 * 0x1000},
    {7, 24 * 0x1000, 30 * 0x1000},
    {8, 24 * 0x1000, 30 * 0x1000},
    {2, 44 * 0x1000, 96 * 0x1000},
    {9, 24 * 0x1000, 78 * 0x1000},
    {10, 24 * 0x1000, 78 * 0x1000}
};

static size_t sTransitionCount;

void Game_StartDatalinkObjectTransition(
    Game_DatalinkPositionObject *object,
    int32_t target_x_fx,
    int32_t target_y_fx,
    int32_t duration,
    int mode
)
{
    const ExpectedTransition *expected;
    ptrdiff_t object_index;

    assert(sTransitionCount <
           sizeof(sExpectedTransitions) / sizeof(sExpectedTransitions[0]));
    expected = &sExpectedTransitions[sTransitionCount++];
    object_index = object - gGameDatalinkPositionObjects;

    assert(object_index == expected->object_index);
    assert(target_x_fx == expected->x_fx);
    assert(target_y_fx == expected->y_fx);
    assert(duration == 20);
    assert(mode == 2);
}

static void TestNineTransitionsAreGroupedByRow(void)
{
    memset(gGameDatalinkPositionObjects, 0xA5,
           sizeof(gGameDatalinkPositionObjects));
    sTransitionCount = 0;

    Game_StartDatalinkPhaseTenReadyFanout(1);

    assert(sTransitionCount ==
           sizeof(sExpectedTransitions) / sizeof(sExpectedTransitions[0]));
    assert(gGameDatalinkPositionObjects[0].unknown_00[0] == 0xA5U);
    assert(gGameDatalinkPositionObjects[10].unknown_38[0] == 0xA5U);
}

int main(void)
{
    TestNineTransitionsAreGroupedByRow();
    return 0;
}
