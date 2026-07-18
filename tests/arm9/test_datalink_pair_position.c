#include "game/datalink_pair_position.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

Game_DatalinkPositionObject
    gGameDatalinkPositionObjects[GAME_DATALINK_POSITION_OBJECT_COUNT];

static void ResetObjects(void)
{
    memset(gGameDatalinkPositionObjects, 0xA5,
           sizeof(gGameDatalinkPositionObjects));
}

static void AssertPosition(
    int object_index,
    int32_t expected_x,
    int32_t expected_y
)
{
    const Game_DatalinkPositionObject *object =
        &gGameDatalinkPositionObjects[object_index];

    assert(object->transition_duration_fx_28 == 0);
    assert(object->position_x_30 == expected_x);
    assert(object->position_y_34 == expected_y);
    assert(object->unknown_00[0] == 0xA5U);
    assert(object->unknown_2c[0] == 0xA5U);
    assert(object->unknown_38[0] == 0xA5U);
}

static void TestPositionsAllThreeObjectPairs(void)
{
    Game_DatalinkPositionObject untouched[5];

    ResetObjects();
    memcpy(untouched, gGameDatalinkPositionObjects, sizeof(untouched));

    Game_PositionDatalinkObjectPairs((const void *)(uintptr_t)0x1234, -4, 2);

    AssertPosition(5, 20 * 0x1000, 32 * 0x1000);
    AssertPosition(6, 20 * 0x1000, 32 * 0x1000);
    AssertPosition(7, 20 * 0x1000, 80 * 0x1000);
    AssertPosition(8, 20 * 0x1000, 80 * 0x1000);
    AssertPosition(9, 20 * 0x1000, 128 * 0x1000);
    AssertPosition(10, 20 * 0x1000, 128 * 0x1000);

    assert(memcmp(untouched, gGameDatalinkPositionObjects,
                  sizeof(untouched)) == 0);
}

static void TestNegativeFixedPointCoordinates(void)
{
    ResetObjects();

    Game_PositionDatalinkObjectPairs(NULL, -25, -127);

    AssertPosition(5, -0x1000, -97 * 0x1000);
    AssertPosition(6, -0x1000, -97 * 0x1000);
    AssertPosition(7, -0x1000, -49 * 0x1000);
    AssertPosition(8, -0x1000, -49 * 0x1000);
    AssertPosition(9, -0x1000, -0x1000);
    AssertPosition(10, -0x1000, -0x1000);
}

static void TestTargetWordWrapOccursBeforeFixedPointShift(void)
{
    ResetObjects();

    Game_PositionDatalinkObjectPairs(NULL, INT32_MAX, INT32_MIN);

    AssertPosition(5, 0x17000, 0x1E000);
    AssertPosition(6, 0x17000, 0x1E000);
    AssertPosition(9, 0x17000, 0x7E000);
    AssertPosition(10, 0x17000, 0x7E000);
}

int main(void)
{
    TestPositionsAllThreeObjectPairs();
    TestNegativeFixedPointCoordinates();
    TestTargetWordWrapOccursBeforeFixedPointShift();
    return 0;
}
