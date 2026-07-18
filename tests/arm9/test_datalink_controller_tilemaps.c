#include "game/datalink_controller.h"

#include "game/datalink_renderer.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct TileCall {
    uint16_t *tilemap;
    int x;
    int y;
    int palette;
    uint16_t tile_id;
} TileCall;

typedef struct RectangleCall {
    uint16_t *tilemap;
    int x;
    int y;
    int width;
    int height;
    int palette;
} RectangleCall;

volatile Game_DatalinkRendererBindings gGameDatalinkRendererBindings;

static uint8_t sPrimarySource[1];
static uint8_t sPrimaryDestination[1];
static uint8_t sPrimaryPublished[1];
static uint8_t sPrimaryAfterFlush[1];
static uint8_t sSecondarySource[1];
static uint8_t sSecondaryDestination[1];
static uint8_t sSecondarySourceAfterFirstCopy[1];
static uint8_t sSecondaryDestinationAfterFirstCopy[1];
static uint8_t sSecondaryCaptured[1];

static size_t sCopyCount;
static TileCall sTileCalls[78];
static size_t sTileCount;
static RectangleCall sPaletteCalls[3];
static size_t sPaletteCount;
static RectangleCall sClearCalls[3];
static size_t sClearCount;
static int sAuxiliarySlots[3];
static size_t sAuxiliaryCount;
static size_t sFlushCount;
static int sTransferTargets[2];
static const void *sTransferSources[2];
static size_t sTransferCount;

void Game_CopyDatalinkControllerTilemapBytes(
    const uint8_t *source,
    uint8_t *destination,
    uint32_t size
)
{
    assert(size == 0x600U);
    if (sCopyCount == 0U) {
        assert(source == sPrimarySource);
        assert(destination == sPrimaryDestination);
        gGameDatalinkRendererBindings.secondary_source =
            sSecondarySourceAfterFirstCopy;
        gGameDatalinkRendererBindings.secondary_destination =
            sSecondaryDestinationAfterFirstCopy;
    } else {
        assert(sCopyCount == 1U);
        assert(source == sSecondarySourceAfterFirstCopy);
        assert(destination == sSecondaryDestinationAfterFirstCopy);
        gGameDatalinkRendererBindings.primary_destination =
            sPrimaryPublished;
        gGameDatalinkRendererBindings.secondary_destination =
            sSecondaryCaptured;
    }
    ++sCopyCount;
}

void Game_SetDatalinkRenderTile(
    uint16_t *tilemap,
    int x,
    int y,
    int palette,
    uint16_t tile_id
)
{
    TileCall *call;

    assert(sTileCount < sizeof(sTileCalls) / sizeof(sTileCalls[0]));
    call = &sTileCalls[sTileCount++];
    call->tilemap = tilemap;
    call->x = x;
    call->y = y;
    call->palette = palette;
    call->tile_id = tile_id;
}

void Game_ClearDatalinkRenderRectangle(
    uint16_t *tilemap,
    int x,
    int y,
    int width,
    int height
)
{
    RectangleCall *call;

    assert(sClearCount < sizeof(sClearCalls) / sizeof(sClearCalls[0]));
    call = &sClearCalls[sClearCount++];
    call->tilemap = tilemap;
    call->x = x;
    call->y = y;
    call->width = width;
    call->height = height;
    call->palette = -1;
}

void Game_SetDatalinkRenderRectanglePalette(
    uint16_t *tilemap,
    int x,
    int y,
    int width,
    int height,
    int palette
)
{
    RectangleCall *call;

    assert(sPaletteCount <
           sizeof(sPaletteCalls) / sizeof(sPaletteCalls[0]));
    call = &sPaletteCalls[sPaletteCount++];
    call->tilemap = tilemap;
    call->x = x;
    call->y = y;
    call->width = width;
    call->height = height;
    call->palette = palette;
}

void Game_RefreshDatalinkRowAuxiliary(int slot)
{
    assert(sAuxiliaryCount <
           sizeof(sAuxiliarySlots) / sizeof(sAuxiliarySlots[0]));
    sAuxiliarySlots[sAuxiliaryCount++] = slot;
}

void Game_FlushDatalinkRenderRange(const void *buffer, uint32_t size)
{
    assert(size == 0x600U);
    if (sFlushCount == 0U) {
        assert(buffer == sPrimaryPublished);
        gGameDatalinkRendererBindings.primary_buffer = sPrimaryAfterFlush;
    } else {
        assert(sFlushCount == 1U);
        assert(buffer == sSecondaryCaptured);
    }
    ++sFlushCount;
}

void Game_TransferDatalinkRenderBuffer(
    int target,
    const void *source,
    uint32_t destination_offset,
    uint32_t size
)
{
    assert(sTransferCount <
           sizeof(sTransferTargets) / sizeof(sTransferTargets[0]));
    assert(destination_offset == 0U);
    assert(size == 0x600U);
    sTransferTargets[sTransferCount] = target;
    sTransferSources[sTransferCount] = source;
    ++sTransferCount;
}

