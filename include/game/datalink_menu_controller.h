#ifndef GAME_DATALINK_MENU_CONTROLLER_H
#define GAME_DATALINK_MENU_CONTROLLER_H

#include <stdint.h>

#define GAME_DATALINK_MENU_PRIMARY_INTERFACE_ADDRESS \
    UINT32_C(0x0217E354)
#define GAME_DATALINK_MENU_SECONDARY_SLOT_ADDRESS \
    UINT32_C(0x0217E498)

typedef struct Game_DatalinkMenuController {
    uint8_t unknown_00[0x04];
    uint32_t mode_04;
    uint8_t unknown_08[0x28];
    uint32_t primary_interface_address_30;
    uint32_t secondary_interface_slot_address_34;
    uint32_t history_depth_38;
    uint32_t selection_history_3c[6];
    uint8_t unknown_54[8];
    uint8_t transition_countdown_5c;
    int8_t selected_row_5d;
    uint8_t transition_phase_5e;
    uint8_t unknown_5f;
} Game_DatalinkMenuController;

extern volatile uint32_t gGameDatalinkMenuSecondaryInterface;
extern volatile uint8_t gGameDatalinkMenuMappedSlot;
extern volatile uint8_t gGameDatalinkControllerScreenResource;
extern void * volatile gGameDatalinkMenuTransferBuffer;

void Game_InitializeDatalinkMenuController(
    Game_DatalinkMenuController *controller
);

void Game_ResumeDatalinkMenuController(
    Game_DatalinkMenuController *controller,
    uint32_t unused_second_argument,
    uint32_t transition_argument,
    uint32_t transition_owner_address
);

void Game_EnterDatalinkMenuFromRight(void);
void Game_ExitDatalinkMenuToLeft(void);
void Game_EnterDatalinkMenuFromLeft(void);
void Game_ExitDatalinkMenuToRight(void);

void Game_UpdateDatalinkMenuController(
    Game_DatalinkMenuController *controller,
    uint32_t unused_second_argument,
    uint32_t unused_third_argument,
    uint32_t unused_fourth_argument
);

void Game_ClearDatalinkMenuBackground(
    Game_DatalinkMenuController *unused_controller
);

#endif
