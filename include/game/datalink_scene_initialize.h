#ifndef GAME_DATALINK_SCENE_INITIALIZE_H
#define GAME_DATALINK_SCENE_INITIALIZE_H

#include "game/datalink_phase_ten_finalizer.h"

#include <stdint.h>

typedef struct Game_DatalinkSceneStaticStateArena {
    volatile uint32_t secondary_guard_000;
    uint8_t unknown_004[0x10];
    volatile uint32_t primary_guard_014;
    uint32_t primary_state_vtable_018;
    uint8_t unknown_01c[0x64];
    uint32_t secondary_state_vtable_080;
    uint8_t unknown_084[0x2c];
    uint32_t secondary_state_data_0b0;
} Game_DatalinkSceneStaticStateArena;

/* Static state arena rooted at target address 0x0217e604. */
extern volatile Game_DatalinkSceneStaticStateArena
    gGameDatalinkSceneStaticStateArena;

/* Shared graphics arena rooted at target address 0x0217a23c. */
extern uint8_t gGameDatalinkSceneGraphicsArena[0xfc];

/* Phase-object storage rooted at target address 0x0217f6f0. */
extern uint8_t gGameDatalinkScenePhaseObjectStorage[0x80c];

/* Shared target word at 0x021348fc. */
extern volatile uint32_t gGameDatalinkSceneSharedWord;

/* 0x020ad070..0x020ad4eb; literal pool begins at 0x020ad4ec. */
void Game_InitializeDatalinkScene_020ad070(
    Game_DatalinkSceneOwner *scene
);

#endif
