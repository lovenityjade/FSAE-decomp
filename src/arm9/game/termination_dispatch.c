#include "game/termination_dispatch.h"

#include <stddef.h>

/*
 * Final dispatcher at 0x020054F4.  It is substantially larger than this unit;
 * the descriptive name remains provisional and no behavior is inferred here.
 */
extern void Game_DispatchTermination(int status);

/*
 * 0x0200C500
 *
 * The head is advanced before each callback.  That ordering is instruction-
 * proven and permits callbacks to mutate or extend the remaining list.
 */
void Game_RunPendingDestructors(void)
{
    while (gGameTerminationNodeHead != NULL) {
        GameTerminationNode *node = gGameTerminationNodeHead;

        gGameTerminationNodeHead = node->next;
        node->callback(node->context, -1);
    }
}

/*
 * 0x020054B0
 *
 * Proof anchors:
 * - state base 0x02127280, cleanup gate at +0x0C;
 * - callback word at +0x04, cleared only after a non-NULL invocation;
 * - pending-node drain call at 0x020054C8 -> 0x0200C500;
 * - final dispatch call at 0x020054E8 -> 0x020054F4.
 */
void Game_RunTermination(int status)
{
    if (gGameFatalRuntimeState.termination_requested == 0) {
        GameTerminationCallback callback;

        Game_RunPendingDestructors();
        callback = gGameTerminationCallback;
        if (callback != NULL) {
            callback();
            gGameTerminationCallback = NULL;
        }
    }

    Game_DispatchTermination(status);
}
