#include "game/player_hud_slot_loop.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct DrawCall {
    int player_index;
    int x;
    int y;
    int effect_frame;
    bool is_current_player;
} DrawCall;

int gGameHudPlayerRotationIndex;

static DrawCall sCalls[4];
static size_t sCallCount;

void Game_DrawPlayerHudSpriteSet(
    int player_index,
    int x,
    int y,
    int effect_frame,
    bool is_current_player
)
{
    DrawCall *call;

    assert(sCallCount < sizeof(sCalls) / sizeof(sCalls[0]));
    call = &sCalls[sCallCount++];
    call->player_index = player_index;
    call->x = x;
    call->y = y;
    call->effect_frame = effect_frame;
    call->is_current_player = is_current_player;
}

static Game_PlayerHudLoopState MakeState(void)
{
    Game_PlayerHudLoopState state = {0};
    int player_index;

    for (player_index = 0; player_index < 4; ++player_index) {
        state.effect_frames[player_index] = -1;
    }
    return state;
}

static void TestRotatedVisitOrderAndFrameUpdates(void)
{
    Game_PlayerHudLoopState state = MakeState();

    state.current_player = 2;
    state.active[0] = 1;
    state.active[2] = 1;
    state.active[3] = 1;
    state.positions[2].x = 20;
    state.positions[2].y = 30;
    state.positions[3].x = 40;
    state.positions[3].y = 50;
    state.effect_frames[1] = 10;
    state.effect_frames[2] = 47;
    state.effect_frames[3] = 5;
    gGameHudPlayerRotationIndex = 1;
    sCallCount = 0;

    Game_UpdateFourPlayerHudSpriteSlots(&state);

    assert(sCallCount == 2);
    assert(sCalls[0].player_index == 2);
    assert(sCalls[0].x == 20);
    assert(sCalls[0].y == 30);
    assert(sCalls[0].effect_frame == 47);
    assert(sCalls[0].is_current_player);
    assert(sCalls[1].player_index == 3);
    assert(sCalls[1].effect_frame == 5);
    assert(!sCalls[1].is_current_player);

    assert(state.effect_frames[0] == -1);
    assert(state.effect_frames[1] == 10);
    assert(state.effect_frames[2] == -1);
    assert(state.effect_frames[3] == 6);
    assert(gGameHudPlayerRotationIndex == 2);
}

static void TestOneZeroCoordinateStillDraws(void)
{
    Game_PlayerHudLoopState state = MakeState();

    state.active[1] = 1;
    state.positions[1].x = 0;
    state.positions[1].y = 12;
    gGameHudPlayerRotationIndex = 0;
    sCallCount = 0;

    Game_UpdateFourPlayerHudSpriteSlots(&state);

    assert(sCallCount == 1);
    assert(sCalls[0].player_index == 1);
    assert(sCalls[0].x == 0);
    assert(sCalls[0].y == 12);
    assert(gGameHudPlayerRotationIndex == 1);
}

static void TestZeroPositionSuppressesDrawButAdvancesFrame(void)
{
    Game_PlayerHudLoopState state = MakeState();

    state.active[3] = 1;
    state.effect_frames[3] = 0;
    gGameHudPlayerRotationIndex = 2;
    sCallCount = 0;

    Game_UpdateFourPlayerHudSpriteSlots(&state);

    assert(sCallCount == 0);
    assert(state.effect_frames[3] == 1);
    assert(gGameHudPlayerRotationIndex == 3);
}

static void TestAllInactiveSlotsPreserveRotationAfterFourAdvances(void)
{
    Game_PlayerHudLoopState state = MakeState();

    state.effect_frames[0] = 9;
    gGameHudPlayerRotationIndex = 3;
    sCallCount = 0;

    Game_UpdateFourPlayerHudSpriteSlots(&state);

    assert(sCallCount == 0);
    assert(state.effect_frames[0] == 9);
    assert(gGameHudPlayerRotationIndex == 3);
}

int main(void)
{
    TestRotatedVisitOrderAndFrameUpdates();
    TestOneZeroCoordinateStillDraws();
    TestZeroPositionSuppressesDrawButAdvancesFrame();
    TestAllInactiveSlotsPreserveRotationAfterFourAdvances();
    return 0;
}
