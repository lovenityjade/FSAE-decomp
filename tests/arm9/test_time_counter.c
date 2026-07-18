#include "game/time_counter.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct GlyphCall {
    uint16_t *screen_tilemap;
    int x;
    int y;
    int palette;
    uint16_t tile_index;
} GlyphCall;

static GlyphCall sCalls[5];
static unsigned int sCallCount;

void Game_SetVerticalTileGlyph(
    uint16_t *screen_tilemap,
    int x,
    int y,
    int palette,
    uint16_t tile_index
)
{
    GlyphCall *call = &sCalls[sCallCount++];

    assert(sCallCount <= 5);
    call->screen_tilemap = screen_tilemap;
    call->x = x;
    call->y = y;
    call->palette = palette;
    call->tile_index = tile_index;
}

static void ExpectCounter(int total_seconds, const uint16_t expected_tiles[5])
{
    uint16_t screen_tilemap[32 * 24];
    unsigned int index;

    memset(screen_tilemap, 0, sizeof(screen_tilemap));
    memset(sCalls, 0, sizeof(sCalls));
    sCallCount = 0;

    Game_DrawTimeCounter(total_seconds, screen_tilemap);

    assert(sCallCount == 5);
    for (index = 0; index < 5; ++index) {
        assert(sCalls[index].screen_tilemap == screen_tilemap);
        assert(sCalls[index].x == 26 + (int)index);
        assert(sCalls[index].y == 21);
        assert(sCalls[index].palette == 3);
        assert(sCalls[index].tile_index == expected_tiles[index]);
    }
}

static void TestZero(void)
{
    static const uint16_t expected[] = {8, 8, 7, 8, 8};

    ExpectCounter(0, expected);
}

static void TestOneMinuteFiveSeconds(void)
{
    static const uint16_t expected[] = {8, 9, 7, 8, 13};

    ExpectCounter(65, expected);
}

static void TestLargestUnsaturatedValue(void)
{
    static const uint16_t expected[] = {17, 17, 7, 13, 17};

    ExpectCounter(5999, expected);
}

static void TestMinutesSaturateButSecondsRemainModuloSixty(void)
{
    static const uint16_t expected[] = {17, 17, 7, 8, 13};

    ExpectCounter(7205, expected);
}

int main(void)
{
    TestZero();
    TestOneMinuteFiveSeconds();
    TestLargestUnsaturatedValue();
    TestMinutesSaturateButSecondsRemainModuloSixty();
    return 0;
}
