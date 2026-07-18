#ifndef GAME_DATALINK_PEER_ROSTER_H
#define GAME_DATALINK_PEER_ROSTER_H

#include "game/datalink_roster.h"

#include <stdint.h>

typedef Game_DatalinkPeerListRecord Game_DatalinkPeerEntry;

typedef struct Game_DatalinkPeerGlyphDescriptor {
    uint32_t resource_address;
    uint32_t glyph_address;
    uint32_t glyph_size;
} Game_DatalinkPeerGlyphDescriptor;

void Game_RefreshDatalinkPeerRoster_020b0d7c(
    Game_DatalinkRosterState *state
);
int Game_SelectDatalinkPeerNetworkRecord_020b0e84(
    Game_DatalinkRosterState *state
);
void Game_StartDatalinkPeerConnection_020b0f8c(void);
void Game_UnlockDatalinkPeerNetworkList_020b0fe8(void);
int Game_BuildDatalinkPeerGlyphDescriptor_020b0ff8(
    Game_DatalinkRosterState *state,
    Game_DatalinkPeerGlyphDescriptor *descriptor,
    int cell_index
);
Game_DatalinkPeerEntry *Game_FindDatalinkPeerEntryForCell_020b108c(
    Game_DatalinkRosterState *state,
    int cell_index
);
void Game_SetupDatalinkPeerRoster_020b10d4(
    Game_DatalinkRosterState *state
);
void Game_ClearDatalinkPeerRosterGraphics_020b11a0(void);
int Game_GetDatalinkPeerRosterCellMap_020b11dc(int index);
void Game_ConfigureDatalinkLobbySubDisplay_020b1224(void);
void Game_ReleaseDatalinkLobbyScene_020b1920(void);

#endif
