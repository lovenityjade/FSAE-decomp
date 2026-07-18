#include "game/datalink_controller.h"

#include "game/datalink_phase_ten_finalizer.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_CONTROLLER_SLOT_COUNT = 3,
    DATALINK_TWEEN_RECORD_COUNT = 32,
    DATALINK_TWEEN_COMPLETION_INDEX = 11,
    DATALINK_TWEEN_DURATION = 20,
    DATALINK_TWEEN_MODE = 1,
    DATALINK_TWEEN_COMPLETE = 0x1000,
    DATALINK_CONTROLLER_LOADING_RESOURCE = 0x0834,
    DATALINK_CONTROLLER_PHASE_INITIALIZE = 0,
    DATALINK_CONTROLLER_PHASE_WAIT = 1
};

typedef struct Game_DatalinkTweenPosition {
    int32_t x;
    int32_t y;
} Game_DatalinkTweenPosition;

static const uint8_t
    sDatalinkParticipantTweenIndices[DATALINK_CONTROLLER_SLOT_COUNT][2] = {
        {5, 6},
        {7, 8},
        {9, 10}
    };

static const uint8_t
    sDatalinkControllerCommands[DATALINK_CONTROLLER_SLOT_COUNT][2] = {
        {11, 12},
        {13, 14},
        {15, 16}
    };

static const Game_DatalinkTweenPosition
    sDatalinkBaseTweenPositions[DATALINK_CONTROLLER_SLOT_COUNT] = {
        {44, 48},
        {44, 96},
        {44, 144}
    };

static const Game_DatalinkTweenPosition
    sDatalinkParticipantTweenPositions[DATALINK_CONTROLLER_SLOT_COUNT] = {
        {24, 30},
        {24, 78},
        {24, 126}
    };

/* Opaque 0x020ACCA0 tween configuration. */
extern void Game_ConfigureDatalinkTween(
    Game_DatalinkTweenRecord *tween,
    int32_t target_x_fx,
    int32_t target_y_fx,
    uint32_t duration,
    int mode
);

/* Opaque 0x020ACD74 fixed-point tween progress query. */
extern int32_t Game_GetDatalinkTweenProgress(
    const Game_DatalinkTweenRecord *tween
);

/* Host-safe representation of the direct flow-state dirty-byte store. */
extern void Game_MarkDatalinkFlowRenderDirty(uint32_t flow_state_address);

/* Opaque 0x020ACE40 transition-stack pop. */
extern void Game_PopDatalinkTransition(uint32_t flow_state_address);

/* External helper 0x02053258 rebuilds the screen resources selected by the 16-bit ID. */
extern void Game_LoadDatalinkScreenResource(uint16_t resource_id);

/* Opaque 0x020AD964 scene-object command. */
extern void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command
);

_Static_assert(
    sizeof(Game_DatalinkTweenRecord) == 0x58,
    "datalink controller tween stride"
);
_Static_assert(
    offsetof(Game_DatalinkTweenRecord, active_03) == 0x03,
    "datalink controller tween active offset"
);
_Static_assert(
    sizeof(gGameDatalinkControllerTweens) /
        sizeof(gGameDatalinkControllerTweens[0]) ==
        DATALINK_TWEEN_RECORD_COUNT,
    "datalink controller tween count"
);

static int32_t Game_DatalinkCoordinateToFx(int32_t coordinate)
{
    return coordinate * 0x1000;
}

