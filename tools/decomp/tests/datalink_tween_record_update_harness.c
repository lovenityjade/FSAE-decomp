#include "game/datalink_controller.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static unsigned int sTweenUpdateCalls;
static unsigned int sAnimationTickCalls;
static Game_DatalinkTweenRecord *sUpdatedTween;
static int32_t sTweenDeltaFx;
static void *sCellAnimation;
static int32_t sAnimationDeltaFx;

bool Game_UpdateDatalinkTween(
    Game_DatalinkTweenRecord *tween,
    int32_t delta_fx)
{
    ++sTweenUpdateCalls;
    sUpdatedTween = tween;
    sTweenDeltaFx = delta_fx;
    return false;
}

void Game_TickCellAnimation_020c4064(
    void *cell_animation,
    int32_t delta_fx)
{
    ++sAnimationTickCalls;
    sCellAnimation = cell_animation;
    sAnimationDeltaFx = delta_fx;
}

int main(void)
{
    Game_DatalinkTweenRecord tween;

    assert(sizeof(tween) == 0x58);
    assert(offsetof(Game_DatalinkTweenRecord, frame_counter_05) == 0x05);
    assert(offsetof(Game_DatalinkTweenRecord, duration_fx_28) == 0x28);
    assert(
        offsetof(Game_DatalinkTweenRecord, cell_animation_delta_fx_50) ==
        0x50);
    assert(
        offsetof(Game_DatalinkTweenRecord, cell_animation_address_54) ==
        0x54);

    memset(&tween, 0, sizeof(tween));
    tween.duration_fx_28 = 5 << 12;
    tween.frame_counter_05 = 118;
    tween.cell_animation_delta_fx_50 = -0x800;
    tween.cell_animation_address_54 = 0x12345678;
    Game_UpdateDatalinkTweenRecord_020acc2c(&tween);
    assert(sTweenUpdateCalls == 1);
    assert(sUpdatedTween == &tween);
    assert(sTweenDeltaFx == 0x1000);
    assert(tween.frame_counter_05 == 119);
    assert(sAnimationTickCalls == 1);
    assert(sCellAnimation == (void *)(uintptr_t)0x12345678);
    assert(sAnimationDeltaFx == -0x800);

    Game_UpdateDatalinkTweenRecord_020acc2c(&tween);
    assert(sTweenUpdateCalls == 2);
    assert(tween.frame_counter_05 == 0);
    assert(sAnimationTickCalls == 2);

    tween.duration_fx_28 = 0;
    tween.frame_counter_05 = 0xff;
    tween.cell_animation_address_54 = 0;
    Game_UpdateDatalinkTweenRecord_020acc2c(&tween);
    assert(sTweenUpdateCalls == 2);
    assert(tween.frame_counter_05 == 0);
    assert(sAnimationTickCalls == 2);

    tween.duration_fx_28 = -1;
    tween.frame_counter_05 = 120;
    Game_UpdateDatalinkTweenRecord_020acc2c(&tween);
    assert(sTweenUpdateCalls == 3);
    assert(tween.frame_counter_05 == 0);
    return 0;
}
