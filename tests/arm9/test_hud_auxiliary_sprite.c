#include "game/hud_auxiliary_sprite.h"

#include "game/player_hud_sprite_animation.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

typedef struct CreateCall {
    int x;
    int y;
    int resource_id;
    int palette;
    uint32_t flags;
    void *sprite_context;
} CreateCall;

static Game_HudSpriteRuntime sRuntime;
Game_HudSpriteRuntime *gGameHudSpriteRuntime = &sRuntime;
int gGameHudSpriteAnimationClock;
static int sManagerStorage;
void *gGameHudSpriteManager = &sManagerStorage;

static CreateCall sCall;
static size_t sCreateCount;
static size_t sRegisterCount;
static int sNextHandle;
static uint16_t sRegisteredHandle;

int Game_CreateHudSprite(
    int x,
    int y,
    int resource_id,
    int palette,
    uint32_t flags,
    void *sprite_context
)
{
    ++sCreateCount;
    assert(sCreateCount == 1);
    sCall.x = x;
    sCall.y = y;
    sCall.resource_id = resource_id;
    sCall.palette = palette;
    sCall.flags = flags;
    sCall.sprite_context = sprite_context;
    return sNextHandle;
}

void Game_RegisterHudSprite(
    void *sprite_manager,
    void *sprite_context,
    uint16_t sprite_handle
)
{
    ++sRegisterCount;
    assert(sRegisterCount == 1);
    assert(sprite_manager == gGameHudSpriteManager);
    assert(sprite_context == sRuntime.sprite_context);
    sRegisteredHandle = sprite_handle;
}

static void ResetHarness(int handle)
{
    sCreateCount = 0;
    sRegisterCount = 0;
    sNextHandle = handle;
    sRegisteredHandle = 0;
    sRuntime.sprite_handle_accumulator = 10;
}

static void TestDirectHelperGeometryAndHandleRules(void)
{
    ResetHarness(0x10001);

    Game_DrawAuxiliaryHudSprite(20, 30);

    assert(sCreateCount == 1);
    assert(sRegisterCount == 1);
    assert(sCall.x == 72);
    assert(sCall.y == 14);
    assert(sCall.resource_id == 36);
    assert(sCall.palette == 0);
    assert(sCall.flags == 0);
    assert(sCall.sprite_context == sRuntime.sprite_context);
    assert(sRegisteredHandle == 1);
    assert(sRuntime.sprite_handle_accumulator == 10 + 0x10001);
}

static void TestZeroPositionSuppressesSpriteAndWrapsClock(void)
{
    Game_PlayerHudLoopState state = {0};

    ResetHarness(100);
    gGameHudSpriteAnimationClock = 23;

    Game_UpdateAuxiliaryHudSpriteAndClock(&state);

    assert(sCreateCount == 0);
    assert(sRegisterCount == 0);
    assert(gGameHudSpriteAnimationClock == 0);
    assert(sRuntime.sprite_handle_accumulator == 10);
}

static void TestOneZeroCoordinateStillDrawsAndAdvancesClock(void)
{
    Game_PlayerHudLoopState state = {0};

    state.auxiliary_y = 12;
    ResetHarness(100);
    gGameHudSpriteAnimationClock = 7;

    Game_UpdateAuxiliaryHudSpriteAndClock(&state);

    assert(sCreateCount == 1);
    assert(sCall.x == 52);
    assert(sCall.y == -4);
    assert(gGameHudSpriteAnimationClock == 8);
}

int main(void)
{
    TestDirectHelperGeometryAndHandleRules();
    TestZeroPositionSuppressesSpriteAndWrapsClock();
    TestOneZeroCoordinateStillDrawsAndAdvancesClock();
    return 0;
}
