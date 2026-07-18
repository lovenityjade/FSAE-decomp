#include "game/datalink_lobby_sync.h"

#include "game/datalink_post_oam.h"
#include "game/datalink_screen_update.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_LOBBY_ERROR_PHASE = 8,
    DATALINK_LOBBY_ERROR_TRANSITION = 2,
    DATALINK_LOBBY_WIRELESS_HOST_READY = 8,
    DATALINK_LOBBY_WIRELESS_CLIENT_READY = 0x0B,
    DATALINK_LOBBY_ERROR_SOUND = 0x40,
    DATALINK_LOBBY_ERROR_TRANSFER_SIZE = 0x400,
    DATALINK_LOBBY_SHARED_RECORDS_ADDRESS_OFFSET = 0x10C,
    DATALINK_LOBBY_SHARED_STATE_WORD_OFFSET = 0x474,
    DATALINK_LOBBY_SHARED_MIRROR_WORD_OFFSET = 0x488,
    DATALINK_LOBBY_PARTICIPANT_RECORD_OFFSET = 0x464,
    DATALINK_LOBBY_PARTICIPANT_RECORD_STRIDE = 0x14,
    DATALINK_LOBBY_SCREEN_SINGLE = 0x083F,
    DATALINK_LOBBY_SCREEN_HOST_DEFAULT = 0x0840,
    DATALINK_LOBBY_SCREEN_CLIENT_DEFAULT = 0x0841,
    DATALINK_LOBBY_SCREEN_SHARED = 0x084B,
    DATALINK_LOBBY_SCREEN_WAITING = 0x084C,
    DATALINK_LOBBY_SCREEN_OFFLINE = 0x084D,
    DATALINK_LOBBY_SCREEN_HOST_FULL = 0x0850,
    DATALINK_LOBBY_SCREEN_CLIENT_FULL = 0x0851,
    DATALINK_LOBBY_SCREEN_MODE_DEFAULT = 0x0852,
    DATALINK_LOBBY_SCREEN_MODE_ALTERNATE = 0x0853
};

static const uint8_t sGameDatalinkLobbySlotCommandShifts[4] = {
    0, 0, 4, 8
};

static const uint8_t sGameDatalinkLobbyAssignmentShifts[4] = {
    0, 0, 2, 4
};

static const uint8_t sGameDatalinkLobbyReadyBitShifts[4] = {
    6, 7, 8, 9
};

extern void *Game_GetDatalinkWirelessContext_020909c8(void);
extern int Game_GetDatalinkWirelessState_02090ea4(void *context);
extern void *Game_GetDatalinkLobbyGraphicsManager_0208da4c(void);
extern int Game_IsDatalinkLobbyRuntimeAvailable_0208de54(void);
extern int Game_GetDatalinkLobbyWirelessParticipantCount_020922d0(
    void *context
);
extern int Game_BeginDatalinkHostShutdown_02091738(void *context);
extern void Game_BeginDatalinkLobbyErrorTransition_0202811c(void);
extern int Game_IsDatalinkLobbyTransitionObjectActive_020a1d08(
    const void *object
);
extern void Game_ResetDatalinkLobbyTransitionObject_020a20e0(void *object);
extern void Game_TransferDatalinkLobbyErrorResource_0209da7c(
    int target,
    const void *source,
    int destination_offset,
    uint32_t size
);
extern void Game_PlayDatalinkSoundEffect(uint32_t sound_id);
extern void Game_LoadDatalinkScreenResource(uint16_t resource_id);

