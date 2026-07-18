#include "game/datalink_roster.h"

#include "game/datalink_controller.h"

#include <stddef.h>
#include <stdint.h>

typedef struct Game_DatalinkRosterPrimaryDisplay {
    uint8_t unknown_00[0x14];
    int32_t animation_state_14;
    uint8_t unknown_18[4];
    int32_t x_fx_1c;
    int32_t y_fx_20;
} Game_DatalinkRosterPrimaryDisplay;

typedef struct Game_DatalinkRosterSecondaryDisplay {
    uint8_t unknown_000[0xC4C];
    int32_t animation_state_c4c;
    uint8_t unknown_c50[4];
    int32_t x_fx_c54;
    int32_t y_fx_c58;
} Game_DatalinkRosterSecondaryDisplay;

typedef struct Game_DatalinkRosterObject {
    uint8_t unknown_00[6];
    uint8_t tween_slot_06;
} Game_DatalinkRosterObject;

extern Game_DatalinkRosterPrimaryDisplay gGameDatalinkRosterPrimaryDisplay;
extern Game_DatalinkRosterSecondaryDisplay
    gGameDatalinkRosterSecondaryDisplay;
extern Game_DatalinkTweenRecord gGameDatalinkRosterDisplayPrimaryTween;
extern Game_DatalinkTweenRecord gGameDatalinkRosterDisplaySecondaryTween;
extern Game_DatalinkTweenRecord gGameDatalinkRosterLeftPrimaryTween;
extern Game_DatalinkTweenRecord gGameDatalinkRosterLeftSecondaryTween;
extern Game_DatalinkTweenRecord gGameDatalinkRosterRightPrimaryTween;
extern Game_DatalinkTweenRecord gGameDatalinkRosterRightSecondaryTween;
extern Game_DatalinkTweenRecord gGameDatalinkRosterLeftEntryTweens[];
extern Game_DatalinkTweenRecord gGameDatalinkRosterRightEntryTweens[];
extern const int32_t gGameDatalinkRosterPrimaryPoint[2];
extern const int32_t gGameDatalinkRosterPrimaryYOffset;
extern const int32_t gGameDatalinkRosterSecondaryPoint[2];
extern const int32_t gGameDatalinkRosterSecondaryYOffset;
extern const int32_t gGameDatalinkRosterLeftPrimaryPoint[2];
extern const int32_t gGameDatalinkRosterLeftPrimaryYOffset;
extern const int32_t gGameDatalinkRosterLeftSecondaryPoint[2];
extern const int32_t gGameDatalinkRosterLeftSecondaryYOffset;
extern const int32_t gGameDatalinkRosterRightPrimaryPoint[2];
extern const int32_t gGameDatalinkRosterRightPrimaryYOffset;
extern const int32_t gGameDatalinkRosterRightSecondaryPoint[2];
extern const int32_t gGameDatalinkRosterRightSecondaryYOffset;
extern uint32_t gGameDatalinkRosterDisplayTweenDuration;
extern uint32_t gGameDatalinkRosterLeftTweenDuration;
extern uint32_t gGameDatalinkRosterRightTweenDuration;
extern uint32_t gGameDatalinkRosterLeftTransitionFrames;
extern uint32_t gGameDatalinkRosterRightTransitionFrames;

extern void MI_CpuFill(void *destination, uint32_t value, uint32_t size);
extern void Game_ConfigureDatalinkRosterObject(
    Game_DatalinkRosterState *state,
    Game_DatalinkRosterObject *object,
    uint32_t object_index
);

#if UINTPTR_MAX == UINT32_MAX
_Static_assert(
    offsetof(Game_DatalinkRosterState, ordered_peers_0b4) == 0xB4,
    "Datalink roster object array offset"
);
_Static_assert(
    offsetof(Game_DatalinkRosterState, peer_count_0c8) == 0xC8,
    "Datalink roster object count offset"
);
_Static_assert(
    offsetof(Game_DatalinkRosterState, completion_timer_100) == 0x100,
    "Datalink roster transition state offset"
);
#endif

/*
 * 0x020AFC70..0x020AFCBB (76 bytes).
 * Establishes mode four, clears the 0x80-byte entry area and resets all
 * recovered object-selection and transition fields.
 */
void Game_InitializeDatalinkRoster_020afc70(
    Game_DatalinkRosterState *state)
{
    *(uint32_t *)((uint8_t *)state + 4) = 4;
    state->completion_timer_100 = 0;
    state->selected_peer_104 = 0;
    state->phase_106 = 0;
    state->resumed_session_107 = 0;
    MI_CpuFill(state->peer_storage_034, 0, sizeof(state->peer_storage_034));
    state->peer_count_0c8 = 0;
    state->screen_resource_0cc = UINT32_MAX;
}

/*
 * 0x020AFCBC..0x020AFD5F (164 bytes).
 * Publishes both display positions and configures their paired tweens in
 * transition mode two.
 */
