#ifndef GAME_DATALINK_LOBBY_SYNC_H
#define GAME_DATALINK_LOBBY_SYNC_H

#include "game/datalink_lobby_scene_initialize.h"
#include "game/link_operation_state.h"

#include <stdbool.h>
#include <stdint.h>

enum {
    GAME_DATALINK_LOBBY_SYNC_SLOT_COUNT = 4,
    GAME_DATALINK_LOBBY_PLAYER_NAME_UNITS = 8,
    GAME_DATALINK_LOBBY_LINK_SCRATCH_SIZE = 0x20
};

typedef struct Game_DatalinkLobbySyncState {
    int8_t slot_assignments_00[4];
    uint8_t player_ready_04[4];
    uint8_t pulse_active_08[4];
    int8_t pulse_timer_0c[4];
    uint8_t participant_state_10[4];
    uint8_t participant_present_14[4];
    uint32_t participant_frame_count_18[4];
    uint8_t phase_28;
    uint8_t unknown_29;
    uint16_t transition_code_2a;
    uint16_t screen_resource_2c;
    uint16_t pending_slot_commands_2e;
    uint16_t packed_shared_state_30;
    uint8_t unknown_32[2];
    uint32_t tween_records_address_34;
    int8_t tween_count_38;
    uint8_t phase_step_39;
    int8_t local_participant_3a;
    int8_t participant_count_3b;
    uint8_t unknown_3c[0x68];
    uint8_t wireless_session_active_a4;
    uint8_t packed_flag_a5;
    uint8_t packed_flag_a6;
    uint8_t unknown_a7;
    uint8_t disconnected_a8;
    uint8_t screen_wait_counter_a9;
    uint8_t unknown_aa[2];
    uint8_t remote_variants_ac[4];
    uint8_t unknown_b0[0x0C];
} Game_DatalinkLobbySyncState;

typedef struct Game_DatalinkLobbyPublishedState {
    uint8_t unknown_00[0x2C];
    uint32_t state_word_2c;
} Game_DatalinkLobbyPublishedState;

extern uint8_t gGameDatalinkLobbyTransitionObject_02171c84[];
extern const uint8_t gGameDatalinkLobbyTransitionTransfer_0216f020[];
extern uint16_t gGameDatalinkLobbyPreviousInputs_0212c584[4];
extern uint16_t gGameDatalinkLobbyInputEdges_0212c58c[4];
extern const uint16_t gGameDatalinkLobbyDefaultInput_0212c514;
extern uint16_t
    gGameDatalinkLobbyPlayerNameCache_02181868
        [GAME_DATALINK_LOBBY_SYNC_SLOT_COUNT]
        [GAME_DATALINK_LOBBY_PLAYER_NAME_UNITS];
extern uint8_t
    gGameDatalinkLobbyLinkScratch_0212c7f0
        [GAME_DATALINK_LOBBY_LINK_SCRATCH_SIZE];

void Game_HandleDatalinkLobbyWirelessLoss_020b2190(void);
void Game_AccumulateDatalinkLobbySlotCommand_020b235c(
    int slot,
    uint32_t command_bits,
    uint32_t value
);
uint32_t Game_GetDatalinkLobbySlotCommand_020b23b8(int slot);
void Game_UnpackDatalinkLobbySharedState_020b23fc(void);
bool Game_IsDatalinkLobbySlotUnassigned_020b2560(int slot);
bool Game_AreDatalinkLobbySlotsAssigned_020b2580(void);
bool Game_AreDatalinkLobbyPlayersReady_020b25c0(void);
void Game_PublishDatalinkLobbyStateWord_020b2614(
    int local_participant,
    Game_DatalinkLobbyPublishedState *published
);
void Game_UpdateDatalinkLobbyScreenResource_020b26d0(
    int participant_count,
    int local_participant
);
void Game_UpdateDatalinkLobbyInputEdges_020b2898(void);
void Game_SynchronizeDatalinkLobbyPlayerNames_020b292c(void);
void Game_StartDatalinkLobbyLinkOperation_020b2a5c(void);

/* B2A90 is shared with the existing link-operation recovery. */
bool Game_LinkOperation_HasSucceeded_020b2a90(void);

#endif
