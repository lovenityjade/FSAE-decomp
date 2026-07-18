#include "game/datalink_status_handler.h"

#include "game/datalink_player_slot.h"

/* External helper 0x02082EE0 receives constant 11 for status values 3 and 4. */
extern void Game_ReportDatalinkStatusEvent(int event_id);

/* External helper 0x020AC038 changes the surrounding state with four integer arguments. */
extern void Game_ChangeDatalinkState(
    int state,
    int participant_count,
    int argument_2,
    int argument_3
);

enum {
    DATALINK_HANDLED_SLOT_COUNT = 3,
    DATALINK_STATUS_EVENT_ID = 11,
    DATALINK_STATUS_TRANSITION_6 = 4,
    DATALINK_STATUS_TRANSITION_7 = 5,
    DATALINK_TRANSITION_PARTICIPANT_COUNT = 4
};

/*
 * 0x020A76D0..0x020A7777 (Ghidra/catalog body size: 168 bytes).
 * Literal pool: 0x020A7778..0x020A777B; next function: 0x020A777C.
 *
 * The signed status byte +0x47 is read for slots 0, 1, and 2.  The jump table
 * covers unsigned values 0..7, so negative statuses take the default path:
 * - 3 and 4 report event 11 through 0x02082EE0;
 * - 6 selects transition 4;
 * - 7 selects transition 5;
 * - 0, 1, 2, 5 and all out-of-range values do nothing.
 *
 * A later slot with status 6/7 replaces an earlier transition selection.
 * When nonzero, 0x020AC038 receives the selected transition, constant 4,
 * zero, zero.  The function then returns one; without a transition it returns
 * zero, independently of any status-3/4 events.
 */
int Game_HandleDatalinkPlayerSlotStatuses(void)
{
    int transition = 0;
    int slot;

    for (slot = 0; slot < DATALINK_HANDLED_SLOT_COUNT; ++slot) {
        switch (gGameDatalinkPlayerSlots[slot].status) {
        case 3:
        case 4:
            Game_ReportDatalinkStatusEvent(DATALINK_STATUS_EVENT_ID);
            break;
        case 6:
            transition = DATALINK_STATUS_TRANSITION_6;
            break;
        case 7:
            transition = DATALINK_STATUS_TRANSITION_7;
            break;
        default:
            break;
        }
    }

    if (transition == 0) {
        return 0;
    }

    Game_ChangeDatalinkState(
        transition,
        DATALINK_TRANSITION_PARTICIPANT_COUNT,
        0,
        0
    );
    return 1;
}
