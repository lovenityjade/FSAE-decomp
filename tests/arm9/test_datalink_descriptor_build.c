#include "game/datalink_descriptor_build.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_PREPARE,
    EVENT_GET_CONTEXT,
    EVENT_MEASURE,
    EVENT_READINESS,
    EVENT_GET_RENDER_VALUE,
    EVENT_RENDER,
    EVENT_FLUSH,
    EVENT_TRANSFER
} Event;

Game_DatalinkSlotDescriptor
    gGameDatalinkSlotDescriptors[GAME_DATALINK_DESCRIPTOR_COUNT];
Game_DatalinkDescriptorRenderer gGameDatalinkDescriptorRenderer;
uint32_t gGameDatalinkDescriptorBufferBase;
uint32_t gGameDatalinkLayoutValue6ac;

static Event sEvents[16];
static size_t sEventCount;
static uint8_t sRenderContext[0x200];
static int sMeasuredWidth;
static int sReady[GAME_DATALINK_DESCRIPTOR_COUNT];
static int sReadinessSlot;
static uint32_t sRenderValue;
static const void *sExpectedRuntimeSubobject;
static uint32_t sFlushedAddress;
static uint32_t sFlushedSize;
static int sTransferChannel;
static uint32_t sTransferSource;
static uint32_t sTransferDestination;
static uint32_t sTransferSize;

