#include "game/hud_tilemap_refresh.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

typedef enum Event {
    EVENT_UPDATE,
    EVENT_TIME,
    EVENT_TOP_METER,
    EVENT_MIDDLE_METER,
    EVENT_BOTTOM_METER,
    EVENT_RIGHT_METER,
    EVENT_FLUSH,
    EVENT_UPLOAD,
    EVENT_WAIT_DMA
} Event;

typedef struct ResourceStorage {
    uint8_t unknown_00[8];
    uint32_t byte_size;
    uint16_t tilemap[32 * 24];
} ResourceStorage;

static Event sEvents[9];
static size_t sEventCount;
static Game_HudRefreshValues *sExpectedValues;
static uint16_t *sExpectedTilemap;
static uint32_t sTransferSize;
static int sDmaChannel;

static void RecordEvent(Event event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

void Game_UpdateHudRefreshValues(Game_HudRefreshValues *values)
{
    assert(values == sExpectedValues);
    RecordEvent(EVENT_UPDATE);
    values->total_seconds = 125;
    values->top_meter_step = 1;
    values->middle_meter_step = 2;
    values->bottom_meter_step = 0;
    values->right_meter_step = 4;
}

void Game_DrawTimeCounter(int total_seconds, uint16_t *screen_tilemap)
{
    RecordEvent(EVENT_TIME);
    assert(total_seconds == 125);
    assert(screen_tilemap == sExpectedTilemap);
}

void Game_DrawTopHudTwoStepMeter(int step, uint16_t *screen_tilemap)
{
    RecordEvent(EVENT_TOP_METER);
    assert(step == 1);
    assert(screen_tilemap == sExpectedTilemap);
}

void Game_DrawMiddleHudTwoStepMeter(int step, uint16_t *screen_tilemap)
{
    RecordEvent(EVENT_MIDDLE_METER);
    assert(step == 2);
    assert(screen_tilemap == sExpectedTilemap);
}

void Game_DrawBottomHudTwoStepMeter(int step, uint16_t *screen_tilemap)
{
    RecordEvent(EVENT_BOTTOM_METER);
    assert(step == 0);
    assert(screen_tilemap == sExpectedTilemap);
}

void Game_DrawRightHudFourStepMeter(int step, uint16_t *screen_tilemap)
{
    RecordEvent(EVENT_RIGHT_METER);
    assert(step == 4);
    assert(screen_tilemap == sExpectedTilemap);
}

void Game_FlushDataCacheRange(void *address, uint32_t byte_size)
{
    RecordEvent(EVENT_FLUSH);
    assert(address == sExpectedTilemap);
    assert(byte_size == sTransferSize);
}

void Game_UploadBackgroundTilemap(
    const void *source,
    uint32_t destination_offset,
    uint32_t byte_size
)
{
    RecordEvent(EVENT_UPLOAD);
    assert(source == sExpectedTilemap);
    assert(destination_offset == 0);
    assert(byte_size == sTransferSize);
}

void Game_WaitForDma(int dma_channel)
{
    RecordEvent(EVENT_WAIT_DMA);
    assert(dma_channel == sDmaChannel);
}

static void TestRefreshOrderAndArguments(void)
{
    static const Event expected_events[] = {
        EVENT_UPDATE,
        EVENT_TIME,
        EVENT_TOP_METER,
        EVENT_MIDDLE_METER,
        EVENT_BOTTOM_METER,
        EVENT_RIGHT_METER,
        EVENT_FLUSH,
        EVENT_UPLOAD,
        EVENT_WAIT_DMA
    };
    Game_HudRefreshValues values = {0};
    ResourceStorage storage = {{0}, sizeof(storage.tilemap), {0}};
    uint16_t *active_tilemap = NULL;
    Game_HudTilemapRefreshContext context;
    size_t index;

    sExpectedValues = &values;
    sExpectedTilemap = storage.tilemap;
    sTransferSize = storage.byte_size;
    sDmaChannel = 2;
    sEventCount = 0;

    context.values = &values;
    context.active_tilemap = &active_tilemap;
    context.resource = (Game_HudTilemapResource *)(void *)&storage;
    context.dma_channel = sDmaChannel;

    Game_RefreshHudTilemap(&context);

    assert(active_tilemap == storage.tilemap);
    assert(sEventCount == sizeof(expected_events) / sizeof(expected_events[0]));
    for (index = 0; index < sEventCount; ++index) {
        assert(sEvents[index] == expected_events[index]);
    }
}

int main(void)
{
    TestRefreshOrderAndArguments();
    return 0;
}
