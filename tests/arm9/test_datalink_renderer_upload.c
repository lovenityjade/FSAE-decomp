#include "game/datalink_renderer_upload.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum {
    FINAL_SOURCE_SIZE = 0x6E80
};

static uint8_t sPrimaryInitial[1];
static uint8_t sPrimaryAfterFirstFlush[1];
static uint8_t sPrimaryAfterSecondFlush[1];
static uint8_t sSecondaryCaptured[1];
static uint8_t sFinalInitial[FINAL_SOURCE_SIZE];
static uint8_t sFinalAfterFlush[FINAL_SOURCE_SIZE];

static volatile Game_DatalinkRenderUploadBindings *sBindings;
static size_t sEventIndex;

void Game_FlushDatalinkRenderRange(const void *buffer, uint32_t size)
{
    if (sEventIndex == 0U) {
        assert(buffer == sPrimaryInitial);
        assert(size == 0x600U);
        sBindings->primary_buffer = sPrimaryAfterFirstFlush;
    } else if (sEventIndex == 1U) {
        assert(buffer == sSecondaryCaptured);
        assert(size == 0x600U);
        sBindings->primary_buffer = sPrimaryAfterSecondFlush;
    } else {
        assert(sEventIndex == 4U);
        assert(buffer == &sFinalInitial[0x6400]);
        assert(size == 0xA80U);
        sBindings->final_source_base = sFinalAfterFlush;
    }
    ++sEventIndex;
}

void Game_TransferDatalinkRenderBuffer(
    int target,
    const void *source,
    uint32_t destination_offset,
    uint32_t size
)
{
    if (sEventIndex == 2U) {
        assert(target == 1);
        assert(source == sPrimaryAfterSecondFlush);
    } else {
        assert(sEventIndex == 3U);
        assert(target == 2);
        assert(source == sSecondaryCaptured);
    }
    assert(destination_offset == 0U);
    assert(size == 0x600U);
    ++sEventIndex;
}

void Game_TransferDatalinkRenderTiles(
    int target,
    const void *source,
    uint32_t destination_offset,
    uint32_t size
)
{
    assert(sEventIndex == 5U);
    assert(target == 1);
    assert(source == &sFinalAfterFlush[0x6400]);
    assert(destination_offset == 0x6400U);
    assert(size == 0xA80U);
    ++sEventIndex;
}

static void TestFlushesAndTransfersInTargetOrder(void)
{
    Game_DatalinkRenderUploadBindings bindings;

    bindings.primary_buffer = sPrimaryInitial;
    bindings.final_source_base = sFinalInitial;
    sBindings = &bindings;
    sEventIndex = 0U;

    Game_UploadDatalinkRenderBuffers(&bindings, sSecondaryCaptured);

    assert(sEventIndex == 6U);
    assert(bindings.primary_buffer == sPrimaryAfterSecondFlush);
    assert(bindings.final_source_base == sFinalAfterFlush);
}

int main(void)
{
    TestFlushesAndTransfersInTargetOrder();
    return 0;
}
