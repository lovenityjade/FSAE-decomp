#include "game/datalink_controller.h"

#include "game/datalink_phase_ten_finalizer.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct ConfigureCall {
    Game_DatalinkTweenRecord *tween;
    int32_t x_fx;
    int32_t y_fx;
    uint32_t duration;
    int mode;
} ConfigureCall;

Game_DatalinkTweenRecord gGameDatalinkControllerTweens[32];
Game_DatalinkTweenRecord gGameDatalinkControllerPrimaryTween;
Game_DatalinkTweenRecord gGameDatalinkControllerSecondaryTween;
Game_DatalinkSceneOwner gGameDatalinkSceneOwner;
volatile uint8_t gGameDatalinkControllerActiveSlot;
volatile uint16_t gGameDatalinkControllerInput;

static int32_t sProgress;
static int sReadyCount;
static bool sPrimaryTransitionComplete;
static ConfigureCall sConfigureCalls[2];
static size_t sConfigureCount;
static int sSceneCommands[8];
static size_t sSceneCommandCount;
static uint32_t sSounds[2];
static size_t sSoundCount;
static int sChangeState;
static int sChangeCount;
static int sCommitCount;
static int sVariantOneCount;
static uint32_t sLockedOwner;
static uint32_t sReturnOwner;
static uint32_t sReturnPhase;
static int sReturnPushCount;
static uint32_t sSceneTransitionAddress;
static int sSceneTransitionCount;
static int sClampCount;

int32_t Game_GetDatalinkTweenProgress(
    const Game_DatalinkTweenRecord *tween
)
{
    assert(tween == &gGameDatalinkControllerTweens[11]);
    return sProgress;
}

void Game_ConfigureDatalinkTween(
    Game_DatalinkTweenRecord *tween,
    int32_t target_x_fx,
    int32_t target_y_fx,
    uint32_t duration,
    int mode
)
{
    ConfigureCall *call;

    assert(sConfigureCount <
           sizeof(sConfigureCalls) / sizeof(sConfigureCalls[0]));
    call = &sConfigureCalls[sConfigureCount++];
    call->tween = tween;
    call->x_fx = target_x_fx;
    call->y_fx = target_y_fx;
    call->duration = duration;
    call->mode = mode;
}

void Game_ClampDatalinkSelection(
    volatile int8_t *selection,
    int minimum,
    int maximum
)
{
    ++sClampCount;
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
    assert(sSceneCommandCount <
           sizeof(sSceneCommands) / sizeof(sSceneCommands[0]));
    sSceneCommands[sSceneCommandCount++] = command;
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
    sChangeState = state;
    ++sChangeCount;
}

void Game_CommitDatalinkScreenUpdate(void)
{
    ++sCommitCount;
}

void CARD_LockBackup(uint32_t owner_address)
{
    sLockedOwner = owner_address;
}

void Game_PushDatalinkControllerReturnPhase(
    uint32_t owner_address,
    uint32_t return_phase
)
{
    sReturnOwner = owner_address;
    sReturnPhase = return_phase;
    ++sReturnPushCount;
}

void Game_PushDatalinkSceneTransition(
    Game_DatalinkSceneOwner *scene,
    uint32_t transition_address
)
{
    assert(scene == &gGameDatalinkSceneOwner);
    sSceneTransitionAddress = transition_address;
    ++sSceneTransitionCount;
}

int Game_CountReadyDatalinkPlayerSlots(void)
{
    return sReadyCount;
}

bool Game_IsDatalinkPrimaryTransitionComplete(void)
{
    return sPrimaryTransitionComplete;
}

void Game_SelectDatalinkControllerVariantOne(
    Game_DatalinkController *controller
)
{
    assert(controller != NULL);
    ++sVariantOneCount;
}

