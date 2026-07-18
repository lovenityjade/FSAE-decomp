#ifndef GAME_DATALINK_SCENE_OAM_H
#define GAME_DATALINK_SCENE_OAM_H

#include "game/datalink_phase_ten_finalizer.h"
#include "game/datalink_sprite_oam.h"

void Game_SubmitVisibleDatalinkSceneOams(
    Game_DatalinkSceneOwner *scene,
    const Game_DatalinkOamEntry *oam_entries,
    int count
);

#endif
