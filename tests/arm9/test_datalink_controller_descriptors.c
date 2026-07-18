#include "game/datalink_controller.h"

#include "game/datalink_descriptor_build.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_PREPARE,
    EVENT_GET_CONTEXT,
    EVENT_MEASURE,
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

static Event sEvents[12];
static size_t sEventCount;
static uint8_t sRenderContext[0x200];
static int sMeasuredWidth;
static uint32_t sRenderValue;
static uint32_t sRuntimeSubobjectAddress;
static uint32_t sFlushedAddress;
static uint32_t sFlushedSize;
static int sTransferChannel;
static uint32_t sTransferSource;
static uint32_t sTransferDestination;
static uint32_t sTransferSize;
static uint32_t sBaseAfterFlush;

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
    assert(resource_id == 0x084FU);
    assert(variant == 0);
    assert(render_context == sRenderContext + 0x1BC);
    return sMeasuredWidth;
}

uint32_t Game_GetDatalinkRenderValue(void)
{
    RecordEvent(EVENT_GET_RENDER_VALUE);
    return sRenderValue;
}

void Game_RenderDatalinkControllerDescriptors(
    Game_DatalinkDescriptorRenderer *renderer,
    uint32_t render_value,
    uint32_t runtime_subobject_address
)
{
    RecordEvent(EVENT_RENDER);
    assert(renderer == &gGameDatalinkDescriptorRenderer);
    assert(render_value == sRenderValue);
    sRuntimeSubobjectAddress = runtime_subobject_address;
}

void Game_FlushDatalinkControllerDescriptorRange(
    uint32_t source_address,
    uint32_t byte_size
)
{
    RecordEvent(EVENT_FLUSH);
    sFlushedAddress = source_address;
    sFlushedSize = byte_size;
    gGameDatalinkDescriptorBufferBase = sBaseAfterFlush;
}

void Game_TransferDatalinkControllerDescriptorRange(
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

static void ResetHarness(Game_DatalinkController *controller)
{
    memset(controller, 0, sizeof(*controller));
    memset(gGameDatalinkSlotDescriptors, 0xA5,
           sizeof(gGameDatalinkSlotDescriptors));
    memset(&gGameDatalinkDescriptorRenderer, 0x5A,
           sizeof(gGameDatalinkDescriptorRenderer));
    memset(sEvents, 0, sizeof(sEvents));
    sEventCount = 0U;
    sMeasuredWidth = 58;
    sRenderValue = UINT32_C(0x12345678);
    sRuntimeSubobjectAddress = UINT32_C(0xFFFFFFFF);
    gGameDatalinkDescriptorBufferBase = UINT32_C(0x00100000);
    gGameDatalinkLayoutValue6ac = UINT32_C(0x89ABCDEF);
    sFlushedAddress = 0U;
    sFlushedSize = 0U;
    sTransferChannel = 0;
    sTransferSource = 0U;
    sTransferDestination = 0U;
    sTransferSize = 0U;
    sBaseAfterFlush = UINT32_C(0x00200000);
}

static void AssertEventOrder(void)
{
    static const Event expected[] = {
        EVENT_PREPARE,
        EVENT_GET_CONTEXT,
        EVENT_MEASURE,
        EVENT_GET_CONTEXT,
        EVENT_GET_RENDER_VALUE,
        EVENT_RENDER,
        EVENT_FLUSH,
        EVENT_TRANSFER
    };
    size_t index;

    assert(sEventCount == sizeof(expected) / sizeof(expected[0]));
    for (index = 0; index < sEventCount; ++index) {
        assert(sEvents[index] == expected[index]);
    }
}

static void AssertCommonPublication(void)
{
    assert(gGameDatalinkDescriptorRenderer.layout_value_38 ==
           UINT32_C(0x89ABCDEF));
    assert(sFlushedAddress == UINT32_C(0x00106400));
    assert(sFlushedSize == 0x0A80U);
    assert(sTransferChannel == 1);
    assert(sTransferSource == UINT32_C(0x00206400));
    assert(sTransferDestination == 0x6400U);
    assert(sTransferSize == 0x0A80U);
    AssertEventOrder();
}

static void AssertDescriptor(
    int slot,
    uint32_t expected_buffer_address,
    int quotient,
    int remainder
)
{
    const Game_DatalinkSlotDescriptor *descriptor =
        &gGameDatalinkSlotDescriptors[slot];

    assert(descriptor->resource_id_00 == 0x084FU);
    assert(descriptor->buffer_address_08 == expected_buffer_address);
    assert(descriptor->tile_offset_quotient_10 == quotient);
    assert(descriptor->tile_offset_remainder_24 == remainder);
    assert(descriptor->variant_index_14 == slot * 6 + 5);
    assert(descriptor->type_26 == 2U);
    assert(descriptor->mode_18 == 14);
    assert(descriptor->unknown_04 == UINT32_C(0xA5A5A5A5));
    assert(descriptor->unknown_1c[0] == 0xA5U);
    assert(descriptor->unknown_28[0] == 0xA5U);
}

static void TestNullOwnerBuildsTwoCenteredDescriptors(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    gGameDatalinkSlotDescriptors[0].buffer_row_0c = 1;
    gGameDatalinkSlotDescriptors[1].buffer_row_0c = 2;

    Game_BuildDatalinkControllerDescriptors(&controller);

    AssertDescriptor(0, UINT32_C(0x00100020), 12, 3);
    AssertDescriptor(1, UINT32_C(0x00100040), 12, 3);
    assert(gGameDatalinkSlotDescriptors[2].resource_id_00 ==
           UINT32_C(0xA5A5A5A5));
    assert(sRuntimeSubobjectAddress == 0U);
    AssertCommonPublication();
}

static void TestBoundOwnerAndNegativeCentering(void)
{
    Game_DatalinkController controller;

    ResetHarness(&controller);
    controller.owner_address_08 = UINT32_C(0x11223300);
    gGameDatalinkSlotDescriptors[0].buffer_row_0c = 3;
    gGameDatalinkSlotDescriptors[1].buffer_row_0c = 4;
    sMeasuredWidth = 275;

    Game_BuildDatalinkControllerDescriptors(&controller);

    AssertDescriptor(0, UINT32_C(0x00100060), -1, -1);
    AssertDescriptor(1, UINT32_C(0x00100080), -1, -1);
    assert(sRuntimeSubobjectAddress == UINT32_C(0x11223330));
    AssertCommonPublication();
}

int main(void)
{
    TestNullOwnerBuildsTwoCenteredDescriptors();
    TestBoundOwnerAndNegativeCentering();
    return 0;
}
