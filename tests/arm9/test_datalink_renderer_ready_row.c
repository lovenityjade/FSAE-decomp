#include "game/datalink_renderer_ready_row.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct CallbackCall {
    uint32_t address;
    uint8_t argument;
    int row;
    uint32_t value;
    uint16_t *tilemap;
} CallbackCall;

static uint16_t sInitialTilemap[1];
static uint16_t sTilemapAfterGlyph[1];
static uint16_t sTilemapAfterTile[1];
static volatile Game_DatalinkRenderPrimaryBinding *sBuffers;
static CallbackCall sCallbackCalls[GAME_DATALINK_RENDER_DESCRIPTOR_COUNT];
static size_t sCallbackCount;
static size_t sFixedCallCount;

void Game_SetDatalink2x2RenderGlyph(
    uint16_t *tilemap,
    int x,
    int y,
    int palette,
    uint16_t tile_id
)
{
    assert(sFixedCallCount == 0U);
    assert(tilemap == sInitialTilemap);
    assert(x == 0x13);
    assert(y == 16);
    assert(palette == 3);
    assert(tile_id == 0x78U);
    sBuffers->primary_buffer = (uint8_t *)(void *)sTilemapAfterGlyph;
    ++sFixedCallCount;
}

void Game_SetDatalinkRenderTile(
    uint16_t *tilemap,
    int x,
    int y,
    int palette,
    uint16_t tile_id
)
{
    assert(sFixedCallCount == 1U);
    assert(tilemap == sTilemapAfterGlyph);
    assert(x == 0x15);
    assert(y == 17);
    assert(palette == 1);
    assert(tile_id == 0x55U);
    sBuffers->primary_buffer = (uint8_t *)(void *)sTilemapAfterTile;
    ++sFixedCallCount;
}

void Game_InvokeDatalinkRenderCallback(
    uint32_t callback_address,
    uint8_t callback_argument,
    int row,
    uint32_t value,
    uint16_t *tilemap
)
{
    CallbackCall *call;

    assert(sFixedCallCount == 2U);
    assert(sCallbackCount < GAME_DATALINK_RENDER_DESCRIPTOR_COUNT);
    call = &sCallbackCalls[sCallbackCount++];
    call->address = callback_address;
    call->argument = callback_argument;
    call->row = row;
    call->value = value;
    call->tilemap = tilemap;
}

static void TestBuildsValuesAndDispatchesAllDescriptors(void)
{
    Game_DatalinkRenderRowMetadata metadata[3];
    Game_DatalinkReadyRowRecord record;
    Game_DatalinkRenderDescriptor descriptors[10];
    Game_DatalinkRenderPrimaryBinding buffers;
    uint32_t values[10] = {0};
    static const uint32_t expected_values[10] = {
        UINT32_C(0x12345678), UINT32_C(0xABCD), 1, 0, 1,
        0, 1, 2, 3, 1
    };
    int index;

    memset(metadata, 0xA5, sizeof(metadata));
    memset(&record, 0, sizeof(record));
    memset(descriptors, 0, sizeof(descriptors));
    record.flags_00 = 0x1202U;
    record.packed_values_05 = 0x1BU;
    record.value_06 = 0xABCDU;
    record.value_18 = UINT32_C(0x12345678);
    for (index = 0; index < 10; ++index) {
        descriptors[index].callback_argument_00 = (uint8_t)(0x20 + index);
        descriptors[index].tile_offset_01 = (uint8_t)(4 + index);
        descriptors[index].callback_address_04 =
            UINT32_C(0x02000000) + (uint32_t)index * 4U;
    }
    buffers.primary_buffer = (uint8_t *)(void *)sInitialTilemap;
    sBuffers = &buffers;
    sFixedCallCount = 0U;
    sCallbackCount = 0U;

    Game_RenderReadyDatalinkRow(
        metadata,
        &record,
        descriptors,
        &buffers,
        values,
        2
    );

    assert(metadata[2].resource_id_00 == 0x232AU);
    assert(metadata[1].resource_id_00 == UINT32_C(0xA5A5A5A5));
    assert(sFixedCallCount == 2U);
    assert(sCallbackCount == 10U);
    for (index = 0; index < 10; ++index) {
        assert(values[index] == expected_values[index]);
        assert(sCallbackCalls[index].address ==
               UINT32_C(0x02000000) + (uint32_t)index * 4U);
        assert(sCallbackCalls[index].argument == (uint8_t)(0x20 + index));
        assert(sCallbackCalls[index].row == 16 + index);
        assert(sCallbackCalls[index].value == expected_values[index]);
        assert(sCallbackCalls[index].tilemap == sTilemapAfterTile);
    }
}

int main(void)
{
    TestBuildsValuesAndDispatchesAllDescriptors();
    return 0;
}
