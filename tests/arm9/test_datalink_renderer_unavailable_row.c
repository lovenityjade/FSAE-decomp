#include "game/datalink_renderer_unavailable_row.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

typedef struct TileCall {
    uint16_t *tilemap;
    int x;
    int y;
    int palette;
    uint16_t tile_id;
} TileCall;

static TileCall sCalls[26];
static size_t sCallCount;

void Game_SetDatalinkRenderTile(
    uint16_t *tilemap,
    int x,
    int y,
    int palette,
    uint16_t tile_id
)
{
    TileCall *call;

    assert(sCallCount < sizeof(sCalls) / sizeof(sCalls[0]));
    call = &sCalls[sCallCount++];
    call->tilemap = tilemap;
    call->x = x;
    call->y = y;
    call->palette = palette;
    call->tile_id = tile_id;
}

static void AssertCall(
    size_t index,
    uint16_t *tilemap,
    int x,
    int y,
    uint16_t tile_id
)
{
    const TileCall *call = &sCalls[index];

    assert(call->tilemap == tilemap);
    assert(call->x == x);
    assert(call->y == y);
    assert(call->palette == 4);
    assert(call->tile_id == tile_id);
}

static void TestAllUnavailableTilesPreserveTargetOrder(void)
{
    uint16_t tilemap[1];
    int cell;

    sCallCount = 0;

    Game_RenderUnavailableDatalinkRow(tilemap, 2);

    assert(sCallCount == 26);
    for (cell = 0; cell < 10; ++cell) {
        AssertCall((size_t)cell * 2U, tilemap, 8 + cell, 18, 0x22);
        AssertCall((size_t)cell * 2U + 1U, tilemap, 8 + cell, 19, 0x42);
    }
    for (cell = 0; cell < 3; ++cell) {
        AssertCall(20U + (size_t)cell * 2U,
                   tilemap, 0x1A + cell, 16, 2);
        AssertCall(21U + (size_t)cell * 2U,
                   tilemap, 0x1A + cell, 17, 0x22);
    }
}

int main(void)
{
    TestAllUnavailableTilesPreserveTargetOrder();
    return 0;
}
