#include "game/datalink_runtime_update.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum EventKind {
    EVENT_EFFECT,
    EVENT_PROCESS,
    EVENT_INTERFACE_UPDATE
} EventKind;

typedef struct Event {
    EventKind kind;
    uint32_t value;
} Event;

uint8_t gGameDatalinkAbortUpdate;
uint32_t gGameDatalinkEffectArgument10;
uint32_t gGameDatalinkEffectArgument14;

static Event sEvents[3];
static size_t sEventCount;
static int sProcessReplacementIndex;

static void RecordEvent(EventKind kind, uint32_t value)
{
    Event *event;

    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    event = &sEvents[sEventCount++];
    event->kind = kind;
    event->value = value;
}

void Game_UpdateDatalinkInterface(uint32_t interface_address)
{
    RecordEvent(EVENT_INTERFACE_UPDATE, interface_address);
}

void Game_TriggerDatalinkPendingEffect(
    int event_id,
    uint32_t argument_14,
    int zero,
    uint32_t argument_10
)
{
    assert(event_id == 1);
    assert(argument_14 == gGameDatalinkEffectArgument14);
    assert(zero == 0);
    assert(argument_10 == gGameDatalinkEffectArgument10);
    RecordEvent(EVENT_EFFECT, 1);
}

void Game_ProcessDatalinkRuntimeState(Game_DatalinkRuntimeUpdateState *state)
{
    assert(state->state_43 == 1);
    RecordEvent(EVENT_PROCESS, 0);
    if (sProcessReplacementIndex >= 0) {
        state->current_interface_2c = (int8_t)sProcessReplacementIndex;
    }
}

static void ResetHarness(Game_DatalinkRuntimeUpdateState *state)
{
    memset(state, 0, sizeof(*state));
    gGameDatalinkAbortUpdate = 0;
    gGameDatalinkEffectArgument10 = UINT32_C(0x10101010);
    gGameDatalinkEffectArgument14 = UINT32_C(0x14141414);
    sEventCount = 0;
    sProcessReplacementIndex = -1;
}

static void TestAbortPathUpdatesOnlyInterfaceB(void)
{
    Game_DatalinkRuntimeUpdateState state;

    ResetHarness(&state);
    state.interface_b_38 = UINT32_C(0xBBBBBBBB);
    state.state_41 = 9;
    state.state_43 = 8;
    state.pending_count_48 = 2;
    gGameDatalinkAbortUpdate = 1;

    Game_UpdateDatalinkRuntime(&state);

    assert(sEventCount == 1);
    assert(sEvents[0].kind == EVENT_INTERFACE_UPDATE);
    assert(sEvents[0].value == UINT32_C(0xBBBBBBBB));
    assert(state.state_41 == 9);
    assert(state.state_43 == 8);
    assert(state.pending_count_48 == 2);
}

static void TestEmptyStackClearsStateAndUsesPostProcessInterface(void)
{
    Game_DatalinkRuntimeUpdateState state;

    ResetHarness(&state);
    state.state_41 = 9;
    state.current_interface_2c = 2;
    state.interfaces_0c[2] = UINT32_C(0x22222222);
    state.interfaces_0c[3] = UINT32_C(0x33333333);
    sProcessReplacementIndex = 3;

    Game_UpdateDatalinkRuntime(&state);

    assert(state.state_41 == 0);
    assert(state.state_43 == 1);
    assert(state.pending_count_48 == 0);
    assert(sEventCount == 2);
    assert(sEvents[0].kind == EVENT_PROCESS);
    assert(sEvents[1].kind == EVENT_INTERFACE_UPDATE);
    assert(sEvents[1].value == UINT32_C(0x33333333));
}

static void TestPendingStackPopsLastValueBeforeEffect(void)
{
    Game_DatalinkRuntimeUpdateState state;

    ResetHarness(&state);
    state.pending_count_48 = 3;
    state.pending_values_4c[0] = UINT32_C(0x11);
    state.pending_values_4c[1] = UINT32_C(0x22);
    state.pending_values_4c[2] = UINT32_C(0x123456AB);
    state.current_interface_2c = 1;
    state.interfaces_0c[1] = UINT32_C(0x11111111);

    Game_UpdateDatalinkRuntime(&state);

    assert(state.pending_count_48 == 2);
    assert(state.state_41 == 0xAB);
    assert(sEventCount == 3);
    assert(sEvents[0].kind == EVENT_EFFECT);
    assert(sEvents[1].kind == EVENT_PROCESS);
    assert(sEvents[2].kind == EVENT_INTERFACE_UPDATE);
    assert(sEvents[2].value == UINT32_C(0x11111111));
}

int main(void)
{
    TestAbortPathUpdatesOnlyInterfaceB();
    TestEmptyStackClearsStateAndUsesPostProcessInterface();
    TestPendingStackPopsLastValueBeforeEffect();
    return 0;
}
