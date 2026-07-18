#include "game/datalink_menu_controller.h"

#include "game/datalink_controller.h"
#include "game/datalink_phase_ten_finalizer.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

Game_DatalinkTweenRecord gGameDatalinkControllerTweens[32];
Game_DatalinkTweenRecord gGameDatalinkControllerPrimaryTween;
Game_DatalinkTweenRecord gGameDatalinkControllerSecondaryTween;
Game_DatalinkControllerPanelOwner gGameDatalinkControllerPanelOwner;
Game_DatalinkSceneOwner gGameDatalinkSceneOwner;
volatile uint16_t gGameDatalinkControllerInput;
volatile uint32_t gGameDatalinkControllerDisplayControl;
volatile uint8_t gGameDatalinkMenuMappedSlot;
volatile uint8_t gGameDatalinkControllerScreenResource;

static uint32_t sSelectionContext;
static int sConnectionState;
static int sContextCount;
static int sConnectionCount;
static int32_t sTweenProgress;
static int sPrimaryAvailable;
static uint32_t sSounds[3];
static size_t sSoundCount;
static int sCommands[8];
static size_t sCommandCount;
static int sClearCount;
static int sEnterRightCount;
static int sEnterLeftCount;
static int sExitLeftCount;
static int sExitRightCount;
static int sPrimaryActionCount;
static int sSecondaryActionCount;
static int sPopCount;
static int sPushCount;
static uint32_t sWirelessBits[2];
static size_t sWirelessBitCount;
static uint32_t sMappedInput;
static int sCommitBeginCount;
static int sCommitComplete;
static int sFinalizeCount;

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

void Game_BeginDatalinkPrimaryMenuAction(
    uint32_t selection_context,
    int zero
)
{
    assert(selection_context == sSelectionContext);
    assert(zero == 0);
    ++sPrimaryActionCount;
}

void Game_BeginDatalinkSecondaryMenuAction(void)
{
    ++sSecondaryActionCount;
}

uint32_t Game_GetDatalinkWirelessSelectionBits(void)
{
    assert(sWirelessBitCount <
           sizeof(sWirelessBits) / sizeof(sWirelessBits[0]));
    return sWirelessBits[sWirelessBitCount++];
}

uint8_t Game_MapDatalinkWirelessSelection(uint32_t selection)
{
    sMappedInput = selection;
    return (uint8_t)(selection + 1U);
}

void Game_BeginDatalinkWirelessSelectionCommit(void)
{
    ++sCommitBeginCount;
}

int Game_IsDatalinkWirelessSelectionCommitComplete(void)
{
    return sCommitComplete;
}

void Game_FinalizeDatalinkWirelessSelection(void)
{
    ++sFinalizeCount;
}

int32_t Game_GetDatalinkTweenProgress(
    const Game_DatalinkTweenRecord *tween
)
{
    assert(tween == &gGameDatalinkControllerTweens[19]);
    return sTweenProgress;
}

