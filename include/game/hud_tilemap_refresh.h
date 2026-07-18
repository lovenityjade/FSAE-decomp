#ifndef GAME_HUD_TILEMAP_REFRESH_H
#define GAME_HUD_TILEMAP_REFRESH_H

#include <stdint.h>

typedef struct Game_HudRefreshValues {
    uint8_t unknown_00[8];
    int total_seconds;
    int top_meter_step;
    int middle_meter_step;
    int bottom_meter_step;
    int right_meter_step;
} Game_HudRefreshValues;

typedef struct Game_HudTilemapResource {
    uint8_t unknown_00[8];
    uint32_t byte_size;
    uint16_t tilemap[];
} Game_HudTilemapResource;

typedef struct Game_HudTilemapRefreshContext {
    Game_HudRefreshValues *values;
    uint16_t **active_tilemap;
    Game_HudTilemapResource *resource;
    int dma_channel;
} Game_HudTilemapRefreshContext;

void Game_RefreshHudTilemap(const Game_HudTilemapRefreshContext *context);

#endif
