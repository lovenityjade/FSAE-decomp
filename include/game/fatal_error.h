#ifndef FSAE_GAME_FATAL_ERROR_H
#define FSAE_GAME_FATAL_ERROR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Proven 16-byte prefix of the runtime state at 0x02127280.  The fatal path
 * writes termination_requested at +0x0C.  The first three words are kept
 * opaque because their ownership belongs to the wider termination runtime.
 */
typedef struct GameFatalRuntimeState {
    uint32_t unknown_00;
    uint32_t unknown_04;
    uint32_t unknown_08;
    uint32_t termination_requested;
} GameFatalRuntimeState;

typedef char GameFatalRuntimeStateSizeCheck[
    sizeof(GameFatalRuntimeState) == 0x10 ? 1 : -1];

/* Recovered data object at 0x02127280. */
extern GameFatalRuntimeState gGameFatalRuntimeState;

/*
 * 0x02005488
 *
 * Entry used by the internal assertion handler.  The machine code returns if
 * the termination dispatcher returns, so this is deliberately not declared
 * noreturn.
 */
void Game_HandleFatalAssertion(void);

#ifdef __cplusplus
}
#endif

#endif /* FSAE_GAME_FATAL_ERROR_H */
