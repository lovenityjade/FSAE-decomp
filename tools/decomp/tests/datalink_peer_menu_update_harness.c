#include "game/datalink_peer_menu_update.h"

#include "game/datalink_phase_thirty.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

typedef enum TestScenario {
    TEST_PHASE_ZERO,
    TEST_SELECTION,
    TEST_CONNECT,
    TEST_COMPLETION,
    TEST_CANCEL_EXIT,
    TEST_CONNECTION_OUTCOME
} TestScenario;

volatile uint16_t gGameDatalinkPeerMenuInput;
Game_DatalinkSceneOwner gGameDatalinkPeerSceneOwner;
Game_DatalinkSceneOwner gGameDatalinkSceneOwner;
Game_DatalinkTweenRecord gGameDatalinkPeerMenuTransitionTween;
Game_DatalinkTweenRecord gGameDatalinkControllerTweens[32];
Game_DatalinkTweenRecord gGameDatalinkControllerPrimaryTween;
Game_DatalinkTweenRecord gGameDatalinkControllerSecondaryTween;
Game_DatalinkControllerPanelOwner gGameDatalinkControllerPanelOwner;
Game_DatalinkPhaseThirtyObjectOwner gGameDatalinkPhaseThirtyObjectOwner;

static TestScenario sScenario;
static uint32_t sWirelessContext;
static uint8_t sWirelessPeer[0x20];
static Game_DatalinkOamEntry sOamEntries[16][3];
static Game_DatalinkPeerSprite sSprites[16];

static unsigned int sEntranceCalls;
static unsigned int sStartDiscoveryCalls;
static unsigned int sRefreshCalls;
static unsigned int sStopDiscoveryCalls;
static unsigned int sSelectCalls;
static unsigned int sConnectCalls;
static unsigned int sConnectedExitCalls;
static unsigned int sCancelledExitCalls;
static unsigned int sArrangeCalls;
static unsigned int sClearCalls;
static unsigned int sFinalizeExitCalls;
static unsigned int sPopCalls;
static unsigned int sCommitCalls;
static unsigned int sOamSubmitCalls;
static unsigned int sResourceLoadCalls;
static unsigned int sChangeStateCalls;
static int sChangedState;
static uint32_t sSounds[8];
static unsigned int sSoundCount;
static int sCommands[32];
static unsigned int sCommandCount;

static void Test_Reset(TestScenario scenario)
{
    unsigned int index;

    sScenario = scenario;
    memset(&gGameDatalinkPeerSceneOwner, 0,
        sizeof(gGameDatalinkPeerSceneOwner));
    memset(&gGameDatalinkSceneOwner, 0, sizeof(gGameDatalinkSceneOwner));
    memset(&gGameDatalinkPeerMenuTransitionTween, 0,
        sizeof(gGameDatalinkPeerMenuTransitionTween));
    memset(gGameDatalinkControllerTweens, 0,
        sizeof(gGameDatalinkControllerTweens));
    memset(&gGameDatalinkPhaseThirtyObjectOwner, 0,
        sizeof(gGameDatalinkPhaseThirtyObjectOwner));
    memset(sWirelessPeer, 0, sizeof(sWirelessPeer));
    for (index = 0; index < 16; ++index) {
        memset(&sSprites[index], 0, sizeof(sSprites[index]));
        sSprites[index].oam_entries_24 = sOamEntries[index];
        sSprites[index].oam_start_index_44 = 1;
    }
    sEntranceCalls = 0;
    sStartDiscoveryCalls = 0;
    sRefreshCalls = 0;
    sStopDiscoveryCalls = 0;
    sSelectCalls = 0;
    sConnectCalls = 0;
    sConnectedExitCalls = 0;
    sCancelledExitCalls = 0;
    sArrangeCalls = 0;
    sClearCalls = 0;
    sFinalizeExitCalls = 0;
    sPopCalls = 0;
    sCommitCalls = 0;
    sOamSubmitCalls = 0;
    sResourceLoadCalls = 0;
    sChangeStateCalls = 0;
    sChangedState = -1;
    sSoundCount = 0;
    sCommandCount = 0;
    gGameDatalinkPeerMenuInput = 0;
}

