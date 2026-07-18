#include "game/datalink_controller.h"

#include <stdint.h>

enum {
    DATALINK_TWEEN_FRAME_DELTA_FX = 0x1000,
    DATALINK_TWEEN_FRAME_PERIOD = 120
};

extern void Game_TickCellAnimation_020c4064(
    void *cell_animation,
    int32_t delta_fx);

/* 0x020acc2c..0x020acc7b; no literal pool follows this body. */
void Game_UpdateDatalinkTweenRecord_020acc2c(
    Game_DatalinkTweenRecord *tween)
{
    uint8_t next_counter;

    if (tween->duration_fx_28 != 0) {
        (void)Game_UpdateDatalinkTween(
            tween, DATALINK_TWEEN_FRAME_DELTA_FX);
    }

    next_counter = (uint8_t)(tween->frame_counter_05 + 1);
    tween->frame_counter_05 = next_counter;
    if (next_counter >= DATALINK_TWEEN_FRAME_PERIOD) {
        tween->frame_counter_05 = 0;
    }

    if (tween->cell_animation_address_54 != 0) {
        Game_TickCellAnimation_020c4064(
            (void *)(uintptr_t)tween->cell_animation_address_54,
            tween->cell_animation_delta_fx_50);
    }
}
