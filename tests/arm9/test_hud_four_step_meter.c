#include "game/hud_four_step_meter.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

typedef struct RectangleCall {
    uint16_t *screen_tilemap;
    int x;
    int y;
    int width;
    int height;
    int palette;
    uint16_t tile_index;
} RectangleCall;

static RectangleCall sCalls[12];
static size_t sCallCount;

void Game_DrawTileRectangle(
    uint16_t *screen_tilemap,
    int x,
    int y,
    int width,
    int height,
    int palette,
    uint16_t tile_index
)
{
    RectangleCall *call;

    assert(sCallCount < sizeof(sCalls) / sizeof(sCalls[0]));
    call = &sCalls[sCallCount++];
    call->screen_tilemap = screen_tilemap;
    call->x = x;
    call->y = y;
    call->width = width;
    call->height = height;
    call->palette = palette;
    call->tile_index = tile_index;
}

static void ExpectRectangle(
    size_t index,
    uint16_t *screen_tilemap,
    int y,
    int height,
    uint16_t tile_index
)
{
    assert(sCalls[index].screen_tilemap == screen_tilemap);
    assert(sCalls[index].x == 27);
    assert(sCalls[index].y == y);
    assert(sCalls[index].width == 2);
    assert(sCalls[index].height == height);
    assert(sCalls[index].palette == 0);
    assert(sCalls[index].tile_index == tile_index);
}

static void ExpectMeter(int step, int expected_filled_slots)
{
    uint16_t screen_tilemap[32 * 24] = {0};
    int slot;
    size_t call_index;

    sCallCount = 0;
    Game_DrawRightHudFourStepMeter(step, screen_tilemap);

    assert(sCallCount == (size_t)(4 + expected_filled_slots * 2));
    for (slot = 0; slot < 4; ++slot) {
        ExpectRectangle(
            (size_t)slot,
            screen_tilemap,
            4 + slot * 4,
            3,
            384
        );
    }

    call_index = 4;
    for (slot = expected_filled_slots - 1; slot >= 0; --slot) {
        int y = 4 + slot * 4;

        ExpectRectangle(call_index++, screen_tilemap, y, 2, 18);
        ExpectRectangle(call_index++, screen_tilemap, y + 2, 1, 20);
    }
}

static void TestEmptyMeter(void)
{
    ExpectMeter(0, 0);
}

static void TestOneStep(void)
{
    ExpectMeter(1, 1);
}

static void TestThreeStepsPreserveDescendingWriteOrder(void)
{
    ExpectMeter(3, 3);
}

static void TestFullMeter(void)
{
    ExpectMeter(4, 4);
}

static void TestOutOfRangeValuesLeaveMeterEmpty(void)
{
    ExpectMeter(-1, 0);
    ExpectMeter(5, 0);
}

int main(void)
{
    TestEmptyMeter();
    TestOneStep();
    TestThreeStepsPreserveDescendingWriteOrder();
    TestFullMeter();
    TestOutOfRangeValuesLeaveMeterEmpty();
    return 0;
}
