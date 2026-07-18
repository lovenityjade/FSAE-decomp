#ifndef GAME_DATALINK_RUNTIME_H
#define GAME_DATALINK_RUNTIME_H

#include <stdint.h>

#define GAME_DATALINK_INTERFACE_A_ADDRESS UINT32_C(0x0217E324)
#define GAME_DATALINK_INTERFACE_B_ADDRESS UINT32_C(0x0217E434)
#define GAME_DATALINK_INTERFACE_C_ADDRESS UINT32_C(0x0217E384)

typedef struct Game_DatalinkStaticArena {
    uint32_t guard_000;
    uint32_t guard_004;
    uint32_t guard_008;
    uint32_t guard_00c;
    uint32_t guard_010;
    uint32_t guard_014;
    uint32_t guard_018;
    uint32_t guard_01c;
    uint32_t extended_menu_primary_vtable_020;
    uint8_t unknown_024[0x2C];
    uint32_t interface_a_vtable_050;
    uint8_t unknown_054[0x2C];
    uint32_t menu_interface_vtable_080;
    uint8_t unknown_084[0x2C];
    uint32_t interface_c_primary_vtable_0b0;
    uint8_t unknown_0b4[0x2C];
    uint32_t interface_c_secondary_vtable_0e0;
    uint8_t unknown_0e4[0x1C];
    uint32_t interface_d_vtable_100;
    uint8_t unknown_104[0x5C];
    uint32_t interface_b_vtable_160;
} Game_DatalinkStaticArena;

typedef struct Game_DatalinkRuntime {
    uint8_t unknown_00[0x04];
    uint32_t field_04;
    uint8_t unknown_08[0x2C];
    uint32_t interface_a_34;
    uint32_t interface_b_38;
    uint32_t interface_c_3c;
    uint8_t local_player_40;
    uint8_t state_41;
    uint8_t unknown_42;
    uint8_t state_43;
    uint8_t unknown_44[0x04];
    uint32_t pending_count_48;
} Game_DatalinkRuntime;

extern Game_DatalinkStaticArena gGameDatalinkStaticArena;
extern uint8_t gGameLocalPlayerSlot;

void Game_ConstructDatalinkRuntime(Game_DatalinkRuntime *runtime);

#endif
