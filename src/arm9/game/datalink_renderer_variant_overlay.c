#include "game/datalink_renderer_variant_overlay.h"

#include <stddef.h>

enum {
    DATALINK_ROW_STRIDE = 6,
    DATALINK_OVERLAY_X = 2,
    DATALINK_OVERLAY_ROW_OFFSET = 4,
    DATALINK_OVERLAY_WIDTH = 0x1C,
    DATALINK_OVERLAY_HEIGHT = 4,
    DATALINK_SELECTED_PALETTE = 4,
    DATALINK_DEFAULT_PALETTE = 3,
    DATALINK_CLEAR_PARTICIPANT_TYPE = 1
};

/* Opaque state+0x0C[index] target-type lookup used elsewhere in Datalink. */
extern int Game_GetDatalinkParticipantType(
    const Game_DatalinkFlowState *state,
    int participant_index
);

/* Opaque 0x0209B4C0 rectangle clear. */
extern void Game_ClearDatalinkRenderRectangle(
    uint16_t *tilemap,
    int x,
    int y,
    int width,
    int height
);

/* Opaque 0x0209B508 rectangle palette update. */
extern void Game_SetDatalinkRenderRectanglePalette(
    uint16_t *tilemap,
    int x,
    int y,
    int width,
    int height,
    int palette
);

_Static_assert(
    offsetof(Game_DatalinkFlowState, active_participant_index_2c) == 0x2C,
    "datalink variant active participant offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, selected_participant_40) == 0x40,
    "datalink variant selected participant offset"
);

/*
 * Semantic internal helper for the row overlay
 * Range: 0x020A8BD0..0x020A8C7F inside catalogued FUN_020A8904.
 * Selected rows receive palette four.  Other rows clear the four-by-28
 * rectangle from both tilemaps only for participant type one; all remaining
 * types receive palette three on the secondary tilemap.  The primary binding
 * is deliberately loaded after the first clear.  This helper is not promoted
 * in Ghidra.
 */
void Game_RenderDatalinkRowVariantOverlay(
    Game_DatalinkFlowState *state,
    volatile Game_DatalinkRenderPrimaryBinding *buffers,
    uint16_t *secondary_tilemap,
    uint8_t slot
)
{
    const volatile int8_t *selected = &state->selected_participant_40;
    const volatile int8_t *active_index =
        &state->active_participant_index_2c;
    int row = slot * DATALINK_ROW_STRIDE + DATALINK_OVERLAY_ROW_OFFSET;

    if (*selected == slot) {
        Game_SetDatalinkRenderRectanglePalette(
            secondary_tilemap,
            DATALINK_OVERLAY_X,
            row,
            DATALINK_OVERLAY_WIDTH,
            DATALINK_OVERLAY_HEIGHT,
            DATALINK_SELECTED_PALETTE
        );
        return;
    }

    if (Game_GetDatalinkParticipantType(state, *active_index) ==
        DATALINK_CLEAR_PARTICIPANT_TYPE) {
        Game_ClearDatalinkRenderRectangle(
            secondary_tilemap,
            DATALINK_OVERLAY_X,
            row,
            DATALINK_OVERLAY_WIDTH,
            DATALINK_OVERLAY_HEIGHT
        );
        Game_ClearDatalinkRenderRectangle(
            (uint16_t *)(void *)buffers->primary_buffer,
            DATALINK_OVERLAY_X,
            row,
            DATALINK_OVERLAY_WIDTH,
            DATALINK_OVERLAY_HEIGHT
        );
        return;
    }

    Game_SetDatalinkRenderRectanglePalette(
        secondary_tilemap,
        DATALINK_OVERLAY_X,
        row,
        DATALINK_OVERLAY_WIDTH,
        DATALINK_OVERLAY_HEIGHT,
        DATALINK_DEFAULT_PALETTE
    );
}