static void Test_PrepareState(
    Game_DatalinkPeerMenuState *state,
    unsigned int peer_count)
{
    unsigned int index;

    memset(state, 0, sizeof(*state));
    state->screen_resource_0cc = UINT32_MAX;
    state->peer_count_0c8 = peer_count;
    for (index = 0; index < peer_count; ++index) {
        Game_DatalinkPeerListRecord *peer = &state->peer_storage_034[index];

        peer->tween_index_06 = (uint8_t)(index * 2U);
        peer->sprite_index_07 = (uint8_t)(index * 2U);
        peer->discovery_age_1c = 9;
        state->ordered_peers_0b4[index] = peer;
    }
}

void *Game_GetDatalinkWirelessContext_020909c8(void)
{
    return &sWirelessContext;
}

int Game_GetDatalinkWirelessState_02090ea4(void *context)
{
    assert(context == &sWirelessContext);
    if (sScenario == TEST_SELECTION) {
        return 5;
    }
    if (sScenario == TEST_CONNECT ||
        sScenario == TEST_CONNECTION_OUTCOME) {
        return 2;
    }
    return 0;
}

int Game_GetDatalinkWirelessPeerCount_02091d38(void *context)
{
    assert(context == &sWirelessContext);
    return sScenario == TEST_CONNECT ? 1 : 0;
}

void *Game_GetDatalinkWirelessPeer_02091d40(
    void *context,
    int index)
{
    assert(context == &sWirelessContext);
    assert(index == 0);
    return sWirelessPeer;
}

int Game_BeginDatalinkWirelessPeerConnection_02091870(
    void *context,
    void *peer,
    int data_size,
    int argument)
{
    assert(context == &sWirelessContext);
    assert(peer == sWirelessPeer);
    assert(data_size == 0x34);
    assert(argument == 0);
    ++sConnectCalls;
    return 1;
}

int Game_GetDatalinkWirelessConnectionOutcome_02091cd8(void *context)
{
    assert(context == &sWirelessContext);
    return sScenario == TEST_CONNECTION_OUTCOME ? 3 : 2;
}

void Game_BeginDatalinkPeerMenuEntrance_020afcbc(
    Game_DatalinkPeerMenuState *state)
{
    assert(state != NULL);
    ++sEntranceCalls;
}

int Game_StartDatalinkPeerDiscovery_020b0f8c(
    Game_DatalinkPeerMenuState *state)
{
    assert(state != NULL);
    ++sStartDiscoveryCalls;
    return 1;
}

void Game_RefreshDatalinkPeerRoster_020b0d7c(
    Game_DatalinkPeerMenuState *state)
{
    assert(state != NULL);
    ++sRefreshCalls;
}

int Game_StopDatalinkPeerDiscovery_020b0fe8(
    Game_DatalinkPeerMenuState *state)
{
    assert(state != NULL);
    ++sStopDiscoveryCalls;
    return 1;
}

int Game_SelectDatalinkWirelessPeer_020b0e84(
    Game_DatalinkPeerMenuState *state)
{
    assert(state != NULL);
    ++sSelectCalls;
    return 1;
}

void Game_BeginDatalinkPeerConnectedExit_020afd84(
    Game_DatalinkPeerMenuState *state)
{
    assert(state != NULL);
    ++sConnectedExitCalls;
}

void Game_BeginDatalinkPeerCancelledExit_020afe3c(
    Game_DatalinkPeerMenuState *state)
{
    assert(state != NULL);
    ++sCancelledExitCalls;
}

void Game_ArrangeDatalinkPeerSprites_020b0a6c(
    Game_DatalinkPeerMenuState *state,
    Game_DatalinkPeerListRecord *peer,
    unsigned int visible_index)
{
    assert(state != NULL);
    assert(peer == state->ordered_peers_0b4[visible_index]);
    ++sArrangeCalls;
}

void Game_ClearDatalinkPeerList_020b0d58(
    Game_DatalinkPeerMenuState *state)
{
    state->peer_count_0c8 = 0;
    ++sClearCalls;
}

void Game_FinalizeDatalinkPeerMenuExit_020280f0(void)
{
    ++sFinalizeExitCalls;
}

void Game_PlayDatalinkSoundEffect(uint32_t sound_id)
{
    assert(sSoundCount < sizeof(sSounds) / sizeof(sSounds[0]));
    sSounds[sSoundCount++] = sound_id;
}

