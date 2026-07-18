#include "game/datalink_multiplayer_session_frame.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

typedef struct Test_ActionCall {
    int participant;
    int channel;
    int action;
} Test_ActionCall;

static Game_DatalinkMultiplayerRoot sRoot;
static Game_DatalinkMultiplayerSessionState sState;
static Game_DatalinkTweenRecord
    sTweens[GAME_DATALINK_MULTIPLAYER_TWEEN_COUNT];
static Game_DatalinkMultiplayerParticipantSample
    sSamples[GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT];

Game_DatalinkMultiplayerRoot *gGameDatalinkMultiplayerRoot = &sRoot;
Game_DatalinkMultiplayerSessionOwner
    gGameDatalinkMultiplayerSessionOwner;
Game_DatalinkMultiplayerGraphicsState
    gGameDatalinkMultiplayerGraphicsState;
GameLinkOperationState gGameLinkOperationState_0212c9a8;
Game_DatalinkMultiplayerCompletionState
    gGameDatalinkMultiplayerCompletionState;
Game_DatalinkMultiplayerSharedState gGameDatalinkMultiplayerSharedState;
Game_DatalinkLobbyPublishedState gGameDatalinkMultiplayerPublishedState;

static int sRuntimeActive;
static int sLocalAuthority;
static uint32_t sActivationMask;
static unsigned int sSampleCalls;
static unsigned int sPrepareCalls;
static unsigned int sNoOpCalls;
static unsigned int sFinalizeSampleCalls;
static unsigned int sPredicateCalls;
static unsigned int sLocalStateUnpackCalls;
static unsigned int sPhaseCalls[9];
static unsigned int sTweenCalls;
static unsigned int sFillCalls;
static unsigned int sSubmitCalls;
static unsigned int sCountCalls;
static unsigned int sMappingCalls;
static unsigned int sFinalizeCalls;
static unsigned int sTableCalls;
static unsigned int sDisconnectCalls;
static unsigned int sSoundTargetCalls;
static unsigned int sDirectSoundCalls;
static unsigned int sEffectSoundCalls;
static int sPublishedCount;
static int sPublishedLocalParticipant;
static int sDirectSound;
static int sEffectSound;
static Test_ActionCall sActions[8];
static unsigned int sActionCount;
static Game_DatalinkMultiplayerFramePayload sPayload;

static void Test_Reset(int runtime_active, uint8_t phase)
{
    unsigned int index;

    memset(&sRoot, 0, sizeof(sRoot));
    memset(&sState, 0, sizeof(sState));
    memset(sTweens, 0, sizeof(sTweens));
    memset(sSamples, 0, sizeof(sSamples));
    memset(&gGameDatalinkMultiplayerGraphicsState, 0,
        sizeof(gGameDatalinkMultiplayerGraphicsState));
    memset(&gGameLinkOperationState_0212c9a8, 0,
        sizeof(gGameLinkOperationState_0212c9a8));
    memset(&gGameDatalinkMultiplayerCompletionState, 0,
        sizeof(gGameDatalinkMultiplayerCompletionState));
    memset(&gGameDatalinkMultiplayerSharedState, 0,
        sizeof(gGameDatalinkMultiplayerSharedState));
    memset(&sPayload, 0, sizeof(sPayload));
    memset(sPhaseCalls, 0, sizeof(sPhaseCalls));
    memset(sActions, 0, sizeof(sActions));

    gGameDatalinkMultiplayerRoot = &sRoot;
    gGameDatalinkMultiplayerSessionOwner.state_04 = &sState;
    sState.tween_records_34 = sTweens;
    sState.tween_count_38 = 2;
    sState.phase_28 = phase;
    for (index = 0;
         index < GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT;
         ++index) {
        sState.remote_variants_ac[index] = 7;
    }
    for (index = 0;
         index < GAME_DATALINK_MULTIPLAYER_PUBLISHED_POSITION_COUNT;
         ++index) {
        sTweens[28 + index].current_x_fx_30 =
            (int32_t)(100 + index) * 0x1000;
        sTweens[28 + index].current_y_fx_34 =
            -(int32_t)(40 + index) * 0x1000;
    }

    gGameDatalinkMultiplayerCompletionState.completion_value_e0 = 5;
    gGameDatalinkMultiplayerSharedState.published_value_f64 =
        UINT32_C(0x89abcdef);
    sRuntimeActive = runtime_active;
    sLocalAuthority = 0;
    sActivationMask = 0;
    sSampleCalls = 0;
    sPrepareCalls = 0;
    sNoOpCalls = 0;
    sFinalizeSampleCalls = 0;
    sPredicateCalls = 0;
    sLocalStateUnpackCalls = 0;
    sTweenCalls = 0;
    sFillCalls = 0;
    sSubmitCalls = 0;
    sCountCalls = 0;
    sMappingCalls = 0;
    sFinalizeCalls = 0;
    sTableCalls = 0;
    sDisconnectCalls = 0;
    sSoundTargetCalls = 0;
    sDirectSoundCalls = 0;
    sEffectSoundCalls = 0;
    sPublishedCount = -1;
    sPublishedLocalParticipant = -1;
    sDirectSound = -1;
    sEffectSound = -1;
    sActionCount = 0;
}