int Game_IsDatalinkPrimaryMenuChoiceAvailable(void)
{
    return sPrimaryAvailable;
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

void Game_ClearDatalinkMenuBackground(
    Game_DatalinkMenuController *controller
)
{
    assert(controller != NULL);
    ++sClearCount;
}

void Game_PopDatalinkMenuSceneTransition(
    Game_DatalinkSceneOwner *scene
)
{
    assert(scene == &gGameDatalinkSceneOwner);
    ++sPopCount;
}

void Game_PushDatalinkMenuSceneTransition(
    Game_DatalinkSceneOwner *scene,
    uint32_t secondary_interface_slot_address
)
{
    assert(scene == &gGameDatalinkSceneOwner);
    assert(secondary_interface_slot_address ==
           GAME_DATALINK_MENU_SECONDARY_SLOT_ADDRESS);
    ++sPushCount;
}

void Game_EnterDatalinkMenuFromRight(void)
{
    ++sEnterRightCount;
}

void Game_ExitDatalinkMenuToLeft(void)
{
    ++sExitLeftCount;
}

void Game_EnterDatalinkMenuFromLeft(void)
{
    ++sEnterLeftCount;
}

void Game_ExitDatalinkMenuToRight(void)
{
    ++sExitRightCount;
}

static void ResetHarness(Game_DatalinkMenuController *controller)
{
    memset(controller, 0, sizeof(*controller));
    memset(&gGameDatalinkSceneOwner, 0,
           sizeof(gGameDatalinkSceneOwner));
    memset(sSounds, 0, sizeof(sSounds));
    memset(sCommands, 0, sizeof(sCommands));
    memset(sWirelessBits, 0, sizeof(sWirelessBits));
    controller->secondary_interface_slot_address_34 =
        GAME_DATALINK_MENU_SECONDARY_SLOT_ADDRESS;
    gGameDatalinkControllerInput = 0U;
    gGameDatalinkControllerDisplayControl = UINT32_C(0xFFFFFFFF);
    gGameDatalinkMenuMappedSlot = 0U;
    gGameDatalinkControllerScreenResource = 0U;
    sSelectionContext = UINT32_C(0x12345678);
    sConnectionState = 0;
    sContextCount = 0;
    sConnectionCount = 0;
    sTweenProgress = 0x1000;
    sPrimaryAvailable = 1;
    sSoundCount = 0U;
    sCommandCount = 0U;
    sClearCount = 0;
    sEnterRightCount = 0;
    sEnterLeftCount = 0;
    sExitLeftCount = 0;
    sExitRightCount = 0;
    sPrimaryActionCount = 0;
    sSecondaryActionCount = 0;
    sPopCount = 0;
    sPushCount = 0;
    sWirelessBitCount = 0U;
    sMappedInput = UINT32_C(0xFFFFFFFF);
    sCommitBeginCount = 0;
    sCommitComplete = 0;
    sFinalizeCount = 0;
}

static void Update(Game_DatalinkMenuController *controller)
{
    Game_UpdateDatalinkMenuController(
        controller,
        UINT32_C(0xAAAAAAAA),
        UINT32_C(0xBBBBBBBB),
        UINT32_C(0xCCCCCCCC)
    );
    assert(sContextCount == 1);
    assert(sConnectionCount == 1);
}

static void AssertTailCommands(int first, int second)
{
    assert(sCommandCount == 3U);
    assert(sCommands[0] == first);
    assert(sCommands[1] == second);
    assert(sCommands[2] == 23);
}

static void TestInitialPhasesEnterFromBothDirections(void)
{
    Game_DatalinkMenuController controller;

    ResetHarness(&controller);
    Update(&controller);
    assert(sClearCount == 1);
    assert(sEnterRightCount == 1);
    assert(controller.transition_phase_5e == 2U);
    AssertTailCommands(19, 22);

    ResetHarness(&controller);
    controller.transition_phase_5e = 1U;
    Update(&controller);
    assert(sEnterLeftCount == 1);
    assert(controller.transition_phase_5e == 2U);
}

static void TestSelectionWaitAndNavigation(void)
{
    Game_DatalinkMenuController controller;

    ResetHarness(&controller);
    controller.transition_phase_5e = 2U;
    sTweenProgress = 0;
    Update(&controller);
    assert((gGameDatalinkControllerDisplayControl & 0x1F00U) ==
           0x1900U);
    AssertTailCommands(19, 22);

    ResetHarness(&controller);
    controller.transition_phase_5e = 2U;
    gGameDatalinkControllerInput = 0x80U;
    Update(&controller);
    assert(controller.selected_row_5d == 1);
    assert(sSounds[0] == 0x3DU);
    assert(gGameDatalinkSceneOwner.cursor_x_fx_574 == 40 * 0x1000);
    assert(gGameDatalinkSceneOwner.cursor_y_fx_578 == 121 * 0x1000);
    assert(sCommands[0] == 3);
    assert(sCommands[1] == 20);
    assert(sCommands[2] == 21);
    assert(sCommands[3] == 23);
}

static void TestConfirmPathsPushHistoryAndExitLeft(void)
{
    Game_DatalinkMenuController controller;

    ResetHarness(&controller);
    controller.transition_phase_5e = 2U;
    gGameDatalinkControllerInput = 1U;
    Update(&controller);
    assert(sPrimaryActionCount == 1);
    assert(controller.transition_phase_5e == 4U);
    assert(controller.history_depth_38 == 1U);
    assert(controller.selection_history_3c[0] == 0U);
    assert(sExitLeftCount == 1);
    assert(sSounds[0] == 0x3EU);

    ResetHarness(&controller);
    controller.transition_phase_5e = 2U;
    controller.selected_row_5d = 1;
    gGameDatalinkControllerInput = 1U;
    Update(&controller);
    assert(sSecondaryActionCount == 1);
    assert(controller.transition_phase_5e == 5U);
    assert(controller.history_depth_38 == 1U);
    assert(controller.selection_history_3c[0] == 1U);
    assert(sExitLeftCount == 1);
}

static void TestCancelAndTransitionWaits(void)
{
    Game_DatalinkMenuController controller;

    ResetHarness(&controller);
    controller.transition_phase_5e = 2U;
    gGameDatalinkControllerInput = 2U;
    Update(&controller);
    assert(controller.transition_phase_5e == 3U);
    assert(sExitRightCount == 1);
    assert(sSounds[0] == 0x44U);

    ResetHarness(&controller);
    controller.transition_phase_5e = 3U;
    Update(&controller);
    assert(sPopCount == 1);

    ResetHarness(&controller);
    controller.transition_phase_5e = 4U;
    sConnectionState = 2;
    Update(&controller);
    assert(sPushCount == 1);
}

static void TestWirelessSelectionAndCommitPhases(void)
{
    Game_DatalinkMenuController controller;

    ResetHarness(&controller);
    controller.transition_phase_5e = 5U;
    sWirelessBits[0] = 0x40U;
    sWirelessBits[1] = 0x4AU;
    Update(&controller);
    assert(sWirelessBitCount == 2U);
    assert(sMappedInput == 0x0AU);
    assert(gGameDatalinkMenuMappedSlot == 0x0BU);
    assert(sCommitBeginCount == 1);
    assert(controller.transition_phase_5e == 6U);

    ResetHarness(&controller);
    controller.transition_phase_5e = 6U;
    sCommitComplete = 1;
    Update(&controller);
    assert(sFinalizeCount == 1);
    assert(controller.transition_phase_5e == 7U);
    assert(controller.transition_countdown_5c == 1U);

    ResetHarness(&controller);
    controller.transition_phase_5e = 7U;
    controller.transition_countdown_5c = 1U;
    Update(&controller);
    assert(controller.transition_countdown_5c == 0U);
    assert(gGameDatalinkControllerScreenResource == 0x84U);
}

int main(void)
{
    TestInitialPhasesEnterFromBothDirections();
    TestSelectionWaitAndNavigation();
    TestConfirmPathsPushHistoryAndExitLeft();
    TestCancelAndTransitionWaits();
    TestWirelessSelectionAndCommitPhases();
    return 0;
}
