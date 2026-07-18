#ifndef GAME_DATALINK_FLOW_PERSISTENCE_H
#define GAME_DATALINK_FLOW_PERSISTENCE_H

#include "game/datalink_initial_phase.h"

#include <stdint.h>

typedef struct Game_DatalinkRowResource {
    uint32_t owner_address_00;
    uint32_t data_address_04;
    uint32_t size_08;
} Game_DatalinkRowResource;

int Game_ResolveDatalinkRowResource(
    uint32_t unused_owner_address,
    Game_DatalinkRowResource *resource,
    uint32_t resource_id
);

void Game_SerializeDatalinkFlowState(
    Game_DatalinkFlowState *state,
    uint8_t **cursor
);

void Game_DeserializeDatalinkFlowState(
    Game_DatalinkFlowState *state,
    const uint8_t **cursor
);

#endif
