#ifndef GAME_DATALINK_BACKGROUND_RESOURCES_H
#define GAME_DATALINK_BACKGROUND_RESOURCES_H

#include <stdint.h>

typedef struct Game_DatalinkPaletteData {
    uint8_t unknown_00[8];
    uint32_t data_size_08;
    void *data_0c;
} Game_DatalinkPaletteData;

typedef struct Game_DatalinkScreenData {
    uint8_t unknown_00[8];
    uint32_t data_size_08;
    uint8_t data_0c[];
} Game_DatalinkScreenData;

typedef struct Game_DatalinkBackgroundResources {
    void *palette_file_00;
    Game_DatalinkPaletteData *palette_data_04;
    void *character_file_08;
    void *character_data_0c;
    void *screen_file_10;
    Game_DatalinkScreenData *screen_data_14;
} Game_DatalinkBackgroundResources;

void Game_LoadDatalinkBackgroundResources(
    Game_DatalinkBackgroundResources *resources,
    int16_t screen_target,
    int32_t character_resource_id,
    uint32_t palette_resource_id,
    uint32_t screen_resource_id,
    uint32_t release_flags
);

#endif
