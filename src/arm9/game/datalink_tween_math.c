#include "game/datalink_tween_math.h"

enum {
    DATALINK_TWEEN_FRACTION_SCALE = 0x200,
    DATALINK_TWEEN_FRACTION_SHIFT = 9,
    DATALINK_TWEEN_PROGRESS_SHIFT = 3,
    DATALINK_TWEEN_SINE_FRACTION_SHIFT = 12,
    DATALINK_TWEEN_SINE_TABLE_STRIDE = 2,
    DATALINK_TWEEN_SINE_SCALE = 0x517
};

/* NitroSDK's interleaved signed Q12 sine/cosine table at 0x020C91FC. */
extern const int16_t FX_SinCosTable_[8192];

static void DatalinkInterpolateVectorAtWeight(
    Game_DatalinkVector2 *output,
    const Game_DatalinkVector2 *start,
    const Game_DatalinkVector2 *end,
    int weight)
{
    int inverse_weight = DATALINK_TWEEN_FRACTION_SCALE - weight;

    output->x =
        (start->x * inverse_weight + end->x * weight) >>
        DATALINK_TWEEN_FRACTION_SHIFT;
    output->y =
        (start->y * inverse_weight + end->y * weight) >>
        DATALINK_TWEEN_FRACTION_SHIFT;
}

static int DatalinkGetSinusoidalWeight(int progress, int sine_scale)
{
    int sine_index = progress >> 1;
    int sine = FX_SinCosTable_[
        sine_index * DATALINK_TWEEN_SINE_TABLE_STRIDE];
    int adjusted_progress = progress +
        ((sine * sine_scale) >> DATALINK_TWEEN_SINE_FRACTION_SHIFT);

    return adjusted_progress >> DATALINK_TWEEN_PROGRESS_SHIFT;
}

/* 0x020ACAB4 */
void Game_DatalinkInterpolateVectorLinear(
    Game_DatalinkVector2 *output,
    const Game_DatalinkVector2 *start,
    const Game_DatalinkVector2 *end,
    int progress)
{
    int weight = progress >> DATALINK_TWEEN_PROGRESS_SHIFT;

    DatalinkInterpolateVectorAtWeight(output, start, end, weight);
}

/*
 * 0x020ACB04..0x020ACB73: complete 112-byte instruction body.
 * Its two-word literal pool at 0x020ACB74 is not part of the body.
 */
void Game_DatalinkInterpolateVectorEaseInSine(
    Game_DatalinkVector2 *output,
    const Game_DatalinkVector2 *start,
    const Game_DatalinkVector2 *end,
    int progress)
{
    int weight = DatalinkGetSinusoidalWeight(
        progress, -DATALINK_TWEEN_SINE_SCALE);

    DatalinkInterpolateVectorAtWeight(output, start, end, weight);
}

/*
 * 0x020ACB7C..0x020ACBEB: complete 112-byte instruction body.
 * Its two-word literal pool at 0x020ACBEC is not part of the body.
 */
void Game_DatalinkInterpolateVectorEaseOutSine(
    Game_DatalinkVector2 *output,
    const Game_DatalinkVector2 *start,
    const Game_DatalinkVector2 *end,
    int progress)
{
    int weight = DatalinkGetSinusoidalWeight(
        progress, DATALINK_TWEEN_SINE_SCALE);

    DatalinkInterpolateVectorAtWeight(output, start, end, weight);
}
