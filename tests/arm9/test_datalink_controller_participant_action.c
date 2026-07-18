#include "game/datalink_controller.h"

#include "game/datalink_phase_ten_finalizer.h"

#include <assert.h>
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
Game_DatalinkControllerPanelOwner gGameDatalinkControllerPanelOwner;
Game_DatalinkSceneOwner gGameDatalinkSceneOwner;
volatile uint8_t gGameDatalinkControllerActiveSlot;
volatile uint16_t gGameDatalinkControllerInput;
volatile uint32_t gGameDatalinkControllerDisplayControl;

static ConfigureCall sConfigureCalls[8];
static size_t sConfigureCount;
static int32_t sProgress;
static uint16_t sLoadedResource;
static uint32_t sSounds[2];
static size_t sSoundCount;
static int sChangedState;
static int sChangeCount;
static int sCommands[8];
static size_t sCommandCount;
static int sRemovedSlot;
static int sRemoveCount;
static int sCommitCount;
static uint32_t sOwnerAddress;
static int sPopCount;
static uint8_t sOwnerSuppress;
static uint8_t sOwnerDirty;
static int8_t sOwnerSelected;
static int sOwnerRefreshCount;

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

int32_t Game_GetDatalinkTweenProgress(
    const Game_DatalinkTweenRecord *tween
)
{
    assert(tween == &gGameDatalinkControllerTweens[17]);
    return sProgress;
}

void Game_LoadDatalinkScreenResource(uint16_t resource_id)
{
    sLoadedResource = resource_id;
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
    ++sChangeCount;
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

void Game_RemoveDatalinkPlayerSlot(int slot)
{
    sRemovedSlot = slot;
    ++sRemoveCount;
}

void Game_CommitDatalinkScreenUpdate(void)
{
    ++sCommitCount;
}

void Game_PopDatalinkTransition(uint32_t owner_address)
{
    assert(owner_address == sOwnerAddress);
    ++sPopCount;
}

void Game_SetDatalinkOwnerSuppressRedraw(
    uint32_t owner_address,
    uint8_t suppress
)
{
    assert(owner_address == sOwnerAddress);
    sOwnerSuppress = suppress;
}

void Game_SetDatalinkOwnerSelectionDirty(
    uint32_t owner_address,
    uint8_t dirty
)
{
    assert(owner_address == sOwnerAddress);
    sOwnerDirty = dirty;
}

void Game_SetDatalinkOwnerSelectedParticipant(
    uint32_t owner_address,
    int8_t participant
)
{
    assert(owner_address == sOwnerAddress);
    sOwnerSelected = participant;
}

void Game_InvokeDatalinkOwnerRefresh(uint32_t owner_address)
{
    assert(owner_address == sOwnerAddress);
    ++sOwnerRefreshCount;
}

static void ResetHarness(Game_DatalinkController *controller)
{
    memset(controller, 0, sizeof(*controller));
    memset(gGameDatalinkControllerTweens, 0xA5,
           sizeof(gGameDatalinkControllerTweens));
    memset(&gGameDatalinkSceneOwner, 0,
           sizeof(gGameDatalinkSceneOwner));
    memset(sConfigureCalls, 0, sizeof(sConfigureCalls));
    memset(sSounds, 0, sizeof(sSounds));
    memset(sCommands, 0, sizeof(sCommands));
    sOwnerAddress = UINT32_C(0x11223344);
    controller->owner_address_08 = sOwnerAddress;
    gGameDatalinkControllerActiveSlot = 2U;
    gGameDatalinkControllerInput = 0U;
    gGameDatalinkControllerDisplayControl = UINT32_C(0xFFFFFFFF);
    sConfigureCount = 0U;
    sProgress = 0x1000;
    sLoadedResource = 0U;
    sSoundCount = 0U;
    sChangedState = -1;
    sChangeCount = 0;
    sCommandCount = 0U;
    sRemovedSlot = -1;
    sRemoveCount = 0;
    sCommitCount = 0;
    sPopCount = 0;
    sOwnerSuppress = 0xFFU;
    sOwnerDirty = 0U;
    sOwnerSelected = -1;
    sOwnerRefreshCount = 0;
}

static void AssertConfigureCall(
    size_t index,
    int tween_index,
    int x,
    int y,
    int mode
)
{
    const ConfigureCall *call = &sConfigureCalls[index];

    assert(call->tween == &gGameDatalinkControllerTweens[tween_index]);
    assert(call->x_fx == x * 0x1000);
    assert(call->y_fx == y * 0x1000);
    assert(call->duration == 12U);
    assert(call->mode == mode);
}

static void AssertTailCommands(int first, int second)
{
    assert(sCommandCount == 3U);
    assert(sCommands[0] == first);
    assert(sCommands[1] == second);
    assert(sCommands[2] == 31);
}

static void TestInitializationSeedsAndAnimatesTwoRows(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);

    assert(Game_UpdateDatalinkControllerParticipantAction(
               &controller
           ) == 0);
    assert(controller.transition_phase_5d == 5U);
    assert(controller.selected_row_5b == 0);
    assert(sLoadedResource == 0x0838U);
    assert(sConfigureCount == 6U);
    AssertConfigureCall(0, 17, 84, 112, 2);
    AssertConfigureCall(1, 18, 84, 112, 2);
    AssertConfigureCall(2, 15, 172, 112, 2);
    AssertConfigureCall(3, 16, 172, 112, 2);
    AssertConfigureCall(4, 31, 255, 0, 2);
    AssertConfigureCall(5, 23, 24, 180, 2);
    assert(gGameDatalinkControllerTweens[17].active_03 == 1U);
    assert(gGameDatalinkControllerTweens[17].duration_fx_28 == 0U);
    assert(gGameDatalinkControllerTweens[17].current_x_fx_30 ==
           84 * 0x1000);
    assert(gGameDatalinkControllerTweens[17].current_y_fx_34 ==
           176 * 0x1000);
    assert(gGameDatalinkSceneOwner.participant_action_visible_ee7 == 1U);
    assert(gGameDatalinkSceneOwner.participant_action_x_fx_f14 ==
           255 * 0x1000);
    assert(gGameDatalinkSceneOwner.participant_action_y_fx_f18 ==
           -32 * 0x1000);
    AssertTailCommands(17, 16);
}

