#include "game/datalink_extended_menu_controller.h"

#include "game/datalink_phase_ten_finalizer.h"
#include "game/datalink_player_slot.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_EXTENDED_MENU_DISCONNECTED = 0,
    DATALINK_EXTENDED_MENU_SESSION_READY = 2,
    DATALINK_EXTENDED_MENU_PARTICIPANT_LIMIT = 4,
    DATALINK_EXTENDED_MENU_SHARED_DATA_STRIDE = 0x34,
    DATALINK_EXTENDED_MENU_SLOT_PAYLOAD_OFFSET = 8,
    DATALINK_EXTENDED_MENU_SLOT_PAYLOAD_SIZE = 14,
    DATALINK_EXTENDED_MENU_FAILURE_SOUND = 0x40
};

extern uint32_t Game_GetDatalinkMenuSelectionContext(void);
extern uint8_t *Game_GetDatalinkRenderContext(void);
extern int Game_GetDatalinkMenuConnectionState(
    uint32_t selection_context
);
extern void Game_BeginDatalinkPrimaryMenuAction(
    uint32_t selection_context,
    int zero
);
extern void OS_WaitVBlankIntr(void);
extern void Game_UpdateDatalinkMenuConnection(
    uint32_t selection_context
);
extern int Game_StartDatalinkWirelessSession(
    uint32_t selection_context,
    int participant_limit,
    int shared_data_stride,
    const void *slot_payload,
    uint32_t slot_payload_size
);
extern void Game_PlayDatalinkSoundEffect(uint32_t sound_id);

_Static_assert(
    offsetof(Game_DatalinkSceneOwner, active_player_slot_53d) == 0x53D,
    "datalink active player-slot offset"
);
_Static_assert(
    sizeof(Game_DatalinkPlayerSlot) == 0x50,
    "datalink player-slot stride"
);

/*
 * 0x020ABF5C
 *
 * Complete 176-byte synchronous connection/session preparation helper
 * through 0x020AC00B. The two-word literal pool at
 * 0x020AC00C..0x020AC013 and independent stubs beginning at 0x020AC014 are
 * excluded.
 */
int Game_PrepareDatalinkExtendedMenuPrimaryAction(void)
{
    uint32_t selection_context =
        Game_GetDatalinkMenuSelectionContext();
    int connection_state;
    uint8_t active_slot;
    const void *slot_payload;

    (void)Game_GetDatalinkRenderContext();
    connection_state =
        Game_GetDatalinkMenuConnectionState(selection_context);
    if (connection_state == DATALINK_EXTENDED_MENU_DISCONNECTED) {
        Game_BeginDatalinkPrimaryMenuAction(selection_context, 0);
        do {
            OS_WaitVBlankIntr();
            Game_UpdateDatalinkMenuConnection(selection_context);
            connection_state =
                Game_GetDatalinkMenuConnectionState(selection_context);
        } while (connection_state != DATALINK_EXTENDED_MENU_SESSION_READY);
    }

    if (connection_state != DATALINK_EXTENDED_MENU_SESSION_READY) {
        return 0;
    }

    active_slot = gGameDatalinkSceneOwner.active_player_slot_53d;
    slot_payload =
        &gGameDatalinkPlayerSlots[active_slot]
             .unknown_00[DATALINK_EXTENDED_MENU_SLOT_PAYLOAD_OFFSET];
    if (Game_StartDatalinkWirelessSession(
            selection_context,
            DATALINK_EXTENDED_MENU_PARTICIPANT_LIMIT,
            DATALINK_EXTENDED_MENU_SHARED_DATA_STRIDE,
            slot_payload,
            DATALINK_EXTENDED_MENU_SLOT_PAYLOAD_SIZE
        ) != 0) {
        return 1;
    }

    Game_PlayDatalinkSoundEffect(
        DATALINK_EXTENDED_MENU_FAILURE_SOUND
    );
    return 0;
}
