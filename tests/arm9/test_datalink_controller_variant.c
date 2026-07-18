#include "game/datalink_controller.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct ConfigureCall {
    Game_DatalinkTweenRecord *tween;
    int32_t x_fx;
    int32_t y_fx;
    uint32_t duration;
    int mode;
} ConfigureCall;

Game_DatalinkTweenRecord gGameDatalinkControllerTweens[32];
Game_DatalinkTweenRecord gGameDatalinkControllerPrimaryTween;
Game_DatalinkTweenRecord gGameDatalinkControllerSecondaryTween;

static ConfigureCall sConfigureCalls[6];
static size_t sConfigureCount;

void Game_ConfigureDatalinkTween(
    Game_DatalinkTweenRecord *tween,
    int32_t target_x_fx,
    int32_t target_y_fx,
    uint32_t duration,
    int mode
)
{
    ConfigureCall *call;

    assert(sConfigureCount <
           sizeof(sConfigureCalls) / sizeof(sConfigureCalls[0]));
    call = &sConfigureCalls[sConfigureCount++];
    call->tween = tween;
    call->x_fx = target_x_fx;
    call->y_fx = target_y_fx;
    call->duration = duration;
    call->mode = mode;
}

static void ResetHarness(void)
{
    memset(gGameDatalinkControllerTweens, 0xA5,
           sizeof(gGameDatalinkControllerTweens));
    memset(sConfigureCalls, 0, sizeof(sConfigureCalls));
    sConfigureCount = 0U;
}

static void AssertConfigureCall(
    size_t call_index,
    int tween_index,
    int32_t x,
    int32_t y,
    int mode
)
{
    const ConfigureCall *call = &sConfigureCalls[call_index];

    assert(call->tween == &gGameDatalinkControllerTweens[tween_index]);
    assert(call->x_fx == x * 0x1000);
    assert(call->y_fx == y * 0x1000);
    assert(call->duration == 20U);
    assert(call->mode == mode);
}

static void AssertSeededTween(int tween_index, int32_t x, int32_t y)
{
    const Game_DatalinkTweenRecord *tween =
        &gGameDatalinkControllerTweens[tween_index];

    assert(tween->active_03 == 1U);
    assert(tween->duration_fx_28 == 0U);
    assert(tween->current_x_fx_30 == x * 0x1000);
    assert(tween->current_y_fx_34 == y * 0x1000);
}

static void TestVariantZeroSeedsAndMovesSixTweensUpward(void)
{
    Game_DatalinkController controller;

    ResetHarness();
    Game_ConfigureDatalinkControllerVariant(&controller, 0);

    assert(sConfigureCount == 6U);
    AssertConfigureCall(0, 11, 128, 96, 2);
    AssertConfigureCall(1, 12, 128, 96, 2);
    AssertConfigureCall(2, 13, 84, 136, 2);
    AssertConfigureCall(3, 14, 84, 136, 2);
    AssertConfigureCall(4, 15, 172, 136, 2);
    AssertConfigureCall(5, 16, 172, 136, 2);
    AssertSeededTween(11, 128, 240);
    AssertSeededTween(12, 128, 240);
    AssertSeededTween(13, 84, 280);
    AssertSeededTween(14, 84, 280);
    AssertSeededTween(15, 172, 280);
    AssertSeededTween(16, 172, 280);
}

static void TestVariantOneTargetsOffsetWithoutSeeding(void)
{
    Game_DatalinkController controller;

    ResetHarness();
    Game_ConfigureDatalinkControllerVariant(&controller, 1);

    assert(sConfigureCount == 6U);
    AssertConfigureCall(0, 11, 128, 240, 1);
    AssertConfigureCall(1, 12, 128, 240, 1);
    AssertConfigureCall(2, 13, 84, 280, 1);
    AssertConfigureCall(3, 14, 84, 280, 1);
    AssertConfigureCall(4, 15, 172, 280, 1);
    AssertConfigureCall(5, 16, 172, 280, 1);
    assert(gGameDatalinkControllerTweens[11].active_03 == 0xA5U);
    assert((uint32_t)gGameDatalinkControllerTweens[11].duration_fx_28 ==
           UINT32_C(0xA5A5A5A5));
}

static void TestUnknownVariantHasNoEffects(void)
{
    Game_DatalinkController controller;

    ResetHarness();
    Game_ConfigureDatalinkControllerVariant(&controller, 2);

    assert(sConfigureCount == 0U);
    assert(gGameDatalinkControllerTweens[11].active_03 == 0xA5U);
}

static void TestWrappersSelectTheirFixedVariants(void)
{
    Game_DatalinkController controller;

    ResetHarness();
    Game_SelectDatalinkControllerVariantZero(&controller);
    assert(sConfigureCount == 6U);
    assert(sConfigureCalls[0].mode == 2);

    ResetHarness();
    Game_SelectDatalinkControllerVariantOne(&controller);
    assert(sConfigureCount == 6U);
    assert(sConfigureCalls[0].mode == 1);
}

int main(void)
{
    TestVariantZeroSeedsAndMovesSixTweensUpward();
    TestVariantOneTargetsOffsetWithoutSeeding();
    TestUnknownVariantHasNoEffects();
    TestWrappersSelectTheirFixedVariants();
    return 0;
}
