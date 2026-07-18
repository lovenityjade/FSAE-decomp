#include "game/format_buffer.h"
#include "game/format_runtime_wide.h"
#include "game/termination_dispatch.h"

#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/* Runtime data recovered from the literal pools in 0x02008110..0x02008263. */
extern void (*gGameFormatConstraintHandler)(void);
extern unsigned char gGameRuntimeSignalMutex;
extern uintptr_t gGameRuntimeSignalOwner;
extern uint32_t gGameRuntimeSignalDepth;
extern void (*gGameRuntimeSignalHandlers[7])(int);

extern int OS_TryLockMutex(void *mutex);
extern void OS_LockMutex(void *mutex);
extern void OS_UnlockMutex(void *mutex);
extern uintptr_t Game_LogCurrentThreadId(void);

/* 0x02008080..0x020080E3; callback literal 0x02007F14 follows. */
int Game_VSNPrintf_02008080(
    char *output,
    uint32_t capacity,
    const char *format,
    va_list arguments
)
{
    GameFormatBuffer state;
    int result;

    state.buffer = (uint8_t *)output;
    state.capacity = capacity;
    state.length = 0;

    result = Game_RunFormatter(
        (GameFormatWriteCallback)GameFormatBuffer_Write,
        &state,
        format,
        arguments,
        0
    );

    if (output != NULL) {
        if ((uint32_t)result < capacity) {
            output[result] = '\0';
        } else if (capacity != 0) {
            output[capacity - 1] = '\0';
        }
    }

    return result;
}

/* 0x020080E4..0x0200810F. */
int Game_SPrintf_020080e4(char *output, const char *format, ...)
{
    int result;
    va_list arguments;

    va_start(arguments, format);
    result = Game_VSNPrintf_02008080(
        output,
        UINT32_MAX,
        format,
        arguments
    );
    va_end(arguments);
    return result;
}

/* 0x02008110..0x02008137; handler-pointer literal follows. */
void Game_RaiseFormatConstraint_02008110(void)
{
    void (*handler)(void) = gGameFormatConstraintHandler;

    if (handler != NULL) {
        handler();
    } else {
        Game_DefaultFormatConstraintHandler_02008138();
    }
}

/* 0x02008138..0x0200813B: the SDK default handler is deliberately empty. */
void Game_DefaultFormatConstraintHandler_02008138(void)
{
}

/* 0x0200813C..0x02008263; five-word runtime-state pool follows. */
int Game_RaiseRuntimeSignal(int signal_number)
{
    void (*handler)(int);
    uintptr_t current_thread;

    if (signal_number < 1 || signal_number > 7) {
        return -1;
    }

    current_thread = Game_LogCurrentThreadId();
    if (OS_TryLockMutex(&gGameRuntimeSignalMutex) == 0) {
        gGameRuntimeSignalOwner = current_thread;
        gGameRuntimeSignalDepth = 1;
    } else if (gGameRuntimeSignalOwner == current_thread) {
        ++gGameRuntimeSignalDepth;
    } else {
        OS_LockMutex(&gGameRuntimeSignalMutex);
        gGameRuntimeSignalOwner = current_thread;
        gGameRuntimeSignalDepth = 1;
    }

    handler = gGameRuntimeSignalHandlers[signal_number - 1];
    if (handler != (void (*)(int))(uintptr_t)1) {
        gGameRuntimeSignalHandlers[signal_number - 1] = NULL;
    }

    --gGameRuntimeSignalDepth;
    if (gGameRuntimeSignalDepth == 0) {
        OS_UnlockMutex(&gGameRuntimeSignalMutex);
    }

    if (handler == (void (*)(int))(uintptr_t)1) {
        return 0;
    }
    if (handler == NULL) {
        if (signal_number != 1) {
            Game_RunTermination(0);
        }
        return 0;
    }

    handler(signal_number);
    return 0;
}

/* 0x02008264..0x0200827F. */
uint32_t Game_StringLength_02008264(const char *string)
{
    const char *cursor = string;

    while (*cursor != '\0') {
        ++cursor;
    }
    return (uint32_t)(cursor - string);
}

/* 0x02008280..0x02008347; the two following constants drive word-at-a-time
 * zero detection in the retail implementation. */
void Game_CopyString_02008280(char *destination, const char *source)
{
    while ((*destination++ = *source++) != '\0') {
    }
}

/* 0x02008348..0x02008397. */
void Game_CopyStringBounded_02008348(
    char *destination,
    const char *source,
    uint32_t count
)
{
    while (count != 0 && *source != '\0') {
        *destination++ = *source++;
        --count;
    }
    while (count != 0) {
        *destination++ = '\0';
        --count;
    }
}

/* 0x02008398..0x020083CF. */
char *Game_FindCharacter_02008398(const char *string, int character)
{
    const unsigned char wanted = (unsigned char)character;

    for (;;) {
        if ((unsigned char)*string == wanted) {
            return (char *)string;
        }
        if (*string++ == '\0') {
            return NULL;
        }
    }
}

typedef struct GameRecoveredStreamHeader {
    uint32_t reserved_00;
    uint32_t flags_04;
} GameRecoveredStreamHeader;

/* 0x020083D0..0x0200843F.  Bits 8 and 9 are the SDK byte/wide orientation
 * markers.  A zero argument queries without changing an unoriented stream. */
int Game_LogPrepareStream(void *stream_pointer, int orientation)
{
    GameRecoveredStreamHeader *stream = (GameRecoveredStreamHeader *)stream_pointer;
    uint32_t flags = stream->flags_04;

    if ((flags & 0x300u) == 0) {
        if (orientation > 0) {
            flags |= 0x200u;
        } else if (orientation < 0) {
            flags |= 0x100u;
        }
        stream->flags_04 = flags;
    }

    if ((flags & 0x200u) != 0) {
        return 1;
    }
    if ((flags & 0x100u) != 0) {
        return -1;
    }
    return 0;
}

/* 0x02008440..0x0200844F; memcpy entry-point literal follows. */
void Game_CopyWideUnits_02008440(
    uint16_t *destination,
    const uint16_t *source,
    uint32_t count
)
{
    while (count-- != 0) {
        *destination++ = *source++;
    }
}

/* 0x02008450..0x02008477. */
uint16_t *Game_FindWideUnit_02008450(
    const uint16_t *string,
    uint16_t value,
    uint32_t count
)
{
    while (count-- != 0) {
        if (*string == value) {
            return (uint16_t *)string;
        }
        ++string;
    }
    return NULL;
}
