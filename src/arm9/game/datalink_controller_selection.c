#include "game/datalink_controller.h"

#include "game/datalink_phase_ten_finalizer.h"
#include "game/datalink_slot_readiness.h"
#include "game/datalink_transition_status.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_CONTROLLER_SLOT_COUNT = 3,
    DATALINK_CONTROLLER_PHASE_SELECT = 0,
    DATALINK_CONTROLLER_PHASE_COMMIT = 5,
    DATALINK_CONTROLLER_PHASE_WAIT_READY = 10,
    DATALINK_CONTROLLER_PHASE_WAIT_COUNT = 20,
    DATALINK_CONTROLLER_PHASE_WAIT_EXIT = 25,
    DATALINK_CONTROLLER_RESULT_NONE = 0,
    DATALINK_CONTROLLER_RESULT_CANCEL = 3,
    DATALINK_CONTROLLER_RESULT_COUNT_READY = 4,
    DATALINK_CONTROLLER_RESULT_SELECTION_READY = 5,
    DATALINK_CONTROLLER_TWEEN_COMPLETE = 0x1000,
    DATALINK_CONTROLLER_CONFIRM_INPUT = 1,
    DATALINK_CONTROLLER_CANCEL_INPUT = 2,
    DATALINK_CONTROLLER_LEFT_INPUT = 0x40,
    DATALINK_CONTROLLER_RIGHT_INPUT = 0x80,
    DATALINK_CONTROLLER_UP_INPUT = 0x20,
    DATALINK_CONTROLLER_DOWN_INPUT = 0x10,
    DATALINK_CONTROLLER_MOVE_SOUND = 0x3D,
    DATALINK_CONTROLLER_CONFIRM_SOUND = 0x3E,
    DATALINK_CONTROLLER_CANCEL_SOUND = 0x44,
    DATALINK_CONTROLLER_CURSOR_COMMAND = 3,
    DATALINK_CONTROLLER_CONFIRM_DURATION = 12,
    DATALINK_CONTROLLER_CONFIRM_MODE = 1,
    DATALINK_CONTROLLER_RETURN_PHASE = 1,
    DATALINK_FX_ONE = 0x1000
};

typedef struct Game_DatalinkControllerPosition {
    int32_t x;
    int32_t y;
} Game_DatalinkControllerPosition;

static const uint8_t
    sDatalinkSelectionTweenIndices[DATALINK_CONTROLLER_SLOT_COUNT][2] = {
        {5, 6},
        {7, 8},
        {9, 10}
    };

static const uint8_t
    sDatalinkSelectionCommands[DATALINK_CONTROLLER_SLOT_COUNT][2] = {
        {11, 12},
        {13, 14},
        {15, 16}
    };

/* Signed four-way neighbor table at 0x020DE7D8. */
static const int8_t
    sDatalinkSelectionNeighbors[DATALINK_CONTROLLER_SLOT_COUNT][4] = {
        {-1, 1, -1, -1},
        {0, -1, -1, 2},
        {0, -1, 1, -1}
    };

/* Eight-byte-stride X/Y tables at 0x020DE8E4 and 0x020DE8E8. */
static const Game_DatalinkControllerPosition
    sDatalinkSelectionCursorPositions[DATALINK_CONTROLLER_SLOT_COUNT] = {
        {40, 97},
        {48, 137},
        {136, 137}
    };

/* Opaque 0x020ACD74 tween-progress query. */
extern int32_t Game_GetDatalinkTweenProgress(
    const Game_DatalinkTweenRecord *tween
);

/* Opaque 0x020ACCA0 tween configuration. */
extern void Game_ConfigureDatalinkTween(
    Game_DatalinkTweenRecord *tween,
    int32_t target_x_fx,
    int32_t target_y_fx,
    uint32_t duration,
    int mode
);

/* Opaque 0x020AC898 signed-byte clamp. */
extern void Game_ClampDatalinkSelection(
    volatile int8_t *selection,
    int minimum,
    int maximum
);

