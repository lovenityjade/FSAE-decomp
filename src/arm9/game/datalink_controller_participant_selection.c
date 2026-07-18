#include "game/datalink_controller.h"

#include "game/datalink_phase_ten_finalizer.h"
#include "game/datalink_slot_readiness.h"
#include "game/datalink_slot_transfer.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_PARTICIPANT_SLOT_COUNT = 3,
    DATALINK_PARTICIPANT_PHASE_INITIALIZE = 0,
    DATALINK_PARTICIPANT_PHASE_SELECT = 5,
    DATALINK_PARTICIPANT_PHASE_TRANSFER = 10,
    DATALINK_PARTICIPANT_PHASE_CANCEL = 15,
    DATALINK_PARTICIPANT_TRANSFER_TIMER = 60,
    DATALINK_PARTICIPANT_TRANSFER_TRIGGER = 30,
    DATALINK_PARTICIPANT_RESULT_NONE = 0,
    DATALINK_PARTICIPANT_RESULT_CANCELLED = 2,
    DATALINK_PARTICIPANT_TWEEN_COMPLETE = 0x1000,
    DATALINK_PARTICIPANT_CONFIRM_INPUT = 1,
    DATALINK_PARTICIPANT_CANCEL_INPUT = 2,
    DATALINK_PARTICIPANT_PREVIOUS_INPUT = 0x40,
    DATALINK_PARTICIPANT_NEXT_INPUT = 0x80,
    DATALINK_PARTICIPANT_MOVE_SOUND = 0x3D,
    DATALINK_PARTICIPANT_CANCEL_SOUND = 0x44,
    DATALINK_PARTICIPANT_CURSOR_COMMAND = 3,
    DATALINK_PARTICIPANT_TAIL_COMMAND_A = 0x1E,
    DATALINK_PARTICIPANT_TAIL_COMMAND_B = 0x17,
    DATALINK_PARTICIPANT_TWEEN_DURATION = 12,
    DATALINK_PARTICIPANT_TWEEN_MODE_IN = 2,
    DATALINK_PARTICIPANT_TWEEN_MODE_OUT = 1,
    DATALINK_PARTICIPANT_MAIN_TWEEN = 30,
    DATALINK_PARTICIPANT_AUX_TWEEN = 23,
    DATALINK_PARTICIPANT_SCREEN_RESOURCE = 0x0836,
    DATALINK_PARTICIPANT_RETURN_RESOURCE = 0x0835,
    DATALINK_PARTICIPANT_DISPLAY_MASK = 0x1F00,
    DATALINK_PARTICIPANT_DISPLAY_VALUE = 0x1900,
    DATALINK_FX_ONE = 0x1000
};

typedef struct Game_DatalinkParticipantPosition {
    int32_t x;
    int32_t y;
} Game_DatalinkParticipantPosition;

static const uint8_t
    sDatalinkParticipantTweenIndices[DATALINK_PARTICIPANT_SLOT_COUNT][2] = {
        {5, 6},
        {7, 8},
        {9, 10}
    };

static const Game_DatalinkParticipantPosition
    sDatalinkMissingParticipantPositions[DATALINK_PARTICIPANT_SLOT_COUNT] = {
        {24, 30},
        {24, 78},
        {24, 126}
    };

static const Game_DatalinkParticipantPosition
    sDatalinkParticipantCursorPositions[DATALINK_PARTICIPANT_SLOT_COUNT] = {
        {9, 29},
        {9, 77},
        {9, 125}
    };

extern volatile uint32_t gGameDatalinkControllerDisplayControl;

/* Opaque 0x020ACCA0 tween configuration. */
extern void Game_ConfigureDatalinkTween(
    Game_DatalinkTweenRecord *tween,
    int32_t target_x_fx,
    int32_t target_y_fx,
    uint32_t duration,
    int mode
);

/* Opaque 0x020ACD74 tween-progress query. */
extern int32_t Game_GetDatalinkTweenProgress(
    const Game_DatalinkTweenRecord *tween
);

