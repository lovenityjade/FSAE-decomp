#ifndef GAME_DATALINK_PLAYER_SLOT_H
#define GAME_DATALINK_PLAYER_SLOT_H

#include <stdint.h>

typedef struct Game_DatalinkPlayerSlot {
    uint8_t unknown_00[0x2C];
    uint8_t reset_flag_2c;
    uint8_t reset_flag_2d;
    uint8_t unknown_2e[0x19];
    int8_t status;
    uint8_t unknown_48[8];
} Game_DatalinkPlayerSlot;

extern Game_DatalinkPlayerSlot gGameDatalinkPlayerSlots[8];

void Game_ResetDatalinkPlayerSlot(int slot);
void Game_RefreshDatalinkPlayerSlot(int slot);
void Game_CleanupDatalinkPlayerSlots(void);

#endif
