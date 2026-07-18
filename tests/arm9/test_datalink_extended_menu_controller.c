#include "game/datalink_extended_menu_controller.h"

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
Game_DatalinkExtendedMenuStaticInterface
    gGameDatalinkExtendedMenuStaticInterface;
Game_DatalinkCompletionOwner gGameDatalinkCompletionOwner;
Game_DatalinkSceneOwner gGameDatalinkSceneOwner;
uint8_t gGameDatalinkAbortUpdate;
uint32_t gGameDatalinkEffectArgument10;
uint32_t gGameDatalinkEffectArgument14;
volatile int8_t gGameDatalinkExtendedMenuFallbackState;

static Event sEvents[6];
static size_t sEventCount;
static Game_DatalinkExtendedMenuController *sExpectedController;
static uint32_t sExpectedTransitionArgument;
static uint32_t sExpectedTransitionOwner;
static uint16_t sLoadedResource;

static void RecordEvent(Event event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

void Game_ConstructDatalinkExtendedMenuControllerBase(
    Game_DatalinkExtendedMenuController *controller
)
{
    assert(controller == sExpectedController);
    assert((gGameDatalinkStaticArena.guard_000 & 1U) != 0U);
    assert((gGameDatalinkStaticArena.guard_01c & 1U) != 0U);
    RecordEvent(EVENT_BASE_CONSTRUCT);
}

void Game_InitializeDatalinkInterface(uint32_t interface_address)
{
    if (interface_address ==
        GAME_DATALINK_EXTENDED_MENU_PRIMARY_INTERFACE_ADDRESS) {
        RecordEvent(EVENT_PRIMARY_INITIALIZE);
    } else {
        assert(interface_address ==
               GAME_DATALINK_EXTENDED_MENU_SECONDARY_INTERFACE_ADDRESS);
        RecordEvent(EVENT_SECONDARY_INITIALIZE);
    }
}

void Game_FinalizeDatalinkExtendedMenuControllerBase(
    Game_DatalinkExtendedMenuController *controller,
    uint32_t primary_interface_address
)
{
    assert(controller == sExpectedController);
    assert(primary_interface_address ==
           GAME_DATALINK_EXTENDED_MENU_PRIMARY_INTERFACE_ADDRESS);
    assert(controller->mode_04 == 3U);
    assert(controller->history_depth_38 == UINT32_C(0xCCCCCCCC));
    assert((uint8_t)controller->selected_row_5e == 0xCCU);
    assert(controller->transition_state_5c == UINT16_C(0xCCCC));
    RecordEvent(EVENT_BASE_FINALIZE);
}

void Game_PushDatalinkExtendedMenuTransition(
    Game_DatalinkSceneOwner *scene,
    uint32_t secondary_interface_address,
    uint32_t transition_argument,
    uint32_t transition_owner_address
)
{
    assert(scene == &gGameDatalinkSceneOwner);
    assert(secondary_interface_address ==
           GAME_DATALINK_EXTENDED_MENU_SECONDARY_INTERFACE_ADDRESS);
    assert(transition_argument == sExpectedTransitionArgument);
    assert(transition_owner_address == sExpectedTransitionOwner);
    RecordEvent(EVENT_PUSH_TRANSITION);
}

void Game_LoadDatalinkScreenResource(uint16_t resource_id)
{
    sLoadedResource = resource_id;
    RecordEvent(EVENT_LOAD_RESOURCE);
}

static void ResetHarness(Game_DatalinkExtendedMenuController *controller)
{
    memset(controller, 0xCC, sizeof(*controller));
    memset(&gGameDatalinkStaticArena, 0,
           sizeof(gGameDatalinkStaticArena));
    memset(&gGameDatalinkExtendedMenuStaticInterface, 0,
           sizeof(gGameDatalinkExtendedMenuStaticInterface));
    memset(&gGameDatalinkCompletionOwner, 0,
           sizeof(gGameDatalinkCompletionOwner));
    memset(&gGameDatalinkSceneOwner, 0,
           sizeof(gGameDatalinkSceneOwner));
    memset(sEvents, 0, sizeof(sEvents));
    sExpectedController = controller;
    sExpectedTransitionArgument = UINT32_C(0x12345678);
    sExpectedTransitionOwner = UINT32_C(0x87654321);
    sEventCount = 0U;
    sLoadedResource = 0U;
    gGameDatalinkAbortUpdate = 0U;
    gGameDatalinkExtendedMenuFallbackState = 0;
}

static void TestConstructorInitializesCompoundStaticInterfaces(void)
{
    Game_DatalinkExtendedMenuController controller;

    ResetHarness(&controller);
    Game_InitializeDatalinkExtendedMenuController(&controller);

    assert(gGameDatalinkStaticArena.guard_000 == 1U);
    assert(gGameDatalinkStaticArena.guard_01c == 1U);
    assert(gGameDatalinkStaticArena.extended_menu_primary_vtable_020 ==
           UINT32_C(0x02124B40));
    assert(gGameDatalinkExtendedMenuStaticInterface.primary_vtable_00 ==
           UINT32_C(0x02126414));
    assert(gGameDatalinkExtendedMenuStaticInterface.secondary_vtable_30 ==
           UINT32_C(0x0212643C));
    assert(gGameDatalinkExtendedMenuStaticInterface.shared_vtable_d0 ==
           UINT32_C(0x02124B40));
    assert(controller.mode_04 == 3U);
    assert(controller.primary_interface_address_30 ==
           GAME_DATALINK_EXTENDED_MENU_PRIMARY_INTERFACE_ADDRESS);
    assert(controller.secondary_interface_address_34 ==
           GAME_DATALINK_EXTENDED_MENU_SECONDARY_INTERFACE_ADDRESS);
    assert(controller.history_depth_38 == 0U);
    assert(controller.transition_state_5c == 0U);
    assert(controller.selected_row_5e == 0);
    assert(controller.transition_phase_5f == 0U);
    assert(controller.force_final_phase_60 == 0U);
    assert(sEventCount == 4U);
    assert(sEvents[0] == EVENT_BASE_CONSTRUCT);
    assert(sEvents[1] == EVENT_PRIMARY_INITIALIZE);
    assert(sEvents[2] == EVENT_SECONDARY_INITIALIZE);
    assert(sEvents[3] == EVENT_BASE_FINALIZE);
}

static void TestConstructorPreservesInitializedStaticInterfaces(void)
{
    Game_DatalinkExtendedMenuController controller;

    ResetHarness(&controller);
    gGameDatalinkStaticArena.guard_000 = 3U;
    gGameDatalinkStaticArena.guard_01c = 5U;
    gGameDatalinkStaticArena.extended_menu_primary_vtable_020 =
        UINT32_C(0x11111111);
    gGameDatalinkExtendedMenuStaticInterface.primary_vtable_00 =
        UINT32_C(0x22222222);

    Game_InitializeDatalinkExtendedMenuController(&controller);
    assert(gGameDatalinkStaticArena.guard_000 == 3U);
    assert(gGameDatalinkStaticArena.guard_01c == 5U);
    assert(gGameDatalinkStaticArena.extended_menu_primary_vtable_020 ==
           UINT32_C(0x11111111));
    assert(gGameDatalinkExtendedMenuStaticInterface.primary_vtable_00 ==
           UINT32_C(0x22222222));
}

static void TestCompletionFourForwardsTransitionAndReturns(void)
{
    Game_DatalinkExtendedMenuController controller;

    ResetHarness(&controller);
    controller.secondary_interface_address_34 =
        GAME_DATALINK_EXTENDED_MENU_SECONDARY_INTERFACE_ADDRESS;
    controller.history_depth_38 = 2U;
    gGameDatalinkAbortUpdate = 1U;
    gGameDatalinkCompletionOwner.completion_flag_d1 = 4U;

    Game_ResumeDatalinkExtendedMenuController(
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

static void TestFallbackMinusOneForcesFinalPhase(void)
{
    Game_DatalinkExtendedMenuController controller;

    ResetHarness(&controller);
    controller.history_depth_38 = 0U;
    controller.force_final_phase_60 = 0U;
    controller.transition_state_5c = UINT16_C(0xFFFF);
    gGameDatalinkAbortUpdate = 1U;
    gGameDatalinkCompletionOwner.completion_flag_d1 = 3U;
    gGameDatalinkExtendedMenuFallbackState = -1;

    Game_ResumeDatalinkExtendedMenuController(
        &controller,
        0U,
        sExpectedTransitionArgument,
        sExpectedTransitionOwner
    );
    assert(gGameDatalinkAbortUpdate == 0U);
    assert(gGameDatalinkExtendedMenuFallbackState == 0);
    assert(controller.force_final_phase_60 == 1U);
    assert(controller.transition_state_5c == 0U);
    assert(controller.transition_phase_5f == 7U);
    assert(sLoadedResource == 0x083CU);
}

static void TestHistoryPopSelectsZeroOrOneResumePhase(void)
{
    Game_DatalinkExtendedMenuController controller;

    ResetHarness(&controller);
    controller.force_final_phase_60 = 0U;
    controller.history_depth_38 = 2U;
    controller.phase_history_3c[1] = 9;

    Game_ResumeDatalinkExtendedMenuController(
        &controller,
        0U,
        sExpectedTransitionArgument,
        sExpectedTransitionOwner
    );
    assert(controller.history_depth_38 == 1U);
    assert(controller.transition_phase_5f == 1U);
    assert(controller.transition_state_5c == 0U);

    ResetHarness(&controller);
    controller.force_final_phase_60 = 0U;
    controller.history_depth_38 = 1U;
    controller.phase_history_3c[0] = 0;
    Game_ResumeDatalinkExtendedMenuController(
        &controller,
        0U,
        sExpectedTransitionArgument,
        sExpectedTransitionOwner
    );
    assert(controller.history_depth_38 == 0U);
    assert(controller.transition_phase_5f == 0U);
}

int main(void)
{
    TestConstructorInitializesCompoundStaticInterfaces();
    TestConstructorPreservesInitializedStaticInterfaces();
    TestCompletionFourForwardsTransitionAndReturns();
    TestFallbackMinusOneForcesFinalPhase();
    TestHistoryPopSelectsZeroOrOneResumePhase();
    return 0;
}
