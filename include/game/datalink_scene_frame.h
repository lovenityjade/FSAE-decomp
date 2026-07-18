#ifndef GAME_DATALINK_SCENE_FRAME_H
#define GAME_DATALINK_SCENE_FRAME_H

#include "game/datalink_scene_command.h"

#include <stdint.h>

typedef struct Game_DatalinkScenePublishedPosition {
    int32_t x;
    int32_t y;
} Game_DatalinkScenePublishedPosition;

extern volatile Game_DatalinkScenePublishedPosition
    gGameDatalinkScenePublishedPositions[4];

void Game_UpdateDatalinkSceneFrame_020ad87c(
    Game_DatalinkSceneOwner *scene
);

#endif
