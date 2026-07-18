#include "game/datalink_controller.h"

#include "game/datalink_phase_ten_finalizer.h"
#include "game/datalink_slot_transfer.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_ACTION_ROW_COUNT = 2,
    DATALINK_ACTION_MAIN_ROW_COUNT = 3,
    DATALINK_ACTION_PHASE_INITIALIZE = 0,
    DATALINK_ACTION_PHASE_SELECT = 5,
    DATALINK_ACTION_PHASE_REMOVE_WAIT = 15,
    DATALINK_ACTION_PHASE_OWNER_COMMIT = 16,
    DATALINK_ACTION_PHASE_EXIT = 20,
    DATALINK_ACTION_PHASE_EXIT_WAIT = 25,
    DATALINK_ACTION_REMOVE_TIMER = 60,
    DATALINK_ACTION_REMOVE_TRIGGER = 30,
    DATALINK_ACTION_COMMIT_TIMER = 2,
    DATALINK_ACTION_TWEEN_COMPLETE = 0x1000,
    DATALINK_ACTION_CONFIRM_INPUT = 1,
    DATALINK_ACTION_CANCEL_INPUT = 2,
    DATALINK_ACTION_NEXT_INPUT = 0x10,
    DATALINK_ACTION_PREVIOUS_INPUT = 0x20,
    DATALINK_ACTION_MOVE_SOUND = 0x3D,
    DATALINK_ACTION_EXIT_SOUND = 0x44,
    DATALINK_ACTION_CURSOR_COMMAND = 3,
    DATALINK_ACTION_TAIL_COMMAND = 31,
    DATALINK_ACTION_TWEEN_DURATION = 12,
    DATALINK_ACTION_TWEEN_MODE_OUT = 1,
    DATALINK_ACTION_TWEEN_MODE_IN = 2,
    DATALINK_ACTION_OVERLAY_TWEEN = 31,
    DATALINK_ACTION_AUXILIARY_TWEEN = 23,
    DATALINK_ACTION_SCREEN_RESOURCE = 0x0838,
    DATALINK_ACTION_RETURN_RESOURCE = 0x0835,
    DATALINK_ACTION_DISPLAY_MASK = 0x1F00,
    DATALINK_ACTION_DISPLAY_VALUE = 0x1900,
    DATALINK_ACTION_REMOVE_STATE = 7,
    DATALINK_FX_ONE = 0x1000
};

typedef struct Game_DatalinkActionPosition {
    int32_t x;
    int32_t y;
} Game_DatalinkActionPosition;

static const uint8_t
    sDatalinkActionTweenIndices[DATALINK_ACTION_ROW_COUNT][2] = {
        {17, 18},
        {15, 16}
    };

static const uint8_t
    sDatalinkMainTweenIndices[DATALINK_ACTION_MAIN_ROW_COUNT][2] = {
        {5, 6},
        {7, 8},
        {9, 10}
    };

static const Game_DatalinkActionPosition
    sDatalinkActionPositions[DATALINK_ACTION_ROW_COUNT] = {
        {84, 112},
        {172, 112}
    };

static const Game_DatalinkActionPosition
    sDatalinkActionCursorPositions[DATALINK_ACTION_ROW_COUNT] = {
        {48, 113},
        {136, 113}
    };

extern volatile uint32_t gGameDatalinkControllerDisplayControl;

extern void Game_ConfigureDatalinkTween(
    Game_DatalinkTweenRecord *tween,
    int32_t target_x_fx,
    int32_t target_y_fx,
    uint32_t duration,
    int mode
);
extern int32_t Game_GetDatalinkTweenProgress(
    const Game_DatalinkTweenRecord *tween
);
extern void Game_LoadDatalinkScreenResource(uint16_t resource_id);
extern void Game_ClampDatalinkSelection(
    volatile int8_t *selection,
    int minimum,
    int maximum
);
extern void Game_PlayDatalinkSoundEffect(uint32_t sound_id);
extern void Game_ChangeDatalinkState(
    int state,
    int participant_count,
    int argument_2,
    int argument_3
);
extern void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command
);
extern void Game_PopDatalinkTransition(uint32_t owner_address);
extern void Game_CommitDatalinkScreenUpdate(void);
extern void Game_SetDatalinkOwnerSuppressRedraw(
    uint32_t owner_address,
    uint8_t suppress
);
extern void Game_SetDatalinkOwnerSelectionDirty(
    uint32_t owner_address,
    uint8_t dirty
);
extern void Game_SetDatalinkOwnerSelectedParticipant(
    uint32_t owner_address,
    int8_t participant
);
extern void Game_InvokeDatalinkOwnerRefresh(uint32_t owner_address);

