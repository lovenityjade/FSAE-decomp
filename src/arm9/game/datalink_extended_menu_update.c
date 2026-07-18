#include "game/datalink_extended_menu_controller.h"

#include "game/datalink_controller.h"
#include "game/datalink_phase_ten_finalizer.h"
#include "game/datalink_phase_thirty.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_EXTENDED_MENU_ROW_COUNT = 2,
    DATALINK_EXTENDED_MENU_PHASE_ENTER_RIGHT = 0,
    DATALINK_EXTENDED_MENU_PHASE_ENTER_LEFT = 1,
    DATALINK_EXTENDED_MENU_PHASE_SELECT = 2,
    DATALINK_EXTENDED_MENU_PHASE_CANCEL_WAIT = 3,
    DATALINK_EXTENDED_MENU_PHASE_PRIMARY_WAIT = 4,
    DATALINK_EXTENDED_MENU_PHASE_PRIMARY_EXIT_WAIT = 5,
    DATALINK_EXTENDED_MENU_PHASE_SECONDARY_EXIT_WAIT = 6,
    DATALINK_EXTENDED_MENU_PHASE_RECOVERY = 7,
    DATALINK_EXTENDED_MENU_TWEEN_COMPLETE = 0x1000,
    DATALINK_EXTENDED_MENU_CONFIRM_INPUT = 1,
    DATALINK_EXTENDED_MENU_CANCEL_INPUT = 2,
    DATALINK_EXTENDED_MENU_PREVIOUS_INPUT = 0x40,
    DATALINK_EXTENDED_MENU_NEXT_INPUT = 0x80,
    DATALINK_EXTENDED_MENU_MOVE_SOUND = 0x3D,
    DATALINK_EXTENDED_MENU_CONFIRM_SOUND = 0x3E,
    DATALINK_EXTENDED_MENU_CANCEL_SOUND = 0x44,
    DATALINK_EXTENDED_MENU_CURSOR_COMMAND = 3,
    DATALINK_EXTENDED_MENU_TAIL_COMMAND = 23,
    DATALINK_EXTENDED_MENU_RECOVERY_COMMAND = 0x35,
    DATALINK_EXTENDED_MENU_COMPLETION_TWEEN = 26,
    DATALINK_EXTENDED_MENU_DISPLAY_MASK = 0x1F00,
    DATALINK_EXTENDED_MENU_DISPLAY_VALUE = 0x1900,
    DATALINK_EXTENDED_MENU_FINAL_RESOURCE = 0x84,
    DATALINK_EXTENDED_MENU_INITIAL_REQUEST_COUNT = 4,
    DATALINK_EXTENDED_MENU_CONNECTION_READY = 8,
    DATALINK_EXTENDED_MENU_RECOVERY_LIMIT = 60,
    DATALINK_EXTENDED_MENU_RECOVERY_X = 224,
    DATALINK_EXTENDED_MENU_RECOVERY_Y = 192,
    DATALINK_EXTENDED_MENU_RESULT_STATE_TWO = 12,
    DATALINK_EXTENDED_MENU_RESULT_STATE_THREE = 14,
    DATALINK_EXTENDED_MENU_RESULT_STATE_OTHER = 13,
    DATALINK_FX_ONE = 0x1000
};

typedef struct Game_DatalinkExtendedMenuCursorPosition {
    int32_t x;
    int32_t y;
} Game_DatalinkExtendedMenuCursorPosition;

static const uint8_t
    sDatalinkExtendedMenuCommands[DATALINK_EXTENDED_MENU_ROW_COUNT][2] = {
        {26, 27},
        {28, 29}
    };

static const Game_DatalinkExtendedMenuCursorPosition
    sDatalinkExtendedMenuCursorPositions[DATALINK_EXTENDED_MENU_ROW_COUNT] = {
        {32, 65},
        {32, 121}
    };

extern volatile uint32_t gGameDatalinkControllerDisplayControl;

extern uint32_t Game_GetDatalinkMenuSelectionContext(void);
extern int Game_GetDatalinkMenuConnectionState(
    uint32_t selection_context
);
extern int Game_IsDatalinkMenuContextReady(uint32_t selection_context);
extern int Game_GetDatalinkMenuContextResult(uint32_t selection_context);
extern void Game_BeginDatalinkSecondaryMenuAction(void);
extern int32_t Game_GetDatalinkTweenProgress(
    const Game_DatalinkTweenRecord *tween
);
extern void Game_ClampDatalinkSelection(
    volatile int8_t *selection,
    int minimum,
    int maximum
);
extern void Game_PlayDatalinkSoundEffect(uint32_t sound_id);
extern void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command
);
extern void Game_PopDatalinkExtendedMenuSceneTransition(
    Game_DatalinkSceneOwner *scene
);
extern void Game_PushDatalinkExtendedMenuSceneTransition(
    Game_DatalinkSceneOwner *scene,
    uint32_t secondary_interface_address
);
extern void Game_CommitDatalinkScreenUpdate(void);
extern void Game_ChangeDatalinkState(
    int state,
    int participant_count,
    int argument_2,
    int argument_3
);

