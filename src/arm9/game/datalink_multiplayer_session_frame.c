#include "game/datalink_multiplayer_session_frame.h"

#include "game/datalink_lobby_sync.h"
#include "game/datalink_peer_roster.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_MULTIPLAYER_SAMPLE_ACTIVE = 1,
    DATALINK_MULTIPLAYER_SAMPLE_LOCAL = 2,
    DATALINK_MULTIPLAYER_SAMPLE_GROUP_MASK = 0x0c,
    DATALINK_MULTIPLAYER_SAMPLE_GROUP_SHIFT = 2,
    DATALINK_MULTIPLAYER_ACTION_GROUP_MASK = 0x0f,
    DATALINK_MULTIPLAYER_ACTION_SHIFT = 12,
    DATALINK_MULTIPLAYER_ACTION_MASK = 0x0f,
    DATALINK_MULTIPLAYER_ATTRIBUTE_SHIFT = 27,
    DATALINK_MULTIPLAYER_ATTRIBUTE_MASK = 3,
    DATALINK_MULTIPLAYER_TRANSIENT_LIMIT = 24,
    DATALINK_MULTIPLAYER_TRANSIENT_PERIOD = 12,
    DATALINK_MULTIPLAYER_TRANSIENT_VISIBLE_END = 6,
    DATALINK_MULTIPLAYER_REMOTE_STATUS = 3,
    DATALINK_MULTIPLAYER_TRANSIENT_STATUS = 9,
    DATALINK_MULTIPLAYER_UNMAPPED_ROLE = 4,
    DATALINK_MULTIPLAYER_INACTIVE_ATTRIBUTE = 2,
    DATALINK_MULTIPLAYER_POSITION_TWEEN_BASE = 28,
    DATALINK_MULTIPLAYER_FX_SHIFT = 12,
    DATALINK_MULTIPLAYER_PENDING_DIRECT_SOUND = 0x8000,
    DATALINK_MULTIPLAYER_PENDING_DIRECT_SOUND_MASK = 0x7fff,
    DATALINK_MULTIPLAYER_PENDING_EFFECT_MASK = 0xff
};

extern void Game_ReadDatalinkParticipantSample_020a31ac(
    int participant,
    Game_DatalinkMultiplayerParticipantSample *sample
);
extern void Game_ActivateDatalinkParticipant_0209cf68(int participant);
extern void Game_PrepareDatalinkParticipantFrame_0209a8b8(void);
extern void *Game_GetDatalinkGraphicsManager_0208da4c(void);
extern int Game_IsDatalinkRuntimeAvailable_0208de54(void);
extern void Game_UpdateDatalinkMultiplayerPhaseZero_020b2b54(void);
extern void Game_UpdateDatalinkMultiplayerPhaseOne_020b32cc(void);
extern void Game_UpdateDatalinkMultiplayerPhaseTwo_020b3998(void);
extern void Game_UpdateDatalinkMultiplayerPhaseThree_020b3a84(void);
extern void Game_UpdateDatalinkMultiplayerPhaseFour_020b3b28(void);
extern void Game_UpdateDatalinkMultiplayerPhaseFive_020b3d70(void);
extern void Game_UpdateDatalinkMultiplayerPhaseSix_020b3ea0(void);
extern void Game_UpdateDatalinkMultiplayerPhaseSeven_020b3ea8(void);
extern void Game_UpdateDatalinkMultiplayerPhaseEight_020b3fcc(void);
extern void MI_CpuFill(void *destination, uint32_t value, uint32_t size);
extern void Game_SubmitDatalinkMultiplayerFrame_020b4150(
    const Game_DatalinkMultiplayerFramePayload *payload
);
extern void Game_FinalizeDatalinkMultiplayerFrame_020b4d68(void);
extern void Game_PrepareDatalinkSoundTarget_0209da7c(
    int target,
    void *context,
    int argument_2,
    int argument_3
);
extern void Game_PlayDatalinkDirectSound_020827a8(int sound_id);
extern void Game_PlayDatalinkSoundEffect_02082ee0(int sound_id);

