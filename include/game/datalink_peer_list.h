#ifndef GAME_DATALINK_PEER_LIST_H
#define GAME_DATALINK_PEER_LIST_H

#include "game/datalink_peer_menu_update.h"

#include <stdint.h>

/* Four signed X/Y bases used by the two peer sprites. Address 0x021263CC. */
extern const int32_t gGameDatalinkPeerSpriteOffsets_021263cc[4];

unsigned int Game_CountDatalinkReadyPeers_020b07bc(
    const Game_DatalinkPeerMenuState *state
);

void Game_UpsertDatalinkPeerRecord_020b0800(
    Game_DatalinkPeerMenuState *state,
    const uint8_t identity[6],
    const void *label,
    uint32_t label_size,
    uint8_t attribute,
    uint8_t sort_key
);

void Game_InitializeDatalinkPeerVisuals_020b09f4(
    Game_DatalinkPeerMenuState *state,
    Game_DatalinkPeerListRecord *peer,
    unsigned int visible_index
);

void Game_ArrangeDatalinkPeerSprites_020b0a6c(
    Game_DatalinkPeerMenuState *state,
    Game_DatalinkPeerListRecord *peer,
    unsigned int visible_index
);

void Game_RemoveDatalinkPeerRecord_020b0b40(
    Game_DatalinkPeerMenuState *state,
    Game_DatalinkPeerListRecord *peer
);

void Game_SortDatalinkPeerRecords_020b0c38(
    Game_DatalinkPeerMenuState *state
);

void Game_ClearDatalinkPeerList_020b0d58(
    Game_DatalinkPeerMenuState *state
);

#endif