_Static_assert(
    offsetof(Game_DatalinkExtendedMenuSession, requested_count_04) == 0x04,
    "extended menu session request-count offset"
);
_Static_assert(
    offsetof(Game_DatalinkExtendedMenuSession, selection_nibble_08) == 0x08,
    "extended menu session selection-nibble offset"
);
_Static_assert(
    offsetof(Game_DatalinkExtendedMenuSession, pending_value_24) == 0x24,
    "extended menu session pending-value offset"
);

static int32_t Game_DatalinkExtendedMenuUpdateCoordinateToFx(
    int32_t coordinate
)
{
    return coordinate * DATALINK_FX_ONE;
}

static void Game_PushDatalinkExtendedMenuHistoryOne(
    Game_DatalinkExtendedMenuController *controller
)
{
    uint32_t depth = controller->history_depth_38;

    controller->history_depth_38 = depth + 1U;
    controller->phase_history_3c[depth] = 1;
}

static void Game_UpdateDatalinkExtendedMenuCursor(
    Game_DatalinkExtendedMenuController *controller,
    int previous_selection
)
{
    int selected_row;

    Game_ClampDatalinkSelection(&controller->selected_row_5e, 0, 1);
    selected_row = controller->selected_row_5e;
    if (selected_row != previous_selection) {
        Game_PlayDatalinkSoundEffect(
            DATALINK_EXTENDED_MENU_MOVE_SOUND
        );
    }

    gGameDatalinkSceneOwner.cursor_transition_duration_56c = 0;
    gGameDatalinkSceneOwner.cursor_x_fx_574 =
        Game_DatalinkExtendedMenuUpdateCoordinateToFx(
            sDatalinkExtendedMenuCursorPositions[selected_row].x
        );
    gGameDatalinkSceneOwner.cursor_y_fx_578 =
        Game_DatalinkExtendedMenuUpdateCoordinateToFx(
            sDatalinkExtendedMenuCursorPositions[selected_row].y
        );
    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkSceneOwner,
        DATALINK_EXTENDED_MENU_CURSOR_COMMAND
    );
}

static void Game_UpdateDatalinkExtendedMenuSelect(
    Game_DatalinkExtendedMenuController *controller,
    uint32_t selection_context,
    int connection_state
)
{
    int previous_selection;
    uint16_t input;

    gGameDatalinkControllerDisplayControl =
        (gGameDatalinkControllerDisplayControl &
         ~(uint32_t)DATALINK_EXTENDED_MENU_DISPLAY_MASK) |
        (uint32_t)DATALINK_EXTENDED_MENU_DISPLAY_VALUE;
    if (Game_GetDatalinkTweenProgress(
            &gGameDatalinkControllerTweens[
                DATALINK_EXTENDED_MENU_COMPLETION_TWEEN
            ]
        ) != DATALINK_EXTENDED_MENU_TWEEN_COMPLETE) {
        return;
    }

    previous_selection = controller->selected_row_5e;
    input = gGameDatalinkControllerInput;
    if (input == DATALINK_EXTENDED_MENU_PREVIOUS_INPUT) {
        controller->selected_row_5e =
            (int8_t)(controller->selected_row_5e - 1);
    } else if (input == DATALINK_EXTENDED_MENU_NEXT_INPUT) {
        controller->selected_row_5e =
            (int8_t)(controller->selected_row_5e + 1);
    } else if (input == DATALINK_EXTENDED_MENU_CANCEL_INPUT) {
        if (connection_state == 0 ||
            Game_IsDatalinkMenuContextReady(selection_context) != 0) {
            Game_ExitDatalinkExtendedMenuToRight();
            Game_PlayDatalinkSoundEffect(
                DATALINK_EXTENDED_MENU_CANCEL_SOUND
            );
            controller->transition_phase_5f =
                DATALINK_EXTENDED_MENU_PHASE_CANCEL_WAIT;
        }
    } else if (input == DATALINK_EXTENDED_MENU_CONFIRM_INPUT) {
        if (controller->selected_row_5e == 0) {
            if (Game_PrepareDatalinkExtendedMenuPrimaryAction() != 0) {
                controller->transition_phase_5f =
                    DATALINK_EXTENDED_MENU_PHASE_PRIMARY_WAIT;
            }
        } else {
            Game_ExitDatalinkExtendedMenuToLeft();
            controller->transition_phase_5f =
                DATALINK_EXTENDED_MENU_PHASE_SECONDARY_EXIT_WAIT;
        }
        Game_PlayDatalinkSoundEffect(
            DATALINK_EXTENDED_MENU_CONFIRM_SOUND
        );
    }

    Game_UpdateDatalinkExtendedMenuCursor(
        controller,
        previous_selection
    );
}