#if UINTPTR_MAX == UINT32_MAX
_Static_assert(
    sizeof(Game_DatalinkMultiplayerParticipantSample) == 0x14,
    "multiplayer participant sample size"
);
_Static_assert(
    offsetof(Game_DatalinkMultiplayerSessionState, phase_28) == 0x28,
    "multiplayer phase offset"
);
_Static_assert(
    offsetof(Game_DatalinkMultiplayerSessionState, tween_records_34) == 0x34,
    "multiplayer tween pointer offset"
);
_Static_assert(
    offsetof(Game_DatalinkMultiplayerSessionState, pending_sound_9c) == 0x9c,
    "multiplayer pending sound offset"
);
_Static_assert(
    offsetof(Game_DatalinkMultiplayerSessionState,
             remote_variants_ac) == 0xac,
    "multiplayer remote-variant offset"
);
_Static_assert(
    sizeof(Game_DatalinkMultiplayerSessionState) == 0xb0,
    "multiplayer session target size"
);
_Static_assert(
    offsetof(Game_DatalinkMultiplayerRoot, participants_464) == 0x464,
    "multiplayer participant array offset"
);
_Static_assert(
    offsetof(Game_DatalinkMultiplayerRoot, frame_counter_52c) == 0x52c,
    "multiplayer frame counter offset"
);
_Static_assert(
    offsetof(Game_DatalinkMultiplayerGraphicsState,
             participant_active_2f28) == 0x2f28,
    "multiplayer active array offset"
);
_Static_assert(
    offsetof(Game_DatalinkMultiplayerGraphicsState, positions_30d8) == 0x30d8,
    "multiplayer published position offset"
);
#endif

_Static_assert(
    sizeof(Game_DatalinkMultiplayerFramePayload) == 0x18,
    "multiplayer frame payload size"
);
_Static_assert(
    offsetof(Game_DatalinkMultiplayerFramePayload,
             participant_attributes_05) == 0x05,
    "multiplayer payload attributes offset"
);
_Static_assert(
    offsetof(Game_DatalinkMultiplayerFramePayload,
             participant_roles_08) == 0x08,
    "multiplayer payload roles offset"
);
_Static_assert(
    offsetof(Game_DatalinkMultiplayerFramePayload,
             participant_authority_0c) == 0x0c,
    "multiplayer payload authority offset"
);
_Static_assert(
    offsetof(Game_DatalinkMultiplayerFramePayload,
             participant_status_10) == 0x10,
    "multiplayer payload status offset"
);
_Static_assert(
    offsetof(Game_DatalinkMultiplayerFramePayload, completion_value_14) == 0x14,
    "multiplayer payload completion offset"
);

static int Game_IsDatalinkMultiplayerRuntimeActive(void)
{
    (void)Game_GetDatalinkGraphicsManager_0208da4c();
    return Game_IsDatalinkRuntimeAvailable_0208de54();
}

static uint32_t Game_GetDatalinkParticipantGroup(
    const Game_DatalinkMultiplayerParticipantSample *sample
)
{
    return (sample->flags_00 & DATALINK_MULTIPLAYER_SAMPLE_GROUP_MASK) >>
        DATALINK_MULTIPLAYER_SAMPLE_GROUP_SHIFT;
}

static void Game_CollectDatalinkMultiplayerParticipants(
    Game_DatalinkMultiplayerRoot *root,
    Game_DatalinkMultiplayerSessionState *state
)
{
    int participant;

    state->local_participant_3a = 0;
    state->active_participant_count_3b = 0;
    for (participant = 0;
         participant < GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT;
         ++participant) {
        Game_DatalinkMultiplayerParticipantSample *sample =
            &root->participants_464[participant];
        uint32_t active;

        Game_ReadDatalinkParticipantSample_020a31ac(participant, sample);
        if ((sample->flags_00 & DATALINK_MULTIPLAYER_SAMPLE_LOCAL) != 0U) {
            state->local_participant_3a = (int8_t)participant;
        }

        active = sample->flags_00 & DATALINK_MULTIPLAYER_SAMPLE_ACTIVE;
        if (active != 0U) {
            state->active_participant_count_3b = (int8_t)(
                state->active_participant_count_3b + 1
            );
            if (gGameDatalinkMultiplayerGraphicsState
                    .participant_active_2f28[participant] == 0U) {
                Game_ActivateDatalinkParticipant_0209cf68(participant);
            }
            gGameDatalinkMultiplayerGraphicsState
                .participant_active_2f28[participant] = active;
            ++state->participant_frame_count_18[participant];
        } else {
            state->participant_frame_count_18[participant] = 0;
            if (gGameDatalinkMultiplayerGraphicsState
                    .participant_active_2f28[participant + 1] == 0U) {
                state->remote_variants_ac[participant] =
                    DATALINK_MULTIPLAYER_INACTIVE_ATTRIBUTE;
            }
            gGameDatalinkMultiplayerGraphicsState
                .participant_active_2f28[participant] = active;
        }
    }
}

