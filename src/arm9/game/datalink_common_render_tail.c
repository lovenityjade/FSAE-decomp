#include "game/datalink_common_render_tail.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_RENDER_ROW_COUNT = 3,
    DATALINK_RENDER_PREAMBLE_COMMAND = 4,
    DATALINK_RENDER_FINAL_COMMAND = 0x17
};

/* ROM map 0x020DE7D0, copied to the parent function's stack preamble. */
static const uint8_t
    sDatalinkRowRenderCommands[DATALINK_RENDER_ROW_COUNT][2] = {
        {5, 6},
        {7, 8},
        {9, 10}
    };

/* Opaque true function boundary 0x020A8904. */
extern void Game_RefreshDatalinkSelectionRender(
    Game_DatalinkFlowState *state
);

/* Opaque 0x020AD964 scene-object command. */
extern void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command
);

_Static_assert(
    offsetof(Game_DatalinkFlowState, selected_participant_40) == 0x40,
    "datalink selected participant offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, suppress_redraw_42) == 0x42,
    "datalink redraw suppression offset"
);

/*
 * Semantic common render tail 0x020A87C8..0x020A883B inside the catalogued
 * FUN_020A8198.  This internal block is not promoted in the Ghidra function
 * registry.
 *
 * Scene command four always runs before the volatile +0x42 read.  Suppression
 * returns immediately.  Otherwise the opaque refresh at 0x020A8904 precedes
 * three rows.  Each row performs a fresh signed +0x40 selection read before
 * choosing its selected/unselected ROM command.  Command 0x17 closes the
 * sequence only after all three row commands return.
 */
void Game_RenderDatalinkCommonTail(Game_DatalinkFlowState *state)
{
    const volatile uint8_t *suppress_redraw = &state->suppress_redraw_42;
    const volatile int8_t *selected_participant =
        &state->selected_participant_40;
    int row;

    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkSceneOwner,
        DATALINK_RENDER_PREAMBLE_COMMAND
    );
    if (*suppress_redraw != 0U) {
        return;
    }

    Game_RefreshDatalinkSelectionRender(state);
    for (row = 0; row < DATALINK_RENDER_ROW_COUNT; ++row) {
        int command_index = row == *selected_participant ? 0 : 1;

        Game_ApplyDatalinkSceneCommand(
            &gGameDatalinkSceneOwner,
            sDatalinkRowRenderCommands[row][command_index]
        );
    }
    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkSceneOwner,
        DATALINK_RENDER_FINAL_COMMAND
    );
}
