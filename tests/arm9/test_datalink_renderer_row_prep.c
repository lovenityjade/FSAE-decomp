#include "game/datalink_renderer_row_prep.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static uint32_t *sExpectedValues;
static uint8_t sExpectedSlot;
static int sReadyResult;
static size_t sQueryCount;

int Game_IsDatalinkPlayerSlotReady(int slot)
{
    int index;

    assert(slot == sExpectedSlot);
    for (index = 0; index < GAME_DATALINK_RENDER_ROW_VALUE_COUNT; ++index) {
        assert(sExpectedValues[index] == 0U);
    }
    ++sQueryCount;
    return sReadyResult;
}

static void ExpectPreparedResult(int ready_result, bool expected)
{
    uint32_t values[GAME_DATALINK_RENDER_ROW_VALUE_COUNT];
    int index;

    for (index = 0; index < GAME_DATALINK_RENDER_ROW_VALUE_COUNT; ++index) {
        values[index] = UINT32_C(0xA5A5A5A5);
    }
    sExpectedValues = values;
    sExpectedSlot = 2;
    sReadyResult = ready_result;
    sQueryCount = 0;

    assert(Game_PrepareDatalinkRenderRow(values, sExpectedSlot) == expected);
    assert(sQueryCount == 1);
}

int main(void)
{
    ExpectPreparedResult(0, false);
    ExpectPreparedResult(1, true);
    ExpectPreparedResult(-1, true);
    return 0;
}
