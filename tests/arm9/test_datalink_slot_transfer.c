#include "game/datalink_slot_transfer.h"

#include "game/datalink_player_slot.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum EventKind {
    EVENT_REFRESH,
    EVENT_SEND,
    EVENT_CLOSE,
    EVENT_RESET
} EventKind;

typedef struct Event {
    EventKind kind;
    int slot;
    const Game_DatalinkPlayerSlot *record;
} Event;

Game_DatalinkPlayerSlot gGameDatalinkPlayerSlots[8];

static Event sEvents[4];
static size_t sEventCount;
static int8_t sRefreshStatus[8];

static void RecordEvent(
    EventKind kind,
    int slot,
    const Game_DatalinkPlayerSlot *record
)
{
    Event *event;

    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    event = &sEvents[sEventCount++];
    event->kind = kind;
    event->slot = slot;
    event->record = record;
}

void Game_RefreshDatalinkPlayerSlot(int slot)
{
    RecordEvent(EVENT_REFRESH, slot, &gGameDatalinkPlayerSlots[slot]);
    gGameDatalinkPlayerSlots[slot].status = sRefreshStatus[slot];
}

void Game_SendDatalinkPlayerSlot(
    int destination_slot,
    const Game_DatalinkPlayerSlot *record
)
{
    RecordEvent(EVENT_SEND, destination_slot, record);
}

void Game_CloseDatalinkPlayerSlot(int slot)
{
    RecordEvent(EVENT_CLOSE, slot, NULL);
}

void Game_ResetDatalinkPlayerSlot(int slot)
{
    int masked_slot = slot & 7;

    RecordEvent(EVENT_RESET, slot, &gGameDatalinkPlayerSlots[masked_slot]);
    memset(&gGameDatalinkPlayerSlots[masked_slot], 0, sizeof(Game_DatalinkPlayerSlot));
    gGameDatalinkPlayerSlots[masked_slot].reset_flag_2c = 1;
    gGameDatalinkPlayerSlots[masked_slot].reset_flag_2d = 1;
}

static void ResetHarness(void)
{
    memset(gGameDatalinkPlayerSlots, 0, sizeof(gGameDatalinkPlayerSlots));
    memset(sRefreshStatus, 0, sizeof(sRefreshStatus));
    sEventCount = 0;
}

static void TestSuccessfulSourceSendsBeforeDestinationRefresh(void)
{
    ResetHarness();
    sRefreshStatus[1] = 0;
    sRefreshStatus[2] = 4;

    Game_TransferDatalinkPlayerSlot(9, 10);

    assert(sEventCount == 3);
    assert(sEvents[0].kind == EVENT_REFRESH);
    assert(sEvents[0].slot == 1);
    assert(sEvents[1].kind == EVENT_SEND);
    assert(sEvents[1].slot == 2);
    assert(sEvents[1].record == &gGameDatalinkPlayerSlots[1]);
    assert(sEvents[2].kind == EVENT_REFRESH);
    assert(sEvents[2].slot == 2);
    assert(gGameDatalinkPlayerSlots[2].status == 7);
}

static void TestFailedSourceSkipsSendAndZeroDestinationRemainsZero(void)
{
    ResetHarness();
    sRefreshStatus[3] = 5;
    sRefreshStatus[4] = 0;

    Game_TransferDatalinkPlayerSlot(3, 4);

    assert(sEventCount == 2);
    assert(sEvents[0].kind == EVENT_REFRESH);
    assert(sEvents[0].slot == 3);
    assert(sEvents[1].kind == EVENT_REFRESH);
    assert(sEvents[1].slot == 4);
    assert(gGameDatalinkPlayerSlots[4].status == 0);
}

static void TestRemovePassesOriginalSlotThenResetsMaskedRecord(void)
{
    ResetHarness();
    memset(&gGameDatalinkPlayerSlots[2], 0xA5, sizeof(Game_DatalinkPlayerSlot));

    Game_RemoveDatalinkPlayerSlot(10);

    assert(sEventCount == 2);
    assert(sEvents[0].kind == EVENT_CLOSE);
    assert(sEvents[0].slot == 10);
    assert(sEvents[1].kind == EVENT_RESET);
    assert(sEvents[1].slot == 10);
    assert(sEvents[1].record == &gGameDatalinkPlayerSlots[2]);
    assert(gGameDatalinkPlayerSlots[2].reset_flag_2c == 1);
    assert(gGameDatalinkPlayerSlots[2].reset_flag_2d == 1);
    assert(gGameDatalinkPlayerSlots[2].status == 0);
}

static void TestNoOpHooksAreCallable(void)
{
    Game_BeginDatalinkSlotUpdate();
    Game_EndDatalinkSlotUpdate();
}

int main(void)
{
    TestSuccessfulSourceSendsBeforeDestinationRefresh();
    TestFailedSourceSkipsSendAndZeroDestinationRemainsZero();
    TestRemovePassesOriginalSlotThenResetsMaskedRecord();
    TestNoOpHooksAreCallable();
    return 0;
}
