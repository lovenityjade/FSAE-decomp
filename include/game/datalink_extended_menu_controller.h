#ifndef GAME_DATALINK_EXTENDED_MENU_CONTROLLER_H
#define GAME_DATALINK_EXTENDED_MENU_CONTROLLER_H

#include <stdint.h>

#define GAME_DATALINK_EXTENDED_MENU_PRIMARY_INTERFACE_ADDRESS \
    UINT32_C(0x0217E2F4)
#define GAME_DATALINK_EXTENDED_MENU_SECONDARY_INTERFACE_ADDRESS \
    UINT32_C(0x0217E4FC)

typedef struct Game_DatalinkExtendedMenuStaticInterface {
    uint32_t primary_vtable_00;
    uint8_t unknown_04[0x2C];
    uint32_t secondary_vtable_30;
    uint8_t unknown_34[0x9C];
    uint32_t shared_vtable_d0;
} Game_DatalinkExtendedMenuStaticInterface;

typedef struct Game_DatalinkExtendedMenuController {
    uint8_t unknown_00[0x04];
    uint32_t mode_04;
    uint8_t unknown_08[0x28];
    uint32_t primary_interface_address_30;
    uint32_t secondary_interface_address_34;
    uint32_t history_depth_38;
    int32_t phase_history_3c[6];
    uint8_t unknown_54[8];
    uint16_t transition_state_5c;
    int8_t selected_row_5e;
    uint8_t transition_phase_5f;
    uint8_t force_final_phase_60;
} Game_DatalinkExtendedMenuController;

typedef struct Game_DatalinkExtendedMenuSession {
    uint8_t unknown_00[4];
    uint16_t requested_count_04;
    uint8_t unknown_06[2];
    uint8_t selection_nibble_08;
    uint8_t unknown_09[0x1B];
    uint32_t pending_value_24;
} Game_DatalinkExtendedMenuSession;

extern Game_DatalinkExtendedMenuStaticInterface
    gGameDatalinkExtendedMenuStaticInterface;
extern volatile int8_t gGameDatalinkExtendedMenuFallbackState;
extern volatile uint32_t gGameDatalinkExtendedMenuSelectionWord;
extern Game_DatalinkExtendedMenuSession gGameDatalinkExtendedMenuSession;
extern volatile uint8_t gGameDatalinkControllerScreenResource;

void Game_InitializeDatalinkExtendedMenuController(
    Game_DatalinkExtendedMenuController *controller
);

void Game_ResumeDatalinkExtendedMenuController(
    Game_DatalinkExtendedMenuController *controller,
    uint32_t unused_second_argument,
    uint32_t transition_argument,
    uint32_t transition_owner_address
);

void Game_EnterDatalinkExtendedMenuFromRight(void);
void Game_EnterDatalinkExtendedMenuFromLeft(void);
void Game_ExitDatalinkExtendedMenuToRight(void);
void Game_ExitDatalinkExtendedMenuToLeft(void);

int Game_PrepareDatalinkExtendedMenuPrimaryAction(void);

void Game_UpdateDatalinkExtendedMenuController(
    Game_DatalinkExtendedMenuController *controller
);

#endif
