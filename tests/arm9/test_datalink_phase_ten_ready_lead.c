#include "game/datalink_phase_ten_ready_lead.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_FIRST_LEAD,
    EVENT_SECOND_LEAD,
    EVENT_THIRD_LEAD
} Event;

Game_DatalinkPositionObject gGameDatalinkPhaseFiveGateObject;
Game_DatalinkPositionObject gGameDatalinkPhaseTenReadySecondLeadObject;
Game_DatalinkPositionObject gGameDatalinkPhaseTenReadyThirdLeadObject;

static Event sEvents[3];
static size_t sEventCount;
static Game_DatalinkFlowState *sExpectedState;

static void RecordEvent(Event event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

void Game_StartDatalinkObjectTransition(
    Game_DatalinkPositionObject *object,
    int32_t target_x_fx,
    int32_t target_y_fx,
    int32_t duration,
    int mode
)
{
    assert(sExpectedState->phase_41 == 10);
    assert(sExpectedState->selection_dirty_43 == 1);
    assert(duration == 20);
    assert(mode == 2);

    if (object == &gGameDatalinkPhaseFiveGateObject) {
        assert(target_x_fx == 0);
        assert(target_y_fx == 48 * 0x1000);
        RecordEvent(EVENT_FIRST_LEAD);
        return;
    }

    if (object == &gGameDatalinkPhaseTenReadySecondLeadObject) {
        assert(target_x_fx == 0);
        assert(target_y_fx == 48 * 0x1000);
        RecordEvent(EVENT_SECOND_LEAD);
        sExpectedState->selected_participant_40 = 2;
        return;
    }

    assert(object == &gGameDatalinkPhaseTenReadyThirdLeadObject);
    assert(target_x_fx == 9 * 0x1000);
    assert(target_y_fx == 77 * 0x1000);
    RecordEvent(EVENT_THIRD_LEAD);
}

static void TestOrderedTransitionsAndSelectionReload(void)
{
    Game_DatalinkFlowState state;

    memset(&state, 0xA5, sizeof(state));
    memset(&gGameDatalinkPhaseFiveGateObject, 0,
           sizeof(gGameDatalinkPhaseFiveGateObject));
    memset(&gGameDatalinkPhaseTenReadySecondLeadObject, 0,
           sizeof(gGameDatalinkPhaseTenReadySecondLeadObject));
    memset(&gGameDatalinkPhaseTenReadyThirdLeadObject, 0,
           sizeof(gGameDatalinkPhaseTenReadyThirdLeadObject));
    state.selected_participant_40 = 1;
    state.phase_41 = 10;
    sExpectedState = &state;
    sEventCount = 0;

    Game_StartDatalinkPhaseTenReadyLeadTransitions(&state);

    assert(state.selected_participant_40 == 2);
    assert(state.phase_41 == 10);
    assert(state.selection_dirty_43 == 1);
    assert(state.phase_timer_45 == 0xA5U);
    assert(sEventCount == 3);
    assert(sEvents[0] == EVENT_FIRST_LEAD);
    assert(sEvents[1] == EVENT_SECOND_LEAD);
    assert(sEvents[2] == EVENT_THIRD_LEAD);
}

int main(void)
{
    TestOrderedTransitionsAndSelectionReload();
    return 0;
}
