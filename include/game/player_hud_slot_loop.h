#ifndef GAME_PLAYER_HUD_SLOT_LOOP_H
#define GAME_PLAYER_HUD_SLOT_LOOP_H

#include <stdint.h>

typedef struct Game_PlayerHudPosition {
    int x;
    int y;
} Game_PlayerHudPosition;

typedef struct Game_PlayerHudLoopState {
    uint8_t unknown_00[0x1C];
    int current_player;
    int effect_frames[4];
    Game_PlayerHudPosition positions[4];
    uint32_t active[4];
    int auxiliary_x;
    int auxiliary_y;
} Game_PlayerHudLoopState;

extern int gGameHudPlayerRotationIndex;

void Game_UpdateFourPlayerHudSpriteSlots(Game_PlayerHudLoopState *state);

#endif
