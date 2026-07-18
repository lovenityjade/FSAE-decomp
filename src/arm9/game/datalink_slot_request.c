#include "game/datalink_slot_request.h"

#include "game/datalink_player_slot.h"
#include "game/datalink_slot_transfer.h"

#include <stdint.h>

/*
 * 0x0205214C adds the channel bit and tail-calls 0x0205687C.  Inspection of
 * that backend confirms it consumes only the slot and record arguments.
 */
extern int Game_SubmitDatalinkPlayerSlot(
    int slot,
    Game_DatalinkPlayerSlot *record
);

enum {
    DATALINK_PLAYER_SLOT_MASK = 7,
    DATALINK_NESTED_UPDATE_FLAGS = 0xC0,
    DATALINK_OWNS_UPDATE_FLAG = 0x80,
    DATALINK_REQUEST_RECORD_TYPE = 0x21
};

/*
 * 0x020A7410..0x020A7473 (Ghidra/catalog body size: 100 bytes).
 * Literal pool: 0x020A7474..0x020A747F.
 *
 * The low three bits select the 0x50-byte record.  Byte +0x04 is set to 0x21
 * and status +0x47 to -128 before 0x0205214C submits it.  If neither high
 * control bit 0x40/0x80 is present, the no-op begin/end hooks bracket the
 * submission; callers already inside such a bracket pass bit 0x80.
 *
 * Ghidra initially retained extra formal parameters because r2/r3 and the
 * aligned stack word survive the wrapper.  The direct backend at 0x0205687C
 * does not read them, so the semantic interface has one argument.
 */
int Game_RequestDatalinkPlayerSlot(int slot_flags)
{
    int slot = slot_flags & DATALINK_PLAYER_SLOT_MASK;
    int owns_update =
        (slot_flags & DATALINK_NESTED_UPDATE_FLAGS) == 0;
    Game_DatalinkPlayerSlot *record = &gGameDatalinkPlayerSlots[slot];
    int result;

    if (owns_update) {
        Game_BeginDatalinkSlotUpdate();
    }

    record->unknown_00[4] = DATALINK_REQUEST_RECORD_TYPE;
    record->status = INT8_MIN;
    result = Game_SubmitDatalinkPlayerSlot(slot, record);

    if (owns_update) {
        Game_EndDatalinkSlotUpdate();
    }
    return result;
}

/*
 * 0x020A7480..0x020A74C3 (Ghidra/catalog body size: 68 bytes).
 * Literal pool: 0x020A74C4..0x020A74CB.
 *
 * The input is a valid array index 0..7.  Both record flags +0x2C/+0x2D are
 * set before entering the update bracket.  0x020A7410 is called with bit 0x80
 * so it does not nest the hooks, then 0x020A723C refreshes the same masked
 * slot before the bracket ends.
 */
void Game_StartDatalinkPlayerSlot(int slot)
{
    Game_DatalinkPlayerSlot *record = &gGameDatalinkPlayerSlots[slot];

    record->reset_flag_2c = 1;
    record->reset_flag_2d = 1;

    Game_BeginDatalinkSlotUpdate();
    (void)Game_RequestDatalinkPlayerSlot(slot | DATALINK_OWNS_UPDATE_FLAG);
    Game_RefreshDatalinkPlayerSlot(slot | DATALINK_OWNS_UPDATE_FLAG);
    Game_EndDatalinkSlotUpdate();
}
