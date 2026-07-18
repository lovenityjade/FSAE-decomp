#ifndef GAME_DATALINK_RENDERER_PREAMBLE_H
#define GAME_DATALINK_RENDERER_PREAMBLE_H

#include <stdint.h>

enum {
    GAME_DATALINK_RENDER_DESCRIPTOR_COUNT = 10
};

typedef struct Game_DatalinkRenderDescriptor {
    uint8_t callback_argument_00;
    uint8_t tile_offset_01;
    uint16_t reserved_02;
    uint32_t callback_address_04;
} Game_DatalinkRenderDescriptor;

void Game_CopyDatalinkRenderDescriptors(
    Game_DatalinkRenderDescriptor
        descriptors[GAME_DATALINK_RENDER_DESCRIPTOR_COUNT]
);
void Game_IssueReadyDatalinkSlotCommands(void);

#endif
