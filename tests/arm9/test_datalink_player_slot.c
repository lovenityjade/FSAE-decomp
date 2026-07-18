#include "game/datalink_player_slot.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

Game_DatalinkPlayerSlot gGameDatalinkPlayerSlots[8];

static int sNextStatus;
static int sReadSlot;
static Game_DatalinkPlayerSlot *sReadRecord;
static size_t sReadCount;

int Game_ReadDatalinkPlayerSlot(
    int slot,
    Game_DatalinkPlayerSlot *record
)
{
    ++sReadCount;
    sReadSlot = slot;
    sReadRecord = record;
    record->unknown_00[0] = 0x5A;
    return sNextStatus;
}

static void FillSlot(Game_DatalinkPlayerSlot *record, uint8_t value)
{
    memset(record, value, sizeof(*record));
}

static void ExpectResetRecord(
    const Game_DatalinkPlayerSlot *record,
    int8_t expected_status
)
{
    const uint8_t *bytes = (const uint8_t *)record;
    size_t index;

    for (index = 0; index < sizeof(*record); ++index) {
        uint8_t expected = 0;

        if (index == 0x2C || index == 0x2D) {
            expected = 1;
        } else if (index == 0x47) {
            expected = (uint8_t)expected_status;
        }
        assert(bytes[index] == expected);
    }
}

static void TestDirectResetMasksSlotAndSetsFlags(void)
{
    FillSlot(&gGameDatalinkPlayerSlots[7], 0xAA);

    Game_ResetDatalinkPlayerSlot(15);

    ExpectResetRecord(&gGameDatalinkPlayerSlots[7], 0);
}

static void TestEveryResettingStatus(void)
{
    static const int resetting_statuses[] = {3, 4, 6, 8};
    size_t index;

    for (index = 0;
         index < sizeof(resetting_statuses) / sizeof(resetting_statuses[0]);
         ++index) {
        FillSlot(&gGameDatalinkPlayerSlots[2], 0xAA);
        sNextStatus = resetting_statuses[index];
        sReadCount = 0;

        Game_RefreshDatalinkPlayerSlot(10);

        assert(sReadCount == 1);
        assert(sReadSlot == 2);
        assert(sReadRecord == &gGameDatalinkPlayerSlots[2]);
        ExpectResetRecord(
            &gGameDatalinkPlayerSlots[2],
            (int8_t)resetting_statuses[index]
        );
    }
}

static void TestNonResettingStatusPreservesRecord(void)
{
    FillSlot(&gGameDatalinkPlayerSlots[5], 0xA5);
    sNextStatus = 5;
    sReadCount = 0;

    Game_RefreshDatalinkPlayerSlot(5);

    assert(sReadCount == 1);
    assert(gGameDatalinkPlayerSlots[5].unknown_00[0] == 0x5A);
    assert(gGameDatalinkPlayerSlots[5].reset_flag_2c == 0xA5);
    assert(gGameDatalinkPlayerSlots[5].reset_flag_2d == 0xA5);
    assert(gGameDatalinkPlayerSlots[5].status == 5);
    assert(gGameDatalinkPlayerSlots[5].unknown_48[0] == 0xA5);
}

static void TestStatusStoresOnlyItsLowByte(void)
{
    FillSlot(&gGameDatalinkPlayerSlots[0], 0);
    sNextStatus = 0x181;

    Game_RefreshDatalinkPlayerSlot(0);

    assert(gGameDatalinkPlayerSlots[0].status == (int8_t)0x81);
}

int main(void)
{
    TestDirectResetMasksSlotAndSetsFlags();
    TestEveryResettingStatus();
    TestNonResettingStatusPreservesRecord();
    TestStatusStoresOnlyItsLowByte();
    return 0;
}