static void Game_UpdateDatalinkExtendedMenuCancelWait(
    uint32_t selection_context,
    int connection_state
)
{
    if (Game_GetDatalinkTweenProgress(
            &gGameDatalinkControllerTweens[
                DATALINK_EXTENDED_MENU_COMPLETION_TWEEN
            ]
        ) != DATALINK_EXTENDED_MENU_TWEEN_COMPLETE) {
        return;
    }

    if (connection_state == 0) {
        Game_PopDatalinkExtendedMenuSceneTransition(
            &gGameDatalinkSceneOwner
        );
    } else if (connection_state != 3) {
        (void)Game_IsDatalinkMenuContextReady(selection_context);
    }
}

static void Game_UpdateDatalinkExtendedMenuPrimaryWait(
    Game_DatalinkExtendedMenuController *controller,
    int connection_state
)
{
    if (connection_state == DATALINK_EXTENDED_MENU_CONNECTION_READY) {
        Game_BeginDatalinkSecondaryMenuAction();
        gGameDatalinkExtendedMenuSession.pending_value_24 = 0U;
        Game_ExitDatalinkExtendedMenuToLeft();
        controller->transition_phase_5f =
            DATALINK_EXTENDED_MENU_PHASE_PRIMARY_EXIT_WAIT;
    }
}

static void Game_UpdateDatalinkExtendedMenuPrimaryExitWait(void)
{
    if (Game_GetDatalinkTweenProgress(
            &gGameDatalinkControllerTweens[
                DATALINK_EXTENDED_MENU_COMPLETION_TWEEN
            ]
        ) == DATALINK_EXTENDED_MENU_TWEEN_COMPLETE) {
        gGameDatalinkControllerScreenResource =
            DATALINK_EXTENDED_MENU_FINAL_RESOURCE;
    }
}

static void Game_UpdateDatalinkExtendedMenuSecondaryExitWait(
    Game_DatalinkExtendedMenuController *controller
)
{
    if (Game_GetDatalinkTweenProgress(
            &gGameDatalinkControllerTweens[
                DATALINK_EXTENDED_MENU_COMPLETION_TWEEN
            ]
        ) == DATALINK_EXTENDED_MENU_TWEEN_COMPLETE) {
        Game_PushDatalinkExtendedMenuHistoryOne(controller);
        Game_PushDatalinkExtendedMenuSceneTransition(
            &gGameDatalinkSceneOwner,
            controller->secondary_interface_address_34
        );
    }
}

static int Game_UpdateDatalinkExtendedMenuRecovery(
    Game_DatalinkExtendedMenuController *controller,
    uint32_t selection_context
)
{
    int16_t counter;

    if (controller->force_final_phase_60 != 0U) {
        int result = Game_GetDatalinkMenuContextResult(selection_context);
        int next_state;

        if (result == 2) {
            next_state = DATALINK_EXTENDED_MENU_RESULT_STATE_TWO;
        } else if (result == 3) {
            next_state = DATALINK_EXTENDED_MENU_RESULT_STATE_THREE;
        } else {
            next_state = DATALINK_EXTENDED_MENU_RESULT_STATE_OTHER;
        }
        Game_ChangeDatalinkState(next_state, 0, 0, 0);
        controller->force_final_phase_60 = 0U;
        return 1;
    }

    counter = (int16_t)controller->transition_state_5c;
    if (counter <= DATALINK_EXTENDED_MENU_RECOVERY_LIMIT) {
        controller->transition_state_5c =
            (uint16_t)((int32_t)counter + 1);
        return 1;
    }

    gGameDatalinkPhaseThirtyObjectOwner.position_transition_duration_69c = 0;
    gGameDatalinkPhaseThirtyObjectOwner.position_x_fx_6a4 =
        Game_DatalinkExtendedMenuUpdateCoordinateToFx(
            DATALINK_EXTENDED_MENU_RECOVERY_X
        );
    gGameDatalinkPhaseThirtyObjectOwner.position_y_fx_6a8 =
        Game_DatalinkExtendedMenuUpdateCoordinateToFx(
            DATALINK_EXTENDED_MENU_RECOVERY_Y
        );
    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkSceneOwner,
        DATALINK_EXTENDED_MENU_RECOVERY_COMMAND
    );
    if (gGameDatalinkControllerInput != DATALINK_EXTENDED_MENU_CONFIRM_INPUT) {
        return 1;
    }

    Game_CommitDatalinkScreenUpdate();
    controller->transition_phase_5f =
        DATALINK_EXTENDED_MENU_PHASE_ENTER_RIGHT;
    controller->transition_state_5c = 0U;
    Game_PlayDatalinkSoundEffect(
        DATALINK_EXTENDED_MENU_CONFIRM_SOUND
    );
    return 1;
}