static void TestSelectionNavigationUpdatesCursor(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 5U;
    controller.selected_row_5b = 0;
    gGameDatalinkControllerInput = 0x10U;

    assert(Game_UpdateDatalinkControllerParticipantAction(
               &controller
           ) == 0);
    assert(controller.selected_row_5b == 1);
    assert(sSounds[0] == 0x3DU);
    assert(gGameDatalinkSceneOwner.cursor_x_fx_574 == 136 * 0x1000);
    assert(gGameDatalinkSceneOwner.cursor_y_fx_578 == 113 * 0x1000);
    assert(sCommands[0] == 3);
    assert(sCommands[1] == 18);
    assert(sCommands[2] == 15);
    assert(sCommands[3] == 31);
}

static void TestRemoveChoiceStartsTimedRemoval(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 5U;
    controller.selected_row_5b = 1;
    gGameDatalinkSceneOwner.participant_action_visible_ee7 = 1U;
    gGameDatalinkSceneOwner.controller_ready_c27 = 1U;
    gGameDatalinkControllerInput = 1U;

    assert(Game_UpdateDatalinkControllerParticipantAction(
               &controller
           ) == 0);
    assert(controller.transition_phase_5d == 15U);
    assert(controller.transition_countdown_5e == 60U);
    assert(sChangedState == 7);
    assert(sChangeCount == 1);
    assert((gGameDatalinkControllerDisplayControl & 0x1F00U) ==
           0x1900U);
    assert(gGameDatalinkControllerTweens[17].active_03 == 0U);
    assert(gGameDatalinkControllerTweens[16].active_03 == 0U);
    assert(gGameDatalinkControllerTweens[5].active_03 == 0U);
    assert(gGameDatalinkControllerTweens[10].active_03 == 0U);
    assert(gGameDatalinkControllerTweens[0].active_03 == 0U);
    assert(gGameDatalinkControllerTweens[2].active_03 == 0U);
    assert(gGameDatalinkSceneOwner.participant_action_visible_ee7 == 0U);
    assert(gGameDatalinkSceneOwner.controller_ready_c27 == 0U);
}