/* Opaque 0x02053258 screen-resource loader. */
extern void Game_LoadDatalinkScreenResource(uint16_t resource_id);

/* Opaque 0x020AC898 signed-byte clamp. */
extern void Game_ClampDatalinkSelection(
    volatile int8_t *selection,
    int minimum,
    int maximum
);

/* Opaque 0x02076FE0 sound-effect dispatcher. */
extern void Game_PlayDatalinkSoundEffect(uint32_t sound_id);

/* Opaque 0x020AC038 surrounding state change. */
extern void Game_ChangeDatalinkState(
    int state,
    int participant_count,
    int argument_2,
    int argument_3
);

/* Opaque 0x020AD964 scene-object command. */
extern void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command
);

/* Opaque 0x020ACE40 transition-stack pop. */
extern void Game_PopDatalinkTransition(uint32_t owner_address);

/* Opaque 0x020AC308 screen-update commit. */
extern void Game_CommitDatalinkScreenUpdate(void);

/* Host-safe views of direct stores and the owner virtual method. */
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

/* Host-safe views of 0x020A7918, 0x020A79D8, and 0x020A777C. */
extern void Game_PositionDatalinkControllerOwnerPairs(
    uint32_t owner_address,
    int32_t x_offset,
    int32_t y_offset
);
extern void Game_PositionDatalinkControllerOwnerTriplet(
    uint32_t owner_address,
    int32_t x_offset,
    int32_t y_offset
);
extern void Game_RebuildDatalinkControllerOwnerDescriptors(
    uint32_t owner_address
);

_Static_assert(
    offsetof(Game_DatalinkController, field_30) == 0x30,
    "datalink participant stack depth offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, transition_stack_34) == 0x34,
    "datalink participant stack offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, participant_slots_58) == 0x58,
    "datalink participant slots offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, participant_count_5a) == 0x5A,
    "datalink participant count offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, transition_countdown_5e) == 0x5E,
    "datalink participant countdown offset"
);
_Static_assert(
    offsetof(Game_DatalinkControllerPanelOwner, first_duration_74c) == 0x74C,
    "datalink participant first panel duration offset"
);
_Static_assert(
    offsetof(Game_DatalinkControllerPanelOwner, first_y_fx_758) == 0x758,
    "datalink participant first panel Y offset"
);
_Static_assert(
    offsetof(Game_DatalinkControllerPanelOwner, second_duration_7a4) == 0x7A4,
    "datalink participant second panel duration offset"
);
_Static_assert(
    offsetof(Game_DatalinkControllerPanelOwner, second_y_fx_7b0) == 0x7B0,
    "datalink participant second panel Y offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, controller_overlay_visible_e8f) == 0xE8F,
    "datalink participant overlay-visible offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, controller_overlay_duration_eb4) == 0xEB4,
    "datalink participant overlay-duration offset"
);

static int32_t Game_DatalinkParticipantCoordinateToFx(int32_t coordinate)
{
    return coordinate * DATALINK_FX_ONE;
}

static void Game_PushDatalinkParticipantStack(
    Game_DatalinkController *controller,
    int32_t value
)
{
    uint32_t depth = controller->field_30;

    controller->field_30 = depth + 1U;
    controller->transition_stack_34[depth] = value;
}

static int32_t Game_PopDatalinkParticipantStack(
    Game_DatalinkController *controller
)
{
    uint32_t depth = controller->field_30 - 1U;

    controller->field_30 = depth;
    return controller->transition_stack_34[depth];
}

static void Game_SeedDatalinkParticipantTween(
    Game_DatalinkTweenRecord *tween,
    int32_t x,
    int32_t y
)
{
    tween->duration_fx_28 = 0;
    tween->current_x_fx_30 = Game_DatalinkParticipantCoordinateToFx(x);
    tween->current_y_fx_34 = Game_DatalinkParticipantCoordinateToFx(y);
    tween->active_03 = 1;
}

