#include "game/format_buffer.h"
#include "game/log_format.h"

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

unsigned char gGameLogOutputStream;
unsigned char gGameLogMutex;
uintptr_t gGameLogOwnerThread;
uint32_t gGameLogLockDepth;

static uint8_t sCapturedBytes[64];
static GameFormatBuffer sCapturedOutput;
static uintptr_t sCurrentThread;
static int sPrepareResult;
static int sTryLockResult;
static unsigned int sBlockingLockCalls;
static unsigned int sUnlockCalls;
static unsigned int sFormatterCalls;

int Game_LogPrepareStream(void *stream, int orientation)
{
    assert(stream == &gGameLogOutputStream);
    assert(orientation == -1);
    return sPrepareResult;
}

int OS_TryLockMutex(void *mutex)
{
    assert(mutex == &gGameLogMutex);
    return sTryLockResult;
}

void OS_LockMutex(void *mutex)
{
    assert(mutex == &gGameLogMutex);
    ++sBlockingLockCalls;
}

void OS_UnlockMutex(void *mutex)
{
    assert(mutex == &gGameLogMutex);
    ++sUnlockCalls;
}

uintptr_t Game_LogCurrentThreadId(void)
{
    return sCurrentThread;
}

int Game_LogStreamWrite(void *context, const void *data, uint32_t byte_count)
{
    assert(context == &gGameLogOutputStream);
    return GameFormatBuffer_Write(&sCapturedOutput, data, byte_count);
}

int Game_RunFormatter(
    GameFormatWriteCallback callback,
    void *context,
    const char *format,
    va_list arguments,
    int mode
)
{
    char formatted[64];
    int length;

    assert(mode == 0);
    ++sFormatterCalls;
    length = vsnprintf(formatted, sizeof(formatted), format, arguments);
    assert(length >= 0);
    assert((size_t)length < sizeof(formatted));
    assert(callback(context, formatted, (uint32_t)length) != 0);
    return length;
}

static void ResetHarness(void)
{
    memset(sCapturedBytes, 0, sizeof(sCapturedBytes));
    sCapturedOutput.buffer = sCapturedBytes;
    sCapturedOutput.capacity = sizeof(sCapturedBytes);
    sCapturedOutput.length = 0;
    sCurrentThread = 0x1234U;
    sPrepareResult = -1;
    sTryLockResult = 0;
    sBlockingLockCalls = 0;
    sUnlockCalls = 0;
    sFormatterCalls = 0;
    gGameLogOwnerThread = 0;
    gGameLogLockDepth = 0;
}

static void TestFreshLockAndFormatting(void)
{
    ResetHarness();

    assert(Game_LogFormat("%s:%d", "assert", 43) == 9);
    assert(sCapturedOutput.length == 9);
    assert(memcmp(sCapturedBytes, "assert:43", 9) == 0);
    assert(gGameLogOwnerThread == sCurrentThread);
    assert(gGameLogLockDepth == 0);
    assert(sBlockingLockCalls == 0);
    assert(sUnlockCalls == 1);
    assert(sFormatterCalls == 1);
}

static void TestRecursiveLockRemainsOwned(void)
{
    ResetHarness();
    sTryLockResult = 1;
    gGameLogOwnerThread = sCurrentThread;
    gGameLogLockDepth = 2;

    assert(Game_LogFormat("ok") == 2);
    assert(gGameLogLockDepth == 2);
    assert(sBlockingLockCalls == 0);
    assert(sUnlockCalls == 0);
}

static void TestContendedLockBlocks(void)
{
    ResetHarness();
    sTryLockResult = 1;
    gGameLogOwnerThread = 0x9999U;
    gGameLogLockDepth = 3;

    assert(Game_LogFormat("x") == 1);
    assert(gGameLogOwnerThread == sCurrentThread);
    assert(gGameLogLockDepth == 0);
    assert(sBlockingLockCalls == 1);
    assert(sUnlockCalls == 1);
}

static void TestRejectedStreamReturnsMinusOne(void)
{
    ResetHarness();
    sPrepareResult = 0;

    assert(Game_LogFormat("unused") == -1);
    assert(sFormatterCalls == 0);
    assert(sUnlockCalls == 0);
}

int main(void)
{
    TestFreshLockAndFormatting();
    TestRecursiveLockRemainsOwned();
    TestContendedLockBlocks();
    TestRejectedStreamReturnsMinusOne();
    return 0;
}