void Game_ClampDatalinkSelection(
    volatile int8_t *selection,
    int minimum,
    int maximum)
{
    if (*selection < minimum) {
        *selection = (int8_t)minimum;
    }
    if (*selection > maximum) {
        *selection = (int8_t)maximum;
    }
}

void Game_LoadDatalinkScreenResource(uint16_t resource_id)
{
    if (sScenario == TEST_SELECTION || sScenario == TEST_CONNECT) {
        assert(resource_id == 0x083e);
    } else {
        assert(resource_id == 0x083d);
    }
    ++sResourceLoadCalls;
}

const Game_DatalinkPeerSprite *Game_GetDatalinkPeerSprite_020a2efc(
    uint8_t sprite_index)
{
    assert(sprite_index < 16);
    return &sSprites[sprite_index];
}

int Game_GetDatalinkPeerOamCapacity_020a4ee0(void)
{
    return 2;
}

void Game_SubmitVisibleDatalinkSceneOams(
    Game_DatalinkSceneOwner *scene,
    const Game_DatalinkOamEntry *oam_entries,
    int count)
{
    unsigned int sprite_index = sOamSubmitCalls;

    assert(scene == &gGameDatalinkPeerSceneOwner);
    assert(count == 2);
    if (sScenario == TEST_SELECTION) {
        static const unsigned int expected[4] = {0, 1, 2, 3};
        sprite_index = expected[sOamSubmitCalls];
    } else {
        sprite_index = sOamSubmitCalls;
    }
    assert(oam_entries == &sOamEntries[sprite_index][1]);
    ++sOamSubmitCalls;
}

void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command)
{
    assert(scene == &gGameDatalinkPeerSceneOwner);
    assert(sCommandCount < sizeof(sCommands) / sizeof(sCommands[0]));
    sCommands[sCommandCount++] = command;
}

int32_t Game_GetDatalinkTweenProgress(
    const Game_DatalinkTweenRecord *tween)
{
    assert(tween == &gGameDatalinkPeerMenuTransitionTween);
    return 0x1000;
}

void Game_ChangeDatalinkState(
    int state,
    int participant_count,
    int argument_2,
    int argument_3)
{
    assert(participant_count == 0);
    assert(argument_2 == 0);
    assert(argument_3 == 0);
    ++sChangeStateCalls;
    sChangedState = state;
}

void Game_CommitDatalinkScreenUpdate(void)
{
    ++sCommitCalls;
}

void Game_DatalinkStateStack_PopState(Game_DatalinkStateStack *stack)
{
    assert((void *)stack == (void *)&gGameDatalinkPeerSceneOwner);
    ++sPopCalls;
}

static void Test_PhaseZero(void)
{
    Game_DatalinkPeerMenuState state;

    Test_Reset(TEST_PHASE_ZERO);
    Test_PrepareState(&state, 0);
    state.phase_106 = 0;
    state.selected_peer_104 = 3;
    Game_UpdateDatalinkPeerMenu_020b0014(&state);

    assert(sEntranceCalls == 1);
    assert(state.phase_106 == 1);
    assert(state.selected_peer_104 == 0);
    assert(state.screen_resource_0cc == 0x083d);
    assert(sResourceLoadCalls == 1);
    assert(sCommandCount == 2);
    assert(sCommands[0] == 0x17);
    assert(sCommands[1] == 0x22);
}

static void Test_SelectionAndRendering(void)
{
    Game_DatalinkPeerMenuState state;

    Test_Reset(TEST_SELECTION);
    Test_PrepareState(&state, 2);
    state.phase_106 = 2;
    state.screen_resource_0cc = 0x083d;
    state.selected_peer_104 = 0;
    gGameDatalinkPeerMenuInput = 0x80;
    gGameDatalinkControllerTweens[0].current_x_fx_30 = -1;
    gGameDatalinkControllerTweens[0].current_y_fx_34 = 0x1234;

    Game_UpdateDatalinkPeerMenu_020b0014(&state);

    assert(sRefreshCalls == 1);
    assert(state.selected_peer_104 == 1);
    assert(sSoundCount == 1 && sSounds[0] == 0x3d);
    assert(gGameDatalinkSceneOwner.cursor_transition_duration_56c == 0);
    assert(gGameDatalinkSceneOwner.cursor_x_fx_574 == 40 * 0x1000);
    assert(gGameDatalinkSceneOwner.cursor_y_fx_578 == 88 * 0x1000);
    assert(state.screen_resource_0cc == 0x083e);
    assert(sResourceLoadCalls == 1);
    assert(sOamSubmitCalls == 4);
    assert(sCommandCount == 5);
    assert(sCommands[0] == 3);
    assert(sCommands[1] == 1);
    assert(sCommands[2] == 2);
    assert(sCommands[3] == 0x17);
    assert(sCommands[4] == 0x22);
    assert(gGameDatalinkControllerTweens[1].duration_fx_28 == 0);
    assert(gGameDatalinkControllerTweens[1].current_x_fx_30 == -0x1000);
    assert(gGameDatalinkControllerTweens[1].current_y_fx_34 == 0x1000);
}

