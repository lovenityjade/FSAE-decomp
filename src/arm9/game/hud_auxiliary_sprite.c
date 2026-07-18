#include "game/hud_auxiliary_sprite.h"

#include "game/player_hud_sprite_animation.h"

#include <stdint.h>

extern int Game_CreateHudSprite(
    int x,
    int y,
    int resource_id,
    int palette,
    uint32_t flags,
    void *sprite_context
);

extern void Game_RegisterHudSprite(
    void *sprite_manager,
    void *sprite_context,
    uint16_t sprite_handle
);

enum {
    HUD_AUXILIARY_CENTER_X = 56,
    HUD_AUXILIARY_CENTER_Y = 16,
    HUD_AUXILIARY_WIDTH = 8,
    HUD_AUXILIARY_HEIGHT = 64,
    HUD_AUXILIARY_RESOURCE = 36,
    HUD_AUXILIARY_PALETTE = 0,
    HUD_SPRITE_CLOCK_PERIOD = 24
};

/*
 * 0x020A6CC0
 *
 * Flags zero select dimensions 8x64 from the tables at 0x020DF054 and
 * 0x020DF03C.  The helper centers resource 36 around (x+56, y+16), submits
 * it with palette zero, registers the halfword handle through 0x020BF6FC,
 * and adds the full handle to runtime offset +0x460.
 */
void Game_DrawAuxiliaryHudSprite(int x, int y)
{
    void *sprite_context = gGameHudSpriteRuntime->sprite_context;
    int sprite_handle = Game_CreateHudSprite(
        x + HUD_AUXILIARY_CENTER_X - HUD_AUXILIARY_WIDTH / 2,
        y + HUD_AUXILIARY_CENTER_Y - HUD_AUXILIARY_HEIGHT / 2,
        HUD_AUXILIARY_RESOURCE,
        HUD_AUXILIARY_PALETTE,
        0,
        sprite_context
    );

    Game_RegisterHudSprite(
        gGameHudSpriteManager,
        sprite_context,
        (uint16_t)sprite_handle
    );
    gGameHudSpriteRuntime->sprite_handle_accumulator += sprite_handle;
}

/*
 * Semantic block 0x020A71CC..0x020A7218 inside 0x020A6FF0.
 *
 * The two words at state+0x60/+0x64 are tested exactly like the player-slot
 * positions: only (0,0) suppresses drawing.  Afterwards, the word at
 * 0x0217E2D0 advances with a signed remainder by 24; its normal range is
 * 0..23 and it drives the main player-sprite frame selection.
 */
void Game_UpdateAuxiliaryHudSpriteAndClock(Game_PlayerHudLoopState *state)
{
    if (state->auxiliary_x != 0 || state->auxiliary_y != 0) {
        Game_DrawAuxiliaryHudSprite(state->auxiliary_x, state->auxiliary_y);
    }

    gGameHudSpriteAnimationClock =
        (gGameHudSpriteAnimationClock + 1) % HUD_SPRITE_CLOCK_PERIOD;
}
