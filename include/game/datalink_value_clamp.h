#ifndef FSAE_GAME_DATALINK_VALUE_CLAMP_H
#define FSAE_GAME_DATALINK_VALUE_CLAMP_H

#include <stdint.h>

void Game_ClampDatalinkSignedByte(
    int8_t *value,
    int minimum,
    int maximum);

#endif
