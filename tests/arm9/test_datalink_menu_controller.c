#include "game/datalink_menu_controller.h"

#include "game/datalink_phase_fifteen.h"
#include "game/datalink_phase_ten_finalizer.h"
#include "game/datalink_runtime.h"
#include "game/datalink_runtime_update.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_BASE_CONSTRUCT,
    EVENT_PRIMARY_INITIALIZE,
    EVENT_SECONDARY_INITIALIZE,
    EVENT_BASE_FINALIZE,
    EVENT_PUSH_TRANSITION,
    EVENT_LOAD_RESOURCE
} Event;

Game_DatalinkStaticArena gGameDatalinkStaticArena;
Game_DatalinkCompletionOwner gGameDatalinkCompletionOwner;
Game_DatalinkSceneOwner gGameDatalinkSceneOwner;
uint8_t gGameDatalinkAbortUpdate;
uint32_t gGameDatalinkEffectArgument10;
uint32_t gGameDatalinkEffectArgument14;
volatile uint32_t gGameDatalinkMenuSecondaryInterface;

static Event sEvents[6];
static size_t sEventCount;
static Game_DatalinkMenuController *sExpectedController;
static int sPrimaryChoiceAvailable;
static uint16_t sLoadedResource;
static uint32_t sExpectedTransitionArgument;
static uint32_t sExpectedTransitionOwner;

