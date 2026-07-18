#include "game/datalink_player_slot.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_CLEANUP_SLOT_COUNT = 3,
    DATALINK_CLEANUP_PERSIST_STATUS = 6,
    DATALINK_CLEANUP_RESET_STATUS = 8,
    DATALINK_CLEANUP_CLEARED_STATUS = 0
};

/* Opaque 0x02056998 per-slot persistent-state write. */
extern void Game_PersistDatalinkPlayerSlot(int slot);

/* Opaque 0x02056738 persistent-state commit; its result is ignored here. */
extern int Game_CommitDatalinkPersistentState(void);

_Static_assert(
    sizeof(Game_DatalinkPlayerSlot) == 0x50,
    "datalink cleanup slot stride"
);
_Static_assert(
    offsetof(Game_DatalinkPlayerSlot, status) == 0x47,
    "datalink cleanup status offset"
);

/*
 * True catalogued function 0x020A88A8..0x020A88FC (88-byte body).  Its sole
 * literal at 0x020A8900 is the slot-array address 0x021350B0; the next true
 * function begins at 0x020A8904.
 *
 * Slots zero through two are visited in ascending order.  A fresh signed
 * status read drives each slot: status six is persisted first and then falls
 * through to the same reset path as status eight.  The status byte is cleared
 * after reset returns.  All other values are untouched.  The global
 * persistent-state commit runs once after all three slots regardless of their
 * statuses.
 */
/* 0x020A88A8 */
void Game_CleanupDatalinkPlayerSlots(void)
{
    int slot;

    for (slot = 0; slot < DATALINK_CLEANUP_SLOT_COUNT; ++slot) {
        volatile int8_t *status = &gGameDatalinkPlayerSlots[slot].status;
        int status_value = *status;

        if (status_value == DATALINK_CLEANUP_PERSIST_STATUS) {
            Game_PersistDatalinkPlayerSlot(slot);
        } else if (status_value != DATALINK_CLEANUP_RESET_STATUS) {
            continue;
        }

        Game_ResetDatalinkPlayerSlot(slot);
        *status = DATALINK_CLEANUP_CLEARED_STATUS;
    }

    (void)Game_CommitDatalinkPersistentState();
}
