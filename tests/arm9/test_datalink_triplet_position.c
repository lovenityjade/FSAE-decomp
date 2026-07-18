#include "game/datalink_triplet_position.h"

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

static void TestPositionsThreeLeadingObjects(void)
{
    Game_DatalinkPositionObject untouched[8];

    ResetObjects();
    memcpy(untouched, &gGameDatalinkPositionObjects[3], sizeof(untouched));

    Game_PositionDatalinkObjectTriplet(
        (const void *)(uintptr_t)0x5678,
        -4,
        2
    );

    AssertPosition(0, 40 * 0x1000, 50 * 0x1000);
    AssertPosition(1, 40 * 0x1000, 98 * 0x1000);
    AssertPosition(2, 40 * 0x1000, 146 * 0x1000);
    assert(memcmp(untouched, &gGameDatalinkPositionObjects[3],
                  sizeof(untouched)) == 0);
}

static void TestNegativeFixedPointCoordinates(void)
{
    ResetObjects();

    Game_PositionDatalinkObjectTriplet(NULL, -45, -145);

    AssertPosition(0, -0x1000, -97 * 0x1000);
    AssertPosition(1, -0x1000, -49 * 0x1000);
    AssertPosition(2, -0x1000, -0x1000);
}

static void TestTargetWordWrapOccursBeforeFixedPointShift(void)
{
    ResetObjects();

    Game_PositionDatalinkObjectTriplet(NULL, INT32_MAX, INT32_MIN);

    AssertPosition(0, 0x2B000, 0x30000);
    AssertPosition(1, 0x2B000, 0x60000);
    AssertPosition(2, 0x2B000, 0x90000);
}

int main(void)
{
    TestPositionsThreeLeadingObjects();
    TestNegativeFixedPointCoordinates();
    TestTargetWordWrapOccursBeforeFixedPointShift();
    return 0;
}