static void Test_Connect(void)
{
    Game_DatalinkPeerMenuState state;
    unsigned int index;

    Test_Reset(TEST_CONNECT);
    Test_PrepareState(&state, 1);
    state.phase_106 = 4;
    state.selected_peer_104 = 0;
    state.selected_wireless_peer_0c4 = sWirelessPeer;
    for (index = 0; index < 6; ++index) {
        state.peer_storage_034[0].identity_00[index] =
            (uint8_t)(0x20 + index);
        sWirelessPeer[8 + index] = (uint8_t)(0x20 + index);
    }

    Game_UpdateDatalinkPeerMenu_020b0014(&state);

    assert(sConnectCalls == 1);
    assert(state.phase_106 == 5);
    assert(sSoundCount == 1 && sSounds[0] == 0x3e);
    assert(state.screen_resource_0cc == 0x083e);
    assert(sOamSubmitCalls == 2);
    assert(sCommandCount == 3);
    assert(sCommands[0] == 0);
    assert(sCommands[1] == 0x17);
    assert(sCommands[2] == 0x22);
}

static void Test_Completion(void)
{
    Game_DatalinkPeerMenuState state;

    Test_Reset(TEST_COMPLETION);
    Test_PrepareState(&state, 1);
    state.phase_106 = 8;
    state.completion_timer_100 = 61;
    gGameDatalinkPeerMenuInput = 1;

    Game_UpdateDatalinkPeerMenu_020b0014(&state);

    assert(state.phase_106 == 0);
    assert(state.completion_timer_100 == 0);
    assert(sCommitCalls == 1);
    assert(sClearCalls == 1);
    assert(sSoundCount == 1 && sSounds[0] == 0x3e);
    assert(sCommandCount == 1 && sCommands[0] == 0x35);
    assert(sResourceLoadCalls == 0);
    assert(sOamSubmitCalls == 0);
    assert(gGameDatalinkPhaseThirtyObjectOwner
        .position_transition_duration_69c == 0);
    assert(gGameDatalinkPhaseThirtyObjectOwner.position_x_fx_6a4 ==
        224 * 0x1000);
    assert(gGameDatalinkPhaseThirtyObjectOwner.position_y_fx_6a8 ==
        192 * 0x1000);
}

static void Test_CancelExit(void)
{
    Game_DatalinkPeerMenuState state;

    Test_Reset(TEST_CANCEL_EXIT);
    Test_PrepareState(&state, 1);
    state.phase_106 = 7;
    state.exit_countdown_102 = 1;

    Game_UpdateDatalinkPeerMenu_020b0014(&state);

    assert(state.exit_countdown_102 == 0);
    assert(sClearCalls == 1);
    assert(sPopCalls == 1);
    assert(sCommandCount == 0);
    assert(sResourceLoadCalls == 0);
}

static void Test_ConnectionOutcome(void)
{
    Game_DatalinkPeerMenuState state;

    Test_Reset(TEST_CONNECTION_OUTCOME);
    Test_PrepareState(&state, 0);
    state.phase_106 = 5;

    Game_UpdateDatalinkPeerMenu_020b0014(&state);

    assert(state.phase_106 == 9);
    assert(state.completion_timer_100 == 0);
    assert(sChangeStateCalls == 1);
    assert(sChangedState == 0x0e);
    assert(sCommandCount == 2);
    assert(sCommands[0] == 0x17);
    assert(sCommands[1] == 0x22);
}

int main(void)
{
    assert(sizeof(Game_DatalinkPeerListRecord) == 0x20);
    Test_PhaseZero();
    Test_SelectionAndRendering();
    Test_Connect();
    Test_Completion();
    Test_CancelExit();
    Test_ConnectionOutcome();
    return 0;
}