_Static_assert(
    offsetof(Game_DatalinkSceneOwner, participant_action_visible_ee7) ==
        0xEE7,
    "datalink participant-action visible offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, participant_action_duration_f0c) ==
        0xF0C,
    "datalink participant-action duration offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, participant_action_x_fx_f14) ==
        0xF14,
    "datalink participant-action X offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, participant_action_y_fx_f18) ==
        0xF18,
    "datalink participant-action Y offset"
);
_Static_assert(
    sizeof(gGameDatalinkControllerTweens) /
        sizeof(gGameDatalinkControllerTweens[0]) == 32U,
    "datalink participant-action tween count"
);

static int32_t Game_DatalinkActionCoordinateToFx(int32_t coordinate)
{
    return coordinate * DATALINK_FX_ONE;
}

static void Game_SeedDatalinkActionTween(
    Game_DatalinkTweenRecord *tween,
    int32_t x,
    int32_t y
)
{
    tween->active_03 = 1U;
    tween->duration_fx_28 = 0;
    tween->current_x_fx_30 = Game_DatalinkActionCoordinateToFx(x);
    tween->current_y_fx_34 = Game_DatalinkActionCoordinateToFx(y);
}

static void Game_ConfigureDatalinkActionRows(int y_offset, int mode)
{
    int row;

    for (row = 0; row < DATALINK_ACTION_ROW_COUNT; ++row) {
        const Game_DatalinkActionPosition *position =
            &sDatalinkActionPositions[row];
        int variant;

        for (variant = 0; variant < 2; ++variant) {
            Game_ConfigureDatalinkTween(
                &gGameDatalinkControllerTweens[
                    sDatalinkActionTweenIndices[row][variant]
                ],
                Game_DatalinkActionCoordinateToFx(position->x),
                Game_DatalinkActionCoordinateToFx(
                    position->y + y_offset
                ),
                DATALINK_ACTION_TWEEN_DURATION,
                mode
            );
        }
    }
}

static void Game_SeedAndConfigureDatalinkActionRows(void)
{
    int row;

    for (row = 0; row < DATALINK_ACTION_ROW_COUNT; ++row) {
        const Game_DatalinkActionPosition *position =
            &sDatalinkActionPositions[row];
        int variant;

        for (variant = 0; variant < 2; ++variant) {
            Game_DatalinkTweenRecord *tween =
                &gGameDatalinkControllerTweens[
                    sDatalinkActionTweenIndices[row][variant]
                ];

            Game_SeedDatalinkActionTween(
                tween,
                position->x,
                position->y + 64
            );
            Game_ConfigureDatalinkTween(
                tween,
                Game_DatalinkActionCoordinateToFx(position->x),
                Game_DatalinkActionCoordinateToFx(position->y),
                DATALINK_ACTION_TWEEN_DURATION,
                DATALINK_ACTION_TWEEN_MODE_IN
            );
        }
    }
}

static void Game_InitializeDatalinkParticipantAction(
    Game_DatalinkController *controller
)
{
    Game_SeedAndConfigureDatalinkActionRows();

    gGameDatalinkSceneOwner.participant_action_visible_ee7 = 1U;
    gGameDatalinkSceneOwner.participant_action_duration_f0c = 0;
    gGameDatalinkSceneOwner.participant_action_x_fx_f14 =
        Game_DatalinkActionCoordinateToFx(255);
    gGameDatalinkSceneOwner.participant_action_y_fx_f18 =
        Game_DatalinkActionCoordinateToFx(-32);
    Game_ConfigureDatalinkTween(
        &gGameDatalinkControllerTweens[DATALINK_ACTION_OVERLAY_TWEEN],
        Game_DatalinkActionCoordinateToFx(255),
        Game_DatalinkActionCoordinateToFx(0),
        DATALINK_ACTION_TWEEN_DURATION,
        DATALINK_ACTION_TWEEN_MODE_IN
    );
    Game_ConfigureDatalinkTween(
        &gGameDatalinkControllerTweens[DATALINK_ACTION_AUXILIARY_TWEEN],
        Game_DatalinkActionCoordinateToFx(24),
        Game_DatalinkActionCoordinateToFx(180),
        DATALINK_ACTION_TWEEN_DURATION,
        DATALINK_ACTION_TWEEN_MODE_IN
    );
    Game_LoadDatalinkScreenResource(DATALINK_ACTION_SCREEN_RESOURCE);
    controller->transition_phase_5d = DATALINK_ACTION_PHASE_SELECT;
    controller->selected_row_5b = 0;
}

