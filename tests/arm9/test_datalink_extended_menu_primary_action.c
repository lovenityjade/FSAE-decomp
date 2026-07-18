#include "game/datalink_extended_menu_controller.h"

#include "game/datalink_phase_ten_finalizer.h"
#include "game/datalink_player_slot.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum HarnessEvent {
    EVENT_CONTEXT,
    EVENT_RENDER_CONTEXT,
    EVENT_CONNECTION_STATE,
    EVENT_BEGIN_CONNECTION,
    EVENT_WAIT_VBLANK,
    EVENT_UPDATE_CONNECTION,
    EVENT_START_SESSION,
    EVENT_SOUND
} HarnessEvent;

Game_DatalinkSceneOwner gGameDatalinkSceneOwner;
Game_DatalinkPlayerSlot gGameDatalinkPlayerSlots[8];

static HarnessEvent sEvents[24];
static size_t sEventCount;
static uint32_t sSelectionContext;
static uint8_t sRenderContext[4];
static int sConnectionStates[8];
static size_t sConnectionStateCount;
static size_t sConnectionStateIndex;
static int sSessionStartResult;
static uint32_t sStartedContext;
static int sStartedParticipantLimit;
static int sStartedSharedDataStride;
static const void *sStartedPayload;
static uint32_t sStartedPayloadSize;
static uint32_t sSound;

