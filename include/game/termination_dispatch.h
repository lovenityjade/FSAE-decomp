#ifndef FSAE_GAME_TERMINATION_DISPATCH_H
#define FSAE_GAME_TERMINATION_DISPATCH_H

#include "game/fatal_error.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*GameTerminationCallback)(void);
typedef void (*GameTerminationNodeCallback)(void *context, int reason);

/*
 * Node layout consumed at 0x0200C500: next at +0, callback at +4 and context
 * at +8.  The callback is always invoked with reason -1.
 */
typedef struct GameTerminationNode {
    struct GameTerminationNode *next;
    GameTerminationNodeCallback callback;
    void *context;
} GameTerminationNode;

#if UINTPTR_MAX == UINT32_MAX
typedef char GameTerminationNodeTargetSizeCheck[
    sizeof(GameTerminationNode) == 0x0C ? 1 : -1];
typedef char GameTerminationNodeCallbackOffsetCheck[
    offsetof(GameTerminationNode, callback) == 0x04 ? 1 : -1];
typedef char GameTerminationNodeContextOffsetCheck[
    offsetof(GameTerminationNode, context) == 0x08 ? 1 : -1];
#endif

/* Recovered list head at 0x021277D0. */
extern GameTerminationNode *gGameTerminationNodeHead;

/*
 * Semantic pointer overlay for the word at 0x02127284
 * (gGameFatalRuntimeState + 4).  A final data-layout pass must merge this
 * target word with the currently opaque field in GameFatalRuntimeState.
 */
extern GameTerminationCallback gGameTerminationCallback;

/* 0x0200C500: detach and invoke every registered termination node. */
void Game_RunPendingDestructors(void);

/* 0x020054B0: optional cleanup followed by unconditional final dispatch. */
void Game_RunTermination(int status);

#ifdef __cplusplus
}
#endif

#endif /* FSAE_GAME_TERMINATION_DISPATCH_H */