static void Game_DeactivateDatalinkParticipantActionObjects(void)
{
    int row;

    for (row = 0; row < DATALINK_ACTION_ROW_COUNT; ++row) {
        gGameDatalinkControllerTweens[
            sDatalinkActionTweenIndices[row][0]
        ].active_03 = 0U;
        gGameDatalinkControllerTweens[
            sDatalinkActionTweenIndices[row][1]
        ].active_03 = 0U;
    }

    for (row = 0; row < DATALINK_ACTION_MAIN_ROW_COUNT; ++row) {
        gGameDatalinkControllerTweens[
            sDatalinkMainTweenIndices[row][0]
        ].active_03 = 0U;
        gGameDatalinkControllerTweens[
            sDatalinkMainTweenIndices[row][1]
        ].active_03 = 0U;
        gGameDatalinkControllerTweens[row].active_03 = 0U;
    }
    gGameDatalinkSceneOwner.participant_action_visible_ee7 = 0U;
    gGameDatalinkSceneOwner.controller_ready_c27 = 0U;
}

static void Game_UpdateDatalinkParticipantActionCursor(
    Game_DatalinkController *controller,
    int previous_selection
)
{
    int row;

    Game_ClampDatalinkSelection(&controller->selected_row_5b, 0, 1);
    row = controller->selected_row_5b;
    if (row != previous_selection) {
        Game_PlayDatalinkSoundEffect(DATALINK_ACTION_MOVE_SOUND);
    }

    gGameDatalinkSceneOwner.cursor_transition_duration_56c = 0;
    gGameDatalinkSceneOwner.cursor_x_fx_574 =
        Game_DatalinkActionCoordinateToFx(
            sDatalinkActionCursorPositions[row].x
        );
    gGameDatalinkSceneOwner.cursor_y_fx_578 =
        Game_DatalinkActionCoordinateToFx(
            sDatalinkActionCursorPositions[row].y
        );
    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkSceneOwner,
        DATALINK_ACTION_CURSOR_COMMAND
    );
}

static void Game_UpdateDatalinkParticipantActionSelect(
    Game_DatalinkController *controller,
    int previous_selection
)
{
    uint16_t input;

    if (Game_GetDatalinkTweenProgress(
            &gGameDatalinkControllerTweens[
                sDatalinkActionTweenIndices[0][0]
            ]
        ) != DATALINK_ACTION_TWEEN_COMPLETE) {
        return;
    }

    input = gGameDatalinkControllerInput;
    if (input == DATALINK_ACTION_NEXT_INPUT) {
        controller->selected_row_5b =
            (int8_t)(controller->selected_row_5b + 1);
    } else if (input == DATALINK_ACTION_PREVIOUS_INPUT) {
        controller->selected_row_5b =
            (int8_t)(controller->selected_row_5b - 1);
    } else if (input == DATALINK_ACTION_CANCEL_INPUT) {
        controller->transition_phase_5d = DATALINK_ACTION_PHASE_EXIT;
    } else if (input == DATALINK_ACTION_CONFIRM_INPUT) {
        if (controller->selected_row_5b == 0) {
            controller->transition_phase_5d =
                DATALINK_ACTION_PHASE_EXIT;
        } else if (controller->selected_row_5b == 1) {
            gGameDatalinkControllerDisplayControl =
                (gGameDatalinkControllerDisplayControl &
                 ~(uint32_t)DATALINK_ACTION_DISPLAY_MASK) |
                (uint32_t)DATALINK_ACTION_DISPLAY_VALUE;
            controller->transition_phase_5d =
                DATALINK_ACTION_PHASE_REMOVE_WAIT;
            controller->transition_countdown_5e =
                DATALINK_ACTION_REMOVE_TIMER;
            Game_ChangeDatalinkState(
                DATALINK_ACTION_REMOVE_STATE,
                0,
                0,
                0
            );
            Game_DeactivateDatalinkParticipantActionObjects();
        }
    }

    Game_UpdateDatalinkParticipantActionCursor(
        controller,
        previous_selection
    );
}

static void Game_UpdateDatalinkParticipantRemoveWait(
    Game_DatalinkController *controller
)
{
    uint8_t countdown = controller->transition_countdown_5e;

    if (countdown == DATALINK_ACTION_REMOVE_TRIGGER) {
        Game_RemoveDatalinkPlayerSlot(
            (int)gGameDatalinkControllerActiveSlot
        );
    }
    countdown = (uint8_t)(countdown - 1U);
    controller->transition_countdown_5e = countdown;
    if (countdown == 0U) {
        controller->transition_countdown_5e =
            DATALINK_ACTION_COMMIT_TIMER;
        controller->transition_phase_5d =
            DATALINK_ACTION_PHASE_OWNER_COMMIT;
        Game_CommitDatalinkScreenUpdate();
    }
}

static void Game_UpdateDatalinkParticipantOwnerCommit(
    Game_DatalinkController *controller
)
{
    uint8_t countdown =
        (uint8_t)(controller->transition_countdown_5e - 1U);

    controller->transition_countdown_5e = countdown;
    if (countdown == 0U) {
        uint32_t owner_address = controller->owner_address_08;

        Game_PopDatalinkTransition(owner_address);
        Game_SetDatalinkOwnerSuppressRedraw(owner_address, 0U);
        Game_SetDatalinkOwnerSelectionDirty(owner_address, 1U);
        Game_SetDatalinkOwnerSelectedParticipant(owner_address, 0);
        Game_InvokeDatalinkOwnerRefresh(owner_address);
        controller->selected_row_5b = 0;
    }
}

