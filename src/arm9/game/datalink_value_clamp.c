#include "game/datalink_value_clamp.h"

/* 0x020AC898 */
void Game_ClampDatalinkSignedByte(
    int8_t *value,
    int minimum,
    int maximum)
{
    if (*value < minimum) {
        *value = (int8_t)minimum;
    } else if (*value > maximum) {
        *value = (int8_t)maximum;
    }
}
