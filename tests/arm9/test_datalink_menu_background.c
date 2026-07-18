#include "game/datalink_menu_controller.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

void * volatile gGameDatalinkMenuTransferBuffer;

static uint8_t sInitialBuffer[1];
static uint8_t sAfterClearBuffer[1];
static uint8_t sAfterFlushBuffer[1];
static uint8_t sAfterPrimaryBuffer[1];
static size_t sEventIndex;

void Game_ClearDatalinkMenuTransferBuffer(
    void *buffer,
    uint32_t size
)
{
    assert(sEventIndex == 0U);
    assert(buffer == sInitialBuffer);
    assert(size == 0x800U);
    gGameDatalinkMenuTransferBuffer = sAfterClearBuffer;
    ++sEventIndex;
}

void Game_FlushDatalinkRenderRange(
    const void *buffer,
    uint32_t size
)
{
    assert(sEventIndex == 1U);
    assert(buffer == sAfterClearBuffer);
    assert(size == 0x800U);
    gGameDatalinkMenuTransferBuffer = sAfterFlushBuffer;
    ++sEventIndex;
}

void Game_TransferDatalinkRenderBuffer(
    int target,
    const void *source,
    uint32_t destination_offset,
    uint32_t size
)
{
    assert(destination_offset == 0U);
    assert(size == 0x800U);
    if (sEventIndex == 2U) {
        assert(target == 1);
        assert(source == sAfterFlushBuffer);
        gGameDatalinkMenuTransferBuffer = sAfterPrimaryBuffer;
    } else {
        assert(sEventIndex == 3U);
        assert(target == 2);
        assert(source == sAfterPrimaryBuffer);
    }
    ++sEventIndex;
}

static void TestClearFlushAndTwoTransfersReloadTheBuffer(void)
{
    Game_DatalinkMenuController controller;

    gGameDatalinkMenuTransferBuffer = sInitialBuffer;
    sEventIndex = 0U;

    Game_ClearDatalinkMenuBackground(&controller);
    assert(sEventIndex == 4U);
}

int main(void)
{
    TestClearFlushAndTwoTransfersReloadTheBuffer();
    return 0;
}
