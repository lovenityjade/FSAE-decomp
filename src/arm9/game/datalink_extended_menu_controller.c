#include "game/datalink_extended_menu_controller.h"

#include "game/datalink_phase_fifteen.h"
#include "game/datalink_phase_ten_finalizer.h"
#include "game/datalink_runtime.h"
#include "game/datalink_runtime_update.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_EXTENDED_MENU_STATIC_INITIALIZED = 1,
    DATALINK_EXTENDED_MENU_PRIMARY_VTABLE = 0x02124B40,
    DATALINK_EXTENDED_MENU_SECONDARY_PRIMARY_VTABLE = 0x02126414,
    DATALINK_EXTENDED_MENU_SECONDARY_AUXILIARY_VTABLE = 0x0212643C,
    DATALINK_EXTENDED_MENU_MODE = 3,
    DATALINK_EXTENDED_MENU_RESUME_RESOURCE = 0x083C,
    DATALINK_EXTENDED_MENU_COMPLETION_STATE = 4,
    DATALINK_EXTENDED_MENU_RESUMED_PHASE = 1,
    DATALINK_EXTENDED_MENU_FINAL_PHASE = 7
};

extern void Game_ConstructDatalinkExtendedMenuControllerBase(
    Game_DatalinkExtendedMenuController *controller
);
extern void Game_InitializeDatalinkInterface(uint32_t interface_address);
extern void Game_FinalizeDatalinkExtendedMenuControllerBase(
    Game_DatalinkExtendedMenuController *controller,
    uint32_t primary_interface_address
);
extern void Game_PushDatalinkExtendedMenuTransition(
    Game_DatalinkSceneOwner *scene,
    uint32_t secondary_interface_address,
    uint32_t transition_argument,
    uint32_t transition_owner_address
);
extern void Game_LoadDatalinkScreenResource(uint16_t resource_id);

_Static_assert(
    offsetof(Game_DatalinkStaticArena, guard_000) == 0x00,
    "extended menu primary guard offset"
);
_Static_assert(
    offsetof(Game_DatalinkStaticArena, guard_01c) == 0x1C,
    "extended menu secondary guard offset"
);
_Static_assert(
    offsetof(Game_DatalinkStaticArena, extended_menu_primary_vtable_020) ==
        0x20,
    "extended menu primary interface offset"
);
_Static_assert(
    offsetof(
        Game_DatalinkExtendedMenuStaticInterface,
        secondary_vtable_30
    ) == 0x30,
    "extended menu secondary auxiliary vtable offset"
);
_Static_assert(
    offsetof(Game_DatalinkExtendedMenuStaticInterface, shared_vtable_d0) ==
        0xD0,
    "extended menu secondary shared vtable offset"
);
_Static_assert(
    offsetof(
        Game_DatalinkExtendedMenuController,
        primary_interface_address_30
    ) == 0x30,
    "extended menu controller primary address offset"
);
_Static_assert(
    offsetof(Game_DatalinkExtendedMenuController, history_depth_38) == 0x38,
    "extended menu controller history depth offset"
);
_Static_assert(
    offsetof(Game_DatalinkExtendedMenuController, transition_state_5c) ==
        0x5C,
    "extended menu controller transition state offset"
);
_Static_assert(
    offsetof(Game_DatalinkExtendedMenuController, selected_row_5e) == 0x5E,
    "extended menu controller selection offset"
);
_Static_assert(
    offsetof(Game_DatalinkExtendedMenuController, transition_phase_5f) ==
        0x5F,
    "extended menu controller phase offset"
);
_Static_assert(
    offsetof(Game_DatalinkExtendedMenuController, force_final_phase_60) ==
        0x60,
    "extended menu controller forced-phase offset"
);

