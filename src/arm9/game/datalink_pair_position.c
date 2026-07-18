#include "game/datalink_pair_position.h"

#include <stddef.h>

typedef struct Game_DatalinkPositionOffset {
    int32_t x;
    int32_t y;
} Game_DatalinkPositionOffset;

enum {
    DATALINK_POSITION_FRACTION_BITS = 12
};

/* ROM table 0x020DE7C8, consumed as three adjacent byte pairs. */
static const uint8_t sGameDatalinkPositionObjectPairs
    [GAME_DATALINK_POSITION_PAIR_COUNT][2] = {
        {5, 6},
        {7, 8},
        {9, 10}
    };

/* First three coordinate pairs in the ROM table at 0x020DE92C. */
static const Game_DatalinkPositionOffset sGameDatalinkPositionOffsets
    [GAME_DATALINK_POSITION_PAIR_COUNT] = {
        {24, 30},
        {24, 78},
        {24, 126}
    };

_Static_assert(
    sizeof(Game_DatalinkPositionObject) == 0x58,
    "datalink positioned-object stride"
);
_Static_assert(
    offsetof(Game_DatalinkPositionObject, transition_duration_fx_28) == 0x28,
    "datalink transition duration offset"
);
_Static_assert(
    offsetof(Game_DatalinkPositionObject, position_x_30) == 0x30,
    "datalink position X offset"
);
_Static_assert(
    offsetof(Game_DatalinkPositionObject, position_y_34) == 0x34,
    "datalink position Y offset"
);
_Static_assert(
    sizeof(Game_DatalinkPositionOffset) == 8,
    "datalink position-table stride"
);

static int32_t Game_DatalinkPositionToFx32(
    int32_t base,
    int32_t offset
)
{
    uint32_t target_word =
        ((uint32_t)base + (uint32_t)offset) <<
        DATALINK_POSITION_FRACTION_BITS;

    if (target_word <= INT32_MAX) {
        return (int32_t)target_word;
    }
    return -1 - (int32_t)(UINT32_MAX - target_word);
}

static void Game_SetDatalinkObjectPosition(
    uint8_t object_index,
    int32_t x,
    int32_t y
)
{
    Game_DatalinkPositionObject *object =
        &gGameDatalinkPositionObjects[object_index];

    object->transition_duration_fx_28 = 0;
    object->position_x_30 = x;
    object->position_y_34 = y;
}

/*
 * 0x020A7918..0x020A79CB (catalog/Ghidra body size: 180 bytes).
 *
 * The three literals at 0x020A79CC..0x020A79D7 select the byte-pair table,
 * the 0x58-byte object array at target address 0x0217EB2C, and the coordinate
 * table.  The next catalogued function begins immediately at 0x020A79D8.
 *
 * Each coordinate is translated by the caller's offsets, converted to the
 * target's 20.12 fixed-point format, and copied to both objects in its pair.
 * The first argument is present at call sites but is not read by this body.
 */
void Game_PositionDatalinkObjectPairs(
    const void *owner,
    int32_t x_offset,
    int32_t y_offset
)
{
    int pair;

    (void)owner;

    for (pair = 0; pair < GAME_DATALINK_POSITION_PAIR_COUNT; ++pair) {
        int32_t x = Game_DatalinkPositionToFx32(
            sGameDatalinkPositionOffsets[pair].x,
            x_offset
        );
        int32_t y = Game_DatalinkPositionToFx32(
            sGameDatalinkPositionOffsets[pair].y,
            y_offset
        );

        Game_SetDatalinkObjectPosition(
            sGameDatalinkPositionObjectPairs[pair][0],
            x,
            y
        );
        Game_SetDatalinkObjectPosition(
            sGameDatalinkPositionObjectPairs[pair][1],
            x,
            y
        );
    }
}
