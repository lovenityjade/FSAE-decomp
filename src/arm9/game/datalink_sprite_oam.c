#include "game/datalink_sprite_oam.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_OAM_SHAPE_COUNT = 3,
    DATALINK_OAM_SIZE_COUNT = 4,
    DATALINK_OAM_X_MASK = 0x01FF,
    DATALINK_OAM_Y_MASK = 0x00FF,
    DATALINK_OAM_SHAPE_SHIFT = 14,
    DATALINK_OAM_SIZE_SHIFT = 14,
    DATALINK_OAM_SHAPE_MASK = 3,
    DATALINK_OAM_SIZE_MASK = 3,
    DATALINK_OAM_PRIORITY_SHIFT = 10,
    DATALINK_OAM_PRIORITY_CLEAR_MASK = 0xF3FF,
    DATALINK_OAM_SCREEN_WIDTH = 256,
    DATALINK_OAM_SCREEN_HEIGHT = 192,
    DATALINK_OAM_AGE_THRESHOLD = 60,
    DATALINK_FX_SHIFT = 12
};

typedef struct Game_DatalinkOamDimensions {
    int32_t width;
    int32_t height;
} Game_DatalinkOamDimensions;

typedef struct Game_DatalinkOamBounds {
    uint32_t minimum_x;
    int32_t minimum_right_edge;
    uint32_t minimum_y;
    uint32_t maximum_bottom_edge;
} Game_DatalinkOamBounds;

static const Game_DatalinkOamDimensions
    sGameDatalinkOamDimensions[
        DATALINK_OAM_SHAPE_COUNT * DATALINK_OAM_SIZE_COUNT
    ] = {
        {8, 8}, {16, 16}, {32, 32}, {64, 64},
        {16, 8}, {32, 8}, {32, 16}, {64, 32},
        {8, 16}, {8, 32}, {16, 32}, {32, 64}
    };

extern uint32_t NNS_G2dMakeCellToOams(
    Game_DatalinkOamEntry *output,
    uint32_t capacity,
    const void *cell,
    const void *affine_transform,
    const Game_DatalinkSpriteTranslation *translation,
    void *attribute_callback,
    void *callback_argument
);

extern void NNS_G2dEntryOamManagerOam(
    Game_DatalinkOamManager *manager,
    const Game_DatalinkOamEntry *oam_entries,
    uint32_t count
);

_Static_assert(
    sizeof(Game_DatalinkOamEntry) == 8,
    "datalink OAM entry size"
);

#if UINTPTR_MAX == UINT32_MAX
_Static_assert(
    offsetof(Game_DatalinkSpriteCellBinding, current_cell_30) == 0x30,
    "datalink sprite cell pointer offset"
);
_Static_assert(
    offsetof(Game_DatalinkSpriteObject, translation_30) == 0x30,
    "datalink sprite translation offset"
);
_Static_assert(
    offsetof(Game_DatalinkSpriteObject, cell_binding_54) == 0x54,
    "datalink sprite cell-binding offset"
);
_Static_assert(
    sizeof(Game_DatalinkSpriteObject) == 0x58,
    "datalink sprite object size"
);
#endif

static uint32_t Game_GetDatalinkOamDimensionIndex(
    const Game_DatalinkOamEntry *entry
)
{
    uint32_t shape =
        ((uint32_t)entry->attribute_0 >> DATALINK_OAM_SHAPE_SHIFT) &
        DATALINK_OAM_SHAPE_MASK;
    uint32_t size =
        ((uint32_t)entry->attribute_1 >> DATALINK_OAM_SIZE_SHIFT) &
        DATALINK_OAM_SIZE_MASK;

    return shape * DATALINK_OAM_SIZE_COUNT + size;
}

static uint32_t Game_GetDatalinkOamX(
    const Game_DatalinkOamEntry *entry
)
{
    return (uint32_t)entry->attribute_1 & DATALINK_OAM_X_MASK;
}

static uint32_t Game_GetDatalinkOamY(
    const Game_DatalinkOamEntry *entry
)
{
    return (uint32_t)entry->attribute_0 & DATALINK_OAM_Y_MASK;
}

