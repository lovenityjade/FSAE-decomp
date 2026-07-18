#ifndef GAME_DATALINK_PEER_MENU_UPDATE_H
#define GAME_DATALINK_PEER_MENU_UPDATE_H

#include "game/datalink_controller.h"
#include "game/datalink_phase_ten_finalizer.h"
#include "game/datalink_sprite_oam.h"

#include <stdint.h>

enum {
    GAME_DATALINK_PEER_MENU_MAX_PEERS = 4
};

typedef struct Game_DatalinkPeerListRecord {
    uint8_t identity_00[6];
    uint8_t tween_index_06;
    uint8_t sprite_index_07;
    uint8_t active_08;
    uint8_t sort_key_09;
    uint8_t attribute_0a;
    uint8_t unknown_0b;
    uint8_t seen_0c;
    uint8_t unknown_0d;
    uint8_t label_0e[0x0e];
    uint32_t discovery_age_1c;
} Game_DatalinkPeerListRecord;

typedef struct Game_DatalinkPeerMenuState {
    uint8_t unknown_000[0x34];
    Game_DatalinkPeerListRecord peer_storage_034[
        GAME_DATALINK_PEER_MENU_MAX_PEERS
    ];
    Game_DatalinkPeerListRecord *ordered_peers_0b4[
        GAME_DATALINK_PEER_MENU_MAX_PEERS
    ];
    void *selected_wireless_peer_0c4;
    uint32_t peer_count_0c8;
    uint32_t screen_resource_0cc;
    uint8_t unknown_0d0[0x30];
    int16_t completion_timer_100;
    int16_t exit_countdown_102;
    int8_t selected_peer_104;
    uint8_t unknown_105;
    uint8_t phase_106;
    uint8_t resumed_session_107;
} Game_DatalinkPeerMenuState;

typedef struct Game_DatalinkPeerSprite {
    uint8_t unknown_00[0x24];
    const Game_DatalinkOamEntry *oam_entries_24;
    uint8_t unknown_28[0x1c];
    int oam_start_index_44;
} Game_DatalinkPeerSprite;

extern volatile uint16_t gGameDatalinkPeerMenuInput;
extern Game_DatalinkSceneOwner gGameDatalinkPeerSceneOwner;
extern Game_DatalinkTweenRecord gGameDatalinkPeerMenuTransitionTween;

/* 0x020b0014..0x020b0777; literal pool begins at 0x020b0778. */
void Game_UpdateDatalinkPeerMenu_020b0014(
    Game_DatalinkPeerMenuState *state
);

#endif
