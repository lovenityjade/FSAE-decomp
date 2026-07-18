#ifndef GAME_DATALINK_PHASE_TEN_FINALIZER_H
#define GAME_DATALINK_PHASE_TEN_FINALIZER_H

#include "game/datalink_initial_phase.h"

#include <stdint.h>

typedef struct Game_DatalinkSceneOwner {
    uint8_t unknown_000[0x53D];
    uint8_t active_player_slot_53d;
    uint8_t screen_rebuild_resource_53e;
    uint8_t unknown_53f[0x2D];
    int32_t cursor_transition_duration_56c;
    uint8_t unknown_570[4];
    int32_t cursor_x_fx_574;
    int32_t cursor_y_fx_578;
    uint8_t unknown_57c[0x570];
    int32_t menu_item_0_duration_aec;
    uint8_t unknown_af0[4];
    int32_t menu_item_0_x_fx_af4;
    int32_t menu_item_0_y_fx_af8;
    uint8_t unknown_afc[0x48];
    int32_t menu_item_1_duration_b44;
    uint8_t unknown_b48[4];
    int32_t menu_item_1_x_fx_b4c;
    int32_t menu_item_1_y_fx_b50;
    uint8_t unknown_b54[0x48];
    int32_t menu_item_2_duration_b9c;
    uint8_t unknown_ba0[4];
    int32_t menu_item_2_x_fx_ba4;
    int32_t menu_item_2_y_fx_ba8;
    uint8_t unknown_bac[0x48];
    int32_t menu_item_3_duration_bf4;
    uint8_t unknown_bf8[4];
    int32_t menu_item_3_x_fx_bfc;
    int32_t menu_item_3_y_fx_c00;
    uint8_t unknown_c04[0x23];
    uint8_t controller_ready_c27;
    uint8_t unknown_c28[0x24];
    int32_t menu_item_4_duration_c4c;
    uint8_t unknown_c50[4];
    int32_t menu_item_4_x_fx_c54;
    int32_t menu_item_4_y_fx_c58;
    uint8_t unknown_c5c[0xF8];
    int32_t extended_menu_item_0_duration_d54;
    uint8_t unknown_d58[4];
    int32_t extended_menu_item_0_x_fx_d5c;
    int32_t extended_menu_item_0_y_fx_d60;
    uint8_t unknown_d64[0x48];
    int32_t extended_menu_item_1_duration_dac;
    uint8_t unknown_db0[4];
    int32_t extended_menu_item_1_x_fx_db4;
    int32_t extended_menu_item_1_y_fx_db8;
    uint8_t unknown_dbc[0x48];
    int32_t extended_menu_item_2_duration_e04;
    uint8_t unknown_e08[4];
    int32_t extended_menu_item_2_x_fx_e0c;
    int32_t extended_menu_item_2_y_fx_e10;
    uint8_t unknown_e14[0x48];
    int32_t extended_menu_item_3_duration_e5c;
    uint8_t unknown_e60[4];
    int32_t extended_menu_item_3_x_fx_e64;
    int32_t extended_menu_item_3_y_fx_e68;
    uint8_t unknown_e6c[0x23];
    uint8_t controller_overlay_visible_e8f;
    uint8_t unknown_e90[0x24];
    int32_t controller_overlay_duration_eb4;
    uint8_t unknown_eb8[4];
    int32_t controller_overlay_x_fx_ebc;
    int32_t controller_overlay_y_fx_ec0;
    uint8_t unknown_ec4[0x23];
    uint8_t participant_action_visible_ee7;
    uint8_t unknown_ee8[0x24];
    int32_t participant_action_duration_f0c;
    uint8_t unknown_f10[4];
    int32_t participant_action_x_fx_f14;
    int32_t participant_action_y_fx_f18;
} Game_DatalinkSceneOwner;

extern Game_DatalinkSceneOwner gGameDatalinkSceneOwner;

void Game_FinalizeDatalinkPhaseTenSelection(
    Game_DatalinkFlowState *state,
    int previous_selection
);

#endif
