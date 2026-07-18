#include "game/datalink_extended_menu_controller.h"

#include "game/datalink_controller.h"
#include "game/datalink_phase_ten_finalizer.h"
#include "game/datalink_phase_thirty.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

Game_DatalinkTweenRecord gGameDatalinkControllerTweens[32];
Game_DatalinkTweenRecord gGameDatalinkControllerPrimaryTween;
Game_DatalinkTweenRecord gGameDatalinkControllerSecondaryTween;
Game_DatalinkControllerPanelOwner gGameDatalinkControllerPanelOwner;
Game_DatalinkSceneOwner gGameDatalinkSceneOwner;
Game_DatalinkPhaseThirtyObjectOwner
    gGameDatalinkPhaseThirtyObjectOwner;
volatile uint16_t gGameDatalinkControllerInput;
volatile uint32_t gGameDatalinkControllerDisplayControl;
volatile uint32_t gGameDatalinkExtendedMenuSelectionWord;
Game_DatalinkExtendedMenuSession gGameDatalinkExtendedMenuSession;
volatile uint8_t gGameDatalinkControllerScreenResource;

static uint32_t sSelectionContext;
static int sConnectionState;
static int sContextResult;
static int sContextReady;
static int sPrimaryPrepared;
static int32_t sTweenProgress;
static int sContextCount;
static int sConnectionCount;
static int sContextReadyCount;
static int sContextResultCount;
static int sPrimaryPrepareCount;
static int sSecondaryActionCount;
static int sEnterRightCount;
static int sEnterLeftCount;
static int sExitRightCount;
static int sExitLeftCount;
static int sPopCount;
static int sPushCount;
static int sCommitCount;
static int sChangeStateCount;
static int sChangedState;
static uint32_t sSounds[4];
static size_t sSoundCount;
static int sCommands[8];
static size_t sCommandCount;

uint32_t Game_GetDatalinkMenuSelectionContext(void)
{
    ++sContextCount;
    return sSelectionContext;
}

int Game_GetDatalinkMenuConnectionState(uint32_t selection_context)
{
    assert(selection_context == sSelectionContext);
    ++sConnectionCount;
    return sConnectionState;
}

int Game_IsDatalinkMenuContextReady(uint32_t selection_context)
{
    assert(selection_context == sSelectionContext);
    ++sContextReadyCount;
    return sContextReady;
}

int Game_GetDatalinkMenuContextResult(uint32_t selection_context)
{
    assert(selection_context == sSelectionContext);
    ++sContextResultCount;
    return sContextResult;
}

int Game_PrepareDatalinkExtendedMenuPrimaryAction(void)
{
    ++sPrimaryPrepareCount;
    return sPrimaryPrepared;
}

void Game_BeginDatalinkSecondaryMenuAction(void)
{
    ++sSecondaryActionCount;
}

int32_t Game_GetDatalinkTweenProgress(
    const Game_DatalinkTweenRecord *tween
)
{
    assert(tween == &gGameDatalinkControllerTweens[26]);
    return sTweenProgress;
}

void Game_ClampDatalinkSelection(
    volatile int8_t *selection,
    int minimum,
    int maximum
)
{
    if (*selection < minimum) {
        *selection = (int8_t)minimum;
    } else if (*selection > maximum) {
        *selection = (int8_t)maximum;
    }
}

void Game_PlayDatalinkSoundEffect(uint32_t sound_id)
{
    assert(sSoundCount < sizeof(sSounds) / sizeof(sSounds[0]));
    sSounds[sSoundCount++] = sound_id;
}

void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command
)
{
    assert(scene == &gGameDatalinkSceneOwner);
    assert(sCommandCount < sizeof(sCommands) / sizeof(sCommands[0]));
    sCommands[sCommandCount++] = command;
}

void Game_PopDatalinkExtendedMenuSceneTransition(
    Game_DatalinkSceneOwner *scene
)
{
    assert(scene == &gGameDatalinkSceneOwner);
    ++sPopCount;
}