_Static_assert(
    sizeof(Game_DatalinkLobbySyncState) == 0xBC,
    "datalink lobby sync-state size"
);
_Static_assert(
    offsetof(Game_DatalinkLobbySyncState, phase_28) == 0x28,
    "datalink lobby phase offset"
);
_Static_assert(
    offsetof(Game_DatalinkLobbySyncState, transition_code_2a) == 0x2A,
    "datalink lobby transition-code offset"
);
_Static_assert(
    offsetof(Game_DatalinkLobbySyncState, screen_resource_2c) == 0x2C,
    "datalink lobby screen-resource offset"
);
_Static_assert(
    offsetof(Game_DatalinkLobbySyncState, pending_slot_commands_2e) == 0x2E,
    "datalink lobby pending-command offset"
);
_Static_assert(
    offsetof(Game_DatalinkLobbySyncState, packed_shared_state_30) == 0x30,
    "datalink lobby packed-state offset"
);
_Static_assert(
    offsetof(Game_DatalinkLobbySyncState, local_participant_3a) == 0x3A,
    "datalink lobby local-participant offset"
);
_Static_assert(
    offsetof(Game_DatalinkLobbySyncState, participant_count_3b) == 0x3B,
    "datalink lobby participant-count offset"
);
_Static_assert(
    offsetof(Game_DatalinkLobbySyncState, wireless_session_active_a4) == 0xA4,
    "datalink lobby wireless-session offset"
);
_Static_assert(
    offsetof(Game_DatalinkLobbySyncState, remote_variants_ac) == 0xAC,
    "datalink lobby remote-variant offset"
);
_Static_assert(
    offsetof(Game_DatalinkLobbyPublishedState, state_word_2c) == 0x2C,
    "datalink lobby published-state offset"
);

static Game_DatalinkLobbySyncState *Game_GetDatalinkLobbySyncState(void)
{
    return (Game_DatalinkLobbySyncState *)(uintptr_t)
        gGameDatalinkLobbyStateSlot.state_address_04;
}

static uint32_t Game_ReadDatalinkLobbyWord(
    const void *base,
    uint32_t offset)
{
    const uint8_t *bytes = (const uint8_t *)base + offset;

    return (uint32_t)bytes[0] |
        ((uint32_t)bytes[1] << 8) |
        ((uint32_t)bytes[2] << 16) |
        ((uint32_t)bytes[3] << 24);
}

static void Game_WriteDatalinkLobbyWord(
    void *base,
    uint32_t offset,
    uint32_t value)
{
    uint8_t *bytes = (uint8_t *)base + offset;

    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
    bytes[2] = (uint8_t)(value >> 16);
    bytes[3] = (uint8_t)(value >> 24);
}

static uint32_t Game_GetDatalinkLobbySharedRecordsAddress(void)
{
    return Game_ReadDatalinkLobbyWord(
        &gGameDatalinkPostOamHardwareState,
        DATALINK_LOBBY_SHARED_RECORDS_ADDRESS_OFFSET
    );
}

static uint32_t Game_GetDatalinkLobbySharedStateWord(void)
{
    const void *records = (const void *)(uintptr_t)
        Game_GetDatalinkLobbySharedRecordsAddress();

    return Game_ReadDatalinkLobbyWord(
        records,
        DATALINK_LOBBY_SHARED_STATE_WORD_OFFSET
    );
}

static uint8_t *Game_GetDatalinkLobbyOutputBytes(void)
{
    return (uint8_t *)(void *)gGameDatalinkPostOamOutput;
}

static void Game_EnterDatalinkLobbyErrorState(
    Game_DatalinkLobbySyncState *state,
    uint8_t hardware_mode,
    int mark_disconnected)
{
    Game_BeginDatalinkLobbyErrorTransition_0202811c();
    state->transition_code_2a = DATALINK_LOBBY_ERROR_TRANSITION;
    state->phase_28 = DATALINK_LOBBY_ERROR_PHASE;
    if (mark_disconnected != 0) {
        state->disconnected_a8 = 1;
    }
    gGameDatalinkPostOamHardwareState.display_state_d0 = hardware_mode;
    gGameDatalinkPostOamHardwareState.display_variant_d1 = 3;
    gGameDatalinkPostOamHardwareState.unknown_0d2[0] = UINT8_MAX;
    Game_CommitDatalinkScreenUpdate();

    if (Game_IsDatalinkLobbyTransitionObjectActive_020a1d08(
            gGameDatalinkLobbyTransitionObject_02171c84
        ) != 0) {
        Game_ResetDatalinkLobbyTransitionObject_020a20e0(
            gGameDatalinkLobbyTransitionObject_02171c84
        );
        Game_TransferDatalinkLobbyErrorResource_0209da7c(
            0,
            gGameDatalinkLobbyTransitionTransfer_0216f020,
            0,
            DATALINK_LOBBY_ERROR_TRANSFER_SIZE
        );
    }
    Game_PlayDatalinkSoundEffect(DATALINK_LOBBY_ERROR_SOUND);
}