static void Game_ApplyDatalinkMultiplayerParticipantActions(
    const Game_DatalinkMultiplayerRoot *root,
    Game_DatalinkMultiplayerSessionState *state
)
{
    int participant;

    if (Game_IsDatalinkMultiplayerRuntimeActive() == 0) {
        return;
    }
    if (state->local_participant_3a != 0) {
        Game_UnpackDatalinkLobbySharedState_020b23fc();
        return;
    }

    for (participant = 1;
         participant < GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT;
         ++participant) {
        const Game_DatalinkMultiplayerParticipantSample *sample =
            &root->participants_464[participant];
        uint32_t group =
            sample->action_word_10 & DATALINK_MULTIPLAYER_ACTION_GROUP_MASK;
        uint32_t action =
            (sample->action_word_10 >> DATALINK_MULTIPLAYER_ACTION_SHIFT) &
            DATALINK_MULTIPLAYER_ACTION_MASK;

        if ((sample->flags_00 & DATALINK_MULTIPLAYER_SAMPLE_ACTIVE) != 0U) {
            state->remote_variants_ac[participant] = (uint8_t)(
                (sample->action_word_10 >>
                 DATALINK_MULTIPLAYER_ATTRIBUTE_SHIFT) &
                DATALINK_MULTIPLAYER_ATTRIBUTE_MASK
            );
        } else {
            state->remote_variants_ac[participant] =
                DATALINK_MULTIPLAYER_INACTIVE_ATTRIBUTE;
        }

        if (action == 1U) {
            if (!Game_IsDatalinkLobbySlotUnassigned_020b2560((int)group)) {
                Game_AccumulateDatalinkLobbySlotCommand_020b235c(
                    participant, 4, 2
                );
            } else {
                Game_AccumulateDatalinkLobbySlotCommand_020b235c(
                    participant, 4, 1
                );
                state->slot_assignments_00[group] = (int8_t)participant;
            }
        } else if (action == 2U) {
            Game_AccumulateDatalinkLobbySlotCommand_020b235c(
                participant, 4, 1
            );
            state->slot_assignments_00[group] = 0;
        } else if (action == 3U) {
            state->slot_assignments_00[group + 4U] = 1;
        }
    }

    state->packed_shared_state_30 = (uint16_t)(
        (state->packed_shared_state_30 & UINT16_C(0xe000)) |
        (uint16_t)state->slot_assignments_00[1] |
        ((uint16_t)state->slot_assignments_00[2] << 2) |
        ((uint16_t)state->slot_assignments_00[3] << 4) |
        ((uint16_t)state->slot_assignments_00[4] << 6) |
        ((uint16_t)state->slot_assignments_00[5] << 7) |
        ((uint16_t)state->slot_assignments_00[6] << 8) |
        ((uint16_t)state->slot_assignments_00[7] << 9) |
        ((uint16_t)state->packed_field_a5 << 10) |
        ((uint16_t)state->packed_field_a6 << 13)
    );
}

static void Game_DispatchDatalinkMultiplayerPhase(uint8_t phase)
{
    switch (phase) {
    case 0:
        Game_UpdateDatalinkMultiplayerPhaseZero_020b2b54();
        break;
    case 1:
        Game_UpdateDatalinkMultiplayerPhaseOne_020b32cc();
        break;
    case 2:
        Game_UpdateDatalinkMultiplayerPhaseTwo_020b3998();
        break;
    case 3:
        Game_UpdateDatalinkMultiplayerPhaseThree_020b3a84();
        break;
    case 4:
        Game_UpdateDatalinkMultiplayerPhaseFour_020b3b28();
        break;
    case 5:
        Game_UpdateDatalinkMultiplayerPhaseFive_020b3d70();
        break;
    case 6:
        Game_UpdateDatalinkMultiplayerPhaseSix_020b3ea0();
        break;
    case 7:
        Game_UpdateDatalinkMultiplayerPhaseSeven_020b3ea8();
        break;
    case 8:
        Game_UpdateDatalinkMultiplayerPhaseEight_020b3fcc();
        break;
    default:
        break;
    }
}

