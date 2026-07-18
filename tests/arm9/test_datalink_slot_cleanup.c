#include "game/datalink_player_slot.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_PERSIST_SLOT,
    EVENT_RESET_SLOT,
    EVENT_COMMIT
} Event;

typedef struct RecordedEvent {
    Event event;
    int slot;
} RecordedEvent;

Game_DatalinkPlayerSlot gGameDatalinkPlayerSlots[8];

static RecordedEvent sEvents[7];
static size_t sEventCount;
static int8_t sExpectedStatusAtCommit[3];

static void RecordEvent(Event event, int slot)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount].event = event;
    sEvents[sEventCount].slot = slot;
    ++sEventCount;
}

void Game_PersistDatalinkPlayerSlot(int slot)
{
    assert(slot >= 0 && slot < 3);
    assert(gGameDatalinkPlayerSlots[slot].status == 6);
    RecordEvent(EVENT_PERSIST_SLOT, slot);
}

void Game_ResetDatalinkPlayerSlot(int slot)
{
    assert(slot >= 0 && slot < 3);
    assert(gGameDatalinkPlayerSlots[slot].status == 6 ||
           gGameDatalinkPlayerSlots[slot].status == 8);
    RecordEvent(EVENT_RESET_SLOT, slot);
    gGameDatalinkPlayerSlots[slot].status = 0x55;
}

int Game_CommitDatalinkPersistentState(void)
{
    int slot;

    for (slot = 0; slot < 3; ++slot) {
        assert(gGameDatalinkPlayerSlots[slot].status ==
               sExpectedStatusAtCommit[slot]);
    }
    RecordEvent(EVENT_COMMIT, -1);
    return -7;
}

static void ResetHarness(int8_t status_0, int8_t status_1, int8_t status_2)
{
    memset(gGameDatalinkPlayerSlots, 0xA5,
           sizeof(gGameDatalinkPlayerSlots));
    gGameDatalinkPlayerSlots[0].status = status_0;
    gGameDatalinkPlayerSlots[1].status = status_1;
    gGameDatalinkPlayerSlots[2].status = status_2;
    sExpectedStatusAtCommit[0] = status_0;
    sExpectedStatusAtCommit[1] = status_1;
    sExpectedStatusAtCommit[2] = status_2;
    sEventCount = 0;
}

static void TestStatusSixPersistsBeforeSharedResetPath(void)
{
    ResetHarness(6, 8, 5);
    sExpectedStatusAtCommit[0] = 0;
    sExpectedStatusAtCommit[1] = 0;

    Game_CleanupDatalinkPlayerSlots();

    assert(sEventCount == 4);
    assert(sEvents[0].event == EVENT_PERSIST_SLOT);
    assert(sEvents[0].slot == 0);
    assert(sEvents[1].event == EVENT_RESET_SLOT);
    assert(sEvents[1].slot == 0);
    assert(sEvents[2].event == EVENT_RESET_SLOT);
    assert(sEvents[2].slot == 1);
    assert(sEvents[3].event == EVENT_COMMIT);
    assert(gGameDatalinkPlayerSlots[0].status == 0);
    assert(gGameDatalinkPlayerSlots[1].status == 0);
    assert(gGameDatalinkPlayerSlots[2].status == 5);
    assert((uint8_t)gGameDatalinkPlayerSlots[3].status == 0xA5U);
}

static void TestOtherStatusesOnlyCommit(void)
{
    ResetHarness(-1, 3, 4);

    Game_CleanupDatalinkPlayerSlots();

    assert(sEventCount == 1);
    assert(sEvents[0].event == EVENT_COMMIT);
    assert(gGameDatalinkPlayerSlots[0].status == -1);
    assert(gGameDatalinkPlayerSlots[1].status == 3);
    assert(gGameDatalinkPlayerSlots[2].status == 4);
}

int main(void)
{
    TestStatusSixPersistsBeforeSharedResetPath();
    TestOtherStatusesOnlyCommit();
    return 0;
}
