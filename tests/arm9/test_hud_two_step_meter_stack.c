#include "game/hud_two_step_meter_stack.h"

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

typedef void (*DrawMeterFunction)(int step, uint16_t *screen_tilemap);

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
    int y,
    int palette,
    uint16_t tile_index
)
{
    assert(sCalls[index].operation == operation);
    assert(sCalls[index].screen_tilemap == screen_tilemap);
    assert(sCalls[index].x == x);
    assert(sCalls[index].y == y);
    assert(sCalls[index].palette == palette);
    assert(sCalls[index].tile_index == tile_index);
}

static void ExpectMeter(
    DrawMeterFunction draw,
    int y,
    int filled_palette,
    uint16_t filled_tile,
    int step,
    const int *filled_x,
    size_t filled_count
)
{
    uint16_t screen_tilemap[32 * 24] = {0};
    size_t index;

    sCallCount = 0;
    draw(step, screen_tilemap);

    assert(sCallCount == 2 + filled_count);
    ExpectCall(0, GLYPH_OPERATION_RESET, screen_tilemap, 2, y, 0, 384);
    ExpectCall(1, GLYPH_OPERATION_RESET, screen_tilemap, 4, y, 0, 384);

    for (index = 0; index < filled_count; ++index) {
        ExpectCall(
            2 + index,
            GLYPH_OPERATION_SET,
            screen_tilemap,
            filled_x[index],
            y,
            filled_palette,
            filled_tile
        );
    }
}

static void TestMeterFamily(
    DrawMeterFunction draw,
    int y,
    int filled_palette,
    uint16_t filled_tile
)
{
    static const int one_step[] = {2};
    static const int two_steps[] = {4, 2};

    ExpectMeter(draw, y, filled_palette, filled_tile, 0, NULL, 0);
    ExpectMeter(draw, y, filled_palette, filled_tile, 1, one_step, 1);
    ExpectMeter(draw, y, filled_palette, filled_tile, 2, two_steps, 2);
    ExpectMeter(draw, y, filled_palette, filled_tile, 3, NULL, 0);
}

static void TestMiddleMeter(void)
{
    TestMeterFamily(Game_DrawMiddleHudTwoStepMeter, 11, 1, 3);
}

static void TestBottomMeter(void)
{
    TestMeterFamily(Game_DrawBottomHudTwoStepMeter, 16, 2, 5);
}

int main(void)
{
    TestMiddleMeter();
    TestBottomMeter();
    return 0;
}
