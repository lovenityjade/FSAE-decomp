#include "game/datalink_controller.h"

#include "game/datalink_phase_ten_finalizer.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_VARIANT_ONE,
    EVENT_CONFIGURE,
    EVENT_PROGRESS,
    EVENT_MARK_DIRTY,
    EVENT_POP_TRANSITION,
    EVENT_LOAD_RESOURCE,
    EVENT_SCENE_COMMAND
} Event;

typedef struct ConfigureCall {
    Game_DatalinkTweenRecord *tween;
    int32_t x_fx;
    int32_t y_fx;
} ConfigureCall;

Game_DatalinkTweenRecord gGameDatalinkControllerTweens[32];
Game_DatalinkTweenRecord gGameDatalinkControllerPrimaryTween;
Game_DatalinkTweenRecord gGameDatalinkControllerSecondaryTween;
Game_DatalinkSceneOwner gGameDatalinkSceneOwner;

static Event sEvents[20];
static size_t sEventCount;
static ConfigureCall sConfigureCalls[11];
static size_t sConfigureCount;
static int sCommands[3];
static size_t sCommandCount;
static int32_t sProgress;
static uint32_t sExpectedFlowAddress;
static Game_DatalinkFlowState sBoundFlowState;

static void RecordEvent(Event event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

void Game_SelectDatalinkControllerVariantOne(
    Game_DatalinkController *controller
)
{
    assert(controller != NULL);
    RecordEvent(EVENT_VARIANT_ONE);
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
    assert(duration == 20U);
    assert(mode == 1);
    call = &sConfigureCalls[sConfigureCount++];
    call->tween = tween;
    call->x_fx = target_x_fx;
    call->y_fx = target_y_fx;
    RecordEvent(EVENT_CONFIGURE);
}

int32_t Game_GetDatalinkTweenProgress(
    const Game_DatalinkTweenRecord *tween
)
{
    assert(tween == &gGameDatalinkControllerTweens[11]);
    RecordEvent(EVENT_PROGRESS);
    return sProgress;
}

static void AssertCompletionTweensActive(void)
{
    static const int active_indices[] = {0, 5, 6, 1, 7, 8, 2, 9, 10};
    size_t index;

    for (index = 0;
         index < sizeof(active_indices) / sizeof(active_indices[0]);
         ++index) {
        assert(gGameDatalinkControllerTweens[
                   active_indices[index]
               ].active_03 == 1U);
    }
    assert(gGameDatalinkControllerTweens[3].active_03 == 0xA5U);
    assert(gGameDatalinkControllerTweens[4].active_03 == 0xA5U);
    assert(gGameDatalinkControllerTweens[11].active_03 == 0xA5U);
}

void Game_MarkDatalinkFlowRenderDirty(uint32_t flow_state_address)
{
    assert(flow_state_address == sExpectedFlowAddress);
    AssertCompletionTweensActive();
    sBoundFlowState.selection_dirty_43 = 1U;
    RecordEvent(EVENT_MARK_DIRTY);
}

void Game_PopDatalinkTransition(uint32_t flow_state_address)
{
    assert(flow_state_address == sExpectedFlowAddress);
    assert(sBoundFlowState.selection_dirty_43 == 1U);
    RecordEvent(EVENT_POP_TRANSITION);
}

void Game_LoadDatalinkScreenResource(uint16_t resource_id)
{
    assert(resource_id == 0x0834U);
    RecordEvent(EVENT_LOAD_RESOURCE);
}

void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command
)
{
    assert(scene == &gGameDatalinkSceneOwner);
    assert(sCommandCount < sizeof(sCommands) / sizeof(sCommands[0]));
    sCommands[sCommandCount++] = command;
    RecordEvent(EVENT_SCENE_COMMAND);
}

static void ResetHarness(Game_DatalinkController *controller)
{
    memset(controller, 0, sizeof(*controller));
    memset(gGameDatalinkControllerTweens, 0xA5,
           sizeof(gGameDatalinkControllerTweens));
    memset(&gGameDatalinkControllerPrimaryTween, 0xA5,
           sizeof(gGameDatalinkControllerPrimaryTween));
    memset(&gGameDatalinkControllerSecondaryTween, 0xA5,
           sizeof(gGameDatalinkControllerSecondaryTween));
    memset(&sBoundFlowState, 0, sizeof(sBoundFlowState));
    sEventCount = 0U;
    sConfigureCount = 0U;
    sCommandCount = 0U;
    sProgress = 0;
    sExpectedFlowAddress = UINT32_C(0x11223344);
    controller->owner_address_08 = sExpectedFlowAddress;
}

