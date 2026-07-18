#include "game/datalink_controller.h"

#include "game/datalink_phase_fifteen.h"
#include "game/datalink_phase_ten_finalizer.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_BASE_CONSTRUCT,
    EVENT_INTERFACE_INITIALIZE,
    EVENT_PUSH_TRANSITION,
    EVENT_LOAD_RESOURCE,
    EVENT_REBUILD_SCREEN,
    EVENT_SELECT_VARIANT_ZERO
} Event;

Game_DatalinkStaticArena gGameDatalinkStaticArena;
Game_DatalinkCompletionOwner gGameDatalinkCompletionOwner;
Game_DatalinkSceneOwner gGameDatalinkSceneOwner;

static Event sEvents[4];
static size_t sEventCount;
static Game_DatalinkController *sExpectedController;
static Game_DatalinkController *sSelectedController;

static void RecordEvent(Event event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

void Game_ConstructDatalinkControllerBase(
    Game_DatalinkController *controller
)
{
    assert(controller == sExpectedController);
    assert((gGameDatalinkStaticArena.guard_00c & 1U) != 0U);
    RecordEvent(EVENT_BASE_CONSTRUCT);
    controller->mode_04 = UINT32_C(0xAAAAAAAA);
}

void Game_InitializeDatalinkInterface(uint32_t interface_address)
{
    assert(interface_address == GAME_DATALINK_INTERFACE_D_ADDRESS);
    assert(sExpectedController->interface_address_54 ==
           GAME_DATALINK_INTERFACE_D_ADDRESS);
    assert(sExpectedController->selected_row_5b == 0U);
    assert(sExpectedController->transition_pending_5c == 0U);
    assert(sExpectedController->auxiliary_state_5f == 0U);
    assert(sExpectedController->mode_04 == 1U);
    assert(sExpectedController->field_30 == UINT32_C(0xCCCCCCCC));
    RecordEvent(EVENT_INTERFACE_INITIALIZE);
}

void Game_PushDatalinkControllerTransition(
    Game_DatalinkSceneOwner *scene,
    uint32_t interface_address,
    uint32_t transition_argument,
    Game_DatalinkController *controller
)
{
    assert(scene == &gGameDatalinkSceneOwner);
    assert(interface_address == GAME_DATALINK_INTERFACE_D_ADDRESS);
    assert(transition_argument == UINT32_C(0x12345678));
    assert(controller == sExpectedController);
    RecordEvent(EVENT_PUSH_TRANSITION);
}

void Game_LoadDatalinkScreenResource(uint16_t resource_id)
{
    assert(resource_id == 0x0835U);
    assert(sExpectedController->transition_pending_5c == 1U);
    assert(sExpectedController->transition_phase_5d == 0U);
    assert(sExpectedController->selected_row_5b == 0U);
    assert(sExpectedController->transition_timer_60 == 0U);
    RecordEvent(EVENT_LOAD_RESOURCE);
}

void Game_RebuildDatalinkScreen(void)
{
    RecordEvent(EVENT_REBUILD_SCREEN);
}

void Game_SelectDatalinkControllerVariantZero(
    Game_DatalinkController *controller
)
{
    sSelectedController = controller;
    RecordEvent(EVENT_SELECT_VARIANT_ZERO);
}

static void ResetController(Game_DatalinkController *controller)
{
    memset(controller, 0xCC, sizeof(*controller));
    sExpectedController = controller;
    sEventCount = 0U;
}

static void TestInitializationSetsGuardBeforeBaseAndClearsFieldLast(void)
{
    Game_DatalinkController controller;

    memset(&gGameDatalinkStaticArena, 0, sizeof(gGameDatalinkStaticArena));
    ResetController(&controller);

    Game_InitializeDatalinkController(&controller);

    assert(gGameDatalinkStaticArena.guard_00c == 1U);
    assert(gGameDatalinkStaticArena.interface_d_vtable_100 ==
           UINT32_C(0x02124AF8));
    assert(controller.field_30 == 0U);
    assert(sEventCount == 2U);
    assert(sEvents[0] == EVENT_BASE_CONSTRUCT);
    assert(sEvents[1] == EVENT_INTERFACE_INITIALIZE);
}

static void TestInitializedGuardPreservesExistingVtable(void)
{
    Game_DatalinkController controller;

    gGameDatalinkStaticArena.guard_00c = 3U;
    gGameDatalinkStaticArena.interface_d_vtable_100 =
        UINT32_C(0x55667788);
    ResetController(&controller);

    Game_InitializeDatalinkController(&controller);

    assert(gGameDatalinkStaticArena.guard_00c == 3U);
    assert(gGameDatalinkStaticArena.interface_d_vtable_100 ==
           UINT32_C(0x55667788));
}

static void TestCompletedOwnerPushesTransitionWithoutReset(void)
{
    Game_DatalinkController controller;

    ResetController(&controller);
    controller.interface_address_54 = GAME_DATALINK_INTERFACE_D_ADDRESS;
    gGameDatalinkCompletionOwner.completion_flag_d0 = 1U;

    Game_BeginDatalinkControllerTransition(
        &controller,
        UINT32_C(0xAAAAAAAA),
        UINT32_C(0x12345678),
        UINT32_C(0xBBBBBBBB)
    );

    assert(sEventCount == 1U);
    assert(sEvents[0] == EVENT_PUSH_TRANSITION);
    assert(controller.transition_pending_5c == 0xCCU);
    assert((uint8_t)controller.selected_row_5b == 0xCCU);
    assert(controller.transition_timer_60 == 0xCCCCU);
}

static void TestIncompleteOwnerResetsAndLoadsResource(void)
{
    Game_DatalinkController controller;

    ResetController(&controller);
    gGameDatalinkCompletionOwner.completion_flag_d0 = 0U;

    Game_BeginDatalinkControllerTransition(
        &controller,
        1U,
        2U,
        3U
    );

    assert(sEventCount == 1U);
    assert(sEvents[0] == EVENT_LOAD_RESOURCE);
}

static void TestTrampolineAndPreparedWrapper(void)
{
    Game_DatalinkController controller;

    sEventCount = 0U;
    sSelectedController = NULL;

    Game_RebuildDatalinkControllerScreen();
    assert(Game_PrepareDatalinkControllerVariant(&controller) == 2);

    assert(sEventCount == 2U);
    assert(sEvents[0] == EVENT_REBUILD_SCREEN);
    assert(sEvents[1] == EVENT_SELECT_VARIANT_ZERO);
    assert(sSelectedController == &controller);
}

int main(void)
{
    TestInitializationSetsGuardBeforeBaseAndClearsFieldLast();
    TestInitializedGuardPreservesExistingVtable();
    TestCompletedOwnerPushesTransitionWithoutReset();
    TestIncompleteOwnerResetsAndLoadsResource();
    TestTrampolineAndPreparedWrapper();
    return 0;
}
