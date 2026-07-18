#include "game/final_termination.h"

#include <stddef.h>
#include <stdint.h>

/* Mutex object at 0x021273D8. */
extern unsigned char gGameFinalizerMutex;

/* Owner and recursion words at 0x02127390 and 0x021273B4. */
extern uintptr_t gGameFinalizerOwnerThread;
extern uint32_t gGameFinalizerLockDepth;

extern int OS_TryLockMutex(void *mutex);
extern void OS_LockMutex(void *mutex);
extern void OS_UnlockMutex(void *mutex);

/*
 * Provisional accessor for [[0x02129938 + 4] + 0x6C].  It represents a target
 * data load and does not claim a standalone retail function.
 */
extern uintptr_t Game_TerminationCurrentThreadId(void);

/*
 * The helper at 0x02005A38 receives NULL here and consequently walks the three runtime
 * stream records through 0x0200563C.  Its final library name is unknown.
 */
extern int Game_CloseRuntimeStream(void *stream);

/*
 * Thunk at 0x0200B10C, targeting the empty hook at 0x0200B128 in this image.
 * The descriptive name deliberately makes no stronger platform claim.
 */
extern void Game_RunPlatformTerminationHook(void);

/*
 * 0x020054F4
 *
 * Proof anchors:
 * - recursive mutex calls at 0x02018298/0x020181F0/0x02018240;
 * - callback count at 0x02127288 and array at 0x02127290;
 * - count is decremented before each indirect callback;
 * - final callback at 0x02127280 is cleared after invocation;
 * - final calls are 0x02005A38(NULL) then thunk 0x0200B10C.
 */
void Game_DispatchTermination(int status)
{
    int try_lock_result;
    uintptr_t current_thread;

    (void)status;
    try_lock_result = OS_TryLockMutex(&gGameFinalizerMutex);
    current_thread = Game_TerminationCurrentThreadId();

    if (try_lock_result == 0) {
        gGameFinalizerOwnerThread = current_thread;
        gGameFinalizerLockDepth = 1;
    } else if (gGameFinalizerOwnerThread == current_thread) {
        ++gGameFinalizerLockDepth;
    } else {
        OS_LockMutex(&gGameFinalizerMutex);
        gGameFinalizerOwnerThread = current_thread;
        gGameFinalizerLockDepth = 1;
    }

    while (gGameFinalCallbackCount > 0) {
        GameTerminationCallback callback;

        --gGameFinalCallbackCount;
        callback = gGameFinalCallbacks[gGameFinalCallbackCount];
        callback();
    }

    --gGameFinalizerLockDepth;
    if (gGameFinalizerLockDepth == 0) {
        OS_UnlockMutex(&gGameFinalizerMutex);
    }

    if (gGameFinalTerminationCallback != NULL) {
        GameTerminationCallback callback = gGameFinalTerminationCallback;

        callback();
        gGameFinalTerminationCallback = NULL;
    }

    (void)Game_CloseRuntimeStream(NULL);
    Game_RunPlatformTerminationHook();
}
