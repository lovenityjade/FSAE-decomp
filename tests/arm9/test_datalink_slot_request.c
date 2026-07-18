#include "game/datalink_slot_request.h"

#include "game/datalink_player_slot.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum EventKind {
    EVENT_BEGIN,
    EVENT_SUBMIT,
    EVENT_REFRESH,
    EVENT_END
} EventKind;

typedef struct Event {
    EventKind kind;
    int slot;
    Game_DatalinkPlayerSlot *record;
} Event;

Game_DatalinkPlayerSlot gGameDatalinkPlayerSlots[8];

static Event sEvents[4];
static size_t sEventCount;
static int sSubmitResult;

static void RecordEvent(
    EventKind kind,
    int slot,
    Game_DatalinkPlayerSlot *record
)
{
    Event *event;

    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    event = &sEvents[sEventCount++];
    event->kind = kind;
    event->slot = slot;
    event->record = record;
}

void Game_BeginDatalinkSlotUpdate(void)
{
    RecordEvent(EVENT_BEGIN, -1, NULL);
}

void Game_EndDatalinkSlotUpdate(void)
{
    RecordEvent(EVENT_END, -1, NULL);
}

int Game_SubmitDatalinkPlayerSlot(
    int slot,
    Game_DatalinkPlayerSlot *record
)
{
    RecordEvent(EVENT_SUBMIT, slot, record);
    assert(record->unknown_00[4] == 0x21);
    assert(record->status == INT8_MIN);
    return sSubmitResult;
}

void Game_RefreshDatalinkPlayerSlot(int slot)
{
    RecordEvent(
        EVENT_REFRESH,
        slot,
        &gGameDatalinkPlayerSlots[slot & 7]
    );
}

static void ResetHarness(void)
{
    memset(gGameDatalinkPlayerSlots, 0, sizeof(gGameDatalinkPlayerSlots));
    sEventCount = 0;
    sSubmitResult = 1234;
}

static void TestPlainRequestOwnsBeginEndAndMasksSlot(void)
{
    int result;

    ResetHarness();
    memset(&gGameDatalinkPlayerSlots[2], 0xA5, sizeof(Game_DatalinkPlayerSlot));

    result = Game_RequestDatalinkPlayerSlot(10);

    assert(result == 1234);
    assert(sEventCount == 3);
    assert(sEvents[0].kind == EVENT_BEGIN);
    assert(sEvents[1].kind == EVENT_SUBMIT);
    assert(sEvents[1].slot == 2);
    assert(sEvents[1].record == &gGameDatalinkPlayerSlots[2]);
    assert(sEvents[2].kind == EVENT_END);
    assert(gGameDatalinkPlayerSlots[2].unknown_00[3] == 0xA5);
    assert(gGameDatalinkPlayerSlots[2].unknown_00[4] == 0x21);
    assert(gGameDatalinkPlayerSlots[2].unknown_00[5] == 0xA5);
    assert(gGameDatalinkPlayerSlots[2].status == INT8_MIN);
}

static void TestHighControlBitsSuppressNestedHooks(void)
{
    int result;

    ResetHarness();

    result = Game_RequestDatalinkPlayerSlot(0x82);

    assert(result == 1234);
    assert(sEventCount == 1);
    assert(sEvents[0].kind == EVENT_SUBMIT);
    assert(sEvents[0].slot == 2);
}

static void TestStartUsesSingleBracketAndRefreshesFlaggedSlot(void)
{
    ResetHarness();

    Game_StartDatalinkPlayerSlot(3);

    assert(gGameDatalinkPlayerSlots[3].reset_flag_2c == 1);
    assert(gGameDatalinkPlayerSlots[3].reset_flag_2d == 1);
    assert(gGameDatalinkPlayerSlots[3].unknown_00[4] == 0x21);
    assert(gGameDatalinkPlayerSlots[3].status == INT8_MIN);

    assert(sEventCount == 4);
    assert(sEvents[0].kind == EVENT_BEGIN);
    assert(sEvents[1].kind == EVENT_SUBMIT);
    assert(sEvents[1].slot == 3);
    assert(sEvents[2].kind == EVENT_REFRESH);
    assert(sEvents[2].slot == 0x83);
    assert(sEvents[2].record == &gGameDatalinkPlayerSlots[3]);
    assert(sEvents[3].kind == EVENT_END);
}

int main(void)
{
    TestPlainRequestOwnsBeginEndAndMasksSlot();
    TestHighControlBitsSuppressNestedHooks();
    TestStartUsesSingleBracketAndRefreshesFlaggedSlot();
    return 0;
}