static Game_DatalinkOamBounds Game_GetDatalinkOamBounds(
    const Game_DatalinkOamEntry *entries,
    int count
)
{
    uint32_t dimension_index =
        Game_GetDatalinkOamDimensionIndex(&entries[0]);
    const Game_DatalinkOamDimensions *dimensions =
        &sGameDatalinkOamDimensions[dimension_index];
    Game_DatalinkOamBounds bounds;
    int index;

    bounds.minimum_x = Game_GetDatalinkOamX(&entries[0]);
    bounds.minimum_right_edge =
        (int32_t)bounds.minimum_x + dimensions->width;
    bounds.minimum_y = Game_GetDatalinkOamY(&entries[0]);
    bounds.maximum_bottom_edge =
        bounds.minimum_y + (uint32_t)dimensions->height;

    for (index = 1; index < count; ++index) {
        uint32_t x = Game_GetDatalinkOamX(&entries[index]);
        uint32_t y = Game_GetDatalinkOamY(&entries[index]);
        int32_t right_edge = (int32_t)x + dimensions->width;
        uint32_t bottom_edge = y + (uint32_t)dimensions->height;

        if (x < bounds.minimum_x) {
            bounds.minimum_x = x;
        }
        if (right_edge < bounds.minimum_right_edge) {
            bounds.minimum_right_edge = right_edge;
        }
        if (y < bounds.minimum_y) {
            bounds.minimum_y = y;
        }
        if (bottom_edge > bounds.maximum_bottom_edge) {
            bounds.maximum_bottom_edge = bottom_edge;
        }
    }

    return bounds;
}

static int Game_AreDatalinkOamBoundsVisible(
    const Game_DatalinkOamBounds *bounds,
    const Game_DatalinkSpriteObject *sprite
)
{
    uint32_t normalized_minimum_y;
    uint32_t normalized_bottom_edge;

    if (bounds->minimum_right_edge < 0 ||
        bounds->minimum_x >= DATALINK_OAM_SCREEN_WIDTH) {
        return 0;
    }

    normalized_minimum_y = bounds->minimum_y & DATALINK_OAM_Y_MASK;
    normalized_bottom_edge =
        bounds->maximum_bottom_edge & DATALINK_OAM_Y_MASK;
    if (normalized_minimum_y >= DATALINK_OAM_SCREEN_HEIGHT &&
        normalized_minimum_y < DATALINK_OAM_SCREEN_WIDTH &&
        bounds->minimum_right_edge >= DATALINK_OAM_SCREEN_HEIGHT &&
        normalized_bottom_edge < DATALINK_OAM_SCREEN_WIDTH) {
        return 0;
    }
    if ((sprite->translation_30.y_fx >> DATALINK_FX_SHIFT) >
            DATALINK_OAM_SCREEN_HEIGHT &&
        normalized_bottom_edge < DATALINK_OAM_SCREEN_HEIGHT) {
        return 0;
    }

    return 1;
}

/*
 * 0x020AC8B4
 *
 * Complete 504-byte cell-to-OAM submission helper through 0x020ACAAB. Its
 * two-word pool begins at 0x020ACAAC and the next independent function begins
 * at 0x020ACAB4; both are excluded.
 */
int Game_SubmitDatalinkSpriteOams(
    Game_DatalinkOamEntry *oam_buffer,
    uint16_t capacity,
    const Game_DatalinkSpriteObject *sprite
)
{
    int count;
    int index;
    Game_DatalinkOamBounds bounds;

    if (sprite->active_03 == 0U) {
        return 0;
    }
    if (sprite->age_gate_04 != 0U &&
        sprite->age_05 < DATALINK_OAM_AGE_THRESHOLD) {
        return 0;
    }

    count = (int)NNS_G2dMakeCellToOams(
        oam_buffer,
        capacity,
        sprite->cell_binding_54->current_cell_30,
        NULL,
        &sprite->translation_30,
        NULL,
        NULL
    );
    for (index = 0; index < count; ++index) {
        oam_buffer[index].attribute_2 = (uint16_t)(
            (oam_buffer[index].attribute_2 &
             DATALINK_OAM_PRIORITY_CLEAR_MASK) |
            ((uint16_t)sprite->oam_priority_01 <<
             DATALINK_OAM_PRIORITY_SHIFT)
        );
    }

    bounds = Game_GetDatalinkOamBounds(oam_buffer, count);
    if (!Game_AreDatalinkOamBoundsVisible(&bounds, sprite)) {
        return 0;
    }

    if (count != 0) {
        NNS_G2dEntryOamManagerOam(
            &gGameDatalinkOamManager,
            oam_buffer,
            (uint32_t)count
        );
    }
    return count;
}