void Game_ConfigureDatalinkRosterDisplayTweens_020afcbc(void)
{
    int32_t primary_x = gGameDatalinkRosterPrimaryPoint[0];
    int32_t primary_y = gGameDatalinkRosterPrimaryPoint[1];
    int32_t secondary_x = gGameDatalinkRosterSecondaryPoint[0];
    int32_t secondary_y = gGameDatalinkRosterSecondaryPoint[1];

    gGameDatalinkRosterPrimaryDisplay.animation_state_14 = 0;
    gGameDatalinkRosterPrimaryDisplay.x_fx_1c = primary_x * 0x1000;
    gGameDatalinkRosterPrimaryDisplay.y_fx_20 =
        (primary_y + gGameDatalinkRosterPrimaryYOffset) * 0x1000;
    gGameDatalinkRosterSecondaryDisplay.animation_state_c4c = 0;
    gGameDatalinkRosterSecondaryDisplay.x_fx_c54 = secondary_x * 0x1000;
    gGameDatalinkRosterSecondaryDisplay.y_fx_c58 =
        (secondary_y + gGameDatalinkRosterSecondaryYOffset) * 0x1000;

    Game_ConfigureDatalinkTween(
        &gGameDatalinkRosterDisplayPrimaryTween,
        primary_x * 0x1000,
        primary_y * 0x1000,
        gGameDatalinkRosterDisplayTweenDuration,
        2
    );
    Game_ConfigureDatalinkTween(
        &gGameDatalinkRosterDisplaySecondaryTween,
        secondary_x * 0x1000,
        secondary_y * 0x1000,
        gGameDatalinkRosterDisplayTweenDuration,
        2
    );
}

/*
 * 0x020AFD84..0x020AFE1B (152 bytes).
 * Starts both panel tweens on the left-transition path and moves every active
 * roster object toward x = -0x80000.
 */
void Game_BeginDatalinkRosterLeftTransition_020afd84(
    Game_DatalinkRosterState *state)
{
    Game_ConfigureDatalinkTween(
        &gGameDatalinkRosterLeftPrimaryTween,
        gGameDatalinkRosterLeftPrimaryPoint[0] * 0x1000,
        (gGameDatalinkRosterLeftPrimaryPoint[1] +
         gGameDatalinkRosterLeftPrimaryYOffset) * 0x1000,
        gGameDatalinkRosterLeftTweenDuration,
        1
    );
    Game_ConfigureDatalinkTween(
        &gGameDatalinkRosterLeftSecondaryTween,
        gGameDatalinkRosterLeftSecondaryPoint[0] * 0x1000,
        (gGameDatalinkRosterLeftSecondaryPoint[1] +
         gGameDatalinkRosterLeftSecondaryYOffset) * 0x1000,
        gGameDatalinkRosterLeftTweenDuration,
        1
    );
    Game_MoveDatalinkRosterObjectsLeft_020afef4(state);
    state->exit_countdown_102 =
        (int16_t)gGameDatalinkRosterLeftTransitionFrames;
}

/*
 * 0x020AFE3C..0x020AFED3 (152 bytes).
 * Symmetric right-transition setup, ending with the recovered transition
 * frame count at state+0x102.
 */
void Game_BeginDatalinkRosterRightTransition_020afe3c(
    Game_DatalinkRosterState *state)
{
    Game_ConfigureDatalinkTween(
        &gGameDatalinkRosterRightPrimaryTween,
        gGameDatalinkRosterRightPrimaryPoint[0] * 0x1000,
        (gGameDatalinkRosterRightPrimaryPoint[1] +
         gGameDatalinkRosterRightPrimaryYOffset) * 0x1000,
        gGameDatalinkRosterRightTweenDuration,
        1
    );
    Game_ConfigureDatalinkTween(
        &gGameDatalinkRosterRightSecondaryTween,
        gGameDatalinkRosterRightSecondaryPoint[0] * 0x1000,
        (gGameDatalinkRosterRightSecondaryPoint[1] +
         gGameDatalinkRosterRightSecondaryYOffset) * 0x1000,
        gGameDatalinkRosterRightTweenDuration,
        1
    );
    Game_MoveDatalinkRosterObjectsRight_020aff84(state);
    state->exit_countdown_102 =
        (int16_t)gGameDatalinkRosterRightTransitionFrames;
}

static void Game_MoveDatalinkRosterObjects(
    Game_DatalinkRosterState *state,
    int32_t target_x_fx,
    Game_DatalinkTweenRecord *entry_tweens,
    uint32_t duration)
{
    uint32_t index;

    for (index = 0; index < state->peer_count_0c8; ++index) {
        Game_DatalinkRosterObject *object =
            (Game_DatalinkRosterObject *)(uintptr_t)
                state->ordered_peers_0b4[index];
        Game_DatalinkTweenRecord *tween =
            &entry_tweens[object->tween_slot_06];

        Game_ConfigureDatalinkTween(
            tween,
            target_x_fx,
            (uint32_t)tween->current_y_fx_34 & UINT32_C(0xFFFFF000),
            duration,
            1
        );
        Game_ConfigureDatalinkRosterObject(state, object, index);
    }
}

/* 0x020AFEF4..0x020AFF7B (136 bytes). */
void Game_MoveDatalinkRosterObjectsLeft_020afef4(
    Game_DatalinkRosterState *state)
{
    if (state->peer_count_0c8 != 0U) {
        Game_MoveDatalinkRosterObjects(
            state,
            -0x80000,
            gGameDatalinkRosterLeftEntryTweens,
            gGameDatalinkRosterLeftTweenDuration
        );
    }
}

/* 0x020AFF84..0x020B000B (136 bytes). */
void Game_MoveDatalinkRosterObjectsRight_020aff84(
    Game_DatalinkRosterState *state)
{
    if (state->peer_count_0c8 != 0U) {
        Game_MoveDatalinkRosterObjects(
            state,
            0x180000,
            gGameDatalinkRosterRightEntryTweens,
            gGameDatalinkRosterRightTweenDuration
        );
    }
}