static void RecordEvent(HarnessEvent event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

uint32_t Game_GetDatalinkMenuSelectionContext(void)
{
    RecordEvent(EVENT_CONTEXT);
    return sSelectionContext;
}

uint8_t *Game_GetDatalinkRenderContext(void)
{
    RecordEvent(EVENT_RENDER_CONTEXT);
    return sRenderContext;
}

int Game_GetDatalinkMenuConnectionState(uint32_t selection_context)
{
    RecordEvent(EVENT_CONNECTION_STATE);
    assert(selection_context == sSelectionContext);
    assert(sConnectionStateIndex < sConnectionStateCount);
    return sConnectionStates[sConnectionStateIndex++];
}

void Game_BeginDatalinkPrimaryMenuAction(
    uint32_t selection_context,
    int zero
)
{
    RecordEvent(EVENT_BEGIN_CONNECTION);
    assert(selection_context == sSelectionContext);
    assert(zero == 0);
}

void OS_WaitVBlankIntr(void)
{
    RecordEvent(EVENT_WAIT_VBLANK);
}

void Game_UpdateDatalinkMenuConnection(uint32_t selection_context)
{
    RecordEvent(EVENT_UPDATE_CONNECTION);
    assert(selection_context == sSelectionContext);
}

int Game_StartDatalinkWirelessSession(
    uint32_t selection_context,
    int participant_limit,
    int shared_data_stride,
    const void *slot_payload,
    uint32_t slot_payload_size
)
{
    RecordEvent(EVENT_START_SESSION);
    sStartedContext = selection_context;
    sStartedParticipantLimit = participant_limit;
    sStartedSharedDataStride = shared_data_stride;
    sStartedPayload = slot_payload;
    sStartedPayloadSize = slot_payload_size;
    return sSessionStartResult;
}

void Game_PlayDatalinkSoundEffect(uint32_t sound_id)
{
    RecordEvent(EVENT_SOUND);
    sSound = sound_id;
}

static void ResetHarness(void)
{
    memset(&gGameDatalinkSceneOwner, 0,
           sizeof(gGameDatalinkSceneOwner));
    memset(gGameDatalinkPlayerSlots, 0,
           sizeof(gGameDatalinkPlayerSlots));
    memset(sEvents, 0, sizeof(sEvents));
    memset(sConnectionStates, 0, sizeof(sConnectionStates));
    sEventCount = 0U;
    sSelectionContext = UINT32_C(0x12345678);
    sConnectionStateCount = 0U;
    sConnectionStateIndex = 0U;
    sSessionStartResult = 1;
    sStartedContext = 0U;
    sStartedParticipantLimit = 0;
    sStartedSharedDataStride = 0;
    sStartedPayload = NULL;
    sStartedPayloadSize = 0U;
    sSound = 0U;
}

static void SetConnectionStates(
    const int *states,
    size_t state_count
)
{
    assert(state_count <=
           sizeof(sConnectionStates) / sizeof(sConnectionStates[0]));
    memcpy(sConnectionStates, states, state_count * sizeof(states[0]));
    sConnectionStateCount = state_count;
}

static void AssertEvents(
    const HarnessEvent *events,
    size_t event_count
)
{
    size_t index;

    assert(sEventCount == event_count);
    for (index = 0U; index < event_count; ++index) {
        assert(sEvents[index] == events[index]);
    }
}

static void AssertSessionArguments(int active_slot)
{
    assert(sStartedContext == sSelectionContext);
    assert(sStartedParticipantLimit == 4);
    assert(sStartedSharedDataStride == 0x34);
    assert(sStartedPayload ==
           &gGameDatalinkPlayerSlots[active_slot].unknown_00[8]);
    assert(sStartedPayloadSize == 14U);
}

static void TestReadyConnectionStartsSession(void)
{
    static const int connection_states[] = {2};
    static const HarnessEvent events[] = {
        EVENT_CONTEXT,
        EVENT_RENDER_CONTEXT,
        EVENT_CONNECTION_STATE,
        EVENT_START_SESSION
    };

    ResetHarness();
    SetConnectionStates(connection_states, 1U);
    gGameDatalinkSceneOwner.active_player_slot_53d = 3U;

    assert(Game_PrepareDatalinkExtendedMenuPrimaryAction() == 1);
    AssertEvents(events, sizeof(events) / sizeof(events[0]));
    AssertSessionArguments(3);
}

static void TestDisconnectedStateWaitsUntilReady(void)
{
    static const int connection_states[] = {0, 1, 2};
    static const HarnessEvent events[] = {
        EVENT_CONTEXT,
        EVENT_RENDER_CONTEXT,
        EVENT_CONNECTION_STATE,
        EVENT_BEGIN_CONNECTION,
        EVENT_WAIT_VBLANK,
        EVENT_UPDATE_CONNECTION,
        EVENT_CONNECTION_STATE,
        EVENT_WAIT_VBLANK,
        EVENT_UPDATE_CONNECTION,
        EVENT_CONNECTION_STATE,
        EVENT_START_SESSION
    };

    ResetHarness();
    SetConnectionStates(connection_states, 3U);
    gGameDatalinkSceneOwner.active_player_slot_53d = 1U;

    assert(Game_PrepareDatalinkExtendedMenuPrimaryAction() == 1);
    AssertEvents(events, sizeof(events) / sizeof(events[0]));
    AssertSessionArguments(1);
}

static void TestOtherConnectionStateIsRejected(void)
{
    static const int connection_states[] = {3};
    static const HarnessEvent events[] = {
        EVENT_CONTEXT,
        EVENT_RENDER_CONTEXT,
        EVENT_CONNECTION_STATE
    };

    ResetHarness();
    SetConnectionStates(connection_states, 1U);

    assert(Game_PrepareDatalinkExtendedMenuPrimaryAction() == 0);
    AssertEvents(events, sizeof(events) / sizeof(events[0]));
    assert(sStartedPayload == NULL);
    assert(sSound == 0U);
}

static void TestSessionFailurePlaysSound(void)
{
    static const int connection_states[] = {2};
    static const HarnessEvent events[] = {
        EVENT_CONTEXT,
        EVENT_RENDER_CONTEXT,
        EVENT_CONNECTION_STATE,
        EVENT_START_SESSION,
        EVENT_SOUND
    };

    ResetHarness();
    SetConnectionStates(connection_states, 1U);
    gGameDatalinkSceneOwner.active_player_slot_53d = 7U;
    sSessionStartResult = 0;

    assert(Game_PrepareDatalinkExtendedMenuPrimaryAction() == 0);
    AssertEvents(events, sizeof(events) / sizeof(events[0]));
    AssertSessionArguments(7);
    assert(sSound == 0x40U);
}

int main(void)
{
    TestReadyConnectionStartsSession();
    TestDisconnectedStateWaitsUntilReady();
    TestOtherConnectionStateIsRejected();
    TestSessionFailurePlaysSound();
    return 0;
}
