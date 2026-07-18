#include "game/final_termination.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
    EVENT_CALLBACK_A = 1,
    EVENT_CALLBACK_B = 2,
    EVENT_FINAL_CALLBACK = 3,
    EVENT_CLOSE_STREAMS = 4,
    EVENT_PLATFORM_HOOK = 5
};

GameTerminationCallback gGameFinalTerminationCallback;
int32_t gGameFinalCallbackCount;
GameTerminationCallback gGameFinalCallbacks[8];

unsigned char gGameFinalizerMutex;
uintptr_t gGameFinalizerOwnerThread;
uint32_t gGameFinalizerLockDepth;

static int sEvents[12];
static unsigned int sEventCount;
static uintptr_t sCurrentThread;
static int sTryLockResult;
static unsigned int sBlockingLockCalls;
static unsigned int sUnlockCalls;

static void RecordA(void)
{
    sEvents[sEventCount++] = EVENT_CALLBACK_A;
}

static void RecordB(void)
{
    sEvents[sEventCount++] = EVENT_CALLBACK_B;
}

static void RecordFinal(void)
{
    sEvents[sEventCount++] = EVENT_FINAL_CALLBACK;
}

int OS_TryLockMutex(void *mutex)
{
    assert(mutex == &gGameFinalizerMutex);
    return sTryLockResult;
}

void OS_LockMutex(void *mutex)
{
    assert(mutex == &gGameFinalizerMutex);
    ++sBlockingLockCalls;
}

void OS_UnlockMutex(void *mutex)
{
    assert(mutex == &gGameFinalizerMutex);
    ++sUnlockCalls;
}

uintptr_t Game_TerminationCurrentThreadId(void)
{
    return sCurrentThread;
}

int Game_CloseRuntimeStream(void *stream)
{
    assert(stream == NULL);
    sEvents[sEventCount++] = EVENT_CLOSE_STREAMS;
    return 0;
}

void Game_RunPlatformTerminationHook(void)
{
    sEvents[sEventCount++] = EVENT_PLATFORM_HOOK;
}

static void ResetHarness(void)
{
    memset(sEvents, 0, sizeof(sEvents));
    memset(gGameFinalCallbacks, 0, sizeof(gGameFinalCallbacks));
    sEventCount = 0;
    sCurrentThread = 0x5678U;
    sTryLockResult = 0;
    sBlockingLockCalls = 0;
    sUnlockCalls = 0;
    gGameFinalizerOwnerThread = 0;
    gGameFinalizerLockDepth = 0;
    gGameFinalCallbackCount = 0;
    gGameFinalTerminationCallback = NULL;
}

static void TestLifoCallbacksAndFinalStages(void)
{
    ResetHarness();
    gGameFinalCallbacks[0] = RecordA;
    gGameFinalCallbacks[1] = RecordB;
    gGameFinalCallbackCount = 2;
    gGameFinalTerminationCallback = RecordFinal;

    Game_DispatchTermination(99);

    assert(sEventCount == 5);
    assert(sEvents[0] == EVENT_CALLBACK_B);
    assert(sEvents[1] == EVENT_CALLBACK_A);
    assert(sEvents[2] == EVENT_FINAL_CALLBACK);
    assert(sEvents[3] == EVENT_CLOSE_STREAMS);
    assert(sEvents[4] == EVENT_PLATFORM_HOOK);
    assert(gGameFinalCallbackCount == 0);
    assert(gGameFinalTerminationCallback == NULL);
    assert(gGameFinalizerOwnerThread == sCurrentThread);
    assert(gGameFinalizerLockDepth == 0);
    assert(sBlockingLockCalls == 0);
    assert(sUnlockCalls == 1);
}

static void TestRecursiveOwnerKeepsOuterLock(void)
{
    ResetHarness();
    sTryLockResult = 1;
    gGameFinalizerOwnerThread = sCurrentThread;
    gGameFinalizerLockDepth = 2;

    Game_DispatchTermination(1);

    assert(gGameFinalizerLockDepth == 2);
    assert(sBlockingLockCalls == 0);
    assert(sUnlockCalls == 0);
    assert(sEvents[0] == EVENT_CLOSE_STREAMS);
    assert(sEvents[1] == EVENT_PLATFORM_HOOK);
}

static void TestContendedLockBlocksAndReplacesOwner(void)
{
    ResetHarness();
    sTryLockResult = 1;
    gGameFinalizerOwnerThread = 0x9999U;
    gGameFinalizerLockDepth = 4;

    Game_DispatchTermination(0);

    assert(gGameFinalizerOwnerThread == sCurrentThread);
    assert(gGameFinalizerLockDepth == 0);
    assert(sBlockingLockCalls == 1);
    assert(sUnlockCalls == 1);
}

int main(void)
{
    TestLifoCallbacksAndFinalStages();
    TestRecursiveOwnerKeepsOuterLock();
    TestContendedLockBlocksAndReplacesOwner();
    return 0;
}
