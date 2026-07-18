#include "game/datalink_glyph_page.h"

#include <stdint.h>

typedef struct Game_DatalinkGlyphTilemapData {
    uint8_t unknown_00[8];
    uint32_t size_08;
    uint16_t cells_0c[1];
} Game_DatalinkGlyphTilemapData;

typedef struct Game_DatalinkGlyphTilemapResource {
    uint8_t unknown_00[0x14];
    uint32_t data_address_14;
} Game_DatalinkGlyphTilemapResource;

typedef struct Game_DatalinkGlyphPageRuntime {
    uint8_t unknown_000[0x600];
    uint32_t upload_resource_address_600;
    uint8_t unknown_604[0xA8];
    uint32_t tilemap_destination_address_6ac;
} Game_DatalinkGlyphPageRuntime;

extern Game_DatalinkGlyphTilemapResource
    gGameDatalinkGlyphTilemapResource;
extern Game_DatalinkGlyphPageRuntime gGameDatalinkGlyphPageRuntime;
extern const int32_t gGameDatalinkGlyphModeIndicatorPoints[4][2];
extern const uint16_t gGameDatalinkGlyphPageHeaderTiles[3];
extern volatile uint8_t gGameDatalinkGlyphPlayerIndex;

extern void Game_FillDatalinkGlyphTilemapRect(
    uint16_t *tilemap,
    int x,
    int y,
    int width,
    int height,
    uint16_t tile
);
extern void Game_TransferDatalinkGlyphTilemap(
    int engine,
    const void *source,
    uint32_t destination_offset,
    uint32_t size
);
extern void MIi_CpuCopyFast(
    const void *source,
    void *destination,
    uint32_t size
);

static Game_DatalinkGlyphTilemapData *Game_GetDatalinkGlyphTilemapData(void)
{
    return (Game_DatalinkGlyphTilemapData *)(uintptr_t)
        gGameDatalinkGlyphTilemapResource.data_address_14;
}

/*
 * 0x020AF7EC..0x020AF8BF (212 bytes).
 * Rewrites the four five-by-two mode indicators and optionally uploads the
 * exact 0x80-byte tilemap window beginning at destination offset 0x480.
 */
void Game_DrawDatalinkGlyphModeIndicators_020af7ec(
    Game_DatalinkGlyphMenuState *state,
    int upload_immediately)
{
    Game_DatalinkGlyphTilemapData *tilemap =
        Game_GetDatalinkGlyphTilemapData();
    uint32_t indicator;

    for (indicator = 0; indicator < 4U; ++indicator) {
        uint16_t tile = 2;
        int keyboard_indicator = (int)state->keyboard_mode_46 - 1;

        if (keyboard_indicator == (int)indicator ||
            state->font_page_47 == (int8_t)indicator) {
            tile = 1;
        }
        if (indicator == 2U && state->transition_counter_48 != 0) {
            tile = 1;
        }

        Game_FillDatalinkGlyphTilemapRect(
            tilemap->cells_0c,
            gGameDatalinkGlyphModeIndicatorPoints[indicator][0] / 8,
            gGameDatalinkGlyphModeIndicatorPoints[indicator][1] / 8,
            5,
            2,
            tile
        );
    }

    if (upload_immediately != 0) {
        Game_TransferDatalinkGlyphTilemap(
            1,
            (const uint8_t *)tilemap + 0x48C,
            0x480,
            0x80
        );
    }
}

/*
 * 0x020AF8C8..0x020AF96B (164 bytes).
 * Selects the four-tile page header, refreshes indicators, mirrors the
 * complete tilemap and submits the secondary upload resource.
 */
void Game_UpdateDatalinkGlyphPage_020af8c8(
    Game_DatalinkGlyphMenuState *state)
{
    Game_DatalinkGlyphTilemapData *tilemap =
        Game_GetDatalinkGlyphTilemapData();
    Game_DatalinkGlyphTilemapData *upload_resource =
        (Game_DatalinkGlyphTilemapData *)(uintptr_t)
            gGameDatalinkGlyphPageRuntime.upload_resource_address_600;
    uint16_t header_tile =
        gGameDatalinkGlyphPageHeaderTiles[gGameDatalinkGlyphPlayerIndex];

    Game_FillDatalinkGlyphTilemapRect(
        tilemap->cells_0c,
        2,
        2,
        4,
        1,
        header_tile
    );
    Game_DrawDatalinkGlyphModeIndicators_020af7ec(state, 0);
    MIi_CpuCopyFast(
        tilemap->cells_0c,
        (void *)(uintptr_t)
            gGameDatalinkGlyphPageRuntime.tilemap_destination_address_6ac,
        tilemap->size_08
    );
    Game_TransferDatalinkGlyphTilemap(
        2,
        upload_resource->cells_0c,
        0,
        upload_resource->size_08
    );
}