void Game_PushDatalinkExtendedMenuSceneTransition(
    Game_DatalinkSceneOwner *scene,
    uint32_t secondary_interface_address
)
{
    assert(scene == &gGameDatalinkSceneOwner);
    assert(secondary_interface_address ==
           GAME_DATALINK_EXTENDED_MENU_SECONDARY_INTERFACE_ADDRESS);
    ++sPushCount;
}

void Game_CommitDatalinkScreenUpdate(void)
{
    ++sCommitCount;
}

void Game_ChangeDatalinkState(
    int state,
    int participant_count,
    int argument_2,
    int argument_3
)
{
    assert(participant_count == 0);
    assert(argument_2 == 0);
    assert(argument_3 == 0);
    sChangedState = state;
    ++sChangeStateCount;
}

void Game_EnterDatalinkExtendedMenuFromRight(void)
{
    ++sEnterRightCount;
}

void Game_EnterDatalinkExtendedMenuFromLeft(void)
{
    ++sEnterLeftCount;
}

void Game_ExitDatalinkExtendedMenuToRight(void)
{
    ++sExitRightCount;
}

void Game_ExitDatalinkExtendedMenuToLeft(void)
{
    ++sExitLeftCount;
}

static void ResetHarness(Game_DatalinkExtendedMenuController *controller)
{
    memset(controller, 0, sizeof(*controller));
    memset(gGameDatalinkControllerTweens, 0,
           sizeof(gGameDatalinkControllerTweens));
    memset(&gGameDatalinkSceneOwner, 0,
           sizeof(gGameDatalinkSceneOwner));
    memset(&gGameDatalinkPhaseThirtyObjectOwner, 0,
           sizeof(gGameDatalinkPhaseThirtyObjectOwner));
    memset(&gGameDatalinkExtendedMenuSession, 0,
           sizeof(gGameDatalinkExtendedMenuSession));
    memset(sSounds, 0, sizeof(sSounds));
    memset(sCommands, 0, sizeof(sCommands));
    controller->secondary_interface_address_34 =
        GAME_DATALINK_EXTENDED_MENU_SECONDARY_INTERFACE_ADDRESS;
    gGameDatalinkControllerInput = 0U;
    gGameDatalinkControllerDisplayControl = UINT32_C(0xFFFFFFFF);
    gGameDatalinkExtendedMenuSelectionWord = UINT32_C(0x1234567B);
    gGameDatalinkControllerScreenResource = 0U;
    sSelectionContext = UINT32_C(0x89ABCDEF);
    sConnectionState = 0;
    sContextResult = 0;
    sContextReady = 0;
    sPrimaryPrepared = 1;
    sTweenProgress = 0x1000;
    sContextCount = 0;
    sConnectionCount = 0;
    sContextReadyCount = 0;
    sContextResultCount = 0;
    sPrimaryPrepareCount = 0;
    sSecondaryActionCount = 0;
    sEnterRightCount = 0;
    sEnterLeftCount = 0;
    sExitRightCount = 0;
    sExitLeftCount = 0;
    sPopCount = 0;
    sPushCount = 0;
    sCommitCount = 0;
    sChangeStateCount = 0;
    sChangedState = -1;
    sSoundCount = 0U;
    sCommandCount = 0U;
}

static void Update(Game_DatalinkExtendedMenuController *controller)
{
    Game_UpdateDatalinkExtendedMenuController(controller);
    assert(sContextCount == 1);
    assert(sConnectionCount == 1);
    assert(gGameDatalinkExtendedMenuSession.selection_nibble_08 == 0x0BU);
}

static void AssertCommandsForRowZero(void)
{
    assert(sCommandCount == 3U);
    assert(sCommands[0] == 26);
    assert(sCommands[1] == 29);
    assert(sCommands[2] == 23);
}