static void Game_BeginDatalinkParticipantActionExit(
    Game_DatalinkController *controller
)
{
    Game_ConfigureDatalinkActionRows(
        64,
        DATALINK_ACTION_TWEEN_MODE_OUT
    );
    Game_ConfigureDatalinkTween(
        &gGameDatalinkControllerTweens[DATALINK_ACTION_OVERLAY_TWEEN],
        Game_DatalinkActionCoordinateToFx(255),
        Game_DatalinkActionCoordinateToFx(-32),
        DATALINK_ACTION_TWEEN_DURATION,
        DATALINK_ACTION_TWEEN_MODE_OUT
    );
    Game_ConfigureDatalinkTween(
        &gGameDatalinkControllerTweens[DATALINK_ACTION_AUXILIARY_TWEEN],
        Game_DatalinkActionCoordinateToFx(24),
        Game_DatalinkActionCoordinateToFx(204),
        DATALINK_ACTION_TWEEN_DURATION,
        DATALINK_ACTION_TWEEN_MODE_OUT
    );
    Game_PlayDatalinkSoundEffect(DATALINK_ACTION_EXIT_SOUND);
    controller->transition_phase_5d = DATALINK_ACTION_PHASE_EXIT_WAIT;
}

static int Game_UpdateDatalinkParticipantActionExitWait(
    Game_DatalinkController *controller
)
{
    if (Game_GetDatalinkTweenProgress(
            &gGameDatalinkControllerTweens[
                sDatalinkActionTweenIndices[0][0]
            ]
        ) != DATALINK_ACTION_TWEEN_COMPLETE) {
        return 0;
    }

    Game_ConfigureDatalinkTween(
        &gGameDatalinkControllerTweens[DATALINK_ACTION_AUXILIARY_TWEEN],
        Game_DatalinkActionCoordinateToFx(24),
        Game_DatalinkActionCoordinateToFx(180),
        DATALINK_ACTION_TWEEN_DURATION,
        DATALINK_ACTION_TWEEN_MODE_IN
    );
    controller->transition_timer_60 = DATALINK_ACTION_RETURN_RESOURCE;
    return 1;
}

static void Game_IssueDatalinkParticipantActionCommands(
    const Game_DatalinkController *controller
)
{
    int row;

    for (row = 0; row < DATALINK_ACTION_ROW_COUNT; ++row) {
        int variant = (controller->selected_row_5b == row) ? 0 : 1;

        Game_ApplyDatalinkSceneCommand(
            &gGameDatalinkSceneOwner,
            sDatalinkActionTweenIndices[row][variant]
        );
    }
    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkSceneOwner,
        DATALINK_ACTION_TAIL_COMMAND
    );
}

/*
 * 0x020AA4AC
 *
 * Complete 1,532-byte two-choice participant-action controller through
 * 0x020AAAA7. Its phase machine animates records 17/18 and 15/16, lets the
 * player keep or remove the active slot, commits the owner after removal, and
 * returns one once its ordinary exit animation completes. The 20-word pool at
 * 0x020AAAA8..0x020AAAF7 and the independent dispatcher at
 * 0x020AAAF8..0x020AAB7F are deliberately excluded.
 */
int Game_UpdateDatalinkControllerParticipantAction(
    Game_DatalinkController *controller
)
{
    uint8_t phase = controller->transition_phase_5d;
    int previous_selection = controller->selected_row_5b;
    int result = 0;

    if (phase == DATALINK_ACTION_PHASE_INITIALIZE) {
        Game_InitializeDatalinkParticipantAction(controller);
    } else if (phase == DATALINK_ACTION_PHASE_SELECT) {
        Game_UpdateDatalinkParticipantActionSelect(
            controller,
            previous_selection
        );
    } else if (phase == DATALINK_ACTION_PHASE_REMOVE_WAIT) {
        Game_UpdateDatalinkParticipantRemoveWait(controller);
    } else if (phase == DATALINK_ACTION_PHASE_OWNER_COMMIT) {
        Game_UpdateDatalinkParticipantOwnerCommit(controller);
    } else if (phase == DATALINK_ACTION_PHASE_EXIT) {
        Game_BeginDatalinkParticipantActionExit(controller);
    } else if (phase == DATALINK_ACTION_PHASE_EXIT_WAIT) {
        result = Game_UpdateDatalinkParticipantActionExitWait(controller);
    }

    Game_IssueDatalinkParticipantActionCommands(controller);
    return result;
}
