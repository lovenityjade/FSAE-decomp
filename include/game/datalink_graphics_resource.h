#ifndef FSAE_GAME_DATALINK_GRAPHICS_RESOURCE_H
#define FSAE_GAME_DATALINK_GRAPHICS_RESOURCE_H

#include <stdint.h>

typedef struct Game_G2dPaletteData {
    uint8_t unknown_00[8];
    uint32_t byte_size_08;
    uint32_t data_address_0c;
} Game_G2dPaletteData;

typedef struct Game_G2dScreenData {
    uint8_t unknown_00[8];
    uint32_t byte_size_08;
    uint32_t data_address_0c;
} Game_G2dScreenData;

typedef struct Game_G2dCharacterData {
    uint8_t unknown_00[0x10];
    uint32_t byte_size_10;
    uint32_t data_address_14;
} Game_G2dCharacterData;

typedef struct Game_DatalinkGraphicsResourceSet {
    void *palette_file_00;
    Game_G2dPaletteData *palette_data_04;
    void *character_file_08;
    Game_G2dCharacterData *character_data_0c;
    void *screen_file_10;
    Game_G2dScreenData *screen_data_14;
    void *cell_bank_file_18;
    void *cell_bank_1c;
    void *animation_bank_file_20;
    void *animation_bank_24;
} Game_DatalinkGraphicsResourceSet;

void Game_LoadDatalinkCharacterResource(
    Game_DatalinkGraphicsResourceSet *resources,
    int16_t target,
    uint32_t resource_id,
    uint32_t destination_offset,
    uint32_t flags);

void Game_LoadDatalinkScreenResourceIntoSet(
    Game_DatalinkGraphicsResourceSet *resources,
    int target,
    uint32_t resource_id);

void Game_LoadDatalinkObjectGraphicsResources(
    Game_DatalinkGraphicsResourceSet *resources,
    uint32_t character_resource_id,
    int character_upload_size,
    uint32_t palette_resource_id,
    int palette_upload_size);

void Game_LoadDatalinkCellAnimationResources(
    Game_DatalinkGraphicsResourceSet *resources,
    uint32_t cell_bank_resource_id,
    uint32_t animation_bank_resource_id);

#endif
