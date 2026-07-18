#include "game/datalink_triplet_position.h"

#include "game/datalink_pair_position.h"

#include <stddef.h>

typedef struct Game_DatalinkTripletPosition {
    int32_t x;
    int32_t y;
} Game_DatalinkTripletPosition;

enum {
    DATALINK_TRIPLET_OBJECT_COUNT = 3,
    DATALINK_POSITION_FRACTION_BITS = 12
};

/* First six words in the ROM coordinate table at 0x020DE8FC. */
static const Game_DatalinkTripletPosition sGameDatalinkTripletPositions
    [DATALINK_TRIPLET_OBJECT_COUNT] = {
        {44, 48},
        {44, 96},
        {44, 144}
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
    sizeof(Game_DatalinkTripletPosition) == 8,
    "datalink triplet coordinate stride"
);

static int32_t Game_DatalinkTripletPositionToFx32(
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

/*
 * 0x020A79D8..0x020A7A57 (catalog/Ghidra body size: 128 bytes).
 *
 * Its literal pool occupies 0x020A7A58..0x020A7A5F.  Raw code resumes at
 * 0x020A7A60, while the next game candidate listed by the catalog is
 * 0x020A8138.  The literals select the six-word coordinate table and the
 * owner at 0x0217E6F0.
 *
 * Target offsets +0x464/+0x46C/+0x470 are owner+0x43C plus the first
 * object's fields +0x28/+0x30/+0x34.  Repeating at +0x58 selects objects
 * one and two, so this routine complements 0x020A7918 in the same recovered
 * object collection.  As in that routine, the first argument is not read.
 */
void Game_PositionDatalinkObjectTriplet(
    const void *owner,
    int32_t x_offset,
    int32_t y_offset
)
{
    int object_index;

    (void)owner;

    for (object_index = 0;
         object_index < DATALINK_TRIPLET_OBJECT_COUNT;
         ++object_index) {
        Game_DatalinkPositionObject *object =
            &gGameDatalinkPositionObjects[object_index];

        object->transition_duration_fx_28 = 0;
        object->position_x_30 = Game_DatalinkTripletPositionToFx32(
            sGameDatalinkTripletPositions[object_index].x,
            x_offset
        );
        object->position_y_34 = Game_DatalinkTripletPositionToFx32(
            sGameDatalinkTripletPositions[object_index].y,
            y_offset
        );
    }
}