static void ResetHarness(Game_DatalinkController *controller)
{
    memset(controller, 0, sizeof(*controller));
    memset((void *)&gGameDatalinkRendererBindings, 0,
           sizeof(gGameDatalinkRendererBindings));
    memset(sTileCalls, 0, sizeof(sTileCalls));
    memset(sPaletteCalls, 0, sizeof(sPaletteCalls));
    memset(sClearCalls, 0, sizeof(sClearCalls));
    memset(sAuxiliarySlots, 0, sizeof(sAuxiliarySlots));
    memset(sTransferTargets, 0, sizeof(sTransferTargets));
    memset(sTransferSources, 0, sizeof(sTransferSources));
    gGameDatalinkRendererBindings.primary_source = sPrimarySource;
    gGameDatalinkRendererBindings.primary_destination =
        sPrimaryDestination;
    gGameDatalinkRendererBindings.secondary_source = sSecondarySource;
    gGameDatalinkRendererBindings.secondary_destination =
        sSecondaryDestination;
    sCopyCount = 0U;
    sTileCount = 0U;
    sPaletteCount = 0U;
    sClearCount = 0U;
    sAuxiliaryCount = 0U;
    sFlushCount = 0U;
    sTransferCount = 0U;
}

static void AssertRectangle(
    const RectangleCall *call,
    int y,
    int palette
)
{
    assert(call->tilemap ==
           (uint16_t *)(void *)sSecondaryCaptured);
    assert(call->x == 2);
    assert(call->y == y);
    assert(call->width == 0x1C);
    assert(call->height == 4);
    assert(call->palette == palette);
}

static void AssertUploads(void)
{
    assert(sFlushCount == 2U);
    assert(sTransferCount == 2U);
    assert(sTransferTargets[0] == 1);
    assert(sTransferSources[0] == sPrimaryAfterFlush);
    assert(sTransferTargets[1] == 2);
    assert(sTransferSources[1] == sSecondaryCaptured);
}

static void TestCleanControllerReturnsWithoutWork(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.auxiliary_state_5f = 0U;

    Game_RefreshDatalinkControllerTilemaps(&controller);

    assert(sCopyCount == 0U);
    assert(sFlushCount == 0U);
    assert(sTransferCount == 0U);
}

static void TestZeroRowsCopiesClearsAndUploads(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.auxiliary_state_5f = 1U;
    controller.participant_count_5a = 0U;

    Game_RefreshDatalinkControllerTilemaps(&controller);

    assert(controller.auxiliary_state_5f == 0U);
    assert(sCopyCount == 2U);
    assert(gGameDatalinkRendererBindings.primary_buffer ==
           sPrimaryAfterFlush);
    assert(sTileCount == 0U);
    assert(sPaletteCount == 0U);
    assert(sAuxiliaryCount == 0U);
    assert(sClearCount == 3U);
    AssertRectangle(&sClearCalls[0], 4, -1);
    AssertRectangle(&sClearCalls[1], 10, -1);
    AssertRectangle(&sClearCalls[2], 16, -1);
    AssertUploads();
}

static void TestTwoRowsRenderSelectedPaletteAndClearThird(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.auxiliary_state_5f = 1U;
    controller.participant_count_5a = 2U;
    controller.selected_row_5b = 1;

    Game_RefreshDatalinkControllerTilemaps(&controller);

    assert(sTileCount == 52U);
    assert(sTileCalls[0].x == 8);
    assert(sTileCalls[0].y == 6);
    assert(sTileCalls[0].palette == 13);
    assert(sTileCalls[0].tile_id == 0x22U);
    assert(sTileCalls[1].y == 7);
    assert(sTileCalls[1].tile_id == 0x42U);
    assert(sTileCalls[26].y == 12);
    assert(sTileCalls[51].x == 28);
    assert(sTileCalls[51].y == 11);
    assert(sPaletteCount == 2U);
    AssertRectangle(&sPaletteCalls[0], 4, 3);
    AssertRectangle(&sPaletteCalls[1], 10, 4);
    assert(sAuxiliaryCount == 2U);
    assert(sAuxiliarySlots[0] == 0);
    assert(sAuxiliarySlots[1] == 1);
    assert(sClearCount == 1U);
    AssertRectangle(&sClearCalls[0], 16, -1);
    AssertUploads();
}

static void TestThreeRowsRenderWithoutClears(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.auxiliary_state_5f = 1U;
    controller.participant_count_5a = 3U;
    controller.selected_row_5b = -1;

    Game_RefreshDatalinkControllerTilemaps(&controller);

    assert(sTileCount == 78U);
    assert(sPaletteCount == 3U);
    AssertRectangle(&sPaletteCalls[0], 4, 3);
    AssertRectangle(&sPaletteCalls[1], 10, 3);
    AssertRectangle(&sPaletteCalls[2], 16, 3);
    assert(sAuxiliaryCount == 3U);
    assert(sClearCount == 0U);
    AssertUploads();
}

int main(void)
{
    TestCleanControllerReturnsWithoutWork();
    TestZeroRowsCopiesClearsAndUploads();
    TestTwoRowsRenderSelectedPaletteAndClearThird();
    TestThreeRowsRenderWithoutClears();
    return 0;
}
