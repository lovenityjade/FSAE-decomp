#include "game/datalink_slot_readiness.h"

#include "game/datalink_player_slot.h"

#include <stdint.h>

enum {
    DATALINK_READY_SLOT_COUNT = 3,
    DATALINK_PAYLOAD_MARKER_OFFSET = 8
};

static uint16_t Game_GetDatalinkPayloadMarker(
    const Game_DatalinkPlayerSlot *record
)
{
    return (uint16_t)(
        (uint16_t)record->unknown_00[DATALINK_PAYLOAD_MARKER_OFFSET] |
        (uint16_t)(
            (uint16_t)record->unknown_00[DATALINK_PAYLOAD_MARKER_OFFSET + 1]
            << 8
        )
    );
}

/*
 * 0x020A7324
 *
 * Only slots 0..2 are accepted.  The target loads the signed status byte at
 * +0x47, performs an unreachable comparison against positive 128, and then
 * accepts status zero.  A slot is reported ready exactly when its little-
 * endian halfword at +0x08 is also nonzero.
 *
 * Callers use the result as a boolean while constructing player data, notably
 * at 0x020A77EC and 0x020A9FB0.
 */
int Game_IsDatalinkPlayerSlotReady(int slot)
{
    const Game_DatalinkPlayerSlot *record;

    if ((unsigned int)slot >= DATALINK_READY_SLOT_COUNT) {
        return 0;
    }

    record = &gGameDatalinkPlayerSlots[slot];
    if (record->status != 0) {
        return 0;
    }

    return Game_GetDatalinkPayloadMarker(record) != 0U;
}

/*
 * 0x020A72F0
 *
 * Calls 0x020A7324 for slots 0, 1, and 2 and adds the three boolean results.
 * The original masks the two partial sums to eight bits.  Since each helper
 * result is 0 or 1, the observable return range is exactly 0..3.
 * Callers compare this count against 0 and 3, or compute 3-count.
 */
int Game_CountReadyDatalinkPlayerSlots(void)
{
    int ready_count = 0;
    int slot;

    for (slot = 0; slot < DATALINK_READY_SLOT_COUNT; ++slot) {
        ready_count += Game_IsDatalinkPlayerSlotReady(slot);
    }
    return ready_count;
}
