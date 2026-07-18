#include "game/datalink_extended_menu_controller.h"

#include "game/datalink_controller.h"
#include "game/datalink_phase_ten_finalizer.h"

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
Game_DatalinkControllerPanelOwner gGameDatalinkControllerPanelOwner;
Game_DatalinkSceneOwner gGameDatalinkSceneOwner;

static ConfigureCall sCalls[5];
static size_t sCallCount;

void Game_ConfigureDatalinkTween(
    Game_DatalinkTweenRecord *tween,
    int32_t target_x_fx,
    int32_t target_y_fx,
    uint32_t duration,
    int mode
)
{
    ConfigureCall *call;

    assert(sCallCount < sizeof(sCalls) / sizeof(sCalls[0]));
    call = &sCalls[sCallCount++];
    call->tween = tween;
    call->x_fx = target_x_fx;
    call->y_fx = target_y_fx;
    call->duration = duration;
    call->mode = mode;
}

static void ResetHarness(void)
{
    memset(gGameDatalinkControllerTweens, 0,
           sizeof(gGameDatalinkControllerTweens));
    memset(&gGameDatalinkSceneOwner, 0xA5,
           sizeof(gGameDatalinkSceneOwner));
    memset(sCalls, 0, sizeof(sCalls));
    sCallCount = 0U;
}

static void AssertCall(
    size_t call_index,
    int tween_index,
    int x,
    int y,
    int mode
)
{
    const ConfigureCall *call = &sCalls[call_index];

    assert(call->tween == &gGameDatalinkControllerTweens[tween_index]);
    assert(call->x_fx == x * 0x1000);
    assert(call->y_fx == y * 0x1000);
    assert(call->duration == 20U);
    assert(call->mode == mode);
}

static void AssertCenteredEntranceCalls(void)
{
    assert(sCallCount == 5U);
    AssertCall(0, 26, 128, 64, 2);
    AssertCall(1, 27, 128, 64, 2);
    AssertCall(2, 28, 128, 120, 2);
    AssertCall(3, 29, 128, 120, 2);
    AssertCall(4, 23, 24, 180, 2);
}

static void AssertSeededScene(int x)
{
    assert(gGameDatalinkSceneOwner.extended_menu_item_0_duration_d54 == 0);
    assert(gGameDatalinkSceneOwner.extended_menu_item_0_x_fx_d5c ==
           x * 0x1000);
    assert(gGameDatalinkSceneOwner.extended_menu_item_0_y_fx_d60 ==
           64 * 0x1000);
    assert(gGameDatalinkSceneOwner.extended_menu_item_1_duration_dac == 0);
    assert(gGameDatalinkSceneOwner.extended_menu_item_1_x_fx_db4 ==
           x * 0x1000);
    assert(gGameDatalinkSceneOwner.extended_menu_item_1_y_fx_db8 ==
           64 * 0x1000);
    assert(gGameDatalinkSceneOwner.extended_menu_item_2_duration_e04 == 0);
    assert(gGameDatalinkSceneOwner.extended_menu_item_2_x_fx_e0c ==
           x * 0x1000);
    assert(gGameDatalinkSceneOwner.extended_menu_item_2_y_fx_e10 ==
           120 * 0x1000);
    assert(gGameDatalinkSceneOwner.extended_menu_item_3_duration_e5c == 0);
    assert(gGameDatalinkSceneOwner.extended_menu_item_3_x_fx_e64 ==
           x * 0x1000);
    assert(gGameDatalinkSceneOwner.extended_menu_item_3_y_fx_e68 ==
           120 * 0x1000);
    assert(gGameDatalinkSceneOwner.menu_item_4_duration_c4c == 0);
    assert(gGameDatalinkSceneOwner.menu_item_4_x_fx_c54 == 24 * 0x1000);
    assert(gGameDatalinkSceneOwner.menu_item_4_y_fx_c58 == 204 * 0x1000);
}

static void TestEntranceFromRight(void)
{
    ResetHarness();
    Game_EnterDatalinkExtendedMenuFromRight();
    AssertSeededScene(384);
    AssertCenteredEntranceCalls();
}

static void TestEntranceFromLeft(void)
{
    ResetHarness();
    Game_EnterDatalinkExtendedMenuFromLeft();
    AssertSeededScene(-128);
    AssertCenteredEntranceCalls();
}

static void TestExitToRight(void)
{
    int32_t untouched_duration;

    ResetHarness();
    untouched_duration =
        gGameDatalinkSceneOwner.extended_menu_item_0_duration_d54;
    Game_ExitDatalinkExtendedMenuToRight();
    assert(sCallCount == 5U);
    AssertCall(0, 26, 384, 64, 1);
    AssertCall(1, 27, 384, 64, 1);
    AssertCall(2, 28, 384, 120, 1);
    AssertCall(3, 29, 384, 120, 1);
    AssertCall(4, 23, 24, 204, 1);
    assert(gGameDatalinkSceneOwner.extended_menu_item_0_duration_d54 ==
           untouched_duration);
}

static void TestExitToLeft(void)
{
    ResetHarness();
    Game_ExitDatalinkExtendedMenuToLeft();
    assert(sCallCount == 5U);
    AssertCall(0, 26, -128, 64, 1);
    AssertCall(1, 27, -128, 64, 1);
    AssertCall(2, 28, -128, 120, 1);
    AssertCall(3, 29, -128, 120, 1);
    AssertCall(4, 23, 24, 204, 1);
}

int main(void)
{
    TestEntranceFromRight();
    TestEntranceFromLeft();
    TestExitToRight();
    TestExitToLeft();
    return 0;
}
