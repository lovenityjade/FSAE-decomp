#include "game/datalink_phase_ten_visibility.h"

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

static void AssertVisibility(int object_index, uint8_t expected)
{
    const Game_DatalinkPositionObject *object =
        &gGameDatalinkPositionObjects[object_index];

    assert(object->visible_03 == expected);
    assert(object->unknown_00[0] == 0xA5U);
    assert(object->unknown_04[0] == 0xA5U);
    assert(object->unknown_38[0] == 0xA5U);
}

static void TestOnlySelectedRowRemainsVisible(void)
{
    Game_DatalinkFlowState state;

    memset(&state, 0xA5, sizeof(state));
    ResetObjects();
    state.selected_participant_40 = 1;

    Game_HideUnselectedDatalinkRows(&state);

    AssertVisibility(0, 0);
    AssertVisibility(5, 0);
    AssertVisibility(6, 0);
    AssertVisibility(1, 0xA5U);
    AssertVisibility(7, 0xA5U);
    AssertVisibility(8, 0xA5U);
    AssertVisibility(2, 0);
    AssertVisibility(9, 0);
    AssertVisibility(10, 0);
    AssertVisibility(3, 0xA5U);
    AssertVisibility(4, 0xA5U);
}

static void TestOutOfRangeSelectionHidesEveryMappedRow(void)
{
    static const int mapped_objects[] = {0, 5, 6, 1, 7, 8, 2, 9, 10};
    Game_DatalinkFlowState state;
    size_t index;

    memset(&state, 0xA5, sizeof(state));
    ResetObjects();
    state.selected_participant_40 = -1;

    Game_HideUnselectedDatalinkRows(&state);

    for (index = 0;
         index < sizeof(mapped_objects) / sizeof(mapped_objects[0]);
         ++index) {
        AssertVisibility(mapped_objects[index], 0);
    }
    AssertVisibility(3, 0xA5U);
    AssertVisibility(4, 0xA5U);
}

int main(void)
{
    TestOnlySelectedRowRemainsVisible();
    TestOutOfRangeSelectionHidesEveryMappedRow();
    return 0;
}