static void Game_SetDatalinkParticipantTweensActive(uint8_t active)
{
    int slot;

    for (slot = 0; slot < DATALINK_PARTICIPANT_SLOT_COUNT; ++slot) {
        gGameDatalinkControllerTweens[
            sDatalinkParticipantTweenIndices[slot][0]
        ].active_03 = active;
        gGameDatalinkControllerTweens[
            sDatalinkParticipantTweenIndices[slot][1]
        ].active_03 = active;
    }
}

static void Game_ConfigureDatalinkParticipantEntryTweens(void)
{
    Game_ConfigureDatalinkTween(
        &gGameDatalinkControllerTweens[DATALINK_PARTICIPANT_MAIN_TWEEN],
        Game_DatalinkParticipantCoordinateToFx(255),
        Game_DatalinkParticipantCoordinateToFx(0),
        DATALINK_PARTICIPANT_TWEEN_DURATION,
        DATALINK_PARTICIPANT_TWEEN_MODE_IN
    );
    Game_ConfigureDatalinkTween(
        &gGameDatalinkControllerTweens[DATALINK_PARTICIPANT_AUX_TWEEN],
        Game_DatalinkParticipantCoordinateToFx(24),
        Game_DatalinkParticipantCoordinateToFx(180),
        DATALINK_PARTICIPANT_TWEEN_DURATION,
        DATALINK_PARTICIPANT_TWEEN_MODE_IN
    );
}

static void Game_ConfigureDatalinkParticipantExitTweens(void)
{
    Game_ConfigureDatalinkTween(
        &gGameDatalinkControllerTweens[DATALINK_PARTICIPANT_MAIN_TWEEN],
        Game_DatalinkParticipantCoordinateToFx(255),
        Game_DatalinkParticipantCoordinateToFx(-32),
        DATALINK_PARTICIPANT_TWEEN_DURATION,
        DATALINK_PARTICIPANT_TWEEN_MODE_OUT
    );
    Game_ConfigureDatalinkTween(
        &gGameDatalinkControllerTweens[DATALINK_PARTICIPANT_AUX_TWEEN],
        Game_DatalinkParticipantCoordinateToFx(24),
        Game_DatalinkParticipantCoordinateToFx(204),
        DATALINK_PARTICIPANT_TWEEN_DURATION,
        DATALINK_PARTICIPANT_TWEEN_MODE_OUT
    );
}

