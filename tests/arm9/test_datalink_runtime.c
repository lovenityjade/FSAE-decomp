#include "game/datalink_runtime.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum EventKind {
    EVENT_BASE_CONSTRUCT,
    EVENT_INTERFACE_INIT,
    EVENT_RESOURCE_INIT,
    EVENT_REFRESH_SLOTS,
    EVENT_BASE_FINALIZE
} EventKind;

typedef struct Event {
    EventKind kind;
    uint32_t value;
    Game_DatalinkRuntime *runtime;
} Event;

Game_DatalinkStaticArena gGameDatalinkStaticArena;
uint8_t gGameLocalPlayerSlot;

static Event sEvents[7];
static size_t sEventCount;

static void RecordEvent(
    EventKind kind,
    uint32_t value,
    Game_DatalinkRuntime *runtime
)
{
    Event *event;

    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    event = &sEvents[sEventCount++];
    event->kind = kind;
    event->value = value;
    event->runtime = runtime;
}

void Game_ConstructDatalinkRuntimeBase(Game_DatalinkRuntime *runtime)
{
    RecordEvent(EVENT_BASE_CONSTRUCT, 0, runtime);
    runtime->field_04 = UINT32_C(0xFFFFFFFF);
}

void Game_InitializeDatalinkInterface(uint32_t interface_address)
{
    RecordEvent(EVENT_INTERFACE_INIT, interface_address, NULL);
}

void Game_InitializeDatalinkResource(int resource_id)
{
    RecordEvent(EVENT_RESOURCE_INIT, (uint32_t)resource_id, NULL);
}

void Game_RefreshAllDatalinkPlayerSlots(void)
{
    RecordEvent(EVENT_REFRESH_SLOTS, 0, NULL);
}

void Game_FinalizeDatalinkRuntimeBase(
    Game_DatalinkRuntime *runtime,
    uint32_t primary_interface_address
)
{
    RecordEvent(EVENT_BASE_FINALIZE, primary_interface_address, runtime);
}

static void ExpectCallOrder(Game_DatalinkRuntime *runtime)
{
    static const EventKind kinds[] = {
        EVENT_BASE_CONSTRUCT,
        EVENT_INTERFACE_INIT,
        EVENT_INTERFACE_INIT,
        EVENT_INTERFACE_INIT,
        EVENT_RESOURCE_INIT,
        EVENT_REFRESH_SLOTS,
        EVENT_BASE_FINALIZE
    };
    static const uint32_t values[] = {
        0,
        GAME_DATALINK_INTERFACE_A_ADDRESS,
        GAME_DATALINK_INTERFACE_B_ADDRESS,
        GAME_DATALINK_INTERFACE_C_ADDRESS,
        UINT32_C(0x193),
        0,
        GAME_DATALINK_INTERFACE_A_ADDRESS
    };
    size_t index;

    assert(sEventCount == sizeof(kinds) / sizeof(kinds[0]));
    for (index = 0; index < sEventCount; ++index) {
        assert(sEvents[index].kind == kinds[index]);
        assert(sEvents[index].value == values[index]);
    }
    assert(sEvents[0].runtime == runtime);
    assert(sEvents[6].runtime == runtime);
}

static void TestFirstConstructionInitializesArenaAndRuntime(void)
{
    Game_DatalinkRuntime runtime;

    memset(&gGameDatalinkStaticArena, 0xA5, sizeof(gGameDatalinkStaticArena));
    gGameDatalinkStaticArena.guard_004 = 2;
    gGameDatalinkStaticArena.guard_014 = 2;
    gGameDatalinkStaticArena.guard_018 = 2;
    memset(&runtime, 0xCC, sizeof(runtime));
    gGameLocalPlayerSlot = 6;
    sEventCount = 0;

    Game_ConstructDatalinkRuntime(&runtime);

    assert(gGameDatalinkStaticArena.guard_004 == 3);
    assert(gGameDatalinkStaticArena.guard_014 == 3);
    assert(gGameDatalinkStaticArena.guard_018 == 3);
    assert(gGameDatalinkStaticArena.interface_a_vtable_050 == UINT32_C(0x02124B40));
    assert(gGameDatalinkStaticArena.interface_b_vtable_160 == UINT32_C(0x02124B64));
    assert(
        gGameDatalinkStaticArena.interface_c_primary_vtable_0b0 ==
        UINT32_C(0x021250E0)
    );
    assert(
        gGameDatalinkStaticArena.interface_c_secondary_vtable_0e0 ==
        UINT32_C(0x02125108)
    );

    assert(runtime.field_04 == 0);
    assert(runtime.interface_a_34 == GAME_DATALINK_INTERFACE_A_ADDRESS);
    assert(runtime.interface_b_38 == GAME_DATALINK_INTERFACE_B_ADDRESS);
    assert(runtime.interface_c_3c == GAME_DATALINK_INTERFACE_C_ADDRESS);
    assert(runtime.local_player_40 == 6);
    assert(runtime.state_41 == 0);
    assert(runtime.unknown_42 == 0xCC);
    assert(runtime.state_43 == 0);
    assert(runtime.unknown_44[0] == 0xCC);
    assert(runtime.pending_count_48 == 0);
    ExpectCallOrder(&runtime);
}

static void TestInitializedGuardsPreserveExistingVtables(void)
{
    Game_DatalinkRuntime runtime = {0};

    gGameDatalinkStaticArena.guard_004 = 1;
    gGameDatalinkStaticArena.guard_014 = 1;
    gGameDatalinkStaticArena.guard_018 = 1;
    gGameDatalinkStaticArena.interface_a_vtable_050 = UINT32_C(0x11111111);
    gGameDatalinkStaticArena.interface_b_vtable_160 = UINT32_C(0x22222222);
    gGameDatalinkStaticArena.interface_c_primary_vtable_0b0 = UINT32_C(0x33333333);
    gGameDatalinkStaticArena.interface_c_secondary_vtable_0e0 = UINT32_C(0x44444444);
    sEventCount = 0;

    Game_ConstructDatalinkRuntime(&runtime);

    assert(gGameDatalinkStaticArena.interface_a_vtable_050 == UINT32_C(0x11111111));
    assert(gGameDatalinkStaticArena.interface_b_vtable_160 == UINT32_C(0x22222222));
    assert(
        gGameDatalinkStaticArena.interface_c_primary_vtable_0b0 ==
        UINT32_C(0x33333333)
    );
    assert(
        gGameDatalinkStaticArena.interface_c_secondary_vtable_0e0 ==
        UINT32_C(0x44444444)
    );
    ExpectCallOrder(&runtime);
}

int main(void)
{
    TestFirstConstructionInitializesArenaAndRuntime();
    TestInitializedGuardsPreserveExistingVtables();
    return 0;
}
