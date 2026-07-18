#include "game/datalink_phase_ten_finalizer.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_CLAMP,
    EVENT_CHANGED_SOUND,
    EVENT_CURSOR_COMMAND
} Event;

Game_DatalinkSceneOwner gGameDatalinkSceneOwner;

static Event sEvents[3];
static size_t sEventCount;
static Game_DatalinkFlowState *sExpectedState;
static bool sMutateSelectionDuringSound;
static int8_t sSelectionAfterSound;
static int32_t sExpectedCursorX;
static int32_t sExpectedCursorY;

static void RecordEvent(Event event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

void Game_ClampDatalinkSelection(
    volatile int8_t *selection,
    int minimum,
    int maximum
)
{
    assert(selection == &sExpectedState->selected_participant_40);
    assert(minimum == 0);
    assert(maximum == 2);
    assert(gGameDatalinkSceneOwner.cursor_transition_duration_56c ==
           (int32_t)0xA5A5A5A5U);
    RecordEvent(EVENT_CLAMP);

    if (*selection < minimum) {
        *selection = (int8_t)minimum;
    } else if (*selection > maximum) {
        *selection = (int8_t)maximum;
    }
}

void Game_PlayDatalinkSoundEffect(uint32_t sound_id)
{
    assert(sound_id == 0x3DU);
    assert(gGameDatalinkSceneOwner.cursor_transition_duration_56c ==
           (int32_t)0xA5A5A5A5U);
    RecordEvent(EVENT_CHANGED_SOUND);

    if (sMutateSelectionDuringSound) {
        sExpectedState->selected_participant_40 = sSelectionAfterSound;
    }
}

void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command
)
{
    assert(scene == &gGameDatalinkSceneOwner);
    assert(command == 3);
    assert(scene->cursor_transition_duration_56c == 0);
    assert(scene->cursor_x_fx_574 == sExpectedCursorX);
    assert(scene->cursor_y_fx_578 == sExpectedCursorY);
    RecordEvent(EVENT_CURSOR_COMMAND);
}

static void ResetHarness(Game_DatalinkFlowState *state)
{
    memset(state, 0xA5, sizeof(*state));
    memset(&gGameDatalinkSceneOwner, 0xA5,
           sizeof(gGameDatalinkSceneOwner));
    sExpectedState = state;
    sMutateSelectionDuringSound = false;
    sSelectionAfterSound = 0;
    sExpectedCursorX = 0;
    sExpectedCursorY = 0;
    sEventCount = 0;
}

static void TestChangedSelectionReloadsAgainAfterSound(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state);
    state.selected_participant_40 = 3;
    sMutateSelectionDuringSound = true;
    sSelectionAfterSound = 0;
    sExpectedCursorX = 9 * 0x1000;
    sExpectedCursorY = 29 * 0x1000;

    Game_FinalizeDatalinkPhaseTenSelection(&state, 1);

    assert(state.selected_participant_40 == 0);
    assert(gGameDatalinkSceneOwner.unknown_000[0] == 0xA5U);
    assert(sEventCount == 3);
    assert(sEvents[0] == EVENT_CLAMP);
    assert(sEvents[1] == EVENT_CHANGED_SOUND);
    assert(sEvents[2] == EVENT_CURSOR_COMMAND);
}

static void TestUnchangedSelectionSkipsSound(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state);
    state.selected_participant_40 = 1;
    sExpectedCursorX = 9 * 0x1000;
    sExpectedCursorY = 77 * 0x1000;

    Game_FinalizeDatalinkPhaseTenSelection(&state, 1);

    assert(state.selected_participant_40 == 1);
    assert(sEventCount == 2);
    assert(sEvents[0] == EVENT_CLAMP);
    assert(sEvents[1] == EVENT_CURSOR_COMMAND);
}

int main(void)
{
    TestChangedSelectionReloadsAgainAfterSound();
    TestUnchangedSelectionSkipsSound();
    return 0;
}