static void Game_InitializeDatalinkParticipantSelection(
    Game_DatalinkController *controller
)
{
    uint32_t owner_address = controller->owner_address_08;
    int ready_count;
    int missing_index = 0;
    int slot;

    Game_SetDatalinkOwnerSuppressRedraw(owner_address, 1);
    Game_PushDatalinkParticipantStack(
        controller,
        gGameDatalinkControllerPanelOwner.first_y_fx_758
    );
    Game_PushDatalinkParticipantStack(
        controller,
        gGameDatalinkControllerPanelOwner.second_y_fx_7b0
    );

    gGameDatalinkSceneOwner.controller_overlay_visible_e8f = 1;
    gGameDatalinkControllerPanelOwner.first_duration_74c = 0;
    gGameDatalinkControllerPanelOwner.first_x_fx_754 = 0;
    gGameDatalinkControllerPanelOwner.first_y_fx_758 = 0;
    gGameDatalinkControllerPanelOwner.second_duration_7a4 = 0;
    gGameDatalinkControllerPanelOwner.second_x_fx_7ac = 0;
    gGameDatalinkControllerPanelOwner.second_y_fx_7b0 = 0;
    gGameDatalinkSceneOwner.controller_overlay_duration_eb4 = 0;
    gGameDatalinkSceneOwner.controller_overlay_x_fx_ebc =
        Game_DatalinkParticipantCoordinateToFx(255);
    gGameDatalinkSceneOwner.controller_overlay_y_fx_ec0 =
        Game_DatalinkParticipantCoordinateToFx(-32);

    Game_ConfigureDatalinkParticipantEntryTweens();
    controller->selected_row_5b = 0;
    ready_count = Game_CountReadyDatalinkPlayerSlots();
    controller->participant_count_5a =
        (uint8_t)(DATALINK_PARTICIPANT_SLOT_COUNT - ready_count);

    for (slot = 0; slot < DATALINK_PARTICIPANT_SLOT_COUNT; ++slot) {
        if (Game_IsDatalinkPlayerSlotReady(slot) == 0) {
            const Game_DatalinkParticipantPosition *position =
                &sDatalinkMissingParticipantPositions[missing_index];
            Game_DatalinkTweenRecord *first =
                &gGameDatalinkControllerTweens[
                    sDatalinkParticipantTweenIndices[slot][0]
                ];
            Game_DatalinkTweenRecord *second =
                &gGameDatalinkControllerTweens[
                    sDatalinkParticipantTweenIndices[slot][1]
                ];

            controller->participant_slots_58[missing_index] =
                (uint8_t)slot;
            Game_SeedDatalinkParticipantTween(
                first,
                position->x,
                position->y
            );
            Game_SeedDatalinkParticipantTween(
                second,
                position->x,
                position->y
            );
            ++missing_index;
        }
    }

    controller->transition_phase_5d = DATALINK_PARTICIPANT_PHASE_SELECT;
    Game_LoadDatalinkScreenResource(DATALINK_PARTICIPANT_SCREEN_RESOURCE);
    Game_BuildDatalinkControllerDescriptors(controller);
    controller->auxiliary_state_5f = 1;
}

static void Game_UpdateDatalinkParticipantCursor(
    Game_DatalinkController *controller,
    int previous_selection
)
{
    volatile int8_t *selection = &controller->selected_row_5b;
    int row;

    Game_ClampDatalinkSelection(
        selection,
        0,
        (int8_t)(controller->participant_count_5a - 1U)
    );
    row = *selection;
    if (row != previous_selection) {
        controller->auxiliary_state_5f = 1;
        Game_PlayDatalinkSoundEffect(DATALINK_PARTICIPANT_MOVE_SOUND);
    }

    gGameDatalinkSceneOwner.cursor_transition_duration_56c = 0;
    gGameDatalinkSceneOwner.cursor_x_fx_574 =
        Game_DatalinkParticipantCoordinateToFx(
            sDatalinkParticipantCursorPositions[row].x
        );
    gGameDatalinkSceneOwner.cursor_y_fx_578 =
        Game_DatalinkParticipantCoordinateToFx(
            sDatalinkParticipantCursorPositions[row].y
        );
    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkSceneOwner,
        DATALINK_PARTICIPANT_CURSOR_COMMAND
    );
}

static void Game_UpdateDatalinkParticipantSelectPhase(
    Game_DatalinkController *controller,
    int previous_selection
)
{
    uint16_t input;

    if (Game_GetDatalinkTweenProgress(
            &gGameDatalinkControllerTweens[
                DATALINK_PARTICIPANT_MAIN_TWEEN
            ]
        ) != DATALINK_PARTICIPANT_TWEEN_COMPLETE) {
        return;
    }

    input = gGameDatalinkControllerInput;
    if (input == DATALINK_PARTICIPANT_NEXT_INPUT) {
        controller->selected_row_5b =
            (int8_t)(controller->selected_row_5b + 1);
    } else if (input == DATALINK_PARTICIPANT_PREVIOUS_INPUT) {
        controller->selected_row_5b =
            (int8_t)(controller->selected_row_5b - 1);
    } else if (input == DATALINK_PARTICIPANT_CANCEL_INPUT) {
        Game_PlayDatalinkSoundEffect(DATALINK_PARTICIPANT_CANCEL_SOUND);
        Game_ConfigureDatalinkParticipantExitTweens();
        Game_SetDatalinkParticipantTweensActive(0);
        controller->transition_phase_5d =
            DATALINK_PARTICIPANT_PHASE_CANCEL;
    } else if (input == DATALINK_PARTICIPANT_CONFIRM_INPUT) {
        controller->transition_phase_5d =
            DATALINK_PARTICIPANT_PHASE_TRANSFER;
        controller->transition_countdown_5e =
            DATALINK_PARTICIPANT_TRANSFER_TIMER;
        Game_ChangeDatalinkState(3, 0, 0, 0);
        gGameDatalinkControllerDisplayControl =
            (gGameDatalinkControllerDisplayControl &
             ~(uint32_t)DATALINK_PARTICIPANT_DISPLAY_MASK) |
            (uint32_t)DATALINK_PARTICIPANT_DISPLAY_VALUE;
        gGameDatalinkSceneOwner.controller_overlay_visible_e8f = 0;
        Game_SetDatalinkParticipantTweensActive(0);
    }

    Game_UpdateDatalinkParticipantCursor(controller, previous_selection);
}

