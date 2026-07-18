#include "game/datalink_phase_ten_confirm.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_CONFIRM_SOUND,
    EVENT_READINESS
} Event;

uint8_t gGameDatalinkSceneSelectedParticipant;
uint8_t gGameDatalinkSelectedParticipantMirror;

static Event sEvents[2];
static size_t sEventCount;
static Game_DatalinkFlowState *sExpectedState;
static bool sMutateSelectionDuringSound;
static int8_t sSelectionAfterSound;
static int sReadinessResult;
static int sExpectedReadinessSlot;

static void RecordEvent(Event event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

void Game_PlayDatalinkSoundEffect(uint32_t sound_id)
{
    assert(sound_id == 0x3EU);
    assert(sExpectedState->phase_41 == 10);
    assert(gGameDatalinkSceneSelectedParticipant == 0xA5U);
    assert(gGameDatalinkSelectedParticipantMirror == 0xA5U);
    RecordEvent(EVENT_CONFIRM_SOUND);

    if (sMutateSelectionDuringSound) {
        sExpectedState->selected_participant_40 = sSelectionAfterSound;
    }
}

int Game_IsDatalinkPlayerSlotReady(int slot)
{
    assert(slot == sExpectedReadinessSlot);
    assert(gGameDatalinkSceneSelectedParticipant == (uint8_t)slot);
    assert(gGameDatalinkSelectedParticipantMirror == (uint8_t)slot);
    assert(sExpectedState->phase_41 == 10);
    RecordEvent(EVENT_READINESS);
    return sReadinessResult;
}

static void ResetHarness(Game_DatalinkFlowState *state)
{
    memset(state, 0xA5, sizeof(*state));
    state->phase_41 = 10;
    gGameDatalinkSceneSelectedParticipant = 0xA5U;
    gGameDatalinkSelectedParticipantMirror = 0xA5U;
    sExpectedState = state;
    sMutateSelectionDuringSound = false;
    sSelectionAfterSound = 0;
    sReadinessResult = 0;
    sExpectedReadinessSlot = 0;
    sEventCount = 0;
}

static void TestZeroReadinessSelectsUnavailableBranch(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state);
    state.selected_participant_40 = 1;
    sExpectedReadinessSlot = 1;

    assert(!Game_PrepareDatalinkPhaseTenConfirmation(&state));

    assert(state.phase_41 == 10);
    assert(gGameDatalinkSceneSelectedParticipant == 1U);
    assert(gGameDatalinkSelectedParticipantMirror == 1U);
    assert(sEventCount == 2);
    assert(sEvents[0] == EVENT_CONFIRM_SOUND);
    assert(sEvents[1] == EVENT_READINESS);
}

static void TestSelectionIsReadAfterSoundAndAsUnsignedByte(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state);
    state.selected_participant_40 = 1;
    sMutateSelectionDuringSound = true;
    sSelectionAfterSound = -1;
    sReadinessResult = -7;
    sExpectedReadinessSlot = 255;

    assert(Game_PrepareDatalinkPhaseTenConfirmation(&state));

    assert(state.selected_participant_40 == -1);
    assert(state.phase_41 == 10);
    assert(gGameDatalinkSceneSelectedParticipant == 0xFFU);
    assert(gGameDatalinkSelectedParticipantMirror == 0xFFU);
    assert(state.phase_timer_45 == 0xA5U);
    assert(sEventCount == 2);
    assert(sEvents[0] == EVENT_CONFIRM_SOUND);
    assert(sEvents[1] == EVENT_READINESS);
}

int main(void)
{
    TestZeroReadinessSelectsUnavailableBranch();
    TestSelectionIsReadAfterSoundAndAsUnsignedByte();
    return 0;
}
