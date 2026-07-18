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

typedef struct RegisterCall {
    void *sprite_manager;
    void *sprite_context;
    uint16_t sprite_handle;
} RegisterCall;

Game_PlayerHudVisualState gGamePlayerHudVisualStates[4];
static Game_HudSpriteRuntime sRuntime;
Game_HudSpriteRuntime *gGameHudSpriteRuntime = &sRuntime;
int gGameHudSpriteAnimationClock;
static int sManagerStorage;
void *gGameHudSpriteManager = &sManagerStorage;

static CreateCall sCreateCalls[3];
static RegisterCall sRegisterCalls[3];
static int sHandles[3];
static size_t sCreateCount;
static size_t sRegisterCount;

int Game_CreateHudSprite(
    int x,
    int y,
    int resource_id,
    int palette,
    uint32_t flags,
    void *sprite_context
)
{
    CreateCall *call;
    size_t index = sCreateCount;

    assert(index < sizeof(sCreateCalls) / sizeof(sCreateCalls[0]));
    call = &sCreateCalls[sCreateCount++];
    call->x = x;
    call->y = y;
    call->resource_id = resource_id;
    call->palette = palette;
    call->flags = flags;
    call->sprite_context = sprite_context;
    return sHandles[index];
}

void Game_RegisterHudSprite(
    void *sprite_manager,
    void *sprite_context,
    uint16_t sprite_handle
)
{
    RegisterCall *call;

    assert(sRegisterCount < sizeof(sRegisterCalls) / sizeof(sRegisterCalls[0]));
    call = &sRegisterCalls[sRegisterCount++];
    call->sprite_manager = sprite_manager;
    call->sprite_context = sprite_context;
    call->sprite_handle = sprite_handle;
}

static void ResetHarness(void)
{
    size_t index;

    sCreateCount = 0;
    sRegisterCount = 0;
    sRuntime.sprite_handle_accumulator = 10;
    for (index = 0; index < 3; ++index) {
        sHandles[index] = (int)(100 + index * 100);
    }
}

static void ExpectCall(
    size_t index,
    int x,
    int y,
    int resource_id,
    int palette,
    uint32_t flags
)
{
    assert(sCreateCalls[index].x == x);
    assert(sCreateCalls[index].y == y);
    assert(sCreateCalls[index].resource_id == resource_id);
    assert(sCreateCalls[index].palette == palette);
    assert(sCreateCalls[index].flags == flags);
    assert(sCreateCalls[index].sprite_context == sRuntime.sprite_context);

    assert(sRegisterCalls[index].sprite_manager == gGameHudSpriteManager);
    assert(sRegisterCalls[index].sprite_context == sRuntime.sprite_context);
    assert(sRegisterCalls[index].sprite_handle == (uint16_t)sHandles[index]);
}

static void TestOtherPlayerWithoutEffect(void)
{
    gGamePlayerHudVisualStates[2].visual_flags = 2;
    gGameHudSpriteAnimationClock = 17;
    ResetHarness();

    Game_DrawPlayerHudSpriteSet(2, 10, 20, -1, false);

    assert(sCreateCount == 2);
    assert(sRegisterCount == 2);
    ExpectCall(0, 62, 4, 34, 9, 0);
    ExpectCall(1, 58, 24, 4, 9, UINT32_C(0x00004000));
    assert(sRuntime.sprite_handle_accumulator == 310);
}

static void TestCurrentPlayerWithEffect(void)
{
    gGamePlayerHudVisualStates[1].visual_flags = 7;
    gGameHudSpriteAnimationClock = 7;
    ResetHarness();

    Game_DrawPlayerHudSpriteSet(1, 100, 50, 13, true);

    assert(sCreateCount == 3);
    assert(sRegisterCount == 3);
    ExpectCall(0, 140, 50, 72, 1, UINT32_C(0x80000000));
    ExpectCall(1, 152, 34, 32, 10, 0);
    ExpectCall(2, 152, 26, 35, 10, 0);
    assert(sRuntime.sprite_handle_accumulator == 610);
}

static void TestEffectAnimationWrapsEveryFourFrames(void)
{
    gGamePlayerHudVisualStates[0].visual_flags = 0;
    gGameHudSpriteAnimationClock = 0;
    ResetHarness();

    Game_DrawPlayerHudSpriteSet(0, 0, 0, 24, true);

    assert(sCreateCount == 3);
    assert(sCreateCalls[0].resource_id == 64);
}

static void TestRegistrationTruncatesButAccumulatorKeepsFullHandle(void)
{
    gGamePlayerHudVisualStates[0].visual_flags = 0;
    gGameHudSpriteAnimationClock = 0;
    ResetHarness();
    sHandles[0] = 0x10001;
    sHandles[1] = 0x20002;

    Game_DrawPlayerHudSpriteSet(0, 0, 0, -1, false);

    assert(sRegisterCalls[0].sprite_handle == 1);
    assert(sRegisterCalls[1].sprite_handle == 2);
    assert(sRuntime.sprite_handle_accumulator == 10 + 0x10001 + 0x20002);
}

int main(void)
{
    TestOtherPlayerWithoutEffect();
    TestCurrentPlayerWithEffect();
    TestEffectAnimationWrapsEveryFourFrames();
    TestRegistrationTruncatesButAccumulatorKeepsFullHandle();
    return 0;
}
