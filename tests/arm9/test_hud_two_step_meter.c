#include "game/hud_two_step_meter.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

typedef enum GlyphOperation {
    GLYPH_OPERATION_RESET,
    GLYPH_OPERATION_SET
} GlyphOperation;

typedef struct GlyphCall {
    GlyphOperation operation;
    uint16_t *screen_tilemap;
    int x;
    int y;
    int palette;
    uint16_t tile_index;
} GlyphCall;

static GlyphCall sCalls[4];
static size_t sCallCount;

static void RecordCall(
    GlyphOperation operation,
    uint16_t *screen_tilemap,
    int x,
    int y,
    int palette,
    uint16_t tile_index
)
{
    GlyphCall *call;

    assert(sCallCount < sizeof(sCalls) / sizeof(sCalls[0]));
    call = &sCalls[sCallCount++];
    call->operation = operation;
    call->screen_tilemap = screen_tilemap;
    call->x = x;
    call->y = y;
    call->palette = palette;
    call->tile_index = tile_index;
}

void Game_Reset2x2TileGlyph(
    uint16_t *screen_tilemap,
    int x,
    int y,
    int palette,
    uint16_t tile_index
)
{
    RecordCall(
        GLYPH_OPERATION_RESET,
        screen_tilemap,
        x,
        y,
        palette,
        tile_index
    );
}

void Game_Set2x2TileGlyph(
    uint16_t *screen_tilemap,
    int x,
    int y,
    int palette,
    uint16_t tile_index
)
{
    RecordCall(
        GLYPH_OPERATION_SET,
        screen_tilemap,
        x,
        y,
        palette,
        tile_index
    );
}

static void ExpectCall(
    size_t index,
    GlyphOperation operation,
    uint16_t *screen_tilemap,
    int x,
    uint16_t tile_index
)
{
    assert(sCalls[index].operation == operation);
    assert(sCalls[index].screen_tilemap == screen_tilemap);
    assert(sCalls[index].x == x);
    assert(sCalls[index].y == 6);
    assert(sCalls[index].palette == 0);
    assert(sCalls[index].tile_index == tile_index);
}

static void ExpectMeter(int step, const int *filled_x, size_t filled_count)
{
    uint16_t screen_tilemap[32 * 24] = {0};
    size_t index;

    sCallCount = 0;
    Game_DrawTopHudTwoStepMeter(step, screen_tilemap);

    assert(sCallCount == 2 + filled_count);
    ExpectCall(0, GLYPH_OPERATION_RESET, screen_tilemap, 2, 384);
    ExpectCall(1, GLYPH_OPERATION_RESET, screen_tilemap, 4, 384);

    for (index = 0; index < filled_count; ++index) {
        ExpectCall(
            2 + index,
            GLYPH_OPERATION_SET,
            screen_tilemap,
            filled_x[index],
            1
        );
    }
}

static void TestEmptyMeter(void)
{
    ExpectMeter(0, NULL, 0);
}

static void TestOneStep(void)
{
    static const int filled_x[] = {2};

    ExpectMeter(1, filled_x, 1);
}

static void TestTwoStepsPreserveOriginalWriteOrder(void)
{
    static const int filled_x[] = {4, 2};

    ExpectMeter(2, filled_x, 2);
}

static void TestOutOfRangeValueLeavesMeterEmpty(void)
{
    ExpectMeter(3, NULL, 0);
}

int main(void)
{
    TestEmptyMeter();
    TestOneStep();
    TestTwoStepsPreserveOriginalWriteOrder();
    TestOutOfRangeValueLeavesMeterEmpty();
    return 0;
}