/*
 * 0x020B2190..0x020B2347: complete 440-byte body. Its five-word global pool
 * at 0x020B2348 is excluded.
 */
void Game_HandleDatalinkLobbyWirelessLoss_020b2190(void)
{
    void *wireless_context = Game_GetDatalinkWirelessContext_020909c8();
    int wireless_state = Game_GetDatalinkWirelessState_02090ea4(
        wireless_context
    );
    Game_DatalinkLobbySyncState *state = Game_GetDatalinkLobbySyncState();
    int host_failure = 0;
    int client_failure = 0;

    if ((uint8_t)(state->phase_28 - 4U) < 3U &&
        state->wireless_session_active_a4 != 0U &&
        (state->participant_count_3b == 1 ||
         Game_GetDatalinkLobbyWirelessParticipantCount_020922d0(
             wireless_context
         ) <
             gGameDatalinkPostOamSession.participant_count_00)) {
        host_failure = 1;
        (void)Game_BeginDatalinkHostShutdown_02091738(wireless_context);
    }

    if (state->phase_28 != DATALINK_LOBBY_ERROR_PHASE) {
        if (state->wireless_session_active_a4 == 0U) {
            if (wireless_state != DATALINK_LOBBY_WIRELESS_CLIENT_READY) {
                client_failure = 1;
            }
        } else if (wireless_state != DATALINK_LOBBY_WIRELESS_HOST_READY) {
            host_failure = 1;
        }
    }

    if (host_failure != 0) {
        Game_EnterDatalinkLobbyErrorState(state, 1, 0);
    }
    if (client_failure != 0) {
        Game_EnterDatalinkLobbyErrorState(state, 2, 1);
    }
}

/*
 * 0x020B235C..0x020B23AF: complete 84-byte body. Its shift-table and state
 * globals at 0x020B23B0 are excluded.
 */
void Game_AccumulateDatalinkLobbySlotCommand_020b235c(
    int slot,
    uint32_t command_bits,
    uint32_t value)
{
    Game_DatalinkLobbySyncState *state = Game_GetDatalinkLobbySyncState();
    uint32_t command = command_bits | (value & 3U);

    state->pending_slot_commands_2e = (uint16_t)(
        state->pending_slot_commands_2e |
        (uint16_t)(command << sGameDatalinkLobbySlotCommandShifts[slot])
    );
}

/*
 * 0x020B23B8..0x020B23F3: complete 60-byte body. Its shift-table and shared
 * state globals at 0x020B23F4 are excluded.
 */
uint32_t Game_GetDatalinkLobbySlotCommand_020b23b8(int slot)
{
    return (Game_GetDatalinkLobbySharedStateWord() >>
            sGameDatalinkLobbySlotCommandShifts[slot]) &
        0x0FU;
}

/*
 * 0x020B23FC..0x020B254F: complete 340-byte body. Its two shift tables,
 * lobby state and output-owner globals at 0x020B2550 are excluded.
 */
void Game_UnpackDatalinkLobbySharedState_020b23fc(void)
{
    Game_DatalinkLobbySyncState *state = Game_GetDatalinkLobbySyncState();
    uint8_t *output = Game_GetDatalinkLobbyOutputBytes();
    uint32_t shared_word = Game_ReadDatalinkLobbyWord(
        output,
        DATALINK_LOBBY_SHARED_STATE_WORD_OFFSET
    );
    unsigned int slot;

    if (state->participant_count_3b > 1) {
        for (slot = 1;
             slot < (unsigned int)state->participant_count_3b;
             ++slot) {
            uint32_t participant_word = Game_ReadDatalinkLobbyWord(
                output,
                DATALINK_LOBBY_SHARED_STATE_WORD_OFFSET +
                    slot * DATALINK_LOBBY_PARTICIPANT_RECORD_STRIDE
            );

            state->remote_variants_ac[slot] =
                (uint8_t)((participant_word >> 27) & 3U);
        }
    }

    for (slot = 1; slot < GAME_DATALINK_LOBBY_SYNC_SLOT_COUNT; ++slot) {
        state->slot_assignments_00[slot] = (int8_t)(
            (shared_word >> (16 + sGameDatalinkLobbyAssignmentShifts[slot])) &
            3U
        );
    }
    for (slot = 0; slot < GAME_DATALINK_LOBBY_SYNC_SLOT_COUNT; ++slot) {
        if (((shared_word >> 16) >>
             sGameDatalinkLobbyReadyBitShifts[slot] & 1U) != 0U) {
            state->player_ready_04[slot] = 1;
        }
    }
    state->packed_flag_a5 = (uint8_t)((shared_word >> 26) & 1U);
    state->packed_flag_a6 = (uint8_t)((shared_word >> 29) & 1U);
}

