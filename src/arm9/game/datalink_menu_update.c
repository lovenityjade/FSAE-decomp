#include "game/datalink_menu_controller.h"

#include "game/datalink_controller.h"
#include "game/datalink_phase_ten_finalizer.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_MENU_ROW_COUNT = 2,
    DATALINK_MENU_PHASE_ENTER_RIGHT = 0,
    DATALINK_MENU_PHASE_ENTER_LEFT = 1,
    DATALINK_MENU_PHASE_SELECT = 2,
    DATALINK_MENU_PHASE_CANCEL_WAIT = 3,
    DATALINK_MENU_PHASE_PRIMARY_WAIT = 4,
    DATALINK_MENU_PHASE_SECONDARY_WAIT = 5,
    DATALINK_MENU_PHASE_COMMIT_WAIT = 6,
    DATALINK_MENU_PHASE_RESOURCE_DELAY = 7,
    DATALINK_MENU_TWEEN_COMPLETE = 0x1000,
    DATALINK_MENU_CONFIRM_INPUT = 1,
    DATALINK_MENU_CANCEL_INPUT = 2,
    DATALINK_MENU_PREVIOUS_INPUT = 0x40,
    DATALINK_MENU_NEXT_INPUT = 0x80,
    DATALINK_MENU_SELECTION_BITS_READY = 0x40,
    DATALINK_MENU_SELECTION_BITS_MASK = 0x0F,
    DATALINK_MENU_MOVE_SOUND = 0x3D,
    DATALINK_MENU_CONFIRM_SOUND = 0x3E,
    DATALINK_MENU_CANCEL_SOUND = 0x44,
    DATALINK_MENU_CURSOR_COMMAND = 3,
    DATALINK_MENU_TAIL_COMMAND = 23,
    DATALINK_MENU_COMPLETION_TWEEN = 19,
    DATALINK_MENU_DISPLAY_MASK = 0x1F00,
    DATALINK_MENU_DISPLAY_VALUE = 0x1900,
    DATALINK_MENU_FINAL_RESOURCE = 0x84,
    DATALINK_FX_ONE = 0x1000
};

typedef struct Game_DatalinkMenuCursorPosition {
    int32_t x;
    int32_t y;
} Game_DatalinkMenuCursorPosition;

static const uint8_t
    sDatalinkMenuCommands[DATALINK_MENU_ROW_COUNT][2] = {
        {19, 20},
        {21, 22}
    };

static const Game_DatalinkMenuCursorPosition
    sDatalinkMenuCursorPositions[DATALINK_MENU_ROW_COUNT] = {
        {40, 65},
        {40, 121}
    };

extern volatile uint32_t gGameDatalinkControllerDisplayControl;

extern uint32_t Game_GetDatalinkMenuSelectionContext(void);
extern int Game_GetDatalinkMenuConnectionState(
    uint32_t selection_context
);
extern void Game_BeginDatalinkPrimaryMenuAction(
    uint32_t selection_context,
    int zero
);
extern void Game_BeginDatalinkSecondaryMenuAction(void);
extern uint32_t Game_GetDatalinkWirelessSelectionBits(void);
extern uint8_t Game_MapDatalinkWirelessSelection(uint32_t selection);
extern void Game_BeginDatalinkWirelessSelectionCommit(void);
extern int Game_IsDatalinkWirelessSelectionCommitComplete(void);
extern void Game_FinalizeDatalinkWirelessSelection(void);
extern int32_t Game_GetDatalinkTweenProgress(
    const Game_DatalinkTweenRecord *tween
);
extern int Game_IsDatalinkPrimaryMenuChoiceAvailable(void);
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
extern void Game_ClearDatalinkMenuBackground(
    Game_DatalinkMenuController *controller
);
extern void Game_PopDatalinkMenuSceneTransition(
    Game_DatalinkSceneOwner *scene
);
extern void Game_PushDatalinkMenuSceneTransition(
    Game_DatalinkSceneOwner *scene,
    uint32_t secondary_interface_slot_address
);

_Static_assert(
    offsetof(Game_DatalinkMenuController, transition_countdown_5c) == 0x5C,
    "datalink menu countdown offset"
);
_Static_assert(
    offsetof(Game_DatalinkMenuController, selected_row_5d) == 0x5D,
    "datalink menu update selection offset"
);
_Static_assert(
    offsetof(Game_DatalinkMenuController, transition_phase_5e) == 0x5E,
    "datalink menu update phase offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, screen_rebuild_resource_53e) == 0x53E,
    "datalink menu scene resource offset"
);

static int32_t Game_DatalinkMenuUpdateCoordinateToFx(int32_t coordinate)
{
    return coordinate * DATALINK_FX_ONE;
}