void Game_ReadDatalinkParticipantSample_020a31ac(
    int participant,
    Game_DatalinkMultiplayerParticipantSample *sample)
{
    assert(participant >= 0);
    assert(participant < GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT);
    assert(sample == &sRoot.participants_464[participant]);
    *sample = sSamples[participant];
    ++sSampleCalls;
}

void Game_ActivateDatalinkParticipant_0209cf68(int participant)
{
    sActivationMask |= UINT32_C(1) << participant;
}

void Game_PrepareDatalinkParticipantFrame_0209a8b8(void)
{
    ++sPrepareCalls;
}

void Game_LinkOperation_NoOp_020b2aac(void)
{
    ++sNoOpCalls;
}

void Game_UpdateDatalinkLobbyInputEdges_020b2898(void)
{
    ++sFinalizeSampleCalls;
}

void *Game_GetDatalinkGraphicsManager_0208da4c(void)
{
    return &gGameDatalinkMultiplayerGraphicsState;
}

int Game_IsDatalinkRuntimeAvailable_0208de54(void)
{
    return sRuntimeActive;
}

bool Game_IsDatalinkLobbySlotUnassigned_020b2560(int group)
{
    assert(group >= 0);
    assert(group < GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT);
    ++sPredicateCalls;
    return true;
}

void Game_AccumulateDatalinkLobbySlotCommand_020b235c(
    int participant,
    uint32_t channel,
    uint32_t action)
{
    Test_ActionCall *call;

    assert(sActionCount < sizeof(sActions) / sizeof(sActions[0]));
    call = &sActions[sActionCount++];
    call->participant = participant;
    call->channel = channel;
    call->action = action;
}

void Game_UnpackDatalinkLobbySharedState_020b23fc(void)
{
    ++sLocalStateUnpackCalls;
}

static void Test_RecordPhase(unsigned int phase)
{
    assert(phase < 9);
    ++sPhaseCalls[phase];
}

void Game_UpdateDatalinkMultiplayerPhaseZero_020b2b54(void)
{
    Test_RecordPhase(0);
}

void Game_UpdateDatalinkMultiplayerPhaseOne_020b32cc(void)
{
    Test_RecordPhase(1);
}

void Game_UpdateDatalinkMultiplayerPhaseTwo_020b3998(void)
{
    Test_RecordPhase(2);
}

void Game_UpdateDatalinkMultiplayerPhaseThree_020b3a84(void)
{
    Test_RecordPhase(3);
}

void Game_UpdateDatalinkMultiplayerPhaseFour_020b3b28(void)
{
    Test_RecordPhase(4);
}

void Game_UpdateDatalinkMultiplayerPhaseFive_020b3d70(void)
{
    Test_RecordPhase(5);
}