/*
 * 0x020B2560..0x020B257B: complete 28-byte predicate. Its state-slot pool
 * word at 0x020B257C is excluded.
 */
bool Game_IsDatalinkLobbySlotUnassigned_020b2560(int slot)
{
    return Game_GetDatalinkLobbySyncState()->slot_assignments_00[slot] == 0;
}

/*
 * 0x020B2580..0x020B25BB: complete 60-byte predicate. Its state-slot pool
 * word at 0x020B25BC is excluded.
 */
bool Game_AreDatalinkLobbySlotsAssigned_020b2580(void)
{
    Game_DatalinkLobbySyncState *state = Game_GetDatalinkLobbySyncState();
    int assigned_count = 1;
    unsigned int slot;

    for (slot = 1; slot < GAME_DATALINK_LOBBY_SYNC_SLOT_COUNT; ++slot) {
        if (state->slot_assignments_00[slot] != 0) {
            ++assigned_count;
        }
    }
    return assigned_count == state->participant_count_3b;
}

/*
 * 0x020B25C0..0x020B260F: complete 80-byte predicate. Its state-slot pool
 * word at 0x020B2610 is excluded.
 */
bool Game_AreDatalinkLobbyPlayersReady_020b25c0(void)
{
    Game_DatalinkLobbySyncState *state = Game_GetDatalinkLobbySyncState();
    unsigned int slot;

    if (state->participant_count_3b < 2) {
        return false;
    }
    for (slot = 0;
         slot < (unsigned int)state->participant_count_3b;
         ++slot) {
        if (state->player_ready_04[slot] == 0U) {
            return false;
        }
    }
    return true;
}

static uint32_t Game_MakeDatalinkLobbyPublishedStateWord(
    const Game_DatalinkLobbySyncState *state)
{
    return state->pending_slot_commands_2e |
        ((uint32_t)state->packed_shared_state_30 << 16);
}

/*
 * 0x020B2614..0x020B26C7: complete 180-byte body. Its state and output-owner
 * pool at 0x020B26C8 is excluded.
 */
void Game_PublishDatalinkLobbyStateWord_020b2614(
    int local_participant,
    Game_DatalinkLobbyPublishedState *published)
{
    Game_DatalinkLobbySyncState *state = Game_GetDatalinkLobbySyncState();
    uint32_t state_word;

    (void)local_participant;
    if ((uint8_t)(state->phase_28 - 1U) < 2U) {
        state->packed_shared_state_30 |= 0x0800U;
    }
    state_word = Game_MakeDatalinkLobbyPublishedStateWord(state);
    (void)Game_GetDatalinkLobbyGraphicsManager_0208da4c();
    if (Game_IsDatalinkLobbyRuntimeAvailable_0208de54() == 0) {
        uint8_t *output = Game_GetDatalinkLobbyOutputBytes();

        Game_WriteDatalinkLobbyWord(
            output,
            DATALINK_LOBBY_SHARED_STATE_WORD_OFFSET,
            state_word
        );
        Game_WriteDatalinkLobbyWord(
            output,
            DATALINK_LOBBY_SHARED_MIRROR_WORD_OFFSET,
            state_word
        );
    } else {
        published->state_word_2c = state_word;
    }
    state->pending_slot_commands_2e = 0;
    state->packed_shared_state_30 = 0;
}

static void Game_SetDatalinkLobbyScreenResource(
    Game_DatalinkLobbySyncState *state,
    uint16_t resource)
{
    state->screen_resource_2c = resource;
}

