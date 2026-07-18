#include "game/player_hud_slot_loop.h"

#include "game/player_hud_sprite_animation.h"

#include <stddef.h>

_Static_assert(
    offsetof(Game_PlayerHudLoopState, current_player) == 0x1C,
    "current player offset"
);
_Static_assert(
    offsetof(Game_PlayerHudLoopState, effect_frames) == 0x20,
    "player effect frame offset"
);
_Static_assert(
    offsetof(Game_PlayerHudLoopState, positions) == 0x30,
    "player HUD position offset"
);
_Static_assert(
    offsetof(Game_PlayerHudLoopState, active) == 0x50,
    "active player offset"
);
_Static_assert(
    offsetof(Game_PlayerHudLoopState, auxiliary_x) == 0x60,
    "auxiliary HUD position offset"
);

enum {
    PLAYER_HUD_SLOT_COUNT = 4,
    PLAYER_HUD_EFFECT_FRAME_LIMIT = 48,
    PLAYER_HUD_EFFECT_INACTIVE = -1
};

static int Game_WrapPlayerHudSlot(int slot)
{
    return slot % PLAYER_HUD_SLOT_COUNT;
}

static void Game_AdvancePlayerHudEffectFrame(
    Game_PlayerHudLoopState *state,
    int player_index
)
{
    int frame = state->effect_frames[player_index];

    if (frame >= 0) {
        ++frame;
        if (frame >= PLAYER_HUD_EFFECT_FRAME_LIMIT) {
            frame = PLAYER_HUD_EFFECT_INACTIVE;
        }
        state->effect_frames[player_index] = frame;
    }
}

static void Game_SelectNextActivePlayerHudSlot(
    Game_PlayerHudLoopState *state
)
{
    int attempts;

    for (attempts = 0; attempts < PLAYER_HUD_SLOT_COUNT; ++attempts) {
        gGameHudPlayerRotationIndex = Game_WrapPlayerHudSlot(
            gGameHudPlayerRotationIndex + 1
        );
        if (state->active[gGameHudPlayerRotationIndex] != 0U) {
            break;
        }
    }
}

/*
 * Semantic block 0x020A70F4..0x020A71CC inside 0x020A6FF0.
 * The call to the recovered sprite routine is at 0x020A7164.
 *
 * Proof anchors:
 * - four iterations visit (rotation+iteration)%4;
 * - active words are read from +0x50 and positions from +0x30 with stride 8;
 * - only the all-zero position suppresses drawing; an active slot still
 *   advances its frame when drawing is suppressed;
 * - effect frames at +0x20 increment while nonnegative and become -1 on
 *   reaching 48;
 * - the boolean fifth argument is current_player(+0x1C)==visited_slot;
 * - after the loop, the rotation index advances at least once and stops at
 *   the first active slot, or after four attempts if all are inactive.
 */
void Game_UpdateFourPlayerHudSpriteSlots(Game_PlayerHudLoopState *state)
{
    int iteration;

    for (iteration = 0; iteration < PLAYER_HUD_SLOT_COUNT; ++iteration) {
        int player_index = Game_WrapPlayerHudSlot(
            gGameHudPlayerRotationIndex + iteration
        );

        if (state->active[player_index] != 0U) {
            Game_PlayerHudPosition *position = &state->positions[player_index];

            if (position->x != 0 || position->y != 0) {
                Game_DrawPlayerHudSpriteSet(
                    player_index,
                    position->x,
                    position->y,
                    state->effect_frames[player_index],
                    state->current_player == player_index
                );
            }
            Game_AdvancePlayerHudEffectFrame(state, player_index);
        }
    }

    Game_SelectNextActivePlayerHudSlot(state);
}
