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
Game_DatalinkControllerPanelOwner gGameDatalinkControllerPanelOwner;
Game_DatalinkSceneOwner gGameDatalinkSceneOwner;
volatile uint8_t gGameDatalinkControllerActiveSlot;
volatile uint16_t gGameDatalinkControllerInput;
volatile uint32_t gGameDatalinkControllerDisplayControl;

static int32_t sProgress;
static int sReadyCount;
static int sReady[3];
static ConfigureCall sConfigureCalls[4];
static size_t sConfigureCount;
static uint16_t sLoadedResource;
static int sBuildDescriptorCount;
static int sTilemapRefreshCount;
static uint32_t sSounds[2];
static size_t sSoundCount;
static int sChangedState;
static int sChangeCount;
static int sSceneCommands[8];
static size_t sSceneCommandCount;
static int sPopTransitionCount;
static int sCommitCount;
static uint32_t sOwnerAddress;
static uint8_t sOwnerSuppress;
static uint8_t sOwnerDirty;
static int8_t sOwnerSelected;
static int sOwnerRefreshCount;
static int32_t sPairYOffset;
static int32_t sTripletYOffset;
static int sOwnerDescriptorCount;
static int sVariantZeroCount;
static int sTransferSource;
static int sTransferDestination;
static int sTransferCount;

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
    assert(tween == &gGameDatalinkControllerTweens[30]);
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
    assert(sSceneCommandCount <
           sizeof(sSceneCommands) / sizeof(sSceneCommands[0]));
    sSceneCommands[sSceneCommandCount++] = command;
}

void Game_PopDatalinkTransition(uint32_t owner_address)
{
    assert(owner_address == sOwnerAddress);
    ++sPopTransitionCount;
}