static void AssertConfigureCall(
    size_t index,
    Game_DatalinkTweenRecord *tween,
    int32_t x,
    int32_t y
)
{
    assert(sConfigureCalls[index].tween == tween);
    assert(sConfigureCalls[index].x_fx == x * 0x1000);
    assert(sConfigureCalls[index].y_fx == y * 0x1000);
}

static void TestPhaseZeroConfiguresElevenTweensThenCommands(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 0U;
    controller.selected_row_5b = 1;

    assert(Game_UpdateDatalinkControllerAnimation(
               &controller, 1U, 2U, 3U
           ) == 0);

    assert(controller.transition_phase_5d == 1U);
    assert(sConfigureCount == 11U);
    AssertConfigureCall(0, &gGameDatalinkControllerPrimaryTween, 0, 0);
    AssertConfigureCall(1, &gGameDatalinkControllerSecondaryTween, 0, 0);
    AssertConfigureCall(2, &gGameDatalinkControllerTweens[0], 44, 48);
    AssertConfigureCall(3, &gGameDatalinkControllerTweens[5], 24, 30);
    AssertConfigureCall(4, &gGameDatalinkControllerTweens[6], 24, 30);
    AssertConfigureCall(5, &gGameDatalinkControllerTweens[1], 44, 96);
    AssertConfigureCall(6, &gGameDatalinkControllerTweens[7], 24, 78);
    AssertConfigureCall(7, &gGameDatalinkControllerTweens[8], 24, 78);
    AssertConfigureCall(8, &gGameDatalinkControllerTweens[2], 44, 144);
    AssertConfigureCall(9, &gGameDatalinkControllerTweens[9], 24, 126);
    AssertConfigureCall(10, &gGameDatalinkControllerTweens[10], 24, 126);
    assert(sEventCount == 15U);
    assert(sEvents[0] == EVENT_VARIANT_ONE);
    assert(sEvents[11] == EVENT_CONFIGURE);
    assert(sCommands[0] == 12);
    assert(sCommands[1] == 13);
    assert(sCommands[2] == 16);
}

static void TestPhaseOneWaitsWithoutCompletionEffects(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 1U;
    controller.selected_row_5b = -1;
    sProgress = 0x0FFF;

    assert(Game_UpdateDatalinkControllerAnimation(
               &controller, 0U, 0U, 0U
           ) == 0);

    assert(sEventCount == 4U);
    assert(sEvents[0] == EVENT_PROGRESS);
    assert(sCommands[0] == 12);
    assert(sCommands[1] == 14);
    assert(sCommands[2] == 16);
    assert(gGameDatalinkControllerTweens[0].active_03 == 0xA5U);
    assert(sBoundFlowState.selection_dirty_43 == 0U);
}

static void TestPhaseOneCompletionActivatesAndTransitionsInOrder(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 1U;
    controller.selected_row_5b = 0;
    sProgress = 0x1000;

    assert(Game_UpdateDatalinkControllerAnimation(
               &controller, 0U, 0U, 0U
           ) == 0);

    assert(sEventCount == 7U);
    assert(sEvents[0] == EVENT_PROGRESS);
    assert(sEvents[1] == EVENT_MARK_DIRTY);
    assert(sEvents[2] == EVENT_POP_TRANSITION);
    assert(sEvents[3] == EVENT_LOAD_RESOURCE);
    assert(sCommands[0] == 11);
    assert(sCommands[1] == 14);
    assert(sCommands[2] == 16);
}

static void TestLaterPhaseOnlyIssuesSelectionCommands(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.transition_phase_5d = 2U;
    controller.selected_row_5b = 2;

    assert(Game_UpdateDatalinkControllerAnimation(
               &controller, 0U, 0U, 0U
           ) == 0);

    assert(sEventCount == 3U);
    assert(sCommands[0] == 12);
    assert(sCommands[1] == 14);
    assert(sCommands[2] == 15);
}

int main(void)
{
    TestPhaseZeroConfiguresElevenTweensThenCommands();
    TestPhaseOneWaitsWithoutCompletionEffects();
    TestPhaseOneCompletionActivatesAndTransitionsInOrder();
    TestLaterPhaseOnlyIssuesSelectionCommands();
    return 0;
}
