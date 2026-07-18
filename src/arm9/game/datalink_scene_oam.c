#include "game/datalink_scene_oam.h"

#include <stdint.h>

enum {
    DATALINK_SCENE_OAM_MANAGER_OFFSET = 0x2540,
    DATALINK_OAM_SHAPE_COUNT = 3,
    DATALINK_OAM_SIZE_COUNT = 4,
    DATALINK_OAM_X_MASK = 0x01FF,
    DATALINK_OAM_Y_MASK = 0x00FF,
    DATALINK_OAM_SHAPE_SHIFT = 14,
    DATALINK_OAM_SIZE_SHIFT = 14,
    DATALINK_OAM_SCREEN_WIDTH = 256,
    DATALINK_OAM_SCREEN_HEIGHT = 192
};

typedef struct Game_DatalinkSceneOamDimensions {
    uint32_t width;
    uint32_t height;
} Game_DatalinkSceneOamDimensions;

typedef struct Game_DatalinkSceneOamBounds {
    uint32_t minimum_x;
    uint32_t minimum_right_edge;
    uint32_t minimum_y;
    uint32_t maximum_bottom_edge;
} Game_DatalinkSceneOamBounds;

static const Game_DatalinkSceneOamDimensions
    sGameDatalinkSceneOamDimensions[
        DATALINK_OAM_SHAPE_COUNT * DATALINK_OAM_SIZE_COUNT
    ] = {
        {8, 8}, {16, 16}, {32, 32}, {64, 64},
        {16, 8}, {32, 8}, {32, 16}, {64, 32},
        {8, 16}, {8, 32}, {16, 32}, {32, 64}
    };

extern void NNS_G2dEntryOamManagerOam(
    Game_DatalinkOamManager *manager,
    const Game_DatalinkOamEntry *oam_entries,
    uint32_t count
);

_Static_assert(
    sizeof(Game_DatalinkOamEntry) == 8,
    "datalink scene OAM entry size"
);

static void Game_CopyDatalinkSceneOamBytes(
    void *destination,
    const void *source,
    unsigned int size)
{
    uint8_t *destination_bytes = (uint8_t *)destination;
    const uint8_t *source_bytes = (const uint8_t *)source;

    while (size != 0U) {
        *destination_bytes = *source_bytes;
        ++destination_bytes;
        ++source_bytes;
        --size;
    }
}

static Game_DatalinkOamManager *Game_GetDatalinkSceneOamManager(
    const Game_DatalinkSceneOwner *scene)
{
    Game_DatalinkOamManager *manager;

    Game_CopyDatalinkSceneOamBytes(
        &manager,
        (const uint8_t *)scene + DATALINK_SCENE_OAM_MANAGER_OFFSET,
        sizeof(manager)
    );
    return manager;
}

static uint32_t Game_GetDatalinkSceneOamDimensionIndex(
    const Game_DatalinkOamEntry *entry)
{
    uint32_t shape =
        (uint32_t)entry->attribute_0 >> DATALINK_OAM_SHAPE_SHIFT;
    uint32_t size =
        (uint32_t)entry->attribute_1 >> DATALINK_OAM_SIZE_SHIFT;

    return shape * DATALINK_OAM_SIZE_COUNT + size;
}

static uint32_t Game_GetDatalinkSceneOamX(
    const Game_DatalinkOamEntry *entry)
{
    return (uint32_t)entry->attribute_1 & DATALINK_OAM_X_MASK;
}

static uint32_t Game_GetDatalinkSceneOamY(
    const Game_DatalinkOamEntry *entry)
{
    return (uint32_t)entry->attribute_0 & DATALINK_OAM_Y_MASK;
}

static Game_DatalinkSceneOamBounds Game_GetDatalinkSceneOamBounds(
    const Game_DatalinkOamEntry *entries,
    int count)
{
    uint32_t dimension_index =
        Game_GetDatalinkSceneOamDimensionIndex(&entries[0]);
    const Game_DatalinkSceneOamDimensions *dimensions =
        &sGameDatalinkSceneOamDimensions[dimension_index];
    Game_DatalinkSceneOamBounds bounds;
    int index;

    bounds.minimum_x = Game_GetDatalinkSceneOamX(&entries[0]);
    bounds.minimum_right_edge = bounds.minimum_x + dimensions->width;
    bounds.minimum_y = Game_GetDatalinkSceneOamY(&entries[0]);
    bounds.maximum_bottom_edge = bounds.minimum_y + dimensions->height;

    for (index = 1; index < count; ++index) {
        uint32_t x = Game_GetDatalinkSceneOamX(&entries[index]);
        uint32_t y = Game_GetDatalinkSceneOamY(&entries[index]);
        uint32_t right_edge = x + dimensions->width;
        uint32_t bottom_edge = y + dimensions->height;

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

/*
 * 0x020AD9A4..0x020ADABB
 *
 * Complete 280-byte body. Its dimension-table pointer pool at 0x020ADABC
 * and the independent adapter beginning at 0x020ADAC0 are excluded.
 */
void Game_SubmitVisibleDatalinkSceneOams(
    Game_DatalinkSceneOwner *scene,
    const Game_DatalinkOamEntry *oam_entries,
    int count)
{
    Game_DatalinkSceneOamBounds bounds =
        Game_GetDatalinkSceneOamBounds(oam_entries, count);

    if ((int32_t)bounds.minimum_right_edge < 0) {
        return;
    }
    if (bounds.minimum_x >= DATALINK_OAM_SCREEN_WIDTH ||
        bounds.minimum_y >= DATALINK_OAM_SCREEN_HEIGHT) {
        return;
    }
    if ((int32_t)bounds.maximum_bottom_edge < 0 || count <= 0) {
        return;
    }

    NNS_G2dEntryOamManagerOam(
        Game_GetDatalinkSceneOamManager(scene),
        oam_entries,
        (uint32_t)(uint16_t)count
    );
}
