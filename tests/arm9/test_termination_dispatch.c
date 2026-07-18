#include "game/termination_dispatch.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

enum {
    EVENT_NODE_A = 1,
    EVENT_NODE_B = 2,
    EVENT_CALLBACK = 3,
    EVENT_DISPATCH = 4
};

GameFatalRuntimeState gGameFatalRuntimeState;
GameTerminationNode *gGameTerminationNodeHead;
GameTerminationCallback gGameTerminationCallback;

static int sEvents[8];
static unsigned int sEventCount;
static int sDispatchedStatus;

static void RecordNode(void *context, int reason)
{
    assert(reason == -1);
    sEvents[sEventCount++] = *(const int *)context;
}

static void RecordCallback(void)
{
    sEvents[sEventCount++] = EVENT_CALLBACK;
}

void Game_DispatchTermination(int status)
{
    sDispatchedStatus = status;
    sEvents[sEventCount++] = EVENT_DISPATCH;
}

static void ResetHarness(void)
{
    memset(&gGameFatalRuntimeState, 0, sizeof(gGameFatalRuntimeState));
    memset(sEvents, 0, sizeof(sEvents));
    sEventCount = 0;
    sDispatchedStatus = 0;
    gGameTerminationNodeHead = NULL;
    gGameTerminationCallback = NULL;
}

static void TestCleanupOrderAndDispatch(void)
{
    static const int event_a = EVENT_NODE_A;
    static const int event_b = EVENT_NODE_B;
    GameTerminationNode node_b = {NULL, RecordNode, (void *)&event_b};
    GameTerminationNode node_a = {&node_b, RecordNode, (void *)&event_a};

    ResetHarness();
    gGameTerminationNodeHead = &node_a;
    gGameTerminationCallback = RecordCallback;

    Game_RunTermination(7);

    assert(sEventCount == 4);
    assert(sEvents[0] == EVENT_NODE_A);
    assert(sEvents[1] == EVENT_NODE_B);
    assert(sEvents[2] == EVENT_CALLBACK);
    assert(sEvents[3] == EVENT_DISPATCH);
    assert(gGameTerminationNodeHead == NULL);
    assert(gGameTerminationCallback == NULL);
    assert(sDispatchedStatus == 7);
}

static void TestRequestedTerminationSkipsCleanup(void)
{
    static const int event_a = EVENT_NODE_A;
    GameTerminationNode node = {NULL, RecordNode, (void *)&event_a};

    ResetHarness();
    gGameFatalRuntimeState.termination_requested = 1;
    gGameTerminationNodeHead = &node;
    gGameTerminationCallback = RecordCallback;

    Game_RunTermination(3);

    assert(sEventCount == 1);
    assert(sEvents[0] == EVENT_DISPATCH);
    assert(gGameTerminationNodeHead == &node);
    assert(gGameTerminationCallback == RecordCallback);
    assert(sDispatchedStatus == 3);
}

static void TestNullCallbackStillDrainsNodes(void)
{
    static const int event_a = EVENT_NODE_A;
    GameTerminationNode node = {NULL, RecordNode, (void *)&event_a};

    ResetHarness();
    gGameTerminationNodeHead = &node;

    Game_RunTermination(1);

    assert(sEventCount == 2);
    assert(sEvents[0] == EVENT_NODE_A);
    assert(sEvents[1] == EVENT_DISPATCH);
    assert(gGameTerminationNodeHead == NULL);
}

int main(void)
{
    TestCleanupOrderAndDispatch();
    TestRequestedTerminationSkipsCleanup();
    TestNullCallbackStillDrainsNodes();
    return 0;
}