static void Game_UpdateDatalinkMultiplayerTweens(
    Game_DatalinkMultiplayerSessionState *state
)
{
    uint32_t index;

    for (index = 0;
         index < (uint32_t)(int32_t)state->tween_count_38;
         ++index) {
        Game_UpdateDatalinkTweenRecord_020acc2c(
            &state->tween_records_34[index]
        );
    }

    for (index = 0;
         index < GAME_DATALINK_MULTIPLAYER_PUBLISHED_POSITION_COUNT;
         ++index) {
        const Game_DatalinkTweenRecord *tween =
            &state->tween_records_34[
                DATALINK_MULTIPLAYER_POSITION_TWEEN_BASE + index
            ];

        gGameDatalinkMultiplayerGraphicsState.positions_30d8[index].x =
            tween->current_x_fx_30 >> DATALINK_MULTIPLAYER_FX_SHIFT;
        gGameDatalinkMultiplayerGraphicsState.positions_30d8[index].y =
            tween->current_y_fx_34 >> DATALINK_MULTIPLAYER_FX_SHIFT;
    }
}

static void Game_UpdateDatalinkMultiplayerTransientTimers(
    Game_DatalinkMultiplayerSessionState *state
)
{
    uint32_t participant;

    for (participant = 0;
         participant < GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT;
         ++participant) {
        if (state->pulse_active_08[participant] != 0U) {
            state->pulse_timer_0c[participant] = (int8_t)(
                state->pulse_timer_0c[participant] + 1
            );
            if (state->pulse_timer_0c[participant] >
                DATALINK_MULTIPLAYER_TRANSIENT_LIMIT) {
                state->pulse_active_08[participant] = 0;
                state->pulse_timer_0c[participant] = 0;
            }
        }
    }
}

static void Game_BuildDatalinkMultiplayerFramePayload(
    const Game_DatalinkMultiplayerRoot *root,
    Game_DatalinkMultiplayerSessionState *state,
    Game_DatalinkMultiplayerFramePayload *payload
)
{
    uint32_t participant;

    MI_CpuFill(payload, 0, sizeof(*payload));
    payload->participant_attributes_05[0] =
        state->remote_variants_ac[1];
    payload->participant_attributes_05[1] =
        state->remote_variants_ac[2];
    payload->participant_attributes_05[2] =
        state->remote_variants_ac[3];
    payload->participant_status_10[0] = 5;
    payload->completion_value_14 = (int16_t)(
        -(int16_t)gGameDatalinkMultiplayerCompletionState
            .completion_value_e0
    );

    for (participant = 0;
         participant < GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT;
         ++participant) {
        const Game_DatalinkMultiplayerParticipantSample *sample =
            &root->participants_464[participant];
        uint32_t group = Game_GetDatalinkParticipantGroup(sample);

        state->participant_state_10[participant] = (uint8_t)
            Game_GetDatalinkPeerRosterCellMap_020b11dc((int)group);
        if (state->phase_28 == 0U) {
            payload->participant_status_10[participant] =
                DATALINK_MULTIPLAYER_REMOTE_STATUS;
        } else if (Game_IsDatalinkMultiplayerRuntimeActive() == 0 ||
                   (sample->flags_00 &
                    DATALINK_MULTIPLAYER_SAMPLE_LOCAL) != 0U ||
                   participant ==
                       (uint32_t)(int32_t)
                           state->slot_assignments_00[group]) {
            payload->participant_status_10[participant] =
                state->participant_state_10[participant];
        } else {
            payload->participant_status_10[participant] =
                DATALINK_MULTIPLAYER_REMOTE_STATUS;
        }

        if (state->pulse_active_08[participant] != 0U &&
            state->pulse_timer_0c[participant] %
                    DATALINK_MULTIPLAYER_TRANSIENT_PERIOD >
                DATALINK_MULTIPLAYER_TRANSIENT_VISIBLE_END) {
            payload->participant_status_10[participant] =
                DATALINK_MULTIPLAYER_TRANSIENT_STATUS;
        }
    }

    if (Game_IsDatalinkMultiplayerRuntimeActive() != 0 &&
        Game_AreDatalinkLobbySlotsAssigned_020b2580()) {
        payload->participant_authority_0c[0] = 1;
    }

    for (participant = 0;
         participant < GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT;
         ++participant) {
        const Game_DatalinkMultiplayerParticipantSample *sample =
            &root->participants_464[participant];
        uint32_t group = Game_GetDatalinkParticipantGroup(sample);
        int mapping = state->slot_assignments_00[group];

        if (mapping == 0 || participant == (uint32_t)mapping) {
            payload->participant_roles_08[participant] =
                (uint8_t)group;
        } else {
            payload->participant_roles_08[participant] =
                DATALINK_MULTIPLAYER_UNMAPPED_ROLE;
        }

        if (mapping != 0 && participant == (uint32_t)mapping) {
            payload->participant_authority_0c[participant] = 1;
            if (Game_IsDatalinkMultiplayerRuntimeActive() == 0) {
                payload->participant_authority_0c[0] = 1;
            }
        } else {
            (void)Game_IsDatalinkMultiplayerRuntimeActive();
        }
    }

    payload->shared_value_00 =
        gGameDatalinkMultiplayerSharedState.published_value_f64;
}