static void Game_ConfigureDatalinkControllerTweens(void)
{
    int slot;

    Game_ConfigureDatalinkTween(
        &gGameDatalinkControllerPrimaryTween,
        0,
        0,
        DATALINK_TWEEN_DURATION,
        DATALINK_TWEEN_MODE
    );
    Game_ConfigureDatalinkTween(
        &gGameDatalinkControllerSecondaryTween,
        0,
        0,
        DATALINK_TWEEN_DURATION,
        DATALINK_TWEEN_MODE
    );

    for (slot = 0; slot < DATALINK_CONTROLLER_SLOT_COUNT; ++slot) {
        const Game_DatalinkTweenPosition *base_position =
            &sDatalinkBaseTweenPositions[slot];
        const Game_DatalinkTweenPosition *participant_position =
            &sDatalinkParticipantTweenPositions[slot];
        uint8_t first_participant =
            sDatalinkParticipantTweenIndices[slot][0];
        uint8_t second_participant =
            sDatalinkParticipantTweenIndices[slot][1];
        int32_t participant_x_fx =
            Game_DatalinkCoordinateToFx(participant_position->x);
        int32_t participant_y_fx =
            Game_DatalinkCoordinateToFx(participant_position->y);

        Game_ConfigureDatalinkTween(
            &gGameDatalinkControllerTweens[slot],
            Game_DatalinkCoordinateToFx(base_position->x),
            Game_DatalinkCoordinateToFx(base_position->y),
            DATALINK_TWEEN_DURATION,
            DATALINK_TWEEN_MODE
        );
        Game_ConfigureDatalinkTween(
            &gGameDatalinkControllerTweens[first_participant],
            participant_x_fx,
            participant_y_fx,
            DATALINK_TWEEN_DURATION,
            DATALINK_TWEEN_MODE
        );
        Game_ConfigureDatalinkTween(
            &gGameDatalinkControllerTweens[second_participant],
            participant_x_fx,
            participant_y_fx,
            DATALINK_TWEEN_DURATION,
            DATALINK_TWEEN_MODE
        );
    }
}

static void Game_ActivateDatalinkControllerTweens(void)
{
    int slot;

    for (slot = 0; slot < DATALINK_CONTROLLER_SLOT_COUNT; ++slot) {
        uint8_t first_participant =
            sDatalinkParticipantTweenIndices[slot][0];
        uint8_t second_participant =
            sDatalinkParticipantTweenIndices[slot][1];
        volatile uint8_t *base_active =
            &gGameDatalinkControllerTweens[slot].active_03;
        volatile uint8_t *first_active =
            &gGameDatalinkControllerTweens[first_participant].active_03;
        volatile uint8_t *second_active =
            &gGameDatalinkControllerTweens[second_participant].active_03;

        *base_active = 1;
        *first_active = 1;
        *second_active = 1;
    }
}

static void Game_IssueDatalinkControllerCommands(
    const Game_DatalinkController *controller
)
{
    const volatile int8_t *selected_row = &controller->selected_row_5b;
    int slot;

    for (slot = 0; slot < DATALINK_CONTROLLER_SLOT_COUNT; ++slot) {
        int variant = (*selected_row == slot) ? 0 : 1;

        Game_ApplyDatalinkSceneCommand(
            &gGameDatalinkSceneOwner,
            sDatalinkControllerCommands[slot][variant]
        );
    }
}

/*
 * 0x020A9064
 *
 * Complete 588-byte controller-animation update through 0x020A92AF.  Phase
 * zero selects variant one and configures two standalone plus nine indexed
 * tweens, then advances to phase one.  Phase one waits for tween record 11 to
 * report 0x1000; completion activates records 0/1/2 and 5..10, marks the
 * bound flow state's render byte dirty, pops its transition, and loads
 * resource 0x0834.  Every phase finishes by issuing three selection-dependent
 * scene commands.  The three additional virtual-method arguments are unused.
 */
int Game_UpdateDatalinkControllerAnimation(
    Game_DatalinkController *controller,
    uint32_t unused_second_argument,
    uint32_t unused_third_argument,
    uint32_t unused_fourth_argument
)
{
    volatile uint8_t *phase = &controller->transition_phase_5d;
    uint8_t phase_value = *phase;

    (void)unused_second_argument;
    (void)unused_third_argument;
    (void)unused_fourth_argument;

    if (phase_value == DATALINK_CONTROLLER_PHASE_INITIALIZE) {
        Game_SelectDatalinkControllerVariantOne(controller);
        Game_ConfigureDatalinkControllerTweens();
        *phase = (uint8_t)(*phase + 1U);
    } else if (phase_value == DATALINK_CONTROLLER_PHASE_WAIT &&
               Game_GetDatalinkTweenProgress(
                   &gGameDatalinkControllerTweens[
                       DATALINK_TWEEN_COMPLETION_INDEX
                   ]
               ) == DATALINK_TWEEN_COMPLETE) {
        uint32_t flow_state_address;

        Game_ActivateDatalinkControllerTweens();
        flow_state_address = controller->owner_address_08;
        Game_MarkDatalinkFlowRenderDirty(flow_state_address);
        flow_state_address = controller->owner_address_08;
        Game_PopDatalinkTransition(flow_state_address);
        Game_LoadDatalinkScreenResource(
            DATALINK_CONTROLLER_LOADING_RESOURCE
        );
    }

    Game_IssueDatalinkControllerCommands(controller);
    return 0;
}
