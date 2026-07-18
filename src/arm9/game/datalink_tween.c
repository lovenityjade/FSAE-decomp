#include "game/datalink_controller.h"
#include "game/datalink_tween_math.h"

#include <stdint.h>

enum {
    DATALINK_TWEEN_FRACTION_BITS = 12,
    DATALINK_TWEEN_LINEAR_ADDRESS = 0x020acab4,
    DATALINK_TWEEN_EASE_A_ADDRESS = 0x020acb04,
    DATALINK_TWEEN_EASE_B_ADDRESS = 0x020acb7c
};

extern void MI_CpuFill8(void *destination, uint8_t value, uint32_t size);

typedef void (*Game_DatalinkTweenInterpolationCallback)(
    Game_DatalinkVector2 *output,
    const Game_DatalinkVector2 *start,
    const Game_DatalinkVector2 *end,
    int progress
);

/* 0x020acbf4..0x020acc27; callback literal at 0x020acc28 is excluded. */
void Game_InitializeDatalinkTweenRecord_020acbf4(
    Game_DatalinkTweenRecord *tween,
    uint8_t slot_index)
{
    MI_CpuFill8(tween, 0, sizeof(*tween));
    tween->slot_index_00 = slot_index;
    tween->active_03 = 1;
    tween->interpolation_callback_address_10 =
        DATALINK_TWEEN_LINEAR_ADDRESS;
    tween->initialized_01 = 1;
}

/* 0x020acc7c..0x020acc9f */
bool Game_AdvanceDatalinkTween_020acc7c(
    Game_DatalinkTweenRecord *tween,
    int32_t delta_fx)
{
    int32_t elapsed_fx = tween->elapsed_fx_2c + delta_fx;

    tween->elapsed_fx_2c = elapsed_fx;
    if (elapsed_fx >= tween->duration_fx_28) {
        tween->elapsed_fx_2c = tween->duration_fx_28;
        return true;
    }
    return false;
}

/*
 * 0x020acca0..0x020acd0f
 *
 * The three callback addresses at 0x020acd10..0x020acd1b form a literal pool
 * outside this 112-byte body.  Mode three and out-of-range modes preserve the
 * existing callback after publishing all other tween parameters.
 */
void Game_ConfigureDatalinkTween(
    Game_DatalinkTweenRecord *tween,
    int32_t target_x_fx,
    int32_t target_y_fx,
    uint32_t duration_frames,
    int mode)
{
    tween->start_x_fx_14 = tween->current_x_fx_30;
    tween->start_y_fx_18 = tween->current_y_fx_34;
    tween->target_x_fx_1c = target_x_fx;
    tween->target_y_fx_20 = target_y_fx;
    tween->duration_fx_28 =
        (int32_t)(duration_frames << DATALINK_TWEEN_FRACTION_BITS);
    tween->elapsed_fx_2c = 0;

    switch (mode) {
    case 0:
        tween->interpolation_callback_address_10 =
            DATALINK_TWEEN_LINEAR_ADDRESS;
        break;
    case 1:
        tween->interpolation_callback_address_10 =
            DATALINK_TWEEN_EASE_A_ADDRESS;
        break;
    case 2:
        tween->interpolation_callback_address_10 =
            DATALINK_TWEEN_EASE_B_ADDRESS;
        break;
    default:
        break;
    }
}

/*
 * 0x020acd1c..0x020acd73
 *
 * The preceding callback-address pool ends at 0x020acd1b. The next helper
 * begins directly at 0x020acd74, so this complete 88-byte body has no pool.
 */
bool Game_UpdateDatalinkTween(
    Game_DatalinkTweenRecord *tween,
    int32_t delta_fx)
{
    Game_DatalinkVector2 interpolated;
    Game_DatalinkTweenInterpolationCallback interpolate;
    bool finished = Game_AdvanceDatalinkTween_020acc7c(tween, delta_fx);
    int progress = Game_GetDatalinkTweenProgress(tween);

    interpolate = (Game_DatalinkTweenInterpolationCallback)(uintptr_t)
        tween->interpolation_callback_address_10;
    interpolate(
        &interpolated,
        (const Game_DatalinkVector2 *)(const void *)&tween->start_x_fx_14,
        (const Game_DatalinkVector2 *)(const void *)&tween->target_x_fx_1c,
        progress);
    tween->current_x_fx_30 = interpolated.x;
    tween->current_y_fx_34 = interpolated.y;
    return finished;
}