static void AssertCommandsForRowOne(size_t first_command)
{
    assert(sCommandCount >= first_command + 3U);
    assert(sCommands[first_command] == 27);
    assert(sCommands[first_command + 1U] == 28);
    assert(sCommands[first_command + 2U] == 23);
}

static void TestEntrancePhases(void)
{
    Game_DatalinkExtendedMenuController controller;

    ResetHarness(&controller);
    controller.transition_phase_5f = 0U;
    Update(&controller);
    assert(gGameDatalinkExtendedMenuSession.requested_count_04 == 4U);
    assert(controller.transition_phase_5f == 2U);
    assert(sEnterRightCount == 1);
    AssertCommandsForRowZero();

    ResetHarness(&controller);
    controller.transition_phase_5f = 1U;
    Update(&controller);
    assert(gGameDatalinkExtendedMenuSession.requested_count_04 == 4U);
    assert(controller.transition_phase_5f == 2U);
    assert(sEnterLeftCount == 1);
    AssertCommandsForRowZero();
}

static void TestSelectionWaitAndMovement(void)
{
    Game_DatalinkExtendedMenuController controller;

    ResetHarness(&controller);
    controller.transition_phase_5f = 2U;
    sTweenProgress = 0x0FFF;
    Update(&controller);
    assert(gGameDatalinkControllerDisplayControl == UINT32_C(0xFFFFF9FF));
    AssertCommandsForRowZero();

    ResetHarness(&controller);
    controller.transition_phase_5f = 2U;
    gGameDatalinkControllerInput = 0x80U;
    Update(&controller);
    assert(controller.selected_row_5e == 1);
    assert(sSoundCount == 1U);
    assert(sSounds[0] == 0x3DU);
    assert(gGameDatalinkSceneOwner.cursor_transition_duration_56c == 0);
    assert(gGameDatalinkSceneOwner.cursor_x_fx_574 == 32 * 0x1000);
    assert(gGameDatalinkSceneOwner.cursor_y_fx_578 == 121 * 0x1000);
    assert(sCommandCount == 4U);
    assert(sCommands[0] == 3);
    AssertCommandsForRowOne(1U);
}

static void TestSelectionConfirmation(void)
{
    Game_DatalinkExtendedMenuController controller;

    ResetHarness(&controller);
    controller.transition_phase_5f = 2U;
    gGameDatalinkControllerInput = 1U;
    Update(&controller);
    assert(sPrimaryPrepareCount == 1);
    assert(controller.transition_phase_5f == 4U);
    assert(sSoundCount == 1U && sSounds[0] == 0x3EU);

    ResetHarness(&controller);
    controller.transition_phase_5f = 2U;
    gGameDatalinkControllerInput = 1U;
    sPrimaryPrepared = 0;
    Update(&controller);
    assert(controller.transition_phase_5f == 2U);
    assert(sSoundCount == 1U && sSounds[0] == 0x3EU);

    ResetHarness(&controller);
    controller.transition_phase_5f = 2U;
    controller.selected_row_5e = 1;
    gGameDatalinkControllerInput = 1U;
    Update(&controller);
    assert(sExitLeftCount == 1);
    assert(controller.transition_phase_5f == 6U);
    assert(sSoundCount == 1U && sSounds[0] == 0x3EU);
}

static void TestSelectionCancellation(void)
{
    Game_DatalinkExtendedMenuController controller;

    ResetHarness(&controller);
    controller.transition_phase_5f = 2U;
    gGameDatalinkControllerInput = 2U;
    Update(&controller);
    assert(sContextReadyCount == 0);
    assert(sExitRightCount == 1);
    assert(controller.transition_phase_5f == 3U);
    assert(sSoundCount == 1U && sSounds[0] == 0x44U);

    ResetHarness(&controller);
    controller.transition_phase_5f = 2U;
    gGameDatalinkControllerInput = 2U;
    sConnectionState = 1;
    sContextReady = 0;
    Update(&controller);
    assert(sContextReadyCount == 1);
    assert(sExitRightCount == 0);
    assert(controller.transition_phase_5f == 2U);
}

