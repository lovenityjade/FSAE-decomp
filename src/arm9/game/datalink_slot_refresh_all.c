#include "game/datalink_slot_refresh_all.h"

#include "game/datalink_player_slot.h"
#include "game/datalink_slot_transfer.h"

enum {
    DATALINK_REFRESHED_PLAYER_SLOT_COUNT = 3
};

/*
 * 0x020A74D4..0x020A74FB (Ghidra/catalog body size: 40 bytes).
 *
 * This is the first real function after the one-instruction hooks at
 * 0x020A74CC and 0x020A74D0.  It opens one update bracket, calls
 * 0x020A723C for slots 0, 1, and 2 in ascending order, and closes the bracket.
 * The next catalog function starts independently at 0x020A74FC.
 */
void Game_RefreshAllDatalinkPlayerSlots(void)
{
    int slot;

    Game_BeginDatalinkSlotUpdate();
    for (slot = 0; slot < DATALINK_REFRESHED_PLAYER_SLOT_COUNT; ++slot) {
        Game_RefreshDatalinkPlayerSlot(slot);
    }
    Game_EndDatalinkSlotUpdate();
}
