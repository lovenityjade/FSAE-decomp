#ifndef GAME_DATALINK_MULTIPLAYER_SESSION_FRAME_H
#define GAME_DATALINK_MULTIPLAYER_SESSION_FRAME_H

#include "game/datalink_controller.h"
#include "game/datalink_lobby_sync.h"

#include <stdint.h>

enum {
    GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT = 4,
    GAME_DATALINK_MULTIPLAYER_MAPPING_COUNT = 8,
    GAME_DATALINK_MULTIPLAYER_PUBLISHED_POSITION_COUNT = 3,
    GAME_DATALINK_MULTIPLAYER_TWEEN_COUNT = 32
};

typedef struct Game_DatalinkMultiplayerParticipantSample {
    uint32_t flags_00;
    uint8_t unknown_04[0x0c];
    uint32_t action_word_10;
} Game_DatalinkMultiplayerParticipantSample;

typedef struct Game_DatalinkMultiplayerPublishedPosition {
    int32_t x;
    int32_t y;
} Game_DatalinkMultiplayerPublishedPosition;

typedef struct Game_DatalinkMultiplayerSessionState {
    int8_t slot_assignments_00[
        GAME_DATALINK_MULTIPLAYER_MAPPING_COUNT
    ];
    uint8_t pulse_active_08[
        GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT
    ];
    int8_t pulse_timer_0c[
        GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT
    ];
    uint8_t participant_state_10[
        GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT
    ];
    uint8_t participant_present_14[
        GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT
    ];
    uint32_t participant_frame_count_18[
        GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT
    ];
    uint8_t phase_28;
    uint8_t unknown_29;
    uint16_t transition_code_2a;
    uint8_t unknown_2c[4];
    uint16_t packed_shared_state_30;
    uint8_t unknown_32[2];
    Game_DatalinkTweenRecord *tween_records_34;
    int8_t tween_count_38;
    uint8_t unknown_39;
    int8_t local_participant_3a;
    int8_t active_participant_count_3b;
    uint8_t unknown_3c[0x60];
    uint16_t pending_sound_9c;
    uint8_t unknown_9e[6];
    uint8_t wireless_session_active_a4;
    int8_t packed_field_a5;
    int8_t packed_field_a6;
    uint8_t unknown_a7[5];
    uint8_t remote_variants_ac[
        GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT
    ];
} Game_DatalinkMultiplayerSessionState;

typedef struct Game_DatalinkMultiplayerSessionOwner {
    uint8_t unknown_00[4];
    Game_DatalinkMultiplayerSessionState *state_04;
} Game_DatalinkMultiplayerSessionOwner;

typedef struct Game_DatalinkMultiplayerRoot {
    uint8_t unknown_000[0x464];
    Game_DatalinkMultiplayerParticipantSample participants_464[
        GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT
    ];
    uint8_t unknown_4b4[0x78];
    uint32_t frame_counter_52c;
} Game_DatalinkMultiplayerRoot;

typedef struct Game_DatalinkMultiplayerGraphicsState {
    uint8_t unknown_0000[0x2f28];
    uint32_t participant_active_2f28[5];
    uint8_t unknown_2f3c[0x19c];
    Game_DatalinkMultiplayerPublishedPosition positions_30d8[
        GAME_DATALINK_MULTIPLAYER_PUBLISHED_POSITION_COUNT
    ];
} Game_DatalinkMultiplayerGraphicsState;

typedef struct Game_DatalinkMultiplayerCompletionState {
    uint8_t unknown_000[0xe0];
    uint32_t completion_value_e0;
} Game_DatalinkMultiplayerCompletionState;

typedef struct Game_DatalinkMultiplayerSharedState {
    uint8_t unknown_000[0xf64];
    uint32_t published_value_f64;
} Game_DatalinkMultiplayerSharedState;

typedef struct Game_DatalinkMultiplayerFramePayload {
    uint32_t shared_value_00;
    uint8_t unknown_04;
    uint8_t participant_attributes_05[3];
    uint8_t participant_roles_08[
        GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT
    ];
    uint8_t participant_authority_0c[
        GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT
    ];
    uint8_t participant_status_10[
        GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT
    ];
    int16_t completion_value_14;
    uint16_t reserved_16;
} Game_DatalinkMultiplayerFramePayload;

/* Recovered global views; absolute-address integration is kept separately. */
extern Game_DatalinkMultiplayerRoot
    *gGameDatalinkMultiplayerRoot; /* 0x0217D348 */
extern Game_DatalinkMultiplayerSessionOwner
    gGameDatalinkMultiplayerSessionOwner; /* 0x02181860 */
extern Game_DatalinkMultiplayerGraphicsState
    gGameDatalinkMultiplayerGraphicsState; /* 0x0217A23C */
extern Game_DatalinkMultiplayerCompletionState
    gGameDatalinkMultiplayerCompletionState; /* 0x0217D23C */
extern Game_DatalinkMultiplayerSharedState
    gGameDatalinkMultiplayerSharedState; /* 0x0217C23C */
extern Game_DatalinkLobbyPublishedState
    gGameDatalinkMultiplayerPublishedState; /* 0x0212C9B8 */

/* 0x020B19D8..0x020B216F; the 32-byte pool follows at 0x020B2170. */
void Game_UpdateDatalinkMultiplayerSessionFrame_020b19d8(void);

#endif
