#include "game/datalink_slot_readiness.h"

#include "game/datalink_player_slot.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

Game_DatalinkPlayerSlot gGameDatalinkPlayerSlots[8];

static void ResetSlots(void)
{
    memset(gGameDatalinkPlayerSlots, 0, sizeof(gGameDatalinkPlayerSlots));
}

static void SetPayloadMarker(int slot, uint16_t marker)
{
    gGameDatalinkPlayerSlots[slot].unknown_00[8] = (uint8_t)marker;
    gGameDatalinkPlayerSlots[slot].unknown_00[9] = (uint8_t)(marker >> 8);
}

static void TestReadySlotRequiresZeroStatusAndPayload(void)
{
    ResetSlots();
    SetPayloadMarker(1, 0x1234);

    assert(Game_IsDatalinkPlayerSlotReady(1) == 1);

    gGameDatalinkPlayerSlots[1].status = 5;
    assert(Game_IsDatalinkPlayerSlotReady(1) == 0);

    gGameDatalinkPlayerSlots[1].status = 0;
    SetPayloadMarker(1, 0);
    assert(Game_IsDatalinkPlayerSlotReady(1) == 0);
}

static void TestHighPayloadByteIsObserved(void)
{
    ResetSlots();
    SetPayloadMarker(2, 0x0100);

    assert(Game_IsDatalinkPlayerSlotReady(2) == 1);
}

static void TestSignedMinus128StatusDoesNotPassDeadComparison(void)
{
    ResetSlots();
    SetPayloadMarker(0, 1);
    gGameDatalinkPlayerSlots[0].status = INT8_MIN;

    assert(Game_IsDatalinkPlayerSlotReady(0) == 0);
}

static void TestOnlyFirstThreeSlotsAreAccepted(void)
{
    ResetSlots();
    SetPayloadMarker(3, 1);
    SetPayloadMarker(7, 1);

    assert(Game_IsDatalinkPlayerSlotReady(-1) == 0);
    assert(Game_IsDatalinkPlayerSlotReady(3) == 0);
    assert(Game_IsDatalinkPlayerSlotReady(7) == 0);
}

static void TestReadyCountRangesFromZeroToThree(void)
{
    int slot;

    ResetSlots();
    assert(Game_CountReadyDatalinkPlayerSlots() == 0);

    for (slot = 0; slot < 3; ++slot) {
        SetPayloadMarker(slot, (uint16_t)(slot + 1));
        assert(Game_CountReadyDatalinkPlayerSlots() == slot + 1);
    }

    SetPayloadMarker(6, 1);
    assert(Game_CountReadyDatalinkPlayerSlots() == 3);
}

int main(void)
{
    TestReadySlotRequiresZeroStatusAndPayload();
    TestHighPayloadByteIsObserved();
    TestSignedMinus128StatusDoesNotPassDeadComparison();
    TestOnlyFirstThreeSlotsAreAccepted();
    TestReadyCountRangesFromZeroToThree();
    return 0;
}
