#ifndef GAME_DATALINK_ROSTER_H
#define GAME_DATALINK_ROSTER_H

#include "game/datalink_peer_menu_update.h"

#include <stdint.h>

typedef Game_DatalinkPeerMenuState Game_DatalinkRosterState;

void Game_InitializeDatalinkRoster_020afc70(
    Game_DatalinkRosterState *state
);
void Game_ConfigureDatalinkRosterDisplayTweens_020afcbc(void);
void Game_BeginDatalinkRosterLeftTransition_020afd84(
    Game_DatalinkRosterState *state
);
void Game_BeginDatalinkRosterRightTransition_020afe3c(
    Game_DatalinkRosterState *state
);
void Game_MoveDatalinkRosterObjectsLeft_020afef4(
    Game_DatalinkRosterState *state
);
void Game_MoveDatalinkRosterObjectsRight_020aff84(
    Game_DatalinkRosterState *state
);

#endif
