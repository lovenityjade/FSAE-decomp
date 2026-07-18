#ifndef FSAE_GAME_TERMINATION_REGISTRATION_H
#define FSAE_GAME_TERMINATION_REGISTRATION_H

#include "game/termination_dispatch.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 0x0200C574
 *
 * Push a caller-owned node onto the termination list at 0x021277D0.  Known
 * callers use this to register destructors for statically initialized objects;
 * context may be NULL.
 */
void Game_RegisterTerminationNode(
    void *context,
    GameTerminationNodeCallback callback,
    GameTerminationNode *node
);

/*
 * No independent unregister routine is claimed: exhaustive literal references
 * to 0x021277D0 identify only this push and the drain at 0x0200C500.
 */

#ifdef __cplusplus
}
#endif

#endif /* FSAE_GAME_TERMINATION_REGISTRATION_H */
