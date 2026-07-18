#ifndef GAME_DATALINK_SPRITE_OAM_H
#define GAME_DATALINK_SPRITE_OAM_H

#include <stdint.h>

typedef struct Game_DatalinkOamEntry {
    uint16_t attribute_0;
    uint16_t attribute_1;
    uint16_t attribute_2;
    uint16_t affine_value;
} Game_DatalinkOamEntry;

typedef struct Game_DatalinkSpriteTranslation {
    int32_t x_fx;
    int32_t y_fx;
} Game_DatalinkSpriteTranslation;

typedef struct Game_DatalinkSpriteCellBinding {
    uint8_t unknown_00[0x30];
    const void *current_cell_30;
} Game_DatalinkSpriteCellBinding;

typedef struct Game_DatalinkSpriteObject {
    uint8_t unknown_00;
    uint8_t oam_priority_01;
    uint8_t unknown_02;
    uint8_t active_03;
    uint8_t age_gate_04;
    uint8_t age_05;
    uint8_t unknown_06[0x2A];
    Game_DatalinkSpriteTranslation translation_30;
    uint8_t unknown_38[0x18];
    uint32_t animation_index_50;
    Game_DatalinkSpriteCellBinding *cell_binding_54;
} Game_DatalinkSpriteObject;

typedef struct Game_DatalinkOamManager {
    uint8_t opaque_00;
} Game_DatalinkOamManager;

extern Game_DatalinkOamManager gGameDatalinkOamManager;

int Game_SubmitDatalinkSpriteOams(
    Game_DatalinkOamEntry *oam_buffer,
    uint16_t capacity,
    const Game_DatalinkSpriteObject *sprite
);

#endif