static int Game_UpdateDatalinkParticipantTransferPhase(
    Game_DatalinkController *controller,
    int selected_row
)
{
    uint8_t countdown;
    uint32_t owner_address;

    gGameDatalinkSceneOwner.controller_ready_c27 = 0;
    countdown = controller->transition_countdown_5e;
    if (countdown == DATALINK_PARTICIPANT_TRANSFER_TRIGGER) {
        Game_TransferDatalinkPlayerSlot(
            gGameDatalinkControllerActiveSlot,
            controller->participant_slots_58[selected_row]
        );
    }

    countdown = (uint8_t)(countdown - 1U);
    controller->transition_countdown_5e = countdown;
    if (countdown != 0U) {
        return 0;
    }

    controller->field_30 -= 2U;
    gGameDatalinkSceneOwner.controller_ready_c27 = 1;
    gGameDatalinkControllerPanelOwner.first_duration_74c = 0;
    gGameDatalinkControllerPanelOwner.first_x_fx_754 = 0;
    gGameDatalinkControllerPanelOwner.first_y_fx_758 = 0;
    gGameDatalinkControllerPanelOwner.second_duration_7a4 = 0;
    gGameDatalinkControllerPanelOwner.second_x_fx_7ac = 0;
    gGameDatalinkControllerPanelOwner.second_y_fx_7b0 = 0;

    owner_address = controller->owner_address_08;
    Game_PopDatalinkTransition(owner_address);
    Game_SetDatalinkOwnerSuppressRedraw(owner_address, 0);
    Game_SetDatalinkOwnerSelectionDirty(owner_address, 1);
    Game_SetDatalinkOwnerSelectedParticipant(owner_address, 0);
    Game_InvokeDatalinkOwnerRefresh(owner_address);
    controller->selected_row_5b = 0;
    Game_CommitDatalinkScreenUpdate();
    return 1;
}

