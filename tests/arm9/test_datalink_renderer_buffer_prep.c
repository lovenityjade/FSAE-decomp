#include "game/datalink_renderer_buffer_prep.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static uint8_t sPrimarySource[1];
static uint8_t sPrimaryDestination[1];
static uint8_t sSecondarySource[1];
static uint8_t sSecondaryDestination[1];
static uint8_t sSecondarySourceAfterFirstCopy[1];
static uint8_t sSecondaryDestinationAfterFirstCopy[1];
static uint8_t sPrimaryDestinationAfterFirstCopy[1];
static uint8_t sPrimaryDestinationAfterSecondCopy[1];

static Game_DatalinkFlowState *sExpectedState;
static volatile Game_DatalinkRenderBufferBindings *sExpectedBuffers;
static size_t sCopyCount;

void Game_CopyDatalinkRenderBytes(
    const uint8_t *source,
    uint8_t *destination,
    uint32_t size
)
{
    assert(size == 0x600U);
    assert(sExpectedState->selection_dirty_43 == 0U);

    if (sCopyCount == 0) {
        assert(source == sPrimarySource);
        assert(destination == sPrimaryDestination);
        sExpectedBuffers->secondary_source = sSecondarySourceAfterFirstCopy;
        sExpectedBuffers->secondary_destination =
            sSecondaryDestinationAfterFirstCopy;
        sExpectedBuffers->primary_destination =
            sPrimaryDestinationAfterFirstCopy;
    } else {
        assert(sCopyCount == 1);
        assert(source == sSecondarySourceAfterFirstCopy);
        assert(destination == sSecondaryDestinationAfterFirstCopy);
        sExpectedBuffers->primary_destination =
            sPrimaryDestinationAfterSecondCopy;
    }
    ++sCopyCount;
}

static void TestClearsCopiesAndPublishesInTargetOrder(void)
{
    Game_DatalinkFlowState state;
    Game_DatalinkRenderBufferBindings buffers;

    memset(&state, 0xA5, sizeof(state));
    memset(&buffers, 0, sizeof(buffers));
    state.selection_dirty_43 = 1;
    buffers.primary_source = sPrimarySource;
    buffers.primary_destination = sPrimaryDestination;
    buffers.secondary_source = sSecondarySource;
    buffers.secondary_destination = sSecondaryDestination;
    buffers.primary_buffer = NULL;
    sExpectedState = &state;
    sExpectedBuffers = &buffers;
    sCopyCount = 0;

    Game_PrepareDatalinkRenderBuffers(&state, &buffers);

    assert(sCopyCount == 2);
    assert(state.selection_dirty_43 == 0U);
    assert(buffers.primary_buffer ==
           sPrimaryDestinationAfterSecondCopy);
    assert(state.phase_41 == 0xA5U);
    assert(state.phase_timer_45 == 0xA5U);
}

int main(void)
{
    TestClearsCopiesAndPublishesInTargetOrder();
    return 0;
}
