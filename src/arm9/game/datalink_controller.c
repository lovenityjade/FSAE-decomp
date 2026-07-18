#include "game/datalink_controller.h"

#include "game/datalink_phase_fifteen.h"
#include "game/datalink_phase_ten_finalizer.h"

#include <stddef.h>

enum {
    DATALINK_STATIC_INITIALIZED = 1,
    DATALINK_INTERFACE_D_VTABLE = 0x02124AF8,
    DATALINK_CONTROLLER_MODE = 1,
    DATALINK_CONTROLLER_SCREEN_RESOURCE = 0x0835,
    DATALINK_CONTROLLER_PREPARED_RESULT = 2
};

extern void Game_ConstructDatalinkControllerBase(
    Game_DatalinkController *controller
);

/* Calls the first virtual method of the interface at the target address. */
extern void Game_InitializeDatalinkInterface(uint32_t interface_address);

/* Host-safe representation of 0x020ACDD8 with its forwarded arguments. */
extern void Game_PushDatalinkControllerTransition(
    Game_DatalinkSceneOwner *scene,
    uint32_t interface_address,
    uint32_t transition_argument,
    Game_DatalinkController *controller
);

/* External helper 0x02053258 rebuilds the screen resources selected by the 16-bit ID. */
extern void Game_LoadDatalinkScreenResource(uint16_t resource_id);

/* Opaque 0x02053340 no-argument screen rebuild. */
extern void Game_RebuildDatalinkScreen(void);

_Static_assert(
    offsetof(Game_DatalinkStaticArena, guard_00c) == 0x0C,
    "datalink controller static guard offset"
);
_Static_assert(
    offsetof(Game_DatalinkStaticArena, interface_d_vtable_100) == 0x100,
    "datalink controller static interface offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, mode_04) == 0x04,
    "datalink controller mode offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, field_30) == 0x30,
    "datalink controller field 30 offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, interface_address_54) == 0x54,
    "datalink controller interface offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, selected_row_5b) == 0x5B,
    "datalink controller selected-row offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, transition_pending_5c) == 0x5C,
    "datalink controller pending offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, transition_phase_5d) == 0x5D,
    "datalink controller phase offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, auxiliary_state_5f) == 0x5F,
    "datalink controller auxiliary-state offset"
);
_Static_assert(
    offsetof(Game_DatalinkController, transition_timer_60) == 0x60,
    "datalink controller timer offset"
);

/*
 * 0x020A8F80
 *
 * Complete 100-byte controller initialization method.  The guarded interface
 * at static-arena +0x100 receives vtable 0x02124AF8 once.  Base construction
 * precedes controller-field initialization and the interface virtual call;
 * field +0x30 is cleared only after that call returns.
 */
void Game_InitializeDatalinkController(Game_DatalinkController *controller)
{
    volatile Game_DatalinkStaticArena *arena =
        &gGameDatalinkStaticArena;
    uint32_t guard = arena->guard_00c;
    volatile uint32_t *interface_address =
        &controller->interface_address_54;
    volatile int8_t *selected_row = &controller->selected_row_5b;
    volatile uint8_t *transition_pending =
        &controller->transition_pending_5c;
    volatile uint8_t *auxiliary_state =
        &controller->auxiliary_state_5f;
    volatile uint32_t *mode = &controller->mode_04;
    volatile uint32_t *field_30 = &controller->field_30;

    if ((guard & DATALINK_STATIC_INITIALIZED) == 0U) {
        arena->interface_d_vtable_100 = DATALINK_INTERFACE_D_VTABLE;
        arena->guard_00c = guard | DATALINK_STATIC_INITIALIZED;
    }

    Game_ConstructDatalinkControllerBase(controller);
    *interface_address = GAME_DATALINK_INTERFACE_D_ADDRESS;
    *selected_row = 0;
    *transition_pending = 0;
    *auxiliary_state = 0;
    *mode = DATALINK_CONTROLLER_MODE;
    Game_InitializeDatalinkInterface(*interface_address);
    *field_30 = 0;
}

/*
 * 0x020A8FF0
 *
 * Complete 76-byte transition method.  A nonzero completion-owner byte +0xD0
 * forwards the interface, transition argument, and controller to the scene's
 * transition stack.  Otherwise the controller is reset into pending phase
 * zero and screen resource 0x0835 is loaded.  The second and fourth incoming
 * virtual-method arguments are not consumed by the target.
 */
void Game_BeginDatalinkControllerTransition(
    Game_DatalinkController *controller,
    uint32_t unused_argument,
    uint32_t transition_argument,
    uint32_t ignored_fourth_argument
)
{
    volatile uint8_t *transition_pending =
        &controller->transition_pending_5c;
    volatile uint8_t *transition_phase =
        &controller->transition_phase_5d;
    volatile int8_t *selected_row = &controller->selected_row_5b;
    volatile uint16_t *transition_timer =
        &controller->transition_timer_60;

    (void)unused_argument;
    (void)ignored_fourth_argument;

    if (gGameDatalinkCompletionOwner.completion_flag_d0 != 0U) {
        Game_PushDatalinkControllerTransition(
            &gGameDatalinkSceneOwner,
            controller->interface_address_54,
            transition_argument,
            controller
        );
        return;
    }

    *transition_pending = 1;
    *transition_phase = 0;
    *selected_row = 0;
    *transition_timer = 0;
    Game_LoadDatalinkScreenResource(DATALINK_CONTROLLER_SCREEN_RESOURCE);
}

/*
 * Semantic vtable trampoline.
 * Range: 0x020A9048..0x020A9053.  The three instructions tail-dispatch to
 * the neighbour at 0x02053340.  It is absent from the game-function catalog and therefore does
 * not receive a standalone semantic-progress annotation.
 */
void Game_RebuildDatalinkControllerScreen(void)
{
    Game_RebuildDatalinkScreen();
}

/*
 * 0x020A9054
 *
 * Complete 16-byte wrapper.  It forwards the incoming controller pointer to the
 * zero-variant dispatcher at 0x020A9494 and returns constant two.
 */
int Game_PrepareDatalinkControllerVariant(
    Game_DatalinkController *controller
)
{
    Game_SelectDatalinkControllerVariantZero(controller);
    return DATALINK_CONTROLLER_PREPARED_RESULT;
}