static void ResetHarness(Game_DatalinkController *controller)
{
    memset(controller, 0, sizeof(*controller));
    memset(gGameDatalinkControllerTweens, 0xA5,
           sizeof(gGameDatalinkControllerTweens));
    memset(&gGameDatalinkSceneOwner, 0,
           sizeof(gGameDatalinkSceneOwner));
    memset(sConfigureCalls, 0, sizeof(sConfigureCalls));
    memset(sSceneCommands, 0, sizeof(sSceneCommands));
    memset(sSounds, 0, sizeof(sSounds));
    gGameDatalinkControllerActiveSlot = 0;
    gGameDatalinkControllerInput = 0;
    sProgress = 0x1000;
    sReadyCount = 0;
    sPrimaryTransitionComplete = false;
    sConfigureCount = 0U;
    sSceneCommandCount = 0U;
    sSoundCount = 0U;
    sChangeState = -1;
    sChangeCount = 0;
    sCommitCount = 0;
    sVariantOneCount = 0;
    sLockedOwner = 0U;
    sReturnOwner = 0U;
    sReturnPhase = 0U;
    sReturnPushCount = 0;
    sSceneTransitionAddress = 0U;
    sSceneTransitionCount = 0;
    sClampCount = 0;
    controller->owner_address_08 = UINT32_C(0x11223344);
    controller->interface_address_54 = UINT32_C(0x55667788);
}

static void AssertTailCommands(int selected_row, size_t offset)
{
    static const int expected[3][3] = {
        {11, 14, 16},
        {12, 13, 16},
        {12, 14, 15}
    };
    int slot;

    for (slot = 0; slot < 3; ++slot) {
        assert(sSceneCommands[offset + (size_t)slot] ==
               expected[selected_row][slot]);
    }
}

static void AssertConfirmationTweens(void)
{
    assert(sVariantOneCount == 1);
    assert(sConfigureCount == 2U);
    assert(sConfigureCalls[0].tween ==
           &gGameDatalinkControllerTweens[4]);
    assert(sConfigureCalls[0].x_fx == 255 * 0x1000);
    assert(sConfigureCalls[0].y_fx == -32 * 0x1000);
    assert(sConfigureCalls[1].tween ==
           &gGameDatalinkControllerTweens[23]);
    assert(sConfigureCalls[1].x_fx == 24 * 0x1000);
    assert(sConfigureCalls[1].y_fx == 204 * 0x1000);
    assert(sConfigureCalls[0].duration == 12U);
    assert(sConfigureCalls[1].duration == 12U);
    assert(sConfigureCalls[0].mode == 1);
    assert(sConfigureCalls[1].mode == 1);
}

static void TestIncompleteSelectionTransitionUsesUnselectedCommands(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.selected_row_5b = 1;
    gGameDatalinkControllerActiveSlot = 1;
    sProgress = 0x0FFF;

    assert(Game_UpdateDatalinkControllerSelection(&controller) == 0);
    assert(gGameDatalinkControllerTweens[0].active_03 == 0U);
    assert(gGameDatalinkControllerTweens[1].active_03 == 1U);
    assert(gGameDatalinkControllerTweens[2].active_03 == 0U);
    assert(gGameDatalinkControllerTweens[5].active_03 == 0U);
    assert(gGameDatalinkControllerTweens[7].active_03 == 1U);
    assert(gGameDatalinkControllerTweens[8].active_03 == 1U);
    assert(gGameDatalinkControllerTweens[10].active_03 == 0U);
    assert(sSceneCommandCount == 3U);
    assert(sSceneCommands[0] == 12);
    assert(sSceneCommands[1] == 14);
    assert(sSceneCommands[2] == 16);
    assert(sClampCount == 0);
}

static void TestNavigationUpdatesCursorAndTail(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.selected_row_5b = 0;
    gGameDatalinkControllerInput = 0x80U;

    assert(Game_UpdateDatalinkControllerSelection(&controller) == 0);
    assert(controller.selected_row_5b == 1);
    assert(sSoundCount == 1U);
    assert(sSounds[0] == 0x3DU);
    assert(gGameDatalinkSceneOwner.cursor_transition_duration_56c == 0);
    assert(gGameDatalinkSceneOwner.cursor_x_fx_574 == 48 * 0x1000);
    assert(gGameDatalinkSceneOwner.cursor_y_fx_578 == 137 * 0x1000);
    assert(sSceneCommands[0] == 3);
    AssertTailCommands(1, 1U);
}

