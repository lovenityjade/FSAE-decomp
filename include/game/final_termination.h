#ifndef FSAE_GAME_FINAL_TERMINATION_H
#define FSAE_GAME_FINAL_TERMINATION_H

#include "game/termination_dispatch.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Semantic overlays for the callback state rooted at 0x02127280:
 *   +0x00 final callback
 *   +0x08 LIFO callback count
 *   +0x10 callback array (absolute address 0x02127290)
 *
 * These overlays describe interior target words; final data placement remains
 * a later linker/layout concern.
 */
extern GameTerminationCallback gGameFinalTerminationCallback;
extern int32_t gGameFinalCallbackCount;
extern GameTerminationCallback gGameFinalCallbacks[];

/*
 * 0x020054F4
 *
 * Final callback and stream-shutdown stage.  The input status is forwarded by
 * 0x020054B0 but is not read by the recovered instructions.
 */
void Game_DispatchTermination(int status);

#ifdef __cplusplus
}
#endif

#endif /* FSAE_GAME_FINAL_TERMINATION_H */
