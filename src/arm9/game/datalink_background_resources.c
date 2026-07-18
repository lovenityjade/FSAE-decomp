#include "game/datalink_background_resources.h"

#include "game/datalink_resource_access.h"

#include <stddef.h>
#include <stdint.h>

#define DATALINK_RESOURCE_NONE UINT32_MAX
#define DATALINK_RESOURCE_ID_MASK UINT32_C(0x7FFFFFFF)

enum {
    DATALINK_RESOURCE_ALIGNMENT = 0x20,
    DATALINK_RESOURCE_RELEASE_CHARACTER = 1U,
    DATALINK_RESOURCE_RELEASE_PALETTE = 2U,
    DATALINK_RESOURCE_RELEASE_SCREEN = 4U,
    DATALINK_RESOURCE_TRANSFER_OFFSET = 0
};

/* Neighbor at 0x020AC594 loads the optional character resource. */
extern void Game_LoadDatalinkCharacterResource(
    Game_DatalinkBackgroundResources *resources,
    int16_t screen_target,
    uint32_t encoded_resource_id,
    uint32_t destination_offset,
    uint32_t release_flags
);

/* 0x0208A164 allocates from the global heap with explicit alignment. */
extern void *Game_AllocateDatalinkResource(
    uint32_t size,
    uint32_t alignment
);

/* 0x0208A188 releases a global-heap allocation. */
extern void Game_FreeDatalinkResource(void *allocation);

/* 0x02015D0C is the byte-copy path used for palette files. */
extern void Game_CopyDatalinkResourceBytes(
    const void *source,
    void *destination,
    uint32_t size
);

/* 0x02015B5C is the fast-copy path used for screen files. */
extern void Game_CopyDatalinkResourceFast(
    const void *source,
    void *destination,
    uint32_t size
);

extern int Game_GetUnpackedDatalinkPaletteData(
    const void *file,
    Game_DatalinkPaletteData **palette_data
);
extern int Game_GetUnpackedDatalinkScreenData(
    const void *file,
    Game_DatalinkScreenData **screen_data
);

/* 0x020144B8 flushes an exact data-cache range. */
extern void Game_FlushDatalinkRenderRange(
    const void *buffer,
    uint32_t size
);

/* External helper at 0x0209DB38 transfers palette data at a selected offset. */
extern void Game_TransferDatalinkPalette(
    const void *source,
    uint32_t destination_offset,
    uint32_t size
);

/* External helper at 0x0209DAA0 transfers background screen data. */
extern void Game_TransferDatalinkScreen(
    int target,
    const void *source,
    uint32_t destination_offset,
    uint32_t size
);

#if UINTPTR_MAX == UINT32_MAX
_Static_assert(
    offsetof(Game_DatalinkPaletteData, data_size_08) == 0x08,
    "datalink palette size offset"
);
_Static_assert(
    offsetof(Game_DatalinkPaletteData, data_0c) == 0x0C,
    "datalink palette data offset"
);
_Static_assert(
    offsetof(Game_DatalinkScreenData, data_0c) == 0x0C,
    "datalink screen data offset"
);
_Static_assert(
    offsetof(Game_DatalinkBackgroundResources, screen_file_10) == 0x10,
    "datalink screen-file offset"
);
_Static_assert(
    offsetof(Game_DatalinkBackgroundResources, screen_data_14) == 0x14,
    "datalink screen-data offset"
);
#endif

static uint32_t Game_GetDatalinkDecodedResourceId(
    uint32_t encoded_resource_id
)
{
    return encoded_resource_id & DATALINK_RESOURCE_ID_MASK;
}

static int Game_UsesDatalinkAlternateResourceArchive(
    uint32_t encoded_resource_id
)
{
    return (int)(
        encoded_resource_id >> 31
    );
}

