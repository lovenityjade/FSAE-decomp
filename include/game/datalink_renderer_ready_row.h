#ifndef GAME_DATALINK_RENDERER_READY_ROW_H
#define GAME_DATALINK_RENDERER_READY_ROW_H

#include "game/datalink_renderer_bindings.h"
#include "game/datalink_renderer_preamble.h"

#include <stdint.h>

typedef struct Game_DatalinkRenderRowMetadata {
    uint32_t resource_id_00;
    uint8_t unknown_04[0x30];
} Game_DatalinkRenderRowMetadata;

typedef struct Game_DatalinkReadyRowRecord {
    uint32_t flags_00;
    uint8_t unknown_04;
    uint8_t packed_values_05;
    uint16_t value_06;
    uint8_t unknown_08[0x10];
    uint32_t value_18;
    uint8_t unknown_1c[0x34];
} Game_DatalinkReadyRowRecord;

void Game_RenderReadyDatalinkRow(
    Game_DatalinkRenderRowMetadata metadata[3],
    const volatile Game_DatalinkReadyRowRecord *record,
    const Game_DatalinkRenderDescriptor
        descriptors[GAME_DATALINK_RENDER_DESCRIPTOR_COUNT],
    volatile Game_DatalinkRenderPrimaryBinding *buffers,
    uint32_t values[GAME_DATALINK_RENDER_DESCRIPTOR_COUNT],
    uint8_t slot
);

#endif