static void TestCancelReturnsThreeAfterCursorUpdate(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.selected_row_5b = 2;
    gGameDatalinkControllerInput = 2U;

    assert(Game_UpdateDatalinkControllerSelection(&controller) == 3);
    assert(sSounds[0] == 0x44U);
    assert(sSceneCommands[0] == 3);
    AssertTailCommands(2, 1U);
}

static void TestFirstConfirmationQueuesReturnPhase(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.selected_row_5b = 0;
    gGameDatalinkControllerInput = 1U;

    assert(Game_UpdateDatalinkControllerSelection(&controller) == 0);
    assert(sSounds[0] == 0x3EU);
    AssertConfirmationTweens();
    assert(sLockedOwner == UINT32_C(0x11223344));
    assert(sReturnPushCount == 1);
    assert(sReturnOwner == UINT32_C(0x11223344));
    assert(sReturnPhase == 1U);
    assert(controller.transition_phase_5d == 25U);
}

static void TestSecondConfirmationDispatchesByReadyCount(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.selected_row_5b = 1;
    gGameDatalinkControllerInput = 1U;
    sReadyCount = 0;
    assert(Game_UpdateDatalinkControllerSelection(&controller) == 0);
    assert(sChangeCount == 1);
    assert(sChangeState == 0);
    assert(controller.transition_phase_5d == 5U);

    ResetHarness(&controller);
    controller.selected_row_5b = 1;
    gGameDatalinkControllerInput = 1U;
    sReadyCount = 2;
    assert(Game_UpdateDatalinkControllerSelection(&controller) == 0);
    AssertConfirmationTweens();
    assert(controller.transition_phase_5d == 20U);
}

static void TestThirdConfirmationDispatchesByReadyCount(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.selected_row_5b = 2;
    gGameDatalinkControllerInput = 1U;
    sReadyCount = 0;
    assert(Game_UpdateDatalinkControllerSelection(&controller) == 0);
    assert(sChangeCount == 1);
    assert(sChangeState == 2);
    assert(controller.transition_phase_5d == 0U);

    ResetHarness(&controller);
    controller.selected_row_5b = 2;
    gGameDatalinkControllerInput = 1U;
    sReadyCount = 1;
    assert(Game_UpdateDatalinkControllerSelection(&controller) == 0);
    AssertConfirmationTweens();
    assert(controller.transition_phase_5d == 10U);
}

static void TestCommitAndWaitPhases(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 5U;
    gGameDatalinkControllerInput = 1U;
    assert(Game_UpdateDatalinkControllerSelection(&controller) == 0);
    assert(gGameDatalinkSceneOwner.controller_ready_c27 == 1U);
    assert(sCommitCount == 1);
    assert(controller.transition_phase_5d == 0U);

    ResetHarness(&controller);
    controller.transition_phase_5d = 10U;
    assert(Game_UpdateDatalinkControllerSelection(&controller) == 5);

    ResetHarness(&controller);
    controller.transition_phase_5d = 20U;
    assert(Game_UpdateDatalinkControllerSelection(&controller) == 4);
}

static void TestExitPhaseQueuesInterfaceOnCompletion(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 25U;
    sPrimaryTransitionComplete = true;

    assert(Game_UpdateDatalinkControllerSelection(&controller) == 0);
    assert(sSceneTransitionCount == 1);
    assert(sSceneTransitionAddress == UINT32_C(0x55667788));
}

int main(void)
{
    TestIncompleteSelectionTransitionUsesUnselectedCommands();
    TestNavigationUpdatesCursorAndTail();
    TestCancelReturnsThreeAfterCursorUpdate();
    TestFirstConfirmationQueuesReturnPhase();
    TestSecondConfirmationDispatchesByReadyCount();
    TestThirdConfirmationDispatchesByReadyCount();
    TestCommitAndWaitPhases();
    TestExitPhaseQueuesInterfaceOnCompletion();
    return 0;
}
