#include "game/datalink_lobby_network_transition.h"

#include "game/datalink_controller.h"
#include "game/datalink_lobby_scene_initialize.h"
#include "game/datalink_post_oam.h"

#include <stdint.h>

enum {
    DATALINK_LOBBY_TWEEN_CAPACITY = 32,
    DATALINK_LOBBY_RESULT_PHASE_OFFSET = 0x28,
    DATALINK_LOBBY_RESULT_CODE_OFFSET = 0x2A,
    DATALINK_LOBBY_PHASE_STEP_OFFSET = 0x39,
    DATALINK_LOBBY_LOCAL_PARTICIPANT_OFFSET = 0x3A
};

extern void *Game_GetDatalinkWirelessContext_020909c8(void);
extern int Game_BeginDatalinkHostShutdown_02091738(void *context);
extern int Game_BeginDatalinkClientShutdown_02091ba0(void *context);
extern void Game_BeginDatalinkLobbyErrorTransition_0202811c(void);

/*
 * 0x020B2AB0..0x020B2B4B (156 bytes).
 * Waits for the host/client completion predicate, publishes lobby result two
 * and phase eight, selects post-OAM display state 1/3, then disables all 32
 * tween records.
 */
void Game_CompleteDatalinkLobbyNetworkTransition_020b2ab0(void)
{
    uint8_t *state = (uint8_t *)&gGameDatalinkLobbyState;
    Game_DatalinkTweenRecord *tweens =
        (Game_DatalinkTweenRecord *)(uintptr_t)
            gGameDatalinkLobbyState.tween_records_address_34;
    void *wireless_context;
    uint32_t index;
    int complete;

    wireless_context = Game_GetDatalinkWirelessContext_020909c8();
    if (state[DATALINK_LOBBY_LOCAL_PARTICIPANT_OFFSET] == 0) {
        complete = Game_BeginDatalinkHostShutdown_02091738(
            wireless_context
        );
    } else {
        complete = Game_BeginDatalinkClientShutdown_02091ba0(
            wireless_context
        );
    }
    if (complete == 0) {
        return;
    }

    Game_BeginDatalinkLobbyErrorTransition_0202811c();
    state[DATALINK_LOBBY_RESULT_CODE_OFFSET] = 2;
    state[DATALINK_LOBBY_RESULT_CODE_OFFSET + 1] = 0;
    state[DATALINK_LOBBY_RESULT_PHASE_OFFSET] = 8;
    state[DATALINK_LOBBY_PHASE_STEP_OFFSET] = 0;
    gGameDatalinkPostOamHardwareState.display_state_d0 = 1;
    gGameDatalinkPostOamHardwareState.display_variant_d1 = 3;

    for (index = 0; index < DATALINK_LOBBY_TWEEN_CAPACITY; ++index) {
        tweens[index].active_03 = 0;
    }
}