/* Opaque 0x02076FE0 sound-effect dispatcher. */
extern void Game_PlayDatalinkSoundEffect(uint32_t sound_id);

/* Opaque 0x020AD964 scene-object command. */
extern void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command
);

/* Opaque 0x020AC038 surrounding state change. */
extern void Game_ChangeDatalinkState(
    int state,
    int participant_count,
    int argument_2,
    int argument_3
);

/* Opaque 0x020AC308 screen-update commit. */
extern void Game_CommitDatalinkScreenUpdate(void);

/* SDK routine reached through the classified 0x020A8158 wrapper. */
extern void CARD_LockBackup(uint32_t owner_address);

/* Host-safe representation of the direct flow-state stack append. */
extern void Game_PushDatalinkControllerReturnPhase(
    uint32_t owner_address,
    uint32_t return_phase
);

/* Opaque 0x020ACDD8 transition-stack operation. */
extern void Game_PushDatalinkSceneTransition(
    Game_DatalinkSceneOwner *scene,
    uint32_t transition_address
);

_Static_assert(
    offsetof(Game_DatalinkController, owner_address_08) == 0x08,
    "datalink controller selection owner offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, interface_address_54) == 0x54,
    "datalink controller selection interface offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, selected_row_5b) == 0x5B,
    "datalink controller selection row offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, transition_phase_5d) == 0x5D,
    "datalink controller selection phase offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, cursor_transition_duration_56c) ==
        0x56C,
    "datalink controller selection cursor duration offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, cursor_x_fx_574) == 0x574,
    "datalink controller selection cursor X offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, cursor_y_fx_578) == 0x578,
    "datalink controller selection cursor Y offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, controller_ready_c27) == 0xC27,
    "datalink controller selection ready offset"
);

static int32_t Game_DatalinkControllerCoordinateToFx(int32_t coordinate)
{
    return coordinate * DATALINK_FX_ONE;
}

static void Game_UpdateDatalinkSelectionVisibility(void)
{
    int slot;

    for (slot = 0; slot < DATALINK_CONTROLLER_SLOT_COUNT; ++slot) {
        uint8_t active =
            (gGameDatalinkControllerActiveSlot == (uint8_t)slot) ? 1U : 0U;
        volatile uint8_t *first_active =
            &gGameDatalinkControllerTweens[
                sDatalinkSelectionTweenIndices[slot][0]
            ].active_03;
        volatile uint8_t *second_active =
            &gGameDatalinkControllerTweens[
                sDatalinkSelectionTweenIndices[slot][1]
            ].active_03;
        volatile uint8_t *slot_active =
            &gGameDatalinkControllerTweens[slot].active_03;

        *first_active = active;
        *second_active = active;
        *slot_active = active;
    }
}

static void Game_IssueDatalinkSelectionCommands(
    const Game_DatalinkController *controller
)
{
    const volatile int8_t *selected_row = &controller->selected_row_5b;
    int slot;

    for (slot = 0; slot < DATALINK_CONTROLLER_SLOT_COUNT; ++slot) {
        int command_variant = (*selected_row == slot) ? 0 : 1;

        Game_ApplyDatalinkSceneCommand(
            &gGameDatalinkSceneOwner,
            sDatalinkSelectionCommands[slot][command_variant]
        );
    }
}

static void Game_IssueDatalinkUnselectedCommands(void)
{
    int slot;

    for (slot = 0; slot < DATALINK_CONTROLLER_SLOT_COUNT; ++slot) {
        Game_ApplyDatalinkSceneCommand(
            &gGameDatalinkSceneOwner,
            sDatalinkSelectionCommands[slot][1]
        );
    }
}

