#include "game/datalink_value_clamp.h"

#include <assert.h>
#include <stdint.h>

static void TestClampsBelowMinimum(void)
{
    int8_t value = -4;

    Game_ClampDatalinkSignedByte(&value, 0, 2);
    assert(value == 0);
}

static void TestClampsAboveMaximum(void)
{
    int8_t value = 5;

    Game_ClampDatalinkSignedByte(&value, 0, 2);
    assert(value == 2);
}

static void TestLeavesInRangeValueUntouched(void)
{
    int8_t value = 1;

    Game_ClampDatalinkSignedByte(&value, 0, 2);
    assert(value == 1);
}

static void TestPreservesSignedComparison(void)
{
    int8_t value = -1;

    Game_ClampDatalinkSignedByte(&value, -3, 3);
    assert(value == -1);

    Game_ClampDatalinkSignedByte(&value, 0, 3);
    assert(value == 0);
}

int main(void)
{
    TestClampsBelowMinimum();
    TestClampsAboveMaximum();
    TestLeavesInRangeValueUntouched();
    TestPreservesSignedComparison();
    return 0;
}
