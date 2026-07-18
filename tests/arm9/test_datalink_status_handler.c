#include "game/datalink_status_handler.h"

#include "game/datalink_player_slot.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

Game_DatalinkPlayerSlot gGameDatalinkPlayerSlots[8];

static int sReportedEvents[3];
static size_t sReportedEventCount;
static size_t sTransitionCount;
static int sTransitionState;
static int sTransitionParticipantCount;
static int sTransitionArgument2;
static int sTransitionArgument3;

void Game_ReportDatalinkStatusEvent(int event_id)
{
    assert(sReportedEventCount < sizeof(sReportedEvents) / sizeof(sReportedEvents[0]));
    sReportedEvents[sReportedEventCount++] = event_id;
}

void Game_ChangeDatalinkState(
    int state,
    int participant_count,
    int argument_2,
    int argument_3
)
{
    ++sTransitionCount;
    assert(sTransitionCount == 1);
    sTransitionState = state;
    sTransitionParticipantCount = participant_count;
    sTransitionArgument2 = argument_2;
    sTransitionArgument3 = argument_3;
}

static void ResetHarness(int8_t status_0, int8_t status_1, int8_t status_2)
{
    memset(gGameDatalinkPlayerSlots, 0, sizeof(gGameDatalinkPlayerSlots));
    gGameDatalinkPlayerSlots[0].status = status_0;
    gGameDatalinkPlayerSlots[1].status = status_1;
    gGameDatalinkPlayerSlots[2].status = status_2;
    sReportedEventCount = 0;
    sTransitionCount = 0;
    sTransitionState = 0;
    sTransitionParticipantCount = 0;
    sTransitionArgument2 = -1;
    sTransitionArgument3 = -1;
}

static void ExpectTransition(int expected_state)
{
    assert(sTransitionCount == 1);
    assert(sTransitionState == expected_state);
    assert(sTransitionParticipantCount == 4);
    assert(sTransitionArgument2 == 0);
    assert(sTransitionArgument3 == 0);
}

static void TestBenignAndOutOfRangeStatusesDoNothing(void)
{
    ResetHarness(0, 2, 5);
    assert(Game_HandleDatalinkPlayerSlotStatuses() == 0);
    assert(sReportedEventCount == 0);
    assert(sTransitionCount == 0);

    ResetHarness(INT8_MIN, 8, -1);
    assert(Game_HandleDatalinkPlayerSlotStatuses() == 0);
    assert(sReportedEventCount == 0);
    assert(sTransitionCount == 0);
}

static void TestStatusesThreeAndFourOnlyReportEvents(void)
{
    ResetHarness(3, 4, 5);

    assert(Game_HandleDatalinkPlayerSlotStatuses() == 0);
    assert(sReportedEventCount == 2);
    assert(sReportedEvents[0] == 11);
    assert(sReportedEvents[1] == 11);
    assert(sTransitionCount == 0);
}

static void TestStatusSixSelectsTransitionFour(void)
{
    ResetHarness(0, 6, 0);

    assert(Game_HandleDatalinkPlayerSlotStatuses() == 1);
    ExpectTransition(4);
}

static void TestStatusSevenSelectsTransitionFive(void)
{
    ResetHarness(0, 0, 7);

    assert(Game_HandleDatalinkPlayerSlotStatuses() == 1);
    ExpectTransition(5);
}

static void TestLastTransitionStatusWins(void)
{
    ResetHarness(7, 6, 7);

    assert(Game_HandleDatalinkPlayerSlotStatuses() == 1);
    ExpectTransition(5);
}

int main(void)
{
    TestBenignAndOutOfRangeStatusesDoNothing();
    TestStatusesThreeAndFourOnlyReportEvents();
    TestStatusSixSelectsTransitionFour();
    TestStatusSevenSelectsTransitionFive();
    TestLastTransitionStatusWins();
    return 0;
}
