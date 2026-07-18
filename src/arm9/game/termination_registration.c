#include "game/termination_registration.h"

/*
 * 0x0200C574
 *
 * Instruction order is next, callback, context, then head.  Publishing the
 * head last ensures a fully initialized node is visible to the drain routine.
 */
void Game_RegisterTerminationNode(
    void *context,
    GameTerminationNodeCallback callback,
    GameTerminationNode *node
)
{
    node->next = gGameTerminationNodeHead;
    node->callback = callback;
    node->context = context;
    gGameTerminationNodeHead = node;
}