static void Game_IssueDatalinkExtendedMenuCommands(
    const Game_DatalinkExtendedMenuController *controller
)
{
    int row;

    for (row = 0; row < DATALINK_EXTENDED_MENU_ROW_COUNT; ++row) {
        int variant = (controller->selected_row_5e == row) ? 0 : 1;

        Game_ApplyDatalinkSceneCommand(
            &gGameDatalinkSceneOwner,
            sDatalinkExtendedMenuCommands[row][variant]
        );
    }
    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkSceneOwner,
        DATALINK_EXTENDED_MENU_TAIL_COMMAND
    );
}

/*
 * 0x020ABB58
 *
 * Complete 980-byte eight-phase extended-menu controller through
 * 0x020ABF2B. The twelve-word pool at 0x020ABF2C..0x020ABF5B and next
 * function at 0x020ABF5C are excluded.
 */
void Game_UpdateDatalinkExtendedMenuController(
    Game_DatalinkExtendedMenuController *controller
)
{
    uint32_t selection_context;
    int connection_state;
    uint8_t phase;

    gGameDatalinkExtendedMenuSession.selection_nibble_08 =
        (uint8_t)(gGameDatalinkExtendedMenuSelectionWord & 0x0FU);
    selection_context = Game_GetDatalinkMenuSelectionContext();
    connection_state =
        Game_GetDatalinkMenuConnectionState(selection_context);
    phase = controller->transition_phase_5f;

    if (phase == DATALINK_EXTENDED_MENU_PHASE_ENTER_RIGHT) {
        gGameDatalinkExtendedMenuSession.requested_count_04 =
            DATALINK_EXTENDED_MENU_INITIAL_REQUEST_COUNT;
        controller->transition_phase_5f =
            DATALINK_EXTENDED_MENU_PHASE_SELECT;
        Game_EnterDatalinkExtendedMenuFromRight();
    } else if (phase == DATALINK_EXTENDED_MENU_PHASE_ENTER_LEFT) {
        gGameDatalinkExtendedMenuSession.requested_count_04 =
            DATALINK_EXTENDED_MENU_INITIAL_REQUEST_COUNT;
        controller->transition_phase_5f =
            DATALINK_EXTENDED_MENU_PHASE_SELECT;
        Game_EnterDatalinkExtendedMenuFromLeft();
    } else if (phase == DATALINK_EXTENDED_MENU_PHASE_SELECT) {
        Game_UpdateDatalinkExtendedMenuSelect(
            controller,
            selection_context,
            connection_state
        );
    } else if (phase == DATALINK_EXTENDED_MENU_PHASE_CANCEL_WAIT) {
        Game_UpdateDatalinkExtendedMenuCancelWait(
            selection_context,
            connection_state
        );
    } else if (phase == DATALINK_EXTENDED_MENU_PHASE_PRIMARY_WAIT) {
        Game_UpdateDatalinkExtendedMenuPrimaryWait(
            controller,
            connection_state
        );
    } else if (phase == DATALINK_EXTENDED_MENU_PHASE_PRIMARY_EXIT_WAIT) {
        Game_UpdateDatalinkExtendedMenuPrimaryExitWait();
    } else if (phase == DATALINK_EXTENDED_MENU_PHASE_SECONDARY_EXIT_WAIT) {
        Game_UpdateDatalinkExtendedMenuSecondaryExitWait(controller);
    } else if (phase == DATALINK_EXTENDED_MENU_PHASE_RECOVERY) {
        if (Game_UpdateDatalinkExtendedMenuRecovery(
                controller,
                selection_context
            ) != 0) {
            return;
        }
    }

    Game_IssueDatalinkExtendedMenuCommands(controller);
}
