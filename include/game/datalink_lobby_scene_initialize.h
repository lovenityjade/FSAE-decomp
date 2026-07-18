#ifndef GAME_DATALINK_LOBBY_SCENE_INITIALIZE_H
#define GAME_DATALINK_LOBBY_SCENE_INITIALIZE_H

#include "game/datalink_controller.h"

#include <stdint.h>

enum {
    GAME_DATALINK_LOBBY_TWEEN_COUNT = 32,
    GAME_DATALINK_LOBBY_GRAPHICS_ARENA_SIZE = 0x30f0
};

typedef struct Game_DatalinkLobbyResourceSet32 {
    uint32_t palette_file_00;
    uint32_t palette_data_04;
    uint32_t character_file_08;
    uint32_t character_data_0c;
    uint32_t screen_file_10;
    uint32_t screen_data_14;
    uint32_t cell_bank_file_18;
    uint32_t cell_bank_1c;
    uint32_t animation_bank_file_20;
    uint32_t animation_bank_24;
} Game_DatalinkLobbyResourceSet32;

typedef struct Game_DatalinkLobbyState {
    uint8_t unknown_00[0x10];
    uint8_t participant_states_10[4];
    uint8_t unknown_14[0x14];
    uint8_t phase_28;
    uint8_t unknown_29[0x0b];
    uint32_t tween_records_address_34;
    int8_t tween_count_38;
    uint8_t unknown_39[3];
    Game_DatalinkLobbyResourceSet32 local_resources_3c;
    uint8_t unknown_64[0x40];
    uint8_t wireless_state_a4;
    uint8_t unknown_a5[8];
    uint8_t fallback_enabled_ad;
    uint8_t fallback_mode_ae;
    uint8_t fallback_variant_af;
    uint32_t first_tile_buffer_address_b0;
    uint32_t second_tile_buffer_address_b4;
    uint32_t large_buffer_address_b8;
} Game_DatalinkLobbyState;

typedef struct Game_DatalinkLobbyStateSlot {
    uint32_t unknown_00;
    uint32_t state_address_04;
} Game_DatalinkLobbyStateSlot;

typedef struct Game_DatalinkLobbyOutput {
    uint8_t unknown_000[0x51c];
    uint32_t first_output_count_51c;
    uint32_t second_output_count_520;
} Game_DatalinkLobbyOutput;

typedef struct Game_DatalinkLobbySceneControl {
    uint8_t unknown_000[0xf4c];
    uint32_t field_f4c;
    uint8_t unknown_f50[0x14];
    uint32_t fallback_state_f64;
    uint8_t unknown_f68[4];
    uint32_t transition_state_f6c;
    uint32_t transition_timer_f70;
} Game_DatalinkLobbySceneControl;

typedef struct Game_DatalinkLobbyTweenDescriptor {
    int32_t sequence;
    int32_t animation_delta_fx;
} Game_DatalinkLobbyTweenDescriptor;

extern Game_DatalinkLobbyState gGameDatalinkLobbyState;
extern Game_DatalinkLobbyStateSlot gGameDatalinkLobbyStateSlot;
extern Game_DatalinkLobbyOutput *gGameDatalinkLobbyOutput;
extern Game_DatalinkLobbySceneControl gGameDatalinkLobbySceneControl;
extern uint32_t gGameDatalinkLobbyLayoutVariant;
extern uint8_t gGameDatalinkLobbyScratchBuffer[0x600];
extern uint8_t gGameDatalinkLobbyAuxiliaryState[0x7c];
extern uint8_t
    gGameDatalinkLobbyGraphicsArena[
        GAME_DATALINK_LOBBY_GRAPHICS_ARENA_SIZE
    ];
extern uint8_t gGameDatalinkLobbyRenderer[0x3c];
extern uint8_t gGameDatalinkLobbyLinkState[0x40];
extern const Game_DatalinkLobbyTweenDescriptor
    gGameDatalinkLobbyTweenDescriptors[GAME_DATALINK_LOBBY_TWEEN_COUNT];
extern const int32_t gGameDatalinkLobbyInitialX;
extern const int32_t gGameDatalinkLobbyVerticalBase;

/* 0x020b1378..0x020b18db; 68-byte literal pool follows. */
void Game_InitializeDatalinkLobbyScene_020b1378(void);

#endif
