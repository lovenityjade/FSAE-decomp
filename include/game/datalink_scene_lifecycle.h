#ifndef GAME_DATALINK_SCENE_LIFECYCLE_H
#define GAME_DATALINK_SCENE_LIFECYCLE_H

#include "game/datalink_phase_ten_finalizer.h"

#include <stdint.h>

typedef struct Game_DatalinkSubDisplayConfigOwner {
    uint8_t unknown_000[0xfc];
    uint32_t screen_base_blocks_fc[4];
} Game_DatalinkSubDisplayConfigOwner;

typedef struct Game_DatalinkSceneResourceGroup {
    uint32_t allocation_address_00;
    uint32_t metadata_04;
    uint32_t allocation_address_08;
    uint32_t metadata_0c;
    uint32_t allocation_address_10;
    uint32_t metadata_14;
    uint32_t allocation_address_18;
    uint32_t metadata_1c;
} Game_DatalinkSceneResourceGroup;

extern volatile uint16_t gGameDatalinkSubBgControl_04001008[4];
extern Game_DatalinkSubDisplayConfigOwner
    gGameDatalinkSubDisplayConfigOwner;

/* 0x020ad528..0x020ad65f; two-word literal pool follows. */
void Game_ConfigureDatalinkSubDisplay_020ad528(void);

/* 0x020ad668..0x020ad6bb: state stack plus two scene bytes. */
void Game_SerializeDatalinkSceneState_020ad668(
    const Game_DatalinkSceneOwner *scene,
    uint8_t **cursor
);

/* 0x020ad6bc..0x020ad70f: inverse scene-state reader. */
void Game_DeserializeDatalinkSceneState_020ad6bc(
    Game_DatalinkSceneOwner *scene,
    const uint8_t **cursor
);

/* 0x020ad710..0x020ad87b: releases scene graphics allocations. */
void Game_ReleaseDatalinkSceneResources_020ad710(
    Game_DatalinkSceneOwner *scene
);

#endif