void Game_UpdateDatalinkMultiplayerPhaseSix_020b3ea0(void)
{
    Test_RecordPhase(6);
}

void Game_UpdateDatalinkMultiplayerPhaseSeven_020b3ea8(void)
{
    Test_RecordPhase(7);
}

void Game_UpdateDatalinkMultiplayerPhaseEight_020b3fcc(void)
{
    Test_RecordPhase(8);
}

void Game_UpdateDatalinkTweenRecord_020acc2c(
    Game_DatalinkTweenRecord *tween)
{
    assert(tween == &sTweens[sTweenCalls]);
    ++sTweenCalls;
}

void MI_CpuFill(void *destination, uint32_t value, uint32_t size)
{
    assert(value == 0);
    assert(size == sizeof(Game_DatalinkMultiplayerFramePayload));
    memset(destination, 0, size);
    ++sFillCalls;
}

int Game_GetDatalinkPeerRosterCellMap_020b11dc(int index)
{
    assert(index >= 0);
    assert(index < GAME_DATALINK_MULTIPLAYER_PARTICIPANT_COUNT);
    return index + 5;
}

bool Game_AreDatalinkLobbySlotsAssigned_020b2580(void)
{
    return sLocalAuthority != 0;
}

void Game_SubmitDatalinkMultiplayerFrame_020b4150(
    const Game_DatalinkMultiplayerFramePayload *payload)
{
    sPayload = *payload;
    ++sSubmitCalls;
}

void Game_UpdateDatalinkLobbyScreenResource_020b26d0(
    int participant_count,
    int local_participant)
{
    sPublishedCount = participant_count;
    sPublishedLocalParticipant = local_participant;
    ++sCountCalls;
}

void Game_SynchronizeDatalinkLobbyPlayerNames_020b292c(void)
{
    ++sMappingCalls;
}

void Game_FinalizeDatalinkMultiplayerFrame_020b4d68(void)
{
    ++sFinalizeCalls;
}

void Game_PublishDatalinkLobbyStateWord_020b2614(
    int local_participant,
    Game_DatalinkLobbyPublishedState *published)
{
    assert(local_participant == sState.local_participant_3a);
    assert(published == &gGameDatalinkMultiplayerPublishedState);
    ++sTableCalls;
}

void Game_HandleDatalinkLobbyWirelessLoss_020b2190(void)
{
    ++sDisconnectCalls;
}

void Game_PrepareDatalinkSoundTarget_0209da7c(
    int target,
    void *context,
    int argument_2,
    int argument_3)
{
    assert(target == (int)sSoundTargetCalls + 1);
    assert(context == 0);
    assert(argument_2 == 0);
    assert(argument_3 == 0);
    ++sSoundTargetCalls;
}

void Game_PlayDatalinkDirectSound_020827a8(int sound_id)
{
    sDirectSound = sound_id;
    ++sDirectSoundCalls;
}

void Game_PlayDatalinkSoundEffect_02082ee0(int sound_id)
{
    sEffectSound = sound_id;
    ++sEffectSoundCalls;
}

static void Test_AssertCommonTail(void)
{
    assert(sSampleCalls == 4);
    assert(sPrepareCalls == 1);
    assert(sNoOpCalls == 1);
    assert(sFinalizeSampleCalls == 1);
    assert(sTweenCalls == 2);
    assert(sFillCalls == 1);
    assert(sCountCalls == 1);
    assert(sMappingCalls == 1);
    assert(sFinalizeCalls == 1);
    assert(sTableCalls == 1);
    assert(gGameDatalinkMultiplayerGraphicsState.positions_30d8[0].x == 100);
    assert(gGameDatalinkMultiplayerGraphicsState.positions_30d8[0].y == -40);
    assert(gGameDatalinkMultiplayerGraphicsState.positions_30d8[1].x == 101);
    assert(gGameDatalinkMultiplayerGraphicsState.positions_30d8[1].y == -41);
    assert(gGameDatalinkMultiplayerGraphicsState.positions_30d8[2].x == 102);
    assert(gGameDatalinkMultiplayerGraphicsState.positions_30d8[2].y == -42);
}

