#ifndef GAME_DATALINK_DESCRIPTOR_BUILD_H
#define GAME_DATALINK_DESCRIPTOR_BUILD_H

#include <stdint.h>

enum {
    GAME_DATALINK_DESCRIPTOR_COUNT = 3
};

typedef struct Game_DatalinkSlotDescriptor {
    uint32_t resource_id_00;
    uint32_t unknown_04;
    uint32_t buffer_address_08;
    int32_t buffer_row_0c;
    int32_t tile_offset_quotient_10;
    int32_t variant_index_14;
    int32_t mode_18;
    uint8_t unknown_1c[8];
    int16_t tile_offset_remainder_24;
    uint16_t type_26;
    uint8_t unknown_28[0x0C];
} Game_DatalinkSlotDescriptor;

typedef struct Game_DatalinkDescriptorRenderer {
    uint8_t unknown_00[0x38];
    uint32_t layout_value_38;
} Game_DatalinkDescriptorRenderer;

extern Game_DatalinkSlotDescriptor
    gGameDatalinkSlotDescriptors[GAME_DATALINK_DESCRIPTOR_COUNT];
extern Game_DatalinkDescriptorRenderer gGameDatalinkDescriptorRenderer;
extern uint32_t gGameDatalinkDescriptorBufferBase;
extern uint32_t gGameDatalinkLayoutValue6ac;

void Game_BuildDatalinkSlotDescriptors(void *runtime);

#endif