static void TestTransitionWaitPhases(void)
{
    Game_DatalinkExtendedMenuController controller;

    ResetHarness(&controller);
    controller.transition_phase_5f = 3U;
    Update(&controller);
    assert(sPopCount == 1);

    ResetHarness(&controller);
    controller.transition_phase_5f = 3U;
    sConnectionState = 2;
    Update(&controller);
    assert(sContextReadyCount == 1);
    assert(sPopCount == 0);

    ResetHarness(&controller);
    controller.transition_phase_5f = 4U;
    sConnectionState = 8;
    gGameDatalinkExtendedMenuSession.pending_value_24 = 9U;
    Update(&controller);
    assert(sSecondaryActionCount == 1);
    assert(gGameDatalinkExtendedMenuSession.pending_value_24 == 0U);
    assert(sExitLeftCount == 1);
    assert(controller.transition_phase_5f == 5U);

    ResetHarness(&controller);
    controller.transition_phase_5f = 5U;
    Update(&controller);
    assert(gGameDatalinkControllerScreenResource == 0x84U);

    ResetHarness(&controller);
    controller.transition_phase_5f = 6U;
    controller.history_depth_38 = 2U;
    Update(&controller);
    assert(controller.history_depth_38 == 3U);
    assert(controller.phase_history_3c[2] == 1);
    assert(sPushCount == 1);
}

static void TestRecoveryResultDispatch(void)
{
    static const int results[3] = {2, 3, 99};
    static const int states[3] = {12, 14, 13};
    Game_DatalinkExtendedMenuController controller;
    size_t index;

    for (index = 0U; index < 3U; ++index) {
        ResetHarness(&controller);
        controller.transition_phase_5f = 7U;
        controller.force_final_phase_60 = 1U;
        sContextResult = results[index];
        Update(&controller);
        assert(sContextResultCount == 1);
        assert(sChangeStateCount == 1);
        assert(sChangedState == states[index]);
        assert(controller.force_final_phase_60 == 0U);
        assert(sCommandCount == 0U);
    }
}

static void TestRecoveryTimerAndCommit(void)
{
    Game_DatalinkExtendedMenuController controller;

    ResetHarness(&controller);
    controller.transition_phase_5f = 7U;
    controller.transition_state_5c = 60U;
    Update(&controller);
    assert(controller.transition_state_5c == 61U);
    assert(sCommandCount == 0U);

    ResetHarness(&controller);
    controller.transition_phase_5f = 7U;
    controller.transition_state_5c = 61U;
    Update(&controller);
    assert(gGameDatalinkPhaseThirtyObjectOwner
               .position_transition_duration_69c == 0);
    assert(gGameDatalinkPhaseThirtyObjectOwner.position_x_fx_6a4 ==
           224 * 0x1000);
    assert(gGameDatalinkPhaseThirtyObjectOwner.position_y_fx_6a8 ==
           192 * 0x1000);
    assert(sCommandCount == 1U && sCommands[0] == 0x35);
    assert(sCommitCount == 0);

    ResetHarness(&controller);
    controller.transition_phase_5f = 7U;
    controller.transition_state_5c = 61U;
    gGameDatalinkControllerInput = 1U;
    Update(&controller);
    assert(sCommitCount == 1);
    assert(controller.transition_phase_5f == 0U);
    assert(controller.transition_state_5c == 0U);
    assert(sSoundCount == 1U && sSounds[0] == 0x3EU);
    assert(sCommandCount == 1U && sCommands[0] == 0x35);
}

int main(void)
{
    TestEntrancePhases();
    TestSelectionWaitAndMovement();
    TestSelectionConfirmation();
    TestSelectionCancellation();
    TestTransitionWaitPhases();
    TestRecoveryResultDispatch();
    TestRecoveryTimerAndCommit();
    return 0;
}
