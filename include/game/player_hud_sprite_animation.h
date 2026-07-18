#ifndef GAME_PLAYER_HUD_SPRITE_ANIMATION_H
#define GAME_PLAYER_HUD_SPRITE_ANIMATION_H

#include <stdbool.h>
#include <stdint.h>

typedef struct Game_PlayerHudVisualState {
    uint8_t unknown_00[0x24];
    uint32_t visual_flags;
    uint8_t unknown_28[0x0C];
} Game_PlayerHudVisualState;

typedef struct Game_HudSpriteRuntime {
    uint8_t unknown_000[0x3E0];
    uint8_t sprite_context[0x80];
    int sprite_handle_accumulator;
} Game_HudSpriteRuntime;

extern Game_PlayerHudVisualState gGamePlayerHudVisualStates[];
extern Game_HudSpriteRuntime *gGameHudSpriteRuntime;
extern int gGameHudSpriteAnimationClock;
extern void *gGameHudSpriteManager;

void Game_DrawPlayerHudSpriteSet(
    int player_index,
    int x,
    int y,
    int effect_frame,
    bool is_current_player
);

#endif