static void Game_InitializeDatalinkExtendedMenuStaticInterfaces(void)
{
    Game_DatalinkStaticArena *arena = &gGameDatalinkStaticArena;

    if ((arena->guard_01c &
         DATALINK_EXTENDED_MENU_STATIC_INITIALIZED) == 0U) {
        gGameDatalinkExtendedMenuStaticInterface.primary_vtable_00 =
            DATALINK_EXTENDED_MENU_SECONDARY_PRIMARY_VTABLE;
        gGameDatalinkExtendedMenuStaticInterface.secondary_vtable_30 =
            DATALINK_EXTENDED_MENU_SECONDARY_AUXILIARY_VTABLE;
        gGameDatalinkExtendedMenuStaticInterface.shared_vtable_d0 =
            DATALINK_EXTENDED_MENU_PRIMARY_VTABLE;
        arena->guard_01c |= DATALINK_EXTENDED_MENU_STATIC_INITIALIZED;
    }

    if ((arena->guard_000 &
         DATALINK_EXTENDED_MENU_STATIC_INITIALIZED) == 0U) {
        arena->extended_menu_primary_vtable_020 =
            DATALINK_EXTENDED_MENU_PRIMARY_VTABLE;
        arena->guard_000 |= DATALINK_EXTENDED_MENU_STATIC_INITIALIZED;
    }
}

/*
 * 0x020AB524
 *
 * Complete 188-byte mode-three controller constructor through 0x020AB5DF.
 * The six-word pool at 0x020AB5E0..0x020AB5F7 is excluded.
 */
void Game_InitializeDatalinkExtendedMenuController(
    Game_DatalinkExtendedMenuController *controller
)
{
    Game_InitializeDatalinkExtendedMenuStaticInterfaces();
    Game_ConstructDatalinkExtendedMenuControllerBase(controller);

    controller->primary_interface_address_30 =
        GAME_DATALINK_EXTENDED_MENU_PRIMARY_INTERFACE_ADDRESS;
    controller->secondary_interface_address_34 =
        GAME_DATALINK_EXTENDED_MENU_SECONDARY_INTERFACE_ADDRESS;
    Game_InitializeDatalinkInterface(
        controller->primary_interface_address_30
    );
    Game_InitializeDatalinkInterface(
        controller->secondary_interface_address_34
    );
    controller->mode_04 = DATALINK_EXTENDED_MENU_MODE;
    Game_FinalizeDatalinkExtendedMenuControllerBase(
        controller,
        controller->primary_interface_address_30
    );

    controller->selected_row_5e = 0;
    controller->force_final_phase_60 = 0U;
    controller->transition_state_5c = 0U;
    controller->transition_phase_5f = 0U;
    controller->history_depth_38 = 0U;
}

/*
 * 0x020AB5F8
 *
 * Complete 192-byte resume/history method through 0x020AB6B7. Its five-word
 * pool at 0x020AB6B8..0x020AB6CB, the independent bx-lr stub at
 * 0x020AB6CC..0x020AB6CF, and next animation at 0x020AB6D0 are excluded.
 */
void Game_ResumeDatalinkExtendedMenuController(
    Game_DatalinkExtendedMenuController *controller,
    uint32_t unused_second_argument,
    uint32_t transition_argument,
    uint32_t transition_owner_address
)
{
    int32_t resumed_phase = 0;
    uint32_t depth;

    (void)unused_second_argument;

    if (gGameDatalinkAbortUpdate != 0U) {
        if (gGameDatalinkCompletionOwner.completion_flag_d1 ==
            DATALINK_EXTENDED_MENU_COMPLETION_STATE) {
            Game_PushDatalinkExtendedMenuTransition(
                &gGameDatalinkSceneOwner,
                controller->secondary_interface_address_34,
                transition_argument,
                transition_owner_address
            );
            return;
        }

        if (gGameDatalinkExtendedMenuFallbackState == -1) {
            controller->force_final_phase_60 = 1U;
            gGameDatalinkExtendedMenuFallbackState = 0;
        }
        gGameDatalinkAbortUpdate = 0U;
        resumed_phase = DATALINK_EXTENDED_MENU_RESUMED_PHASE;
    }

    depth = controller->history_depth_38;
    if (depth != 0U) {
        --depth;
        controller->history_depth_38 = depth;
        resumed_phase = controller->phase_history_3c[depth];
    }

    controller->transition_state_5c = 0U;
    if (controller->force_final_phase_60 != 0U) {
        controller->transition_phase_5f =
            DATALINK_EXTENDED_MENU_FINAL_PHASE;
    } else if (resumed_phase == 0) {
        controller->transition_phase_5f = 0U;
    } else {
        controller->transition_phase_5f =
            DATALINK_EXTENDED_MENU_RESUMED_PHASE;
    }
    Game_LoadDatalinkScreenResource(
        DATALINK_EXTENDED_MENU_RESUME_RESOURCE
    );
}