static void Test_OfflinePhaseZeroCollectsAndPublishes(void)
{
    unsigned int index;

    Test_Reset(0, 0);
    sSamples[0].flags_00 = 1;
    sSamples[1].flags_00 = 4;
    sSamples[2].flags_00 = 9;
    sSamples[3].flags_00 = 12;
    for (index = 0; index < 4; ++index) {
        sState.participant_frame_count_18[index] = 9;
    }
    sState.pulse_active_08[1] = 1;
    sState.pulse_timer_0c[1] = 24;
    sState.pulse_active_08[2] = 1;
    sState.pulse_timer_0c[2] = 6;

    Game_UpdateDatalinkMultiplayerSessionFrame_020b19d8();

    Test_AssertCommonTail();
    assert(sRoot.frame_counter_52c == 1);
    assert(sPublishedCount == 2);
    assert(sPublishedLocalParticipant == 0);
    assert(sActivationMask == UINT32_C(0x5));
    assert(sState.participant_frame_count_18[0] == 10);
    assert(sState.participant_frame_count_18[1] == 0);
    assert(sState.participant_frame_count_18[2] == 10);
    assert(sState.participant_frame_count_18[3] == 0);
    assert(sState.remote_variants_ac[1] == 2);
    assert(sState.remote_variants_ac[3] == 2);
    assert(sState.pulse_active_08[1] == 0);
    assert(sState.pulse_timer_0c[1] == 0);
    assert(sState.pulse_active_08[2] == 1);
    assert(sState.pulse_timer_0c[2] == 7);
    assert(sPhaseCalls[0] == 1);
    assert(gGameLinkOperationState_0212c9a8.lobby_session_visible_0e == 1);
    assert(sSubmitCalls == 1);
    assert(sPayload.shared_value_00 == UINT32_C(0x89abcdef));
    assert(sPayload.participant_attributes_05[0] == 2);
    assert(sPayload.participant_attributes_05[1] == 7);
    assert(sPayload.participant_attributes_05[2] == 2);
    assert(sPayload.participant_roles_08[0] == 0);
    assert(sPayload.participant_roles_08[1] == 1);
    assert(sPayload.participant_roles_08[2] == 2);
    assert(sPayload.participant_roles_08[3] == 3);
    assert(sPayload.participant_status_10[0] == 3);
    assert(sPayload.participant_status_10[1] == 3);
    assert(sPayload.participant_status_10[2] == 9);
    assert(sPayload.participant_status_10[3] == 3);
    assert(sPayload.completion_value_14 == -5);
    assert(sState.participant_present_14[0] == 1);
    assert(sState.participant_present_14[1] == 0);
    assert(sState.participant_present_14[2] == 1);
    assert(sState.participant_present_14[3] == 0);
    assert(sDisconnectCalls == 0);
}