static void TestRemovalTriggerAndCommitTransition(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 15U;
    controller.transition_countdown_5e = 30U;

    assert(Game_UpdateDatalinkControllerParticipantAction(
               &controller
           ) == 0);
    assert(sRemoveCount == 1);
    assert(sRemovedSlot == 2);
    assert(controller.transition_countdown_5e == 29U);

    ResetHarness(&controller);
    controller.transition_phase_5d = 15U;
    controller.transition_countdown_5e = 1U;

    assert(Game_UpdateDatalinkControllerParticipantAction(
               &controller
           ) == 0);
    assert(controller.transition_phase_5d == 16U);
    assert(controller.transition_countdown_5e == 2U);
    assert(sCommitCount == 1);
}

static void TestOwnerCommitRefreshesAndResetsSelection(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 16U;
    controller.transition_countdown_5e = 1U;
    controller.selected_row_5b = 1;

    assert(Game_UpdateDatalinkControllerParticipantAction(
               &controller
           ) == 0);
    assert(controller.transition_countdown_5e == 0U);
    assert(controller.selected_row_5b == 0);
    assert(sPopCount == 1);
    assert(sOwnerSuppress == 0U);
    assert(sOwnerDirty == 1U);
    assert(sOwnerSelected == 0);
    assert(sOwnerRefreshCount == 1);
    AssertTailCommands(17, 16);
}

static void TestExitStartsAllTweens(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 20U;

    assert(Game_UpdateDatalinkControllerParticipantAction(
               &controller
           ) == 0);
    assert(controller.transition_phase_5d == 25U);
    assert(sConfigureCount == 6U);
    AssertConfigureCall(0, 17, 84, 176, 1);
    AssertConfigureCall(1, 18, 84, 176, 1);
    AssertConfigureCall(2, 15, 172, 176, 1);
    AssertConfigureCall(3, 16, 172, 176, 1);
    AssertConfigureCall(4, 31, 255, -32, 1);
    AssertConfigureCall(5, 23, 24, 204, 1);
    assert(sSounds[0] == 0x44U);
}

static void TestExitCompletionReturnsOneAndSchedulesResource(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 25U;

    assert(Game_UpdateDatalinkControllerParticipantAction(
               &controller
           ) == 1);
    assert(controller.transition_timer_60 == 0x0835U);
    assert(sConfigureCount == 1U);
    AssertConfigureCall(0, 23, 24, 180, 2);
    AssertTailCommands(17, 16);

    ResetHarness(&controller);
    controller.transition_phase_5d = 25U;
    sProgress = 0;
    assert(Game_UpdateDatalinkControllerParticipantAction(
               &controller
           ) == 0);
    assert(controller.transition_timer_60 == 0U);
    assert(sConfigureCount == 0U);
}

int main(void)
{
    TestInitializationSeedsAndAnimatesTwoRows();
    TestSelectionNavigationUpdatesCursor();
    TestRemoveChoiceStartsTimedRemoval();
    TestRemovalTriggerAndCommitTransition();
    TestOwnerCommitRefreshesAndResetsSelection();
    TestExitStartsAllTweens();
    TestExitCompletionReturnsOneAndSchedulesResource();
    return 0;
}
