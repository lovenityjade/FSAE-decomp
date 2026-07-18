#include "game/player_hud_sprite_animation.h"

#include <stddef.h>

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

_Static_assert(sizeof(Game_PlayerHudVisualState) == 0x34, "player HUD stride");
_Static_assert(
    offsetof(Game_PlayerHudVisualState, visual_flags) == 0x24,
    "player HUD visual flags offset"
);
_Static_assert(
    offsetof(Game_HudSpriteRuntime, sprite_context) == 0x3E0,
    "HUD sprite context offset"
);
_Static_assert(
    offsetof(Game_HudSpriteRuntime, sprite_handle_accumulator) == 0x460,
    "HUD sprite accumulator offset"
);

enum {
    PLAYER_HUD_CENTER_X = 56,
    PLAYER_HUD_CENTER_Y = 16,
    PLAYER_HUD_PALETTE_BASE = 7,

    PLAYER_HUD_EFFECT_FRAME_TICKS = 6,
    PLAYER_HUD_EFFECT_FRAME_COUNT = 4,
    PLAYER_HUD_EFFECT_RESOURCE_BASE = 64,
    PLAYER_HUD_EFFECT_RESOURCE_STRIDE = 4,
    PLAYER_HUD_EFFECT_PALETTE = 1,
    PLAYER_HUD_EFFECT_WIDTH = 32,
    PLAYER_HUD_EFFECT_HEIGHT = 32,

    PLAYER_HUD_MAIN_FRAME_TICKS = 8,
    PLAYER_HUD_MAIN_RESOURCE_BASE = 32,
    PLAYER_HUD_MAIN_WIDTH = 8,
    PLAYER_HUD_MAIN_HEIGHT = 64,

    PLAYER_HUD_CURRENT_RESOURCE = 35,
    PLAYER_HUD_CURRENT_Y_OFFSET = 8,

    PLAYER_HUD_OTHER_RESOURCE_STRIDE = 2,
    PLAYER_HUD_OTHER_WIDTH = 16,
    PLAYER_HUD_OTHER_HEIGHT = 8
};

static const uint32_t PLAYER_HUD_EFFECT_FLAGS = UINT32_C(0x80000000);
static const uint32_t PLAYER_HUD_OTHER_FLAGS = UINT32_C(0x00004000);

static void Game_SubmitHudSprite(
    int x,
    int y,
    int resource_id,
    int palette,
    uint32_t flags
)
{
    void *sprite_context = gGameHudSpriteRuntime->sprite_context;
    int sprite_handle = Game_CreateHudSprite(
        x,
        y,
        resource_id,
        palette,
        flags,
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
 * 0x020A6A50
 *
 * Proof anchors:
 * - the sole caller at 0x020A7164 iterates four player slots and passes the
 *   slot index, its x/y fields, an effect frame, and whether it is current;
 * - the player record has stride 0x34 and its word at +0x24 supplies the low
 *   two-bit palette variant used as variant+7;
 * - nonnegative effect frames select resource 64 + ((frame/6)%4)*4 and use
 *   a centered 32x32 shape with palette 1 and flags 0x80000000;
 * - the main sprite selects resource 32 + clock/8 and uses an 8x64 centered
 *   shape;
 * - current slots add resource 35 above that shape, while other slots use
 *   resource player_index*2 with flags 0x4000 and a 16x8 shape;
 * - each result from 0x02098E48 is passed as a halfword to 0x020BF6FC, while
 *   its full signed value is added to runtime offset +0x460.
 */
void Game_DrawPlayerHudSpriteSet(
    int player_index,
    int x,
    int y,
    int effect_frame,
    bool is_current_player
)
{
    int palette =
        (int)(gGamePlayerHudVisualStates[player_index].visual_flags & 3U) +
        PLAYER_HUD_PALETTE_BASE;

    if (effect_frame >= 0) {
        int frame =
            (effect_frame / PLAYER_HUD_EFFECT_FRAME_TICKS) %
            PLAYER_HUD_EFFECT_FRAME_COUNT;

        Game_SubmitHudSprite(
            x + PLAYER_HUD_CENTER_X - PLAYER_HUD_EFFECT_WIDTH / 2,
            y + PLAYER_HUD_CENTER_Y - PLAYER_HUD_EFFECT_HEIGHT / 2,
            PLAYER_HUD_EFFECT_RESOURCE_BASE +
                frame * PLAYER_HUD_EFFECT_RESOURCE_STRIDE,
            PLAYER_HUD_EFFECT_PALETTE,
            PLAYER_HUD_EFFECT_FLAGS
        );
    }

    Game_SubmitHudSprite(
        x + PLAYER_HUD_CENTER_X - PLAYER_HUD_MAIN_WIDTH / 2,
        y + PLAYER_HUD_CENTER_Y - PLAYER_HUD_MAIN_HEIGHT / 2,
        PLAYER_HUD_MAIN_RESOURCE_BASE +
            gGameHudSpriteAnimationClock / PLAYER_HUD_MAIN_FRAME_TICKS,
        palette,
        0
    );

    if (is_current_player) {
        Game_SubmitHudSprite(
            x + PLAYER_HUD_CENTER_X - PLAYER_HUD_MAIN_WIDTH / 2,
            y + PLAYER_HUD_CENTER_Y - PLAYER_HUD_MAIN_HEIGHT / 2 -
                PLAYER_HUD_CURRENT_Y_OFFSET,
            PLAYER_HUD_CURRENT_RESOURCE,
            palette,
            0
        );
    } else {
        Game_SubmitHudSprite(
            x + PLAYER_HUD_CENTER_X - PLAYER_HUD_OTHER_WIDTH / 2,
            y + PLAYER_HUD_CENTER_Y - PLAYER_HUD_OTHER_HEIGHT / 2 -
                PLAYER_HUD_CURRENT_Y_OFFSET,
            player_index * PLAYER_HUD_OTHER_RESOURCE_STRIDE,
            palette,
            PLAYER_HUD_OTHER_FLAGS
        );
    }
}