static void RecordEvent(Event event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

void Game_PrepareDatalinkDescriptorBuild(void)
{
    RecordEvent(EVENT_PREPARE);
}

uint8_t *Game_GetDatalinkRenderContext(void)
{
    RecordEvent(EVENT_GET_CONTEXT);
    return sRenderContext;
}

int Game_MeasureDatalinkResource(
    uint32_t resource_id,
    int variant,
    const void *render_context
)
{
    RecordEvent(EVENT_MEASURE);
    assert(resource_id == 0x084F);
    assert(variant == 0);
    assert(render_context == sRenderContext + 0x1BC);
    return sMeasuredWidth;
}

int Game_IsDatalinkPlayerSlotReady(int slot)
{
    RecordEvent(EVENT_READINESS);
    assert(slot == sReadinessSlot);
    ++sReadinessSlot;
    return sReady[slot];
}

uint32_t Game_GetDatalinkRenderValue(void)
{
    RecordEvent(EVENT_GET_RENDER_VALUE);
    return sRenderValue;
}

void Game_RenderDatalinkSlotDescriptors(
    Game_DatalinkDescriptorRenderer *renderer,
    uint32_t render_value,
    const void *runtime_subobject
)
{
    RecordEvent(EVENT_RENDER);
    assert(renderer == &gGameDatalinkDescriptorRenderer);
    assert(render_value == sRenderValue);
    assert(runtime_subobject == sExpectedRuntimeSubobject);
}

void Game_FlushDatalinkDescriptorRange(
    uint32_t source_address,
    uint32_t byte_size
)
{
    RecordEvent(EVENT_FLUSH);
    sFlushedAddress = source_address;
    sFlushedSize = byte_size;
}

void Game_TransferDatalinkDescriptorRange(
    int channel,
    uint32_t source_address,
    uint32_t destination_offset,
    uint32_t byte_size
)
{
    RecordEvent(EVENT_TRANSFER);
    sTransferChannel = channel;
    sTransferSource = source_address;
    sTransferDestination = destination_offset;
    sTransferSize = byte_size;
}

static void ResetHarness(void)
{
    memset(gGameDatalinkSlotDescriptors, 0xA5,
           sizeof(gGameDatalinkSlotDescriptors));
    memset(&gGameDatalinkDescriptorRenderer, 0x5A,
           sizeof(gGameDatalinkDescriptorRenderer));
    memset(sReady, 0, sizeof(sReady));
    sEventCount = 0;
    sReadinessSlot = 0;
    sMeasuredWidth = 58;
    sRenderValue = 0x12345678U;
    sExpectedRuntimeSubobject = NULL;
    gGameDatalinkDescriptorBufferBase = 0x00100000U;
    gGameDatalinkLayoutValue6ac = 0x89ABCDEFU;
    sFlushedAddress = 0;
    sFlushedSize = 0;
    sTransferChannel = 0;
    sTransferSource = 0;
    sTransferDestination = 0;
    sTransferSize = 0;
}

static void AssertCommonPublication(void)
{
    assert(gGameDatalinkDescriptorRenderer.layout_value_38 ==
           gGameDatalinkLayoutValue6ac);
    assert(sFlushedAddress == 0x00106400U);
    assert(sFlushedSize == 0x0A80U);
    assert(sTransferChannel == 1);
    assert(sTransferSource == 0x00106400U);
    assert(sTransferDestination == 0x6400U);
    assert(sTransferSize == 0x0A80U);
}

static void TestMixedSlotsAndCallOrder(void)
{
    static const Event expected_events[] = {
        EVENT_PREPARE,
        EVENT_GET_CONTEXT,
        EVENT_MEASURE,
        EVENT_READINESS,
        EVENT_READINESS,
        EVENT_READINESS,
        EVENT_GET_CONTEXT,
        EVENT_GET_RENDER_VALUE,
        EVENT_RENDER,
        EVENT_FLUSH,
        EVENT_TRANSFER
    };
    uint8_t runtime[0x80];
    size_t index;

    ResetHarness();
    gGameDatalinkSlotDescriptors[0].buffer_row_0c = 1;
    gGameDatalinkSlotDescriptors[1].buffer_row_0c = 2;
    gGameDatalinkSlotDescriptors[2].buffer_row_0c = 3;
    sReady[1] = 1;
    sExpectedRuntimeSubobject = runtime + 0x30;

    Game_BuildDatalinkSlotDescriptors(runtime);

    assert(gGameDatalinkSlotDescriptors[0].resource_id_00 == 0x084FU);
    assert(gGameDatalinkSlotDescriptors[0].buffer_address_08 == 0x00100020U);
    assert(gGameDatalinkSlotDescriptors[0].tile_offset_quotient_10 == 12);
    assert(gGameDatalinkSlotDescriptors[0].tile_offset_remainder_24 == 3);
    assert(gGameDatalinkSlotDescriptors[0].variant_index_14 == 5);
    assert(gGameDatalinkSlotDescriptors[0].type_26 == 2);
    assert(gGameDatalinkSlotDescriptors[0].mode_18 == 14);

    assert(gGameDatalinkSlotDescriptors[1].resource_id_00 == 9001U);
    assert(gGameDatalinkSlotDescriptors[1].buffer_address_08 == 0x00100040U);
    assert(gGameDatalinkSlotDescriptors[1].tile_offset_quotient_10 == 8);
    assert(gGameDatalinkSlotDescriptors[1].tile_offset_remainder_24 == 0);
    assert(gGameDatalinkSlotDescriptors[1].variant_index_14 == 10);
    assert(gGameDatalinkSlotDescriptors[1].type_26 == 3);
    assert(gGameDatalinkSlotDescriptors[1].mode_18 == 11);

    assert(gGameDatalinkSlotDescriptors[2].resource_id_00 == 0x084FU);
    assert(gGameDatalinkSlotDescriptors[2].buffer_address_08 == 0x00100060U);
    assert(gGameDatalinkSlotDescriptors[2].tile_offset_quotient_10 == 12);
    assert(gGameDatalinkSlotDescriptors[2].tile_offset_remainder_24 == 3);
    assert(gGameDatalinkSlotDescriptors[2].variant_index_14 == 17);
    assert(gGameDatalinkSlotDescriptors[2].type_26 == 2);
    assert(gGameDatalinkSlotDescriptors[2].mode_18 == 14);

    assert(gGameDatalinkSlotDescriptors[0].unknown_04 == 0xA5A5A5A5U);
    assert(gGameDatalinkSlotDescriptors[0].unknown_1c[0] == 0xA5U);
    assert(gGameDatalinkSlotDescriptors[0].unknown_28[0] == 0xA5U);
    assert(gGameDatalinkSlotDescriptors[0].buffer_row_0c == 1);
    AssertCommonPublication();

    assert(sEventCount == sizeof(expected_events) / sizeof(expected_events[0]));
    for (index = 0; index < sEventCount; ++index) {
        assert(sEvents[index] == expected_events[index]);
    }
}

static void TestNegativeCenteringUsesSignedTruncation(void)
{
    int slot;

    ResetHarness();
    sMeasuredWidth = 275;

    Game_BuildDatalinkSlotDescriptors(NULL);

    for (slot = 0; slot < GAME_DATALINK_DESCRIPTOR_COUNT; ++slot) {
        assert(gGameDatalinkSlotDescriptors[slot].tile_offset_quotient_10 == -1);
        assert(gGameDatalinkSlotDescriptors[slot].tile_offset_remainder_24 == -1);
    }
    AssertCommonPublication();
}

int main(void)
{
    TestMixedSlotsAndCallOrder();
    TestNegativeCenteringUsesSignedTruncation();
    return 0;
}
