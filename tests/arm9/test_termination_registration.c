#include "game/termination_registration.h"

#include <assert.h>
#include <stddef.h>

enum {
    EVENT_A = 1,
    EVENT_B = 2
};

GameTerminationNode *gGameTerminationNodeHead;

static int sEvents[4];
static unsigned int sEventCount;

static void RecordEvent(void *context, int reason)
{
    assert(reason == -1);
    sEvents[sEventCount++] = context != NULL ? *(const int *)context : 0;
}

static void TestRegistrationIsLifo(void)
{
    static const int event_a = EVENT_A;
    static const int event_b = EVENT_B;
    GameTerminationNode node_a;
    GameTerminationNode node_b;

    gGameTerminationNodeHead = NULL;
    Game_RegisterTerminationNode((void *)&event_a, RecordEvent, &node_a);
    Game_RegisterTerminationNode((void *)&event_b, RecordEvent, &node_b);

    assert(gGameTerminationNodeHead == &node_b);
    assert(node_b.next == &node_a);
    assert(node_b.callback == RecordEvent);
    assert(node_b.context == &event_b);
    assert(node_a.next == NULL);
    assert(node_a.callback == RecordEvent);
    assert(node_a.context == &event_a);
}

static void TestNullContextIsPreserved(void)
{
    GameTerminationNode node;

    gGameTerminationNodeHead = NULL;
    Game_RegisterTerminationNode(NULL, RecordEvent, &node);

    assert(gGameTerminationNodeHead == &node);
    assert(node.context == NULL);
}

/*
 * Integration proof for the existing 0x0200C500 reconstruction.  This test is
 * prepared for a later host link, but this task validates syntax only.
 */
static void TestRegisteredNodesDrainInReverseOrder(void)
{
    static const int event_a = EVENT_A;
    static const int event_b = EVENT_B;
    GameTerminationNode node_a;
    GameTerminationNode node_b;

    sEventCount = 0;
    gGameTerminationNodeHead = NULL;
    Game_RegisterTerminationNode((void *)&event_a, RecordEvent, &node_a);
    Game_RegisterTerminationNode((void *)&event_b, RecordEvent, &node_b);

    Game_RunPendingDestructors();

    assert(sEventCount == 2);
    assert(sEvents[0] == EVENT_B);
    assert(sEvents[1] == EVENT_A);
    assert(gGameTerminationNodeHead == NULL);
}

int main(void)
{
    TestRegistrationIsLifo();
    TestNullContextIsPreserved();
    TestRegisteredNodesDrainInReverseOrder();
    return 0;
}
