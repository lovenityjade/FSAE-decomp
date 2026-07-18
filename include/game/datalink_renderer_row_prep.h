#ifndef GAME_DATALINK_RENDERER_ROW_PREP_H
#define GAME_DATALINK_RENDERER_ROW_PREP_H

#include <stdbool.h>
#include <stdint.h>

enum {
    GAME_DATALINK_RENDER_ROW_VALUE_COUNT = 10
};

bool Game_PrepareDatalinkRenderRow(
    uint32_t values[GAME_DATALINK_RENDER_ROW_VALUE_COUNT],
    uint8_t slot
);

#endif