static void Game_PushDatalinkMenuSelection(
    Game_DatalinkMenuController *controller
)
{
    uint32_t depth = controller->history_depth_38;

    controller->history_depth_38 = depth + 1U;
    controller->selection_history_3c[depth] =
        (uint32_t)(int32_t)controller->selected_row_5d;
}

static void Game_UpdateDatalinkMenuCursor(
    Game_DatalinkMenuController *controller,
    int previous_selection
)
{
    int minimum =
        (Game_IsDatalinkPrimaryMenuChoiceAvailable() == 0) ? 1 : 0;
    int selected_row;

    Game_ClampDatalinkSelection(
        &controller->selected_row_5d,
        minimum,
        1
    );
    selected_row = controller->selected_row_5d;
    if (selected_row != previous_selection) {
        Game_PlayDatalinkSoundEffect(DATALINK_MENU_MOVE_SOUND);
    }

    gGameDatalinkSceneOwner.cursor_transition_duration_56c = 0;
    gGameDatalinkSceneOwner.cursor_x_fx_574 =
        Game_DatalinkMenuUpdateCoordinateToFx(
            sDatalinkMenuCursorPositions[selected_row].x
        );
    gGameDatalinkSceneOwner.cursor_y_fx_578 =
        Game_DatalinkMenuUpdateCoordinateToFx(
            sDatalinkMenuCursorPositions[selected_row].y
        );
    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkSceneOwner,
        DATALINK_MENU_CURSOR_COMMAND
    );
}

static void Game_UpdateDatalinkMenuSelectionPhase(
    Game_DatalinkMenuController *controller,
    uint32_t selection_context,
    int connection_state,
    int previous_selection
)
{
    uint16_t input;

    gGameDatalinkControllerDisplayControl =
        (gGameDatalinkControllerDisplayControl &
         ~(uint32_t)DATALINK_MENU_DISPLAY_MASK) |
        (uint32_t)DATALINK_MENU_DISPLAY_VALUE;
    if (Game_GetDatalinkTweenProgress(
            &gGameDatalinkControllerTweens[
                DATALINK_MENU_COMPLETION_TWEEN
            ]
        ) != DATALINK_MENU_TWEEN_COMPLETE) {
        return;
    }

    input = gGameDatalinkControllerInput;
    if (input == DATALINK_MENU_PREVIOUS_INPUT) {
        controller->selected_row_5d =
            (int8_t)(controller->selected_row_5d - 1);
    } else if (input == DATALINK_MENU_NEXT_INPUT) {
        controller->selected_row_5d =
            (int8_t)(controller->selected_row_5d + 1);
    } else if (input == DATALINK_MENU_CANCEL_INPUT) {
        controller->transition_phase_5e =
            DATALINK_MENU_PHASE_CANCEL_WAIT;
        Game_ExitDatalinkMenuToRight();
        Game_PlayDatalinkSoundEffect(DATALINK_MENU_CANCEL_SOUND);
    } else if (input == DATALINK_MENU_CONFIRM_INPUT) {
        if (controller->selected_row_5d == 0 &&
            connection_state == 0) {
            Game_BeginDatalinkPrimaryMenuAction(selection_context, 0);
            controller->transition_phase_5e =
                DATALINK_MENU_PHASE_PRIMARY_WAIT;
            Game_PushDatalinkMenuSelection(controller);
            Game_ExitDatalinkMenuToLeft();
            Game_PlayDatalinkSoundEffect(
                DATALINK_MENU_CONFIRM_SOUND
            );
        } else if (controller->selected_row_5d == 1) {
            controller->transition_phase_5e =
                DATALINK_MENU_PHASE_SECONDARY_WAIT;
            Game_BeginDatalinkSecondaryMenuAction();
            Game_PushDatalinkMenuSelection(controller);
            Game_ExitDatalinkMenuToLeft();
            Game_PlayDatalinkSoundEffect(
                DATALINK_MENU_CONFIRM_SOUND
            );
        }
    }

    Game_UpdateDatalinkMenuCursor(controller, previous_selection);
}

static void Game_UpdateDatalinkMenuCancelWait(void)
{
    if (Game_GetDatalinkTweenProgress(
            &gGameDatalinkControllerTweens[
                DATALINK_MENU_COMPLETION_TWEEN
            ]
        ) == DATALINK_MENU_TWEEN_COMPLETE) {
        Game_PopDatalinkMenuSceneTransition(
            &gGameDatalinkSceneOwner
        );
    }
}

static void Game_UpdateDatalinkMenuPrimaryWait(
    const Game_DatalinkMenuController *controller,
    int connection_state
)
{
    if (Game_GetDatalinkTweenProgress(
            &gGameDatalinkControllerTweens[
                DATALINK_MENU_COMPLETION_TWEEN
            ]
        ) == DATALINK_MENU_TWEEN_COMPLETE &&
        connection_state == 2) {
        Game_PushDatalinkMenuSceneTransition(
            &gGameDatalinkSceneOwner,
            controller->secondary_interface_slot_address_34
        );
    }
}

