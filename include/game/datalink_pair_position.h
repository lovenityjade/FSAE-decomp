#ifndef GAME_DATALINK_PAIR_POSITION_H
#define GAME_DATALINK_PAIR_POSITION_H

#include <stdint.h>

enum {
    GAME_DATALINK_POSITION_PAIR_COUNT = 3,
    GAME_DATALINK_POSITION_OBJECT_COUNT = 11
};

typedef struct Game_DatalinkPositionObject {
    uint8_t unknown_00[3];
    uint8_t visible_03;
    uint8_t unknown_04[0x24];
    int32_t transition_duration_fx_28;
    uint8_t unknown_2c[4];
    int32_t position_x_30;
    int32_t position_y_34;
    uint8_t unknown_38[0x20];
} Game_DatalinkPositionObject;

extern Game_DatalinkPositionObject
    gGameDatalinkPositionObjects[GAME_DATALINK_POSITION_OBJECT_COUNT];

void Game_PositionDatalinkObjectPairs(
    const void *owner,
    int32_t x_offset,
    int32_t y_offset
);

#endif
