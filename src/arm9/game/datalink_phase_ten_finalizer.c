#include "game/datalink_phase_ten_finalizer.h"

#include <stddef.h>

enum {
    DATALINK_SELECTION_COUNT = 3,
    DATALINK_SELECTION_MIN = 0,
    DATALINK_SELECTION_MAX = 2,
    DATALINK_SELECTION_CHANGED_SOUND_ID = 0x3D,
    DATALINK_CURSOR_COMMAND = 3,
    DATALINK_FX_ONE = 0x1000
};

/* ROM tables 0x020DE914 (X) and 0x020DE918 (Y), eight-byte stride. */
static const int32_t sDatalinkCursorX[DATALINK_SELECTION_COUNT] = {
    9, 9, 9
};
static const int32_t sDatalinkCursorY[DATALINK_SELECTION_COUNT] = {
    29, 77, 125
};

/* Opaque 0x020AC898 signed-byte clamp. */
extern void Game_ClampDatalinkSelection(
    volatile int8_t *selection,
    int minimum,
    int maximum
);

/* Opaque 0x02076FE0 sound-effect dispatcher. */
extern void Game_PlayDatalinkSoundEffect(uint32_t sound_id);

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
    offsetof(Game_DatalinkSceneOwner, cursor_transition_duration_56c) == 0x56C,
    "datalink cursor duration offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, cursor_x_fx_574) == 0x574,
    "datalink cursor X offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, cursor_y_fx_578) == 0x578,
    "datalink cursor Y offset"
);

/*
 * Extracted shared phase-ten finalizer 0x020A864C..0x020A86AF inside the
 * catalogued FUN_020A8198.  This helper is not a promoted Ghidra function.
 *
 * `previous_selection` is the signed value saved by the phase decoder before
 * any action handler.  0x020AC898 clamps state+0x40 to 0..2.  The first
 * volatile reload controls optional sound 0x3D.  After that sound returns,
 * cursor duration at scene+0x56C is cleared and a second volatile selection
 * reload indexes the ROM coordinate tables.  The 20.12 X/Y fields are written
 * before scene command three runs.
 */
void Game_FinalizeDatalinkPhaseTenSelection(
    Game_DatalinkFlowState *state,
    int previous_selection
)
{
    volatile int8_t *selected_participant =
        &state->selected_participant_40;
    volatile int32_t *cursor_duration =
        &gGameDatalinkSceneOwner.cursor_transition_duration_56c;
    volatile int32_t *cursor_x = &gGameDatalinkSceneOwner.cursor_x_fx_574;
    volatile int32_t *cursor_y = &gGameDatalinkSceneOwner.cursor_y_fx_578;
    int clamped_selection;
    int table_selection;

    Game_ClampDatalinkSelection(
        selected_participant,
        DATALINK_SELECTION_MIN,
        DATALINK_SELECTION_MAX
    );
    clamped_selection = *selected_participant;
    if (previous_selection != clamped_selection) {
        Game_PlayDatalinkSoundEffect(
            DATALINK_SELECTION_CHANGED_SOUND_ID
        );
    }

    *cursor_duration = 0;
    table_selection = *selected_participant;
    *cursor_x = sDatalinkCursorX[table_selection] * DATALINK_FX_ONE;
    *cursor_y = sDatalinkCursorY[table_selection] * DATALINK_FX_ONE;
    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkSceneOwner,
        DATALINK_CURSOR_COMMAND
    );
}
