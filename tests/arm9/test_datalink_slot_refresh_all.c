#include "game/datalink_slot_refresh_all.h"

#include <assert.h>
#include <stddef.h>

typedef enum EventKind {
    EVENT_BEGIN,
    EVENT_REFRESH,
    EVENT_END
} EventKind;

typedef struct Event {
    EventKind kind;
    int slot;
} Event;

static Event sEvents[5];
static size_t sEventCount;

static void RecordEvent(EventKind kind, int slot)
{
    Event *event;

    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    event = &sEvents[sEventCount++];
    event->kind = kind;
    event->slot = slot;
}

void Game_BeginDatalinkSlotUpdate(void)
{
    RecordEvent(EVENT_BEGIN, -1);
}

void Game_RefreshDatalinkPlayerSlot(int slot)
{
    RecordEvent(EVENT_REFRESH, slot);
}

void Game_EndDatalinkSlotUpdate(void)
{
    RecordEvent(EVENT_END, -1);
}

static void TestRefreshesExactlyThreeSlotsInsideOneBracket(void)
{
    static const Event expected[] = {
        {EVENT_BEGIN, -1},
        {EVENT_REFRESH, 0},
        {EVENT_REFRESH, 1},
        {EVENT_REFRESH, 2},
        {EVENT_END, -1}
    };
    size_t index;

    sEventCount = 0;
    Game_RefreshAllDatalinkPlayerSlots();

    assert(sEventCount == sizeof(expected) / sizeof(expected[0]));
    for (index = 0; index < sEventCount; ++index) {
        assert(sEvents[index].kind == expected[index].kind);
        assert(sEvents[index].slot == expected[index].slot);
    }
}

int main(void)
{
    TestRefreshesExactlyThreeSlotsInsideOneBracket();
    return 0;
}