static void Game_PlayDatalinkMultiplayerPendingSound(
    const Game_DatalinkMultiplayerSessionState *state
)
{
    uint16_t sound = state->pending_sound_9c;

    if (sound == 0U) {
        return;
    }
    Game_PrepareDatalinkSoundTarget_0209da7c(1, 0, 0, 0);
    Game_PrepareDatalinkSoundTarget_0209da7c(2, 0, 0, 0);
    if ((sound & DATALINK_MULTIPLAYER_PENDING_DIRECT_SOUND) != 0U) {
        Game_PlayDatalinkDirectSound_020827a8(
            sound & DATALINK_MULTIPLAYER_PENDING_DIRECT_SOUND_MASK
        );
    } else {
        Game_PlayDatalinkSoundEffect_02082ee0(
            sound & DATALINK_MULTIPLAYER_PENDING_EFFECT_MASK
        );
    }
}

/*
 * 0x020B19D8..0x020B216F (1944 bytes).
 * Samples all four peers, dispatches the nine lobby phases and publishes the
 * normalized 24-byte multiplayer frame. The pool at 0x020B2170 is excluded.
 */
void Game_UpdateDatalinkMultiplayerSessionFrame_020b19d8(void)
{
    Game_DatalinkMultiplayerRoot *root = gGameDatalinkMultiplayerRoot;
    Game_DatalinkMultiplayerSessionState *state =
        gGameDatalinkMultiplayerSessionOwner.state_04;
    Game_DatalinkMultiplayerFramePayload payload;
    int participant;

    ++root->frame_counter_52c;
    Game_CollectDatalinkMultiplayerParticipants(root, state);
    Game_PrepareDatalinkParticipantFrame_0209a8b8();
    Game_LinkOperation_NoOp_020b2aac();
    Game_UpdateDatalinkLobbyInputEdges_020b2898();
    Game_ApplyDatalinkMultiplayerParticipantActions(root, state);

    gGameLinkOperationState_0212c9a8.lobby_session_visible_0e =
        (uint8_t)(state->phase_28 == 7U ? 0U : 1U);
    Game_DispatchDatalinkMultiplayerPhase(state->phase_28);
    Game_UpdateDatalinkMultiplayerTweens(state);
    Game_UpdateDatalinkMultiplayerTransientTimers(state);
    Game_BuildDatalinkMultiplayerFramePayload(root, state, &payload);

    if (state->phase_28 != 8U) {
        Game_SubmitDatalinkMultiplayerFrame_020b4150(&payload);
    }
    for (participant = 0;
         participant < GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT;
         ++participant) {
        state->participant_present_14[participant] = (uint8_t)(
            root->participants_464[participant].flags_00 &
            DATALINK_MULTIPLAYER_SAMPLE_ACTIVE
        );
    }

    Game_UpdateDatalinkLobbyScreenResource_020b26d0(
        state->active_participant_count_3b,
        state->local_participant_3a
    );
    Game_SynchronizeDatalinkLobbyPlayerNames_020b292c();
    Game_FinalizeDatalinkMultiplayerFrame_020b4d68();
    Game_PublishDatalinkLobbyStateWord_020b2614(
        state->local_participant_3a,
        &gGameDatalinkMultiplayerPublishedState
    );

    if (Game_IsDatalinkMultiplayerRuntimeActive() != 0 &&
        state->phase_28 != 7U) {
        Game_HandleDatalinkLobbyWirelessLoss_020b2190();
    }
    Game_PlayDatalinkMultiplayerPendingSound(state);
}
