#ifndef FSAE_GAME_TIME_COUNTER_H
#define FSAE_GAME_TIME_COUNTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 0x020A650C
 *
 * Draw a seconds counter as MM:SS into a 32-column NNS screen tilemap.  The
 * minutes display saturates at 99; seconds retain their modulo-60 value.
 */
void Game_DrawTimeCounter(int total_seconds, uint16_t *screen_tilemap);

#ifdef __cplusplus
}
#endif

#endif /* FSAE_GAME_TIME_COUNTER_H */
