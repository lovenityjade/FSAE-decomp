#ifndef GAME_DATALINK_PHASE_FIFTEEN_H
#define GAME_DATALINK_PHASE_FIFTEEN_H

#include "game/datalink_phase_ten_finalizer.h"

#include <stdint.h>

typedef struct Game_DatalinkCompletionOwner {
    uint8_t unknown_000[0xD0];
    uint8_t completion_flag_d0;
    uint8_t completion_flag_d1;
} Game_DatalinkCompletionOwner;

extern Game_DatalinkCompletionOwner gGameDatalinkCompletionOwner;

void Game_UpdateDatalinkPhaseFifteen(void);

#endif