static void Game_ConfigureDatalinkConfirmationTweens(
    Game_DatalinkController *controller
)
{
    Game_SelectDatalinkControllerVariantOne(controller);
    Game_ConfigureDatalinkTween(
        &gGameDatalinkControllerTweens[4],
        Game_DatalinkControllerCoordinateToFx(255),
        Game_DatalinkControllerCoordinateToFx(-32),
        DATALINK_CONTROLLER_CONFIRM_DURATION,
        DATALINK_CONTROLLER_CONFIRM_MODE
    );
    Game_ConfigureDatalinkTween(
        &gGameDatalinkControllerTweens[23],
        Game_DatalinkControllerCoordinateToFx(24),
        Game_DatalinkControllerCoordinateToFx(204),
        DATALINK_CONTROLLER_CONFIRM_DURATION,
        DATALINK_CONTROLLER_CONFIRM_MODE
    );
}

static void Game_UpdateDatalinkSelectionCursor(
    Game_DatalinkController *controller
)
{
    volatile int8_t *selected_row = &controller->selected_row_5b;
    int row;

    Game_ClampDatalinkSelection(
        selected_row,
        0,
        DATALINK_CONTROLLER_SLOT_COUNT - 1
    );
    gGameDatalinkSceneOwner.cursor_transition_duration_56c = 0;
    row = *selected_row;
    gGameDatalinkSceneOwner.cursor_x_fx_574 =
        Game_DatalinkControllerCoordinateToFx(
            sDatalinkSelectionCursorPositions[row].x
        );
    gGameDatalinkSceneOwner.cursor_y_fx_578 =
        Game_DatalinkControllerCoordinateToFx(
            sDatalinkSelectionCursorPositions[row].y
        );
    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkSceneOwner,
        DATALINK_CONTROLLER_CURSOR_COMMAND
    );
}

static void Game_ConfirmDatalinkControllerSelection(
    Game_DatalinkController *controller
)
{
    volatile uint8_t *phase = &controller->transition_phase_5d;
    int selected_row = controller->selected_row_5b;

    if (selected_row == 0) {
        Game_ConfigureDatalinkConfirmationTweens(controller);
        CARD_LockBackup(controller->owner_address_08);
        Game_PushDatalinkControllerReturnPhase(
            controller->owner_address_08,
            DATALINK_CONTROLLER_RETURN_PHASE
        );
        *phase = DATALINK_CONTROLLER_PHASE_WAIT_EXIT;
    } else if (selected_row == 1) {
        int ready_count = Game_CountReadyDatalinkPlayerSlots();

        if (ready_count == 0) {
            Game_ChangeDatalinkState(0, 0, 0, 0);
            *phase = DATALINK_CONTROLLER_PHASE_COMMIT;
        } else if (ready_count == DATALINK_CONTROLLER_SLOT_COUNT) {
            Game_ChangeDatalinkState(1, 0, 0, 0);
            *phase = DATALINK_CONTROLLER_PHASE_COMMIT;
        } else {
            Game_ConfigureDatalinkConfirmationTweens(controller);
            *phase = DATALINK_CONTROLLER_PHASE_WAIT_COUNT;
        }
    } else if (selected_row == 2) {
        if (Game_CountReadyDatalinkPlayerSlots() == 0) {
            Game_ChangeDatalinkState(2, 0, 0, 0);
        } else {
            Game_ConfigureDatalinkConfirmationTweens(controller);
            *phase = DATALINK_CONTROLLER_PHASE_WAIT_READY;
        }
    }
}

static void Game_MoveDatalinkControllerSelection(
    Game_DatalinkController *controller,
    int direction
)
{
    int row = controller->selected_row_5b;
    int next_row = sDatalinkSelectionNeighbors[row][direction];

    if (next_row != -1) {
        controller->selected_row_5b = (int8_t)next_row;
        Game_PlayDatalinkSoundEffect(DATALINK_CONTROLLER_MOVE_SOUND);
    }
}