static int Game_UpdateDatalinkParticipantCancelPhase(
    Game_DatalinkController *controller
)
{
    uint32_t owner_address;
    int32_t restored_y_fx;
    int32_t y_offset;

    if (Game_GetDatalinkTweenProgress(
            &gGameDatalinkControllerTweens[
                DATALINK_PARTICIPANT_MAIN_TWEEN
            ]
        ) != DATALINK_PARTICIPANT_TWEEN_COMPLETE) {
        return DATALINK_PARTICIPANT_RESULT_NONE;
    }

    Game_ConfigureDatalinkTween(
        &gGameDatalinkControllerTweens[DATALINK_PARTICIPANT_AUX_TWEEN],
        Game_DatalinkParticipantCoordinateToFx(24),
        Game_DatalinkParticipantCoordinateToFx(180),
        DATALINK_PARTICIPANT_TWEEN_DURATION,
        DATALINK_PARTICIPANT_TWEEN_MODE_IN
    );
    gGameDatalinkControllerPanelOwner.second_y_fx_7b0 =
        Game_PopDatalinkParticipantStack(controller);
    restored_y_fx = Game_PopDatalinkParticipantStack(controller);
    gGameDatalinkControllerPanelOwner.first_y_fx_758 = restored_y_fx;

    owner_address = controller->owner_address_08;
    Game_SetDatalinkOwnerSuppressRedraw(owner_address, 0);
    y_offset = (-gGameDatalinkControllerPanelOwner.second_y_fx_7b0) /
        DATALINK_FX_ONE;
    Game_PositionDatalinkControllerOwnerPairs(owner_address, 0, y_offset);
    Game_PositionDatalinkControllerOwnerTriplet(owner_address, 0, y_offset);
    Game_SetDatalinkOwnerSelectionDirty(owner_address, 1);
    Game_RebuildDatalinkControllerOwnerDescriptors(owner_address);
    Game_SelectDatalinkControllerVariantZero(controller);
    controller->transition_timer_60 =
        DATALINK_PARTICIPANT_RETURN_RESOURCE;
    return DATALINK_PARTICIPANT_RESULT_CANCELLED;
}

static void Game_IssueDatalinkParticipantCommands(
    const Game_DatalinkController *controller
)
{
    const volatile uint8_t *participant_count =
        &controller->participant_count_5a;
    int row = 0;
    int count = *participant_count;

    while (row < count) {
        uint8_t participant_slot = controller->participant_slots_58[row];
        int variant = (controller->selected_row_5b == row) ? 0 : 1;

        Game_ApplyDatalinkSceneCommand(
            &gGameDatalinkSceneOwner,
            sDatalinkParticipantTweenIndices[participant_slot][variant]
        );
        ++row;
        count = *participant_count;
    }

    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkSceneOwner,
        DATALINK_PARTICIPANT_TAIL_COMMAND_A
    );
    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkSceneOwner,
        DATALINK_PARTICIPANT_TAIL_COMMAND_B
    );
}

/*
 * 0x020A9E0C
 *
 * Complete 1,604-byte participant-selection controller through 0x020AA44F.
 * Phase zero builds the missing-player list, seeds its object pairs, prepares
 * two overlay tweens, descriptors, and resource 0x0836. Phase five waits for
 * tween 30 and handles selection, confirm, or cancel. Phase ten performs the
 * timed slot transfer and commits the owner on expiry. Phase 15 restores the
 * saved panel state and returns two after its exit tween. All ordinary paths
 * refresh tilemaps and issue participant-dependent commands plus 30 and 23.
 * The 23-word pool at 0x020AA450..0x020AA4AB is data and the next function
 * begins at 0x020AA4AC.
 */
int Game_UpdateDatalinkControllerParticipantSelection(
    Game_DatalinkController *controller
)
{
    uint8_t phase = controller->transition_phase_5d;
    int previous_selection = controller->selected_row_5b;

    if (phase == DATALINK_PARTICIPANT_PHASE_INITIALIZE) {
        Game_InitializeDatalinkParticipantSelection(controller);
    } else if (phase == DATALINK_PARTICIPANT_PHASE_SELECT) {
        Game_UpdateDatalinkParticipantSelectPhase(
            controller,
            previous_selection
        );
    } else if (phase == DATALINK_PARTICIPANT_PHASE_TRANSFER) {
        if (Game_UpdateDatalinkParticipantTransferPhase(
                controller,
                previous_selection
            ) != 0) {
            return DATALINK_PARTICIPANT_RESULT_NONE;
        }
    } else if (phase == DATALINK_PARTICIPANT_PHASE_CANCEL) {
        int result = Game_UpdateDatalinkParticipantCancelPhase(controller);

        if (result != DATALINK_PARTICIPANT_RESULT_NONE) {
            return result;
        }
    }

    Game_RefreshDatalinkControllerTilemaps(controller);
    Game_IssueDatalinkParticipantCommands(controller);
    return DATALINK_PARTICIPANT_RESULT_NONE;
}
