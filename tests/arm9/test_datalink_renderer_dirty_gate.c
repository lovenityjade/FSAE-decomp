#include "game/datalink_renderer_dirty_gate.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

static void ExpectDirtyResult(uint8_t dirty, bool expected)
{
    Game_DatalinkFlowState state;

    memset(&state, 0xA5, sizeof(state));
    state.selection_dirty_43 = dirty;

    assert(Game_ShouldRebuildDatalinkRender(&state) == expected);
    assert(state.selection_dirty_43 == dirty);
    assert((uint8_t)state.selected_participant_40 == 0xA5U);
    assert(state.phase_timer_45 == 0xA5U);
}

int main(void)
{
    ExpectDirtyResult(0, false);
    ExpectDirtyResult(1, true);
    ExpectDirtyResult(0xFF, true);
    return 0;
}
