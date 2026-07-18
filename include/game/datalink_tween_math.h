#ifndef FSAE_GAME_DATALINK_TWEEN_MATH_H
#define FSAE_GAME_DATALINK_TWEEN_MATH_H

#include <stdint.h>

typedef struct Game_DatalinkVector2 {
    int32_t x;
    int32_t y;
} Game_DatalinkVector2;

void Game_DatalinkInterpolateVectorLinear(
    Game_DatalinkVector2 *output,
    const Game_DatalinkVector2 *start,
    const Game_DatalinkVector2 *end,
    int progress);

void Game_DatalinkInterpolateVectorEaseInSine(
    Game_DatalinkVector2 *output,
    const Game_DatalinkVector2 *start,
    const Game_DatalinkVector2 *end,
    int progress);

void Game_DatalinkInterpolateVectorEaseOutSine(
    Game_DatalinkVector2 *output,
    const Game_DatalinkVector2 *start,
    const Game_DatalinkVector2 *end,
    int progress);

#endif