static void Test_OnlineActionsPackMappingsAndAuthority(void)
{
    Test_Reset(1, 2);
    sLocalAuthority = 1;
    sSamples[0].flags_00 = 1;
    sSamples[1].flags_00 = 5;
    sSamples[1].action_word_10 =
        (UINT32_C(2) << 27) | (UINT32_C(1) << 12) | 1U;
    sSamples[2].flags_00 = 9;
    sSamples[2].action_word_10 =
        (UINT32_C(1) << 27) | (UINT32_C(2) << 12) | 2U;
    sSamples[3].flags_00 = 13;
    sSamples[3].action_word_10 =
        (UINT32_C(3) << 27) | (UINT32_C(3) << 12) | 3U;
    sState.slot_assignments_00[3] = 3;
    sState.slot_assignments_00[4] = 1;
    sState.packed_field_a5 = 1;
    sState.packed_field_a6 = 1;

    Game_UpdateDatalinkMultiplayerSessionFrame_020b19d8();

    Test_AssertCommonTail();
    assert(sPublishedCount == 4);
    assert(sPublishedLocalParticipant == 0);
    assert(sPredicateCalls == 1);
    assert(sActionCount == 2);
    assert(sActions[0].participant == 1);
    assert(sActions[0].channel == 4);
    assert(sActions[0].action == 1);
    assert(sActions[1].participant == 2);
    assert(sActions[1].channel == 4);
    assert(sActions[1].action == 1);
    assert(sState.slot_assignments_00[1] == 1);
    assert(sState.slot_assignments_00[2] == 0);
    assert(sState.slot_assignments_00[7] == 1);
    assert(sState.packed_shared_state_30 == UINT16_C(0x2671));
    assert(sPhaseCalls[2] == 1);
    assert(sSubmitCalls == 1);
    assert(sPayload.participant_attributes_05[0] == 2);
    assert(sPayload.participant_attributes_05[1] == 1);
    assert(sPayload.participant_attributes_05[2] == 3);
    assert(sPayload.participant_roles_08[0] == 0);
    assert(sPayload.participant_roles_08[1] == 1);
    assert(sPayload.participant_roles_08[2] == 2);
    assert(sPayload.participant_roles_08[3] == 3);
    assert(sPayload.participant_status_10[0] == 5);
    assert(sPayload.participant_status_10[1] == 6);
    assert(sPayload.participant_status_10[2] == 3);
    assert(sPayload.participant_status_10[3] == 8);
    assert(sPayload.participant_authority_0c[0] == 1);
    assert(sPayload.participant_authority_0c[1] == 1);
    assert(sPayload.participant_authority_0c[2] == 0);
    assert(sPayload.participant_authority_0c[3] == 1);
    assert(sLocalStateUnpackCalls == 0);
    assert(sDisconnectCalls == 1);
}

static void Test_PhaseSevenHandlesLocalParticipantAndDirectSound(void)
{
    unsigned int index;

    Test_Reset(1, 7);
    for (index = 0; index < 4; ++index) {
        sSamples[index].flags_00 = 1U | (index << 2);
    }
    sSamples[2].flags_00 |= 2U;
    sState.pending_sound_9c = UINT16_C(0x8123);

    Game_UpdateDatalinkMultiplayerSessionFrame_020b19d8();

    Test_AssertCommonTail();
    assert(sPublishedLocalParticipant == 2);
    assert(sLocalStateUnpackCalls == 1);
    assert(sActionCount == 0);
    assert(sPhaseCalls[7] == 1);
    assert(gGameLinkOperationState_0212c9a8.lobby_session_visible_0e == 0);
    assert(sSubmitCalls == 1);
    assert(sDisconnectCalls == 0);
    assert(sSoundTargetCalls == 2);
    assert(sDirectSoundCalls == 1);
    assert(sDirectSound == 0x123);
    assert(sEffectSoundCalls == 0);
}

static void Test_PhaseEightSuppressesPayloadAndMasksEffectSound(void)
{
    unsigned int index;

    Test_Reset(0, 8);
    for (index = 0; index < 4; ++index) {
        sSamples[index].flags_00 = 1U | (index << 2);
    }
    sState.pending_sound_9c = UINT16_C(0x0456);

    Game_UpdateDatalinkMultiplayerSessionFrame_020b19d8();

    Test_AssertCommonTail();
    assert(sPhaseCalls[8] == 1);
    assert(sSubmitCalls == 0);
    assert(sSoundTargetCalls == 2);
    assert(sDirectSoundCalls == 0);
    assert(sEffectSoundCalls == 1);
    assert(sEffectSound == 0x56);
}

int main(void)
{
    Test_OfflinePhaseZeroCollectsAndPublishes();
    Test_OnlineActionsPackMappingsAndAuthority();
    Test_PhaseSevenHandlesLocalParticipantAndDirectSound();
    Test_PhaseEightSuppressesPayloadAndMasksEffectSound();
    return 0;
}
