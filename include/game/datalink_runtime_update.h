#ifndef GAME_DATALINK_RUNTIME_UPDATE_H
#define GAME_DATALINK_RUNTIME_UPDATE_H

#include <stdint.h>

typedef struct Game_DatalinkRuntimeUpdateState {
    uint8_t unknown_00[0x0C];
    uint32_t interfaces_0c[8];
    int8_t current_interface_2c;
    uint8_t unknown_2d[7];
    uint32_t interface_a_34;
    uint32_t interface_b_38;
    uint32_t interface_c_3c;
    uint8_t local_player_40;
    uint8_t state_41;
    uint8_t unknown_42;
    uint8_t state_43;
    uint8_t unknown_44[4];
    uint32_t pending_count_48;
    uint32_t pending_values_4c[8];
} Game_DatalinkRuntimeUpdateState;

extern uint8_t gGameDatalinkAbortUpdate;
extern uint32_t gGameDatalinkEffectArgument10;
extern uint32_t gGameDatalinkEffectArgument14;

void Game_UpdateDatalinkRuntime(Game_DatalinkRuntimeUpdateState *state);

#endif