/*
 * 0x020A94B4
 *
 * Complete 1,448-byte controller-selection state machine through
 * 0x020A9A5B. Phase zero synchronizes the visible three-object groups, waits
 * for tween 11, handles four-way navigation, cancel, and three distinct
 * confirmation paths, then clamps and publishes the cursor position. Phases
 * five, ten, twenty, and twenty-five respectively commit the screen update,
 * report completion results five/four, or queue the controller interface
 * after the primary transition. Every non-early path issues three
 * selection-dependent scene commands. The 17 words at 0x020A9A5C..0x020A9A9F
 * are the routine's literal pool and remain data.
 */
int Game_UpdateDatalinkControllerSelection(
    Game_DatalinkController *controller
)
{
    volatile uint8_t *phase = &controller->transition_phase_5d;
    uint8_t phase_value = *phase;
    int result = DATALINK_CONTROLLER_RESULT_NONE;

    if (phase_value == DATALINK_CONTROLLER_PHASE_SELECT) {
        uint16_t input;

        Game_UpdateDatalinkSelectionVisibility();
        if (Game_GetDatalinkTweenProgress(
                &gGameDatalinkControllerTweens[11]
            ) != DATALINK_CONTROLLER_TWEEN_COMPLETE) {
            Game_IssueDatalinkUnselectedCommands();
            return DATALINK_CONTROLLER_RESULT_NONE;
        }

        input = gGameDatalinkControllerInput;
        if (input == DATALINK_CONTROLLER_LEFT_INPUT) {
            Game_MoveDatalinkControllerSelection(controller, 0);
        } else if (input == DATALINK_CONTROLLER_RIGHT_INPUT) {
            Game_MoveDatalinkControllerSelection(controller, 1);
        } else if (input == DATALINK_CONTROLLER_UP_INPUT) {
            Game_MoveDatalinkControllerSelection(controller, 2);
        } else if (input == DATALINK_CONTROLLER_DOWN_INPUT) {
            Game_MoveDatalinkControllerSelection(controller, 3);
        } else if (input == DATALINK_CONTROLLER_CANCEL_INPUT) {
            Game_PlayDatalinkSoundEffect(DATALINK_CONTROLLER_CANCEL_SOUND);
            result = DATALINK_CONTROLLER_RESULT_CANCEL;
        } else if (input == DATALINK_CONTROLLER_CONFIRM_INPUT) {
            Game_PlayDatalinkSoundEffect(DATALINK_CONTROLLER_CONFIRM_SOUND);
            Game_ConfirmDatalinkControllerSelection(controller);
        }

        Game_UpdateDatalinkSelectionCursor(controller);
    } else if (phase_value == DATALINK_CONTROLLER_PHASE_COMMIT) {
        gGameDatalinkSceneOwner.controller_ready_c27 = 0;
        if (gGameDatalinkControllerInput ==
            DATALINK_CONTROLLER_CONFIRM_INPUT) {
            gGameDatalinkSceneOwner.controller_ready_c27 = 1;
            Game_CommitDatalinkScreenUpdate();
            *phase = DATALINK_CONTROLLER_PHASE_SELECT;
        }
    } else if (phase_value == DATALINK_CONTROLLER_PHASE_WAIT_READY) {
        if (Game_GetDatalinkTweenProgress(
                &gGameDatalinkControllerTweens[11]
            ) == DATALINK_CONTROLLER_TWEEN_COMPLETE) {
            result = DATALINK_CONTROLLER_RESULT_SELECTION_READY;
        }
    } else if (phase_value == DATALINK_CONTROLLER_PHASE_WAIT_COUNT) {
        if (Game_GetDatalinkTweenProgress(
                &gGameDatalinkControllerTweens[11]
            ) == DATALINK_CONTROLLER_TWEEN_COMPLETE) {
            result = DATALINK_CONTROLLER_RESULT_COUNT_READY;
        }
    } else if (phase_value == DATALINK_CONTROLLER_PHASE_WAIT_EXIT &&
               Game_IsDatalinkPrimaryTransitionComplete()) {
        Game_PushDatalinkSceneTransition(
            &gGameDatalinkSceneOwner,
            controller->interface_address_54
        );
    }

    Game_IssueDatalinkSelectionCommands(controller);
    return result;
}
