#include "game/datalink_menu_controller.h"

#include "game/datalink_phase_fifteen.h"
#include "game/datalink_phase_ten_finalizer.h"
#include "game/datalink_runtime.h"
#include "game/datalink_runtime_update.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_MENU_STATIC_INITIALIZED = 1,
    DATALINK_MENU_PRIMARY_VTABLE = 0x02124B40,
    DATALINK_MENU_SECONDARY_VTABLE = 0x02124B1C,
    DATALINK_MENU_MODE = 2,
    DATALINK_MENU_DEFAULT_SELECTION = 1,
    DATALINK_MENU_PRIMARY_RESOURCE = 0x083B,
    DATALINK_MENU_RESTRICTED_RESOURCE = 0x07EF,
    DATALINK_MENU_COMPLETION_STATE_FIRST = 3,
    DATALINK_MENU_COMPLETION_STATE_COUNT = 2
};

extern void Game_ConstructDatalinkMenuControllerBase(
    Game_DatalinkMenuController *controller
);
extern void Game_InitializeDatalinkInterface(uint32_t interface_address);
extern void Game_FinalizeDatalinkMenuControllerBase(
    Game_DatalinkMenuController *controller,
    uint32_t primary_interface_address
);
extern void Game_PushDatalinkMenuTransition(
    Game_DatalinkSceneOwner *scene,
    uint32_t secondary_interface_slot_address,
    uint32_t transition_argument,
    uint32_t transition_owner_address
);
extern int Game_IsDatalinkPrimaryMenuChoiceAvailable(void);
extern void Game_LoadDatalinkScreenResource(uint16_t resource_id);

_Static_assert(
    offsetof(Game_DatalinkStaticArena, guard_008) == 0x08,
    "datalink menu primary guard offset"
);
_Static_assert(
    offsetof(Game_DatalinkStaticArena, guard_010) == 0x10,
    "datalink menu secondary guard offset"
);
_Static_assert(
    offsetof(Game_DatalinkStaticArena, menu_interface_vtable_080) == 0x80,
    "datalink menu primary interface offset"
);
_Static_assert(
    offsetof(Game_DatalinkMenuController, primary_interface_address_30) ==
        0x30,
    "datalink menu primary address offset"
);
_Static_assert(
    offsetof(
        Game_DatalinkMenuController,
        secondary_interface_slot_address_34
    ) == 0x34,
    "datalink menu secondary slot offset"
);
_Static_assert(
    offsetof(Game_DatalinkMenuController, history_depth_38) == 0x38,
    "datalink menu history depth offset"
);
_Static_assert(
    offsetof(Game_DatalinkMenuController, selection_history_3c) == 0x3C,
    "datalink menu history array offset"
);
_Static_assert(
    offsetof(Game_DatalinkMenuController, selected_row_5d) == 0x5D,
    "datalink menu selection offset"
);
_Static_assert(
    offsetof(Game_DatalinkMenuController, transition_phase_5e) == 0x5E,
    "datalink menu phase offset"
);

static void Game_InitializeDatalinkMenuStaticInterfaces(void)
{
    Game_DatalinkStaticArena *arena = &gGameDatalinkStaticArena;

    if ((arena->guard_008 & DATALINK_MENU_STATIC_INITIALIZED) == 0U) {
        arena->menu_interface_vtable_080 =
            DATALINK_MENU_PRIMARY_VTABLE;
        arena->guard_008 |= DATALINK_MENU_STATIC_INITIALIZED;
    }
    if ((arena->guard_010 & DATALINK_MENU_STATIC_INITIALIZED) == 0U) {
        arena->guard_010 |= DATALINK_MENU_STATIC_INITIALIZED;
        gGameDatalinkMenuSecondaryInterface =
            DATALINK_MENU_SECONDARY_VTABLE;
    }
}

/*
 * 0x020AAB80
 *
 * Complete 156-byte menu-controller constructor through 0x020AAC1B. The
 * five-word pool at 0x020AAC1C..0x020AAC2F is deliberately excluded.
 */
void Game_InitializeDatalinkMenuController(
    Game_DatalinkMenuController *controller
)
{
    Game_InitializeDatalinkMenuStaticInterfaces();
    Game_ConstructDatalinkMenuControllerBase(controller);

    controller->mode_04 = DATALINK_MENU_MODE;
    controller->primary_interface_address_30 =
        GAME_DATALINK_MENU_PRIMARY_INTERFACE_ADDRESS;
    controller->secondary_interface_slot_address_34 =
        GAME_DATALINK_MENU_SECONDARY_SLOT_ADDRESS;
    Game_InitializeDatalinkInterface(
        controller->primary_interface_address_30
    );
    Game_InitializeDatalinkInterface(
        gGameDatalinkMenuSecondaryInterface
    );
    Game_FinalizeDatalinkMenuControllerBase(
        controller,
        controller->primary_interface_address_30
    );
    controller->history_depth_38 = 0U;
    controller->selected_row_5d = 0;
}

/*
 * 0x020AAC30
 *
 * Complete 188-byte resume/history method through 0x020AACEB. Its four-word
 * pool at 0x020AACEC..0x020AACFB, the noncatalogued trampoline and literal at
 * 0x020AACFC..0x020AAD07, and the next function at 0x020AAD08 are excluded.
 */
void Game_ResumeDatalinkMenuController(
    Game_DatalinkMenuController *controller,
    uint32_t unused_second_argument,
    uint32_t transition_argument,
    uint32_t transition_owner_address
)
{
    uint32_t depth;

    (void)unused_second_argument;

    if (gGameDatalinkAbortUpdate != 0U) {
        uint8_t completion_state =
            gGameDatalinkCompletionOwner.completion_flag_d1;

        if ((uint8_t)(completion_state -
                      DATALINK_MENU_COMPLETION_STATE_FIRST) <
            DATALINK_MENU_COMPLETION_STATE_COUNT) {
            Game_PushDatalinkMenuTransition(
                &gGameDatalinkSceneOwner,
                controller->secondary_interface_slot_address_34,
                transition_argument,
                transition_owner_address
            );
            return;
        }
        gGameDatalinkAbortUpdate = 0U;
    }

    depth = controller->history_depth_38;
    if (depth == 0U) {
        controller->transition_phase_5e = 0U;
    } else {
        --depth;
        controller->transition_phase_5e = 1U;
        controller->history_depth_38 = depth;
        controller->selected_row_5d =
            (int8_t)controller->selection_history_3c[depth];
    }

    if (Game_IsDatalinkPrimaryMenuChoiceAvailable() == 0) {
        controller->selected_row_5d =
            DATALINK_MENU_DEFAULT_SELECTION;
        Game_LoadDatalinkScreenResource(
            DATALINK_MENU_RESTRICTED_RESOURCE
        );
    } else {
        Game_LoadDatalinkScreenResource(
            DATALINK_MENU_PRIMARY_RESOURCE
        );
    }
}
