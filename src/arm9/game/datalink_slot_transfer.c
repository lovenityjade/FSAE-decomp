#include "game/datalink_slot_transfer.h"

#include "game/datalink_player_slot.h"

/* 0x0205214C dispatches a record to the destination link channel. */
extern void Game_SendDatalinkPlayerSlot(
    int destination_slot,
    const Game_DatalinkPlayerSlot *record
);

/* 0x020521A4 closes or releases the selected link-player channel. */
extern void Game_CloseDatalinkPlayerSlot(int slot);

enum {
    DATALINK_PLAYER_SLOT_MASK = 7,
    DATALINK_TRANSFER_FAILED_STATUS = 7
};

/* 0x020A74CC..0x020A74CF: one bx lr instruction. */
void Game_BeginDatalinkSlotUpdate(void)
{
}

/* 0x020A74D0..0x020A74D3: one bx lr instruction. */
void Game_EndDatalinkSlotUpdate(void)
{
}

/*
 * 0x020A7370..0x020A73E3 (Ghidra body size: 116 bytes).
 *
 * Both inputs are masked with 7.  The source is refreshed first; status zero
 * permits 0x0205214C to send its 0x50-byte record to the destination.  The
 * destination is refreshed afterwards, and any nonzero resulting status is
 * replaced with 7.  Calls to the no-op hooks at 0x020A74CC/74D0 bracket the
 * operation in the original binary.
 */
void Game_TransferDatalinkPlayerSlot(int source_slot, int destination_slot)
{
    int source = source_slot & DATALINK_PLAYER_SLOT_MASK;
    int destination = destination_slot & DATALINK_PLAYER_SLOT_MASK;

    Game_BeginDatalinkSlotUpdate();

    Game_RefreshDatalinkPlayerSlot(source);
    if (gGameDatalinkPlayerSlots[source].status == 0) {
        Game_SendDatalinkPlayerSlot(
            destination,
            &gGameDatalinkPlayerSlots[source]
        );
    }

    Game_RefreshDatalinkPlayerSlot(destination);
    if (gGameDatalinkPlayerSlots[destination].status != 0) {
        gGameDatalinkPlayerSlots[destination].status =
            DATALINK_TRANSFER_FAILED_STATUS;
    }

    Game_EndDatalinkSlotUpdate();
}

/*
 * 0x020A73EC..0x020A740F (Ghidra body size: 36 bytes).
 *
 * The unmodified argument is passed to 0x020521A4, then 0x020A72B0 resets
 * the corresponding masked record.  The same no-op hooks bracket the pair.
 */
void Game_RemoveDatalinkPlayerSlot(int slot)
{
    Game_BeginDatalinkSlotUpdate();
    Game_CloseDatalinkPlayerSlot(slot);
    Game_ResetDatalinkPlayerSlot(slot);
    Game_EndDatalinkSlotUpdate();
}