static void Game_UpdateDatalinkLobbyHostScreen(
    Game_DatalinkLobbySyncState *state,
    int participant_count)
{
    switch (state->phase_28) {
    case 0:
        if (participant_count == 1) {
            Game_SetDatalinkLobbyScreenResource(
                state,
                DATALINK_LOBBY_SCREEN_SINGLE
            );
        } else if (participant_count == 4) {
            Game_SetDatalinkLobbyScreenResource(
                state,
                DATALINK_LOBBY_SCREEN_HOST_FULL
            );
        } else {
            Game_SetDatalinkLobbyScreenResource(
                state,
                DATALINK_LOBBY_SCREEN_HOST_DEFAULT
            );
        }
        break;
    case 1:
    case 2:
    case 4:
        Game_SetDatalinkLobbyScreenResource(
            state,
            DATALINK_LOBBY_SCREEN_SHARED
        );
        break;
    case 5:
        Game_SetDatalinkLobbyScreenResource(
            state,
            state->phase_step_39 == 1U ?
                DATALINK_LOBBY_SCREEN_MODE_ALTERNATE :
                DATALINK_LOBBY_SCREEN_MODE_DEFAULT
        );
        break;
    default:
        break;
    }
}

static void Game_UpdateDatalinkLobbyClientScreen(
    Game_DatalinkLobbySyncState *state,
    int participant_count,
    int local_participant)
{
    switch (state->phase_28) {
    case 0:
        Game_SetDatalinkLobbyScreenResource(
            state,
            participant_count == 4 ?
                DATALINK_LOBBY_SCREEN_CLIENT_FULL :
                DATALINK_LOBBY_SCREEN_CLIENT_DEFAULT
        );
        break;
    case 1:
    case 2:
        Game_SetDatalinkLobbyScreenResource(
            state,
            DATALINK_LOBBY_SCREEN_WAITING
        );
        state->screen_wait_counter_a9 = 0;
        break;
    case 4: {
        const void *records = (const void *)(uintptr_t)
            Game_GetDatalinkLobbySharedRecordsAddress();
        uint32_t participant_word = Game_ReadDatalinkLobbyWord(
            records,
            DATALINK_LOBBY_PARTICIPANT_RECORD_OFFSET +
                (uint32_t)local_participant *
                    DATALINK_LOBBY_PARTICIPANT_RECORD_STRIDE
        );
        uint32_t slot = (participant_word & 0x0FU) >> 2;

        if (state->slot_assignments_00[slot] != 0) {
            if (state->screen_wait_counter_a9 < 5U) {
                ++state->screen_wait_counter_a9;
            } else {
                Game_SetDatalinkLobbyScreenResource(
                    state,
                    DATALINK_LOBBY_SCREEN_SHARED
                );
            }
        } else {
            Game_SetDatalinkLobbyScreenResource(
                state,
                DATALINK_LOBBY_SCREEN_WAITING
            );
        }
        break;
    }
    case 5:
        Game_SetDatalinkLobbyScreenResource(
            state,
            state->phase_step_39 == 1U ?
                DATALINK_LOBBY_SCREEN_MODE_ALTERNATE :
                DATALINK_LOBBY_SCREEN_MODE_DEFAULT
        );
        break;
    default:
        break;
    }
}

/*
 * 0x020B26D0..0x020B286F: complete 416-byte body. Its state, nine resource
 * constants and shared-record global at 0x020B2870 are excluded.
 */
void Game_UpdateDatalinkLobbyScreenResource_020b26d0(
    int participant_count,
    int local_participant)
{
    Game_DatalinkLobbySyncState *state = Game_GetDatalinkLobbySyncState();
    uint16_t previous_resource = state->screen_resource_2c;

    if (state->phase_28 == 7U) {
        return;
    }
    (void)Game_GetDatalinkLobbyGraphicsManager_0208da4c();
    if (Game_IsDatalinkLobbyRuntimeAvailable_0208de54() == 0) {
        Game_SetDatalinkLobbyScreenResource(
            state,
            DATALINK_LOBBY_SCREEN_OFFLINE
        );
    } else if (local_participant == 0) {
        Game_UpdateDatalinkLobbyHostScreen(state, participant_count);
    } else {
        Game_UpdateDatalinkLobbyClientScreen(
            state,
            participant_count,
            local_participant
        );
    }

    if (previous_resource != state->screen_resource_2c) {
        Game_LoadDatalinkScreenResource(state->screen_resource_2c);
    }
}
