#include "game/log_format.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/* Recovered global output-stream object at 0x020DF628. */
extern unsigned char gGameLogOutputStream;

/* Recovered mutex object at 0x02127420. */
extern unsigned char gGameLogMutex;

/*
 * Semantic names for the words at 0x0212739C and 0x021273C0.  They hold the
 * owner thread identifier and recursion depth respectively.
 */
extern uintptr_t gGameLogOwnerThread;
extern uint32_t gGameLogLockDepth;

/*
 * the helper at 0x020083D0.  The exact library name is unknown.  The log path proceeds only
 * when this call with orientation -1 returns a negative value.
 */
extern int Game_LogPrepareStream(void *stream, int orientation);

/* TwlSDK mutex APIs identified by their instruction bodies and call context. */
extern int OS_TryLockMutex(void *mutex);
extern void OS_LockMutex(void *mutex);
extern void OS_UnlockMutex(void *mutex);

/*
 * Provisional accessor for the target load [[0x02129938 + 4] + 0x6C].  This
 * does not claim that a standalone accessor exists in the retail binary.
 */
extern uintptr_t Game_LogCurrentThreadId(void);

/* Stream writer at 0x02007EE8, passed as the formatter callback. */
extern int Game_LogStreamWrite(
    void *context,
    const void *data,
    uint32_t byte_count
);

/*
 * Core formatter at 0x02007638.  The descriptive name and va_list signature
 * are semantic C; the retail ABI receives a pointer to the saved varargs and
 * a fifth stack argument fixed to zero by this caller.
 */
extern int Game_RunFormatter(
    GameFormatWriteCallback callback,
    void *context,
    const char *format,
    va_list arguments,
    int mode
);

/*
 * 0x02007F58
 *
 * Call proof:
 * - 0x020056AC passes the assertion format and four arguments here;
 * - 0x0200807C contains callback address 0x02007EE8;
 * - 0x02008068 contains stream address 0x020DF628;
 * - mutex calls are 0x02018298/0x020181F0/0x02018240.
 */
int Game_LogFormat(const char *format, ...)
{
    uintptr_t current_thread;
    int result;
    va_list arguments;

    if (Game_LogPrepareStream(&gGameLogOutputStream, -1) >= 0) {
        return -1;
    }

    current_thread = Game_LogCurrentThreadId();
    if (OS_TryLockMutex(&gGameLogMutex) == 0) {
        gGameLogOwnerThread = current_thread;
        gGameLogLockDepth = 1;
    } else if (gGameLogOwnerThread == current_thread) {
        ++gGameLogLockDepth;
    } else {
        OS_LockMutex(&gGameLogMutex);
        gGameLogOwnerThread = current_thread;
        gGameLogLockDepth = 1;
    }

    va_start(arguments, format);
    result = Game_RunFormatter(
        Game_LogStreamWrite,
        &gGameLogOutputStream,
        format,
        arguments,
        0
    );
    va_end(arguments);

    --gGameLogLockDepth;
    if (gGameLogLockDepth == 0) {
        OS_UnlockMutex(&gGameLogMutex);
    }

    return result;
}
