#include "game/datalink_player_slot.h"

#include <stddef.h>
#include <string.h>

/*
 * 0x02052160 adds the link-channel bit to the masked slot and dispatches to
 * helper 0x020562EC, filling or updating the supplied 0x50-byte record.
 */
extern int Game_ReadDatalinkPlayerSlot(
    int slot,
    Game_DatalinkPlayerSlot *record
);

_Static_assert(sizeof(Game_DatalinkPlayerSlot) == 0x50, "datalink slot stride");
_Static_assert(
    offsetof(Game_DatalinkPlayerSlot, reset_flag_2c) == 0x2C,
    "datalink reset flag A offset"
);
_Static_assert(
    offsetof(Game_DatalinkPlayerSlot, reset_flag_2d) == 0x2D,
    "datalink reset flag B offset"
);
_Static_assert(
    offsetof(Game_DatalinkPlayerSlot, status) == 0x47,
    "datalink status offset"
);

enum {
    DATALINK_PLAYER_SLOT_MASK = 7
};

static int Game_MaskDatalinkPlayerSlot(int slot)
{
    return slot & DATALINK_PLAYER_SLOT_MASK;
}

/*
 * 0x020A72B0
 *
 * The index is masked with 7 and multiplied by the 0x50-byte record stride.
 * 0x02025A50 clears exactly 0x50 bytes, after which bytes +0x2C and +0x2D
 * are both set to one.
 */
void Game_ResetDatalinkPlayerSlot(int slot)
{
    Game_DatalinkPlayerSlot *record =
        &gGameDatalinkPlayerSlots[Game_MaskDatalinkPlayerSlot(slot)];

    memset(record, 0, sizeof(*record));
    record->reset_flag_2c = 1;
    record->reset_flag_2d = 1;
}

static int Game_DatalinkStatusResetsSlot(int status)
{
    return status == 3 || status == 4 || status == 6 || status == 8;
}

/*
 * 0x020A723C
 *
 * The result of 0x02052160 drives a nine-entry jump table.  Results 3, 4, 6,
 * and 8 call 0x020A72B0; every other result skips reset.  The low byte of the
 * original result is then stored at record+0x47, including after a reset.
 * Callers at 0x020A738C, 0x020A73BC and 0x020A74B8 repeatedly use this helper
 * while updating the link-player records.
 */
void Game_RefreshDatalinkPlayerSlot(int slot)
{
    int masked_slot = Game_MaskDatalinkPlayerSlot(slot);
    Game_DatalinkPlayerSlot *record = &gGameDatalinkPlayerSlots[masked_slot];
    int status = Game_ReadDatalinkPlayerSlot(masked_slot, record);

    if (Game_DatalinkStatusResetsSlot(status)) {
        Game_ResetDatalinkPlayerSlot(masked_slot);
    }
    record->status = (int8_t)status;
}
