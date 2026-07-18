#ifndef GAME_DATALINK_SCENE_COMMAND_H
#define GAME_DATALINK_SCENE_COMMAND_H

#include "game/datalink_phase_ten_finalizer.h"

/* 0x020ad020..0x020ad06f: clears and initializes 96 tween records. */
void Game_InitializeDatalinkSceneTweenRecords_020ad020(
    Game_DatalinkSceneOwner *scene
);

void Game_ResetDatalinkSceneOamCount(
    Game_DatalinkSceneOwner *scene
);

void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command
);

#endif
