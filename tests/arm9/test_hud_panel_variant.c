#include "game/hud_panel_variant.h"

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

static RectangleCall sCall;
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
    ++sCallCount;
    assert(sCallCount == 1);
    sCall.screen_tilemap = screen_tilemap;
    sCall.x = x;
    sCall.y = y;
    sCall.width = width;
    sCall.height = height;
    sCall.palette = palette;
    sCall.tile_index = tile_index;
}

static void ExpectVariant(
    void (*draw)(int variant, uint16_t *screen_tilemap),
    int variant,
    int expected_x,
    int expected_width,
    uint16_t expected_tile
)
{
    uint16_t screen_tilemap[32 * 24] = {0};

    sCallCount = 0;
    draw(variant, screen_tilemap);

    assert(sCallCount == 1);
    assert(sCall.screen_tilemap == screen_tilemap);
    assert(sCall.x == expected_x);
    assert(sCall.y == 21);
    assert(sCall.width == expected_width);
    assert(sCall.height == 2);
    assert(sCall.palette == 4);
    assert(sCall.tile_index == expected_tile);
}

static void TestAllLeftPanelVariants(void)
{
    static const uint16_t expected_tiles[] = {
        0x0090,
        0x00D0,
        0x00C0,
        0x0110,
        0x0100,
        0x0150,
        0x0140
    };
    size_t variant;

    for (variant = 0; variant < sizeof(expected_tiles) / sizeof(expected_tiles[0]);
         ++variant) {
        ExpectVariant(
            Game_DrawLeftHudPanelVariant,
            (int)variant,
            6,
            16,
            expected_tiles[variant]
        );
    }
}

static void TestAllRightPanelVariants(void)
{
    static const uint16_t expected_tiles[] = {
        0x0040,
        0x0044,
        0x0048,
        0x004C,
        0x0050,
        0x0054,
        0x0058,
        0x005C,
        0x0080,
        0x0084,
        0x0088,
        0x008C
    };
    size_t variant;

    for (variant = 0; variant < sizeof(expected_tiles) / sizeof(expected_tiles[0]);
         ++variant) {
        ExpectVariant(
            Game_DrawRightHudPanelVariant,
            (int)variant,
            22,
            4,
            expected_tiles[variant]
        );
    }
}

int main(void)
{
    TestAllLeftPanelVariants();
    TestAllRightPanelVariants();
    return 0;
}