void Game_CommitDatalinkScreenUpdate(void)
{
    ++sCommitCount;
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

void Game_PositionDatalinkControllerOwnerPairs(
    uint32_t owner_address,
    int32_t x_offset,
    int32_t y_offset
)
{
    assert(owner_address == sOwnerAddress);
    assert(x_offset == 0);
    sPairYOffset = y_offset;
}

void Game_PositionDatalinkControllerOwnerTriplet(
    uint32_t owner_address,
    int32_t x_offset,
    int32_t y_offset
)
{
    assert(owner_address == sOwnerAddress);
    assert(x_offset == 0);
    sTripletYOffset = y_offset;
}

void Game_RebuildDatalinkControllerOwnerDescriptors(
    uint32_t owner_address
)
{
    assert(owner_address == sOwnerAddress);
    ++sOwnerDescriptorCount;
}

int Game_CountReadyDatalinkPlayerSlots(void)
{
    return sReadyCount;
}

int Game_IsDatalinkPlayerSlotReady(int slot)
{
    assert(slot >= 0 && slot < 3);
    return sReady[slot];
}

void Game_TransferDatalinkPlayerSlot(int source_slot, int destination_slot)
{
    sTransferSource = source_slot;
    sTransferDestination = destination_slot;
    ++sTransferCount;
}

void Game_BuildDatalinkControllerDescriptors(
    const Game_DatalinkController *controller
)
{
    assert(controller != NULL);
    ++sBuildDescriptorCount;
}

void Game_RefreshDatalinkControllerTilemaps(
    Game_DatalinkController *controller
)
{
    assert(controller != NULL);
    ++sTilemapRefreshCount;
}

void Game_SelectDatalinkControllerVariantZero(
    Game_DatalinkController *controller
)
{
    assert(controller != NULL);
    ++sVariantZeroCount;
}

static void ResetHarness(Game_DatalinkController *controller)
{
    memset(controller, 0, sizeof(*controller));
    memset(gGameDatalinkControllerTweens, 0xA5,
           sizeof(gGameDatalinkControllerTweens));
    memset(&gGameDatalinkControllerPanelOwner, 0,
           sizeof(gGameDatalinkControllerPanelOwner));
    memset(&gGameDatalinkSceneOwner, 0,
           sizeof(gGameDatalinkSceneOwner));
    memset(sReady, 0, sizeof(sReady));
    memset(sConfigureCalls, 0, sizeof(sConfigureCalls));
    memset(sSounds, 0, sizeof(sSounds));
    memset(sSceneCommands, 0, sizeof(sSceneCommands));
    sOwnerAddress = UINT32_C(0x11223344);
    controller->owner_address_08 = sOwnerAddress;
    gGameDatalinkControllerActiveSlot = 2U;
    gGameDatalinkControllerInput = 0U;
    gGameDatalinkControllerDisplayControl = UINT32_C(0xFFFFFFFF);
    sProgress = 0x1000;
    sReadyCount = 1;
    sReady[1] = 1;
    sConfigureCount = 0U;
    sLoadedResource = 0U;
    sBuildDescriptorCount = 0;
    sTilemapRefreshCount = 0;
    sSoundCount = 0U;
    sChangedState = -1;
    sChangeCount = 0;
    sSceneCommandCount = 0U;
    sPopTransitionCount = 0;
    sCommitCount = 0;
    sOwnerSuppress = 0xFFU;
    sOwnerDirty = 0U;
    sOwnerSelected = -1;
    sOwnerRefreshCount = 0;
    sPairYOffset = 0;
    sTripletYOffset = 0;
    sOwnerDescriptorCount = 0;
    sVariantZeroCount = 0;
    sTransferSource = -1;
    sTransferDestination = -1;
    sTransferCount = 0;
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
    assert(sSceneCommandCount == 4U);
    assert(sSceneCommands[0] == first);
    assert(sSceneCommands[1] == second);
    assert(sSceneCommands[2] == 30);
    assert(sSceneCommands[3] == 23);
}

static void TestInitializationBuildsTwoMissingParticipants(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.field_30 = 1U;
    gGameDatalinkControllerPanelOwner.first_y_fx_758 = 0x3000;
    gGameDatalinkControllerPanelOwner.second_y_fx_7b0 = 0x5000;

    assert(Game_UpdateDatalinkControllerParticipantSelection(
               &controller
           ) == 0);

    assert(sOwnerSuppress == 1U);
    assert(controller.field_30 == 3U);
    assert(controller.transition_stack_34[1] == 0x3000);
    assert(controller.transition_stack_34[2] == 0x5000);
    assert(controller.participant_count_5a == 2U);
    assert(controller.participant_slots_58[0] == 0U);
    assert(controller.participant_slots_58[1] == 2U);
    assert(controller.selected_row_5b == 0);
    assert(controller.transition_phase_5d == 5U);
    assert(controller.auxiliary_state_5f == 1U);
    assert(sLoadedResource == 0x0836U);
    assert(sBuildDescriptorCount == 1);
    assert(sConfigureCount == 2U);
    AssertConfigureCall(0, 30, 255, 0, 2);
    AssertConfigureCall(1, 23, 24, 180, 2);
    assert(gGameDatalinkControllerTweens[5].current_y_fx_34 ==
           30 * 0x1000);
    assert(gGameDatalinkControllerTweens[6].active_03 == 1U);
    assert(gGameDatalinkControllerTweens[9].current_y_fx_34 ==
           78 * 0x1000);
    assert(gGameDatalinkControllerTweens[10].active_03 == 1U);
    assert(sTilemapRefreshCount == 1);
    AssertTailCommands(5, 10);
}

static void TestSelectPhaseNavigationMarksTilemapDirty(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 5U;
    controller.participant_count_5a = 2U;
    controller.participant_slots_58[0] = 0U;
    controller.participant_slots_58[1] = 2U;
    controller.selected_row_5b = 0;
    controller.auxiliary_state_5f = 0U;
    gGameDatalinkControllerInput = 0x80U;

    assert(Game_UpdateDatalinkControllerParticipantSelection(
               &controller
           ) == 0);
    assert(controller.selected_row_5b == 1);
    assert(controller.auxiliary_state_5f == 1U);
    assert(sSounds[0] == 0x3DU);
    assert(gGameDatalinkSceneOwner.cursor_x_fx_574 == 9 * 0x1000);
    assert(gGameDatalinkSceneOwner.cursor_y_fx_578 == 77 * 0x1000);
    assert(sSceneCommands[0] == 3);
}

static void TestSelectPhaseConfirmStartsTransfer(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 5U;
    controller.participant_count_5a = 2U;
    controller.participant_slots_58[0] = 0U;
    controller.participant_slots_58[1] = 2U;
    gGameDatalinkControllerInput = 1U;

    assert(Game_UpdateDatalinkControllerParticipantSelection(
               &controller
           ) == 0);
    assert(controller.transition_phase_5d == 10U);
    assert(controller.transition_countdown_5e == 60U);
    assert(sChangeCount == 1);
    assert(sChangedState == 3);
    assert((gGameDatalinkControllerDisplayControl & 0x1F00U) ==
           0x1900U);
    assert(gGameDatalinkSceneOwner.controller_overlay_visible_e8f == 0U);
    assert(gGameDatalinkControllerTweens[5].active_03 == 0U);
    assert(gGameDatalinkControllerTweens[10].active_03 == 0U);
}

static void TestSelectPhaseCancelStartsExit(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 5U;
    controller.participant_count_5a = 2U;
    controller.participant_slots_58[0] = 0U;
    controller.participant_slots_58[1] = 2U;
    gGameDatalinkControllerInput = 2U;

    assert(Game_UpdateDatalinkControllerParticipantSelection(
               &controller
           ) == 0);
    assert(sSounds[0] == 0x44U);
    assert(controller.transition_phase_5d == 15U);
    assert(sConfigureCount == 2U);
    AssertConfigureCall(0, 30, 255, -32, 1);
    AssertConfigureCall(1, 23, 24, 204, 1);
    assert(gGameDatalinkControllerTweens[5].active_03 == 0U);
}

static void TestTransferTriggerUsesMappedParticipant(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 10U;
    controller.transition_countdown_5e = 30U;
    controller.selected_row_5b = 1;
    controller.participant_count_5a = 2U;
    controller.participant_slots_58[0] = 0U;
    controller.participant_slots_58[1] = 2U;

    assert(Game_UpdateDatalinkControllerParticipantSelection(
               &controller
           ) == 0);
    assert(sTransferCount == 1);
    assert(sTransferSource == 2);
    assert(sTransferDestination == 2);
    assert(controller.transition_countdown_5e == 29U);
    assert(gGameDatalinkSceneOwner.controller_ready_c27 == 0U);
    assert(sTilemapRefreshCount == 1);
}

static void TestTransferExpiryCommitsOwnerAndReturnsEarly(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 10U;
    controller.transition_countdown_5e = 1U;
    controller.field_30 = 4U;
    controller.selected_row_5b = 1;

    assert(Game_UpdateDatalinkControllerParticipantSelection(
               &controller
           ) == 0);
    assert(controller.field_30 == 2U);
    assert(controller.selected_row_5b == 0);
    assert(gGameDatalinkSceneOwner.controller_ready_c27 == 1U);
    assert(sPopTransitionCount == 1);
    assert(sOwnerSuppress == 0U);
    assert(sOwnerDirty == 1U);
    assert(sOwnerSelected == 0);
    assert(sOwnerRefreshCount == 1);
    assert(sCommitCount == 1);
    assert(sTilemapRefreshCount == 0);
    assert(sSceneCommandCount == 0U);
}

static void TestCancelCompletionRestoresStackAndReturnsTwo(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 15U;
    controller.field_30 = 2U;
    controller.transition_stack_34[0] = 0x3000;
    controller.transition_stack_34[1] = -0x5000;

    assert(Game_UpdateDatalinkControllerParticipantSelection(
               &controller
           ) == 2);
    assert(controller.field_30 == 0U);
    assert(gGameDatalinkControllerPanelOwner.first_y_fx_758 == 0x3000);
    assert(gGameDatalinkControllerPanelOwner.second_y_fx_7b0 == -0x5000);
    assert(sConfigureCount == 1U);
    AssertConfigureCall(0, 23, 24, 180, 2);
    assert(sOwnerSuppress == 0U);
    assert(sPairYOffset == 5);
    assert(sTripletYOffset == 5);
    assert(sOwnerDirty == 1U);
    assert(sOwnerDescriptorCount == 1);
    assert(sVariantZeroCount == 1);
    assert(controller.transition_timer_60 == 0x0835U);
    assert(sTilemapRefreshCount == 0);
    assert(sSceneCommandCount == 0U);
}

int main(void)
{
    TestInitializationBuildsTwoMissingParticipants();
    TestSelectPhaseNavigationMarksTilemapDirty();
    TestSelectPhaseConfirmStartsTransfer();
    TestSelectPhaseCancelStartsExit();
    TestTransferTriggerUsesMappedParticipant();
    TestTransferExpiryCommitsOwnerAndReturnsEarly();
    TestCancelCompletionRestoresStackAndReturnsTwo();
    return 0;
}