static void Game_UpdateDatalinkMenuSecondaryWait(
    Game_DatalinkMenuController *controller
)
{
    uint32_t selection_bits =
        Game_GetDatalinkWirelessSelectionBits();

    if ((selection_bits & DATALINK_MENU_SELECTION_BITS_READY) != 0U) {
        selection_bits = Game_GetDatalinkWirelessSelectionBits();
        gGameDatalinkMenuMappedSlot =
            Game_MapDatalinkWirelessSelection(
                selection_bits & DATALINK_MENU_SELECTION_BITS_MASK
            );
        Game_BeginDatalinkWirelessSelectionCommit();
        controller->transition_phase_5e =
            DATALINK_MENU_PHASE_COMMIT_WAIT;
    }
}

static void Game_UpdateDatalinkMenuCommitWait(
    Game_DatalinkMenuController *controller
)
{
    if (Game_IsDatalinkWirelessSelectionCommitComplete() != 0) {
        Game_FinalizeDatalinkWirelessSelection();
        controller->transition_countdown_5c = 1U;
        controller->transition_phase_5e =
            DATALINK_MENU_PHASE_RESOURCE_DELAY;
    }
}

static void Game_UpdateDatalinkMenuResourceDelay(
    Game_DatalinkMenuController *controller
)
{
    uint8_t countdown =
        (uint8_t)(controller->transition_countdown_5c - 1U);

    controller->transition_countdown_5c = countdown;
    if (countdown == 0U) {
        gGameDatalinkControllerScreenResource =
            DATALINK_MENU_FINAL_RESOURCE;
    }
}

static void Game_IssueDatalinkMenuCommands(
    const Game_DatalinkMenuController *controller
)
{
    int row;

    for (row = 0; row < DATALINK_MENU_ROW_COUNT; ++row) {
        int variant = (controller->selected_row_5d == row) ? 0 : 1;

        Game_ApplyDatalinkSceneCommand(
            &gGameDatalinkSceneOwner,
            sDatalinkMenuCommands[row][variant]
        );
    }
    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkSceneOwner,
        DATALINK_MENU_TAIL_COMMAND
    );
}

/*
 * 0x020AB190
 *
 * Complete 784-byte eight-phase menu controller through 0x020AB49F. The
 * nine-word pool at 0x020AB4A0..0x020AB4C3 and next function at 0x020AB4C4
 * are deliberately excluded.
 */
void Game_UpdateDatalinkMenuController(
    Game_DatalinkMenuController *controller,
    uint32_t unused_second_argument,
    uint32_t unused_third_argument,
    uint32_t unused_fourth_argument
)
{
    uint32_t selection_context =
        Game_GetDatalinkMenuSelectionContext();
    int connection_state =
        Game_GetDatalinkMenuConnectionState(selection_context);
    uint8_t phase = controller->transition_phase_5e;
    int previous_selection = controller->selected_row_5d;

    (void)unused_second_argument;
    (void)unused_third_argument;
    (void)unused_fourth_argument;

    if (phase == DATALINK_MENU_PHASE_ENTER_RIGHT) {
        Game_ClearDatalinkMenuBackground(controller);
        Game_EnterDatalinkMenuFromRight();
        controller->transition_phase_5e =
            DATALINK_MENU_PHASE_SELECT;
    } else if (phase == DATALINK_MENU_PHASE_ENTER_LEFT) {
        Game_EnterDatalinkMenuFromLeft();
        controller->transition_phase_5e =
            DATALINK_MENU_PHASE_SELECT;
    } else if (phase == DATALINK_MENU_PHASE_SELECT) {
        Game_UpdateDatalinkMenuSelectionPhase(
            controller,
            selection_context,
            connection_state,
            previous_selection
        );
    } else if (phase == DATALINK_MENU_PHASE_CANCEL_WAIT) {
        Game_UpdateDatalinkMenuCancelWait();
    } else if (phase == DATALINK_MENU_PHASE_PRIMARY_WAIT) {
        Game_UpdateDatalinkMenuPrimaryWait(
            controller,
            connection_state
        );
    } else if (phase == DATALINK_MENU_PHASE_SECONDARY_WAIT) {
        Game_UpdateDatalinkMenuSecondaryWait(controller);
    } else if (phase == DATALINK_MENU_PHASE_COMMIT_WAIT) {
        Game_UpdateDatalinkMenuCommitWait(controller);
    } else if (phase == DATALINK_MENU_PHASE_RESOURCE_DELAY) {
        Game_UpdateDatalinkMenuResourceDelay(controller);
    }

    Game_IssueDatalinkMenuCommands(controller);
}
