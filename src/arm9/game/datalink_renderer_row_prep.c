#include "game/datalink_renderer_row_prep.h"

#include "game/datalink_slot_readiness.h"

/*
 * Semantic internal helper for the coherent operation at
 * Range: 0x020A89CC..0x020A89EF inside catalogued FUN_020A8904.  The parent
 * clears exactly ten 32-bit local row values before querying readiness for the
 * current low-byte slot.  The volatile destination keeps this source free of
 * an implicit memset/linker dependency.  This helper is not promoted in the
 * Ghidra function registry.
 */
bool Game_PrepareDatalinkRenderRow(
    uint32_t values[GAME_DATALINK_RENDER_ROW_VALUE_COUNT],
    uint8_t slot
)
{
    volatile uint32_t *destination = values;
    int index;

    for (index = 0; index < GAME_DATALINK_RENDER_ROW_VALUE_COUNT; ++index) {
        destination[index] = 0;
    }
    return Game_IsDatalinkPlayerSlotReady(slot) != 0;
}