static void RecordEvent(Event event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

void Game_ConstructDatalinkMenuControllerBase(
    Game_DatalinkMenuController *controller
)
{
    assert(controller == sExpectedController);
    assert((gGameDatalinkStaticArena.guard_008 & 1U) != 0U);
    assert((gGameDatalinkStaticArena.guard_010 & 1U) != 0U);
    RecordEvent(EVENT_BASE_CONSTRUCT);
}

void Game_InitializeDatalinkInterface(uint32_t interface_address)
{
    if (interface_address ==
        GAME_DATALINK_MENU_PRIMARY_INTERFACE_ADDRESS) {
        assert(sExpectedController->mode_04 == 2U);
        assert(sExpectedController->primary_interface_address_30 ==
               GAME_DATALINK_MENU_PRIMARY_INTERFACE_ADDRESS);
        assert(sExpectedController->secondary_interface_slot_address_34 ==
               GAME_DATALINK_MENU_SECONDARY_SLOT_ADDRESS);
        RecordEvent(EVENT_PRIMARY_INITIALIZE);
    } else {
        assert(interface_address == UINT32_C(0x02124B1C));
        RecordEvent(EVENT_SECONDARY_INITIALIZE);
    }
}

void Game_FinalizeDatalinkMenuControllerBase(
    Game_DatalinkMenuController *controller,
    uint32_t primary_interface_address
)
{
    assert(controller == sExpectedController);
    assert(primary_interface_address ==
           GAME_DATALINK_MENU_PRIMARY_INTERFACE_ADDRESS);
    assert(controller->history_depth_38 == UINT32_C(0xCCCCCCCC));
    assert((uint8_t)controller->selected_row_5d == 0xCCU);
    RecordEvent(EVENT_BASE_FINALIZE);
}

void Game_PushDatalinkMenuTransition(
    Game_DatalinkSceneOwner *scene,
    uint32_t secondary_interface_slot_address,
    uint32_t transition_argument,
    uint32_t transition_owner_address
)
{
    assert(scene == &gGameDatalinkSceneOwner);
    assert(secondary_interface_slot_address ==
           GAME_DATALINK_MENU_SECONDARY_SLOT_ADDRESS);
    assert(transition_argument == sExpectedTransitionArgument);
    assert(transition_owner_address == sExpectedTransitionOwner);
    RecordEvent(EVENT_PUSH_TRANSITION);
}

int Game_IsDatalinkPrimaryMenuChoiceAvailable(void)
{
    return sPrimaryChoiceAvailable;
}

void Game_LoadDatalinkScreenResource(uint16_t resource_id)
{
    sLoadedResource = resource_id;
    RecordEvent(EVENT_LOAD_RESOURCE);
}

static void ResetHarness(Game_DatalinkMenuController *controller)
{
    memset(controller, 0xCC, sizeof(*controller));
    memset(&gGameDatalinkStaticArena, 0,
           sizeof(gGameDatalinkStaticArena));
    memset(&gGameDatalinkCompletionOwner, 0,
           sizeof(gGameDatalinkCompletionOwner));
    memset(&gGameDatalinkSceneOwner, 0,
           sizeof(gGameDatalinkSceneOwner));
    memset(sEvents, 0, sizeof(sEvents));
    sExpectedController = controller;
    sEventCount = 0U;
    sPrimaryChoiceAvailable = 1;
    sLoadedResource = 0U;
    sExpectedTransitionArgument = UINT32_C(0x12345678);
    sExpectedTransitionOwner = UINT32_C(0x87654321);
    gGameDatalinkAbortUpdate = 0U;
    gGameDatalinkMenuSecondaryInterface = 0U;
}

static void TestConstructorInitializesBothGuardedInterfaces(void)
{
    Game_DatalinkMenuController controller;

    ResetHarness(&controller);
    Game_InitializeDatalinkMenuController(&controller);

    assert(gGameDatalinkStaticArena.guard_008 == 1U);
    assert(gGameDatalinkStaticArena.guard_010 == 1U);
    assert(gGameDatalinkStaticArena.menu_interface_vtable_080 ==
           UINT32_C(0x02124B40));
    assert(gGameDatalinkMenuSecondaryInterface ==
           UINT32_C(0x02124B1C));
    assert(controller.mode_04 == 2U);
    assert(controller.primary_interface_address_30 ==
           GAME_DATALINK_MENU_PRIMARY_INTERFACE_ADDRESS);
    assert(controller.secondary_interface_slot_address_34 ==
           GAME_DATALINK_MENU_SECONDARY_SLOT_ADDRESS);
    assert(controller.history_depth_38 == 0U);
    assert(controller.selected_row_5d == 0);
    assert(sEventCount == 4U);
    assert(sEvents[0] == EVENT_BASE_CONSTRUCT);
    assert(sEvents[1] == EVENT_PRIMARY_INITIALIZE);
    assert(sEvents[2] == EVENT_SECONDARY_INITIALIZE);
    assert(sEvents[3] == EVENT_BASE_FINALIZE);
}

static void TestConstructorPreservesInitializedStaticObjects(void)
{
    Game_DatalinkMenuController controller;

    ResetHarness(&controller);
    gGameDatalinkStaticArena.guard_008 = 5U;
    gGameDatalinkStaticArena.guard_010 = 3U;
    gGameDatalinkStaticArena.menu_interface_vtable_080 =
        UINT32_C(0x11111111);
    gGameDatalinkMenuSecondaryInterface = UINT32_C(0x22222222);

    Game_InitializeDatalinkMenuController(&controller);
    assert(gGameDatalinkStaticArena.guard_008 == 5U);
    assert(gGameDatalinkStaticArena.guard_010 == 3U);
    assert(gGameDatalinkStaticArena.menu_interface_vtable_080 ==
           UINT32_C(0x11111111));
    assert(gGameDatalinkMenuSecondaryInterface ==
           UINT32_C(0x22222222));
}

static void TestCompletionStateForwardsTransitionAndReturns(void)
{
    Game_DatalinkMenuController controller;

    ResetHarness(&controller);
    controller.secondary_interface_slot_address_34 =
        GAME_DATALINK_MENU_SECONDARY_SLOT_ADDRESS;
    controller.history_depth_38 = 2U;
    gGameDatalinkAbortUpdate = 1U;
    gGameDatalinkCompletionOwner.completion_flag_d1 = 3U;

    Game_ResumeDatalinkMenuController(
        &controller,
        UINT32_C(0xAAAAAAAA),
        sExpectedTransitionArgument,
        sExpectedTransitionOwner
    );
    assert(sEventCount == 1U);
    assert(sEvents[0] == EVENT_PUSH_TRANSITION);
    assert(gGameDatalinkAbortUpdate == 1U);
    assert(controller.history_depth_38 == 2U);
    assert(sLoadedResource == 0U);
}

static void TestHistoryPopRestoresSelectionAndPrimaryResource(void)
{
    Game_DatalinkMenuController controller;

    ResetHarness(&controller);
    controller.history_depth_38 = 2U;
    controller.selection_history_3c[0] = 0U;
    controller.selection_history_3c[1] = UINT32_C(0x123456AB);
    controller.selected_row_5d = 7;
    gGameDatalinkAbortUpdate = 1U;
    gGameDatalinkCompletionOwner.completion_flag_d1 = 2U;

    Game_ResumeDatalinkMenuController(
        &controller,
        0U,
        sExpectedTransitionArgument,
        sExpectedTransitionOwner
    );
    assert(gGameDatalinkAbortUpdate == 0U);
    assert(controller.history_depth_38 == 1U);
    assert(controller.transition_phase_5e == 1U);
    assert((uint8_t)controller.selected_row_5d == 0xABU);
    assert(sLoadedResource == 0x083BU);
    assert(sEventCount == 1U);
    assert(sEvents[0] == EVENT_LOAD_RESOURCE);
}

static void TestEmptyHistoryUsesRestrictedDefault(void)
{
    Game_DatalinkMenuController controller;

    ResetHarness(&controller);
    controller.history_depth_38 = 0U;
    controller.transition_phase_5e = 9U;
    controller.selected_row_5d = 0;
    sPrimaryChoiceAvailable = 0;

    Game_ResumeDatalinkMenuController(
        &controller,
        0U,
        sExpectedTransitionArgument,
        sExpectedTransitionOwner
    );
    assert(controller.history_depth_38 == 0U);
    assert(controller.transition_phase_5e == 0U);
    assert(controller.selected_row_5d == 1);
    assert(sLoadedResource == 0x07EFU);
}

int main(void)
{
    TestConstructorInitializesBothGuardedInterfaces();
    TestConstructorPreservesInitializedStaticObjects();
    TestCompletionStateForwardsTransitionAndReturns();
    TestHistoryPopRestoresSelectionAndPrimaryResource();
    TestEmptyHistoryUsesRestrictedDefault();
    return 0;
}