static void Game_LoadDatalinkPaletteResource(
    Game_DatalinkBackgroundResources *resources,
    uint32_t encoded_resource_id,
    uint32_t release_flags
)
{
    uint32_t resource_id =
        Game_GetDatalinkDecodedResourceId(encoded_resource_id);
    int alternate_archive =
        Game_UsesDatalinkAlternateResourceArchive(encoded_resource_id);
    uint32_t resource_size = Game_GetDatalinkResourceSize(
        resource_id,
        alternate_archive
    );
    const void *source;

    resources->palette_file_00 = Game_AllocateDatalinkResource(
        resource_size,
        DATALINK_RESOURCE_ALIGNMENT
    );
    source = Game_GetDatalinkResourceAddress(
        resource_id,
        alternate_archive
    );
    Game_CopyDatalinkResourceBytes(
        source,
        resources->palette_file_00,
        resource_size
    );
    (void)Game_GetUnpackedDatalinkPaletteData(
        resources->palette_file_00,
        &resources->palette_data_04
    );
    Game_FlushDatalinkRenderRange(
        resources->palette_data_04->data_0c,
        resources->palette_data_04->data_size_08
    );
    Game_TransferDatalinkPalette(
        resources->palette_data_04->data_0c,
        DATALINK_RESOURCE_TRANSFER_OFFSET,
        resources->palette_data_04->data_size_08
    );

    if ((release_flags & DATALINK_RESOURCE_RELEASE_PALETTE) != 0U) {
        Game_FreeDatalinkResource(resources->palette_file_00);
        resources->palette_file_00 = NULL;
        resources->palette_data_04 = NULL;
    }
}

static void Game_LoadDatalinkScreenResource(
    Game_DatalinkBackgroundResources *resources,
    int16_t screen_target,
    uint32_t encoded_resource_id,
    uint32_t release_flags
)
{
    uint32_t resource_id =
        Game_GetDatalinkDecodedResourceId(encoded_resource_id);
    int alternate_archive =
        Game_UsesDatalinkAlternateResourceArchive(encoded_resource_id);
    uint32_t resource_size = Game_GetDatalinkResourceSize(
        resource_id,
        alternate_archive
    );
    const void *source;

    resources->screen_file_10 = Game_AllocateDatalinkResource(
        resource_size,
        DATALINK_RESOURCE_ALIGNMENT
    );
    source = Game_GetDatalinkResourceAddress(
        resource_id,
        alternate_archive
    );
    Game_CopyDatalinkResourceFast(
        source,
        resources->screen_file_10,
        resource_size
    );
    (void)Game_GetUnpackedDatalinkScreenData(
        resources->screen_file_10,
        &resources->screen_data_14
    );
    Game_FlushDatalinkRenderRange(
        resources->screen_data_14->data_0c,
        resources->screen_data_14->data_size_08
    );
    Game_TransferDatalinkScreen(
        screen_target,
        resources->screen_data_14->data_0c,
        DATALINK_RESOURCE_TRANSFER_OFFSET,
        resources->screen_data_14->data_size_08
    );

    if ((release_flags & DATALINK_RESOURCE_RELEASE_SCREEN) != 0U) {
        Game_FreeDatalinkResource(resources->screen_file_10);
        resources->screen_file_10 = NULL;
        resources->screen_data_14 = NULL;
    }
}

/*
 * 0x020AC428
 *
 * Complete 364-byte background-resource loader through 0x020AC593. It owns no
 * literal pool. The independent character-resource loader beginning at
 * 0x020AC594 is excluded.
 */
void Game_LoadDatalinkBackgroundResources(
    Game_DatalinkBackgroundResources *resources,
    int16_t screen_target,
    int32_t character_resource_id,
    uint32_t palette_resource_id,
    uint32_t screen_resource_id,
    uint32_t release_flags
)
{
    if (character_resource_id != -1) {
        Game_LoadDatalinkCharacterResource(
            resources,
            screen_target,
            (uint32_t)character_resource_id,
            DATALINK_RESOURCE_TRANSFER_OFFSET,
            release_flags
        );
    }

    if (palette_resource_id != DATALINK_RESOURCE_NONE) {
        Game_LoadDatalinkPaletteResource(
            resources,
            palette_resource_id,
            release_flags
        );
    }

    if (screen_resource_id != DATALINK_RESOURCE_NONE) {
        Game_LoadDatalinkScreenResource(
            resources,
            screen_target,
            screen_resource_id,
            release_flags
        );
    }
}
