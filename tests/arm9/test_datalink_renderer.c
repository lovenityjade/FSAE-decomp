#include "game/datalink_renderer.h"

#include "game/datalink_renderer_buffer_prep.h"
#include "game/datalink_renderer_preamble.h"
#include "game/datalink_renderer_row_prep.h"
#include "game/datalink_renderer_upload.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum Event {
    EVENT_COPY_DESCRIPTORS,
    EVENT_ISSUE_COMMANDS,
    EVENT_DIRTY_GATE,
    EVENT_PREPARE_BUFFERS,
    EVENT_PREPARE_ROW,
    EVENT_READY_ROW,
    EVENT_UNAVAILABLE_ROW,
    EVENT_VARIANT_OVERLAY,
    EVENT_AUXILIARY,
    EVENT_UPLOAD
} Event;

volatile Game_DatalinkRendererBindings gGameDatalinkRendererBindings;
Game_DatalinkRenderRowMetadata gGameDatalinkRenderRowMetadata[3];
volatile Game_DatalinkReadyRowRecord gGameDatalinkReadyRowRecords[3];

static uint8_t sCapturedSecondary[1];
static uint8_t sChangedSecondary[1];
static uint8_t sPrimaryBeforeRows[1];
static uint8_t sPrimaryBeforeUpload[1];
static Game_DatalinkFlowState *sExpectedState;
static bool sDirty;
static const bool sReady[3] = {true, false, true};
static Event sEvents[20];
static uint8_t sEventSlots[20];
static size_t sEventCount;

static void RecordEvent(Event event, uint8_t slot)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount] = event;
    sEventSlots[sEventCount] = slot;
    ++sEventCount;
}

void Game_CopyDatalinkRenderDescriptors(
    Game_DatalinkRenderDescriptor descriptors[10]
)
{
    int index;

    RecordEvent(EVENT_COPY_DESCRIPTORS, 0xFFU);
    for (index = 0; index < 10; ++index) {
        descriptors[index].callback_address_04 =
            UINT32_C(0x02000000) + (uint32_t)index * 4U;
    }
}

void Game_IssueReadyDatalinkSlotCommands(void)
{
    RecordEvent(EVENT_ISSUE_COMMANDS, 0xFFU);
}

bool Game_ShouldRebuildDatalinkRender(
    const Game_DatalinkFlowState *state
)
{
    assert(state == sExpectedState);
    RecordEvent(EVENT_DIRTY_GATE, 0xFFU);
    return sDirty;
}

void Game_PrepareDatalinkRenderBuffers(
    Game_DatalinkFlowState *state,
    volatile Game_DatalinkRenderBufferBindings *buffers
)
{
    assert(state == sExpectedState);
    assert(buffers == &gGameDatalinkRendererBindings);
    RecordEvent(EVENT_PREPARE_BUFFERS, 0xFFU);
    buffers->secondary_source = sCapturedSecondary;
    buffers->primary_buffer = sPrimaryBeforeRows;
}

bool Game_PrepareDatalinkRenderRow(uint32_t values[10], uint8_t slot)
{
    int index;

    assert(slot < 3U);
    RecordEvent(EVENT_PREPARE_ROW, slot);
    for (index = 0; index < 10; ++index) {
        values[index] = (uint32_t)slot * 16U + (uint32_t)index;
    }
    return sReady[slot];
}

void Game_RenderReadyDatalinkRow(
    Game_DatalinkRenderRowMetadata metadata[3],
    const volatile Game_DatalinkReadyRowRecord *record,
    const Game_DatalinkRenderDescriptor descriptors[10],
    volatile Game_DatalinkRenderPrimaryBinding *buffers,
    uint32_t values[10],
    uint8_t slot
)
{
    int index;

    assert(sReady[slot]);
    assert(metadata == gGameDatalinkRenderRowMetadata);
    assert(record == &gGameDatalinkReadyRowRecords[slot]);
    assert(buffers == &gGameDatalinkRendererBindings);
    assert(buffers->primary_buffer == sPrimaryBeforeRows);
    for (index = 0; index < 10; ++index) {
        assert(descriptors[index].callback_address_04 ==
               UINT32_C(0x02000000) + (uint32_t)index * 4U);
        assert(values[index] ==
               (uint32_t)slot * 16U + (uint32_t)index);
    }
    RecordEvent(EVENT_READY_ROW, slot);
}

void Game_RenderUnavailableDatalinkRow(
    uint16_t *tilemap,
    uint8_t slot
)
{
    assert(slot == 1U);
    assert(tilemap == (uint16_t *)(void *)sCapturedSecondary);
    RecordEvent(EVENT_UNAVAILABLE_ROW, slot);
    gGameDatalinkRendererBindings.secondary_source = sChangedSecondary;
}

void Game_RenderDatalinkRowVariantOverlay(
    Game_DatalinkFlowState *state,
    volatile Game_DatalinkRenderPrimaryBinding *buffers,
    uint16_t *secondary_tilemap,
    uint8_t slot
)
{
    assert(state == sExpectedState);
    assert(buffers == &gGameDatalinkRendererBindings);
    assert(secondary_tilemap == (uint16_t *)(void *)sCapturedSecondary);
    RecordEvent(EVENT_VARIANT_OVERLAY, slot);
    if (slot == 2U) {
        buffers->primary_buffer = sPrimaryBeforeUpload;
    }
}

void Game_RefreshDatalinkRowAuxiliaryIfNeeded(
    Game_DatalinkFlowState *state,
    uint8_t slot
)
{
    assert(state == sExpectedState);
    RecordEvent(EVENT_AUXILIARY, slot);
}

void Game_UploadDatalinkRenderBuffers(
    volatile Game_DatalinkRenderUploadBindings *buffers,
    uint8_t *captured_secondary_buffer
)
{
    assert(buffers == &gGameDatalinkRendererBindings);
    assert(buffers->primary_buffer == sPrimaryBeforeUpload);
    assert(captured_secondary_buffer == sCapturedSecondary);
    assert(buffers->secondary_source == sChangedSecondary);
    RecordEvent(EVENT_UPLOAD, 0xFFU);
}

static void ResetHarness(Game_DatalinkFlowState *state, bool dirty)
{
    memset(state, 0, sizeof(*state));
    memset((void *)&gGameDatalinkRendererBindings, 0,
           sizeof(gGameDatalinkRendererBindings));
    sExpectedState = state;
    sDirty = dirty;
    sEventCount = 0U;
}

static void TestCleanFrameStopsAfterCommandsAndGate(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state, false);

    Game_UpdateDatalinkRenderer(&state);

    assert(sEventCount == 3U);
    assert(sEvents[0] == EVENT_COPY_DESCRIPTORS);
    assert(sEvents[1] == EVENT_ISSUE_COMMANDS);
    assert(sEvents[2] == EVENT_DIRTY_GATE);
}

static void TestDirtyFramePreservesFullThreeRowOrderAndCapture(void)
{
    static const Event expected_events[] = {
        EVENT_COPY_DESCRIPTORS,
        EVENT_ISSUE_COMMANDS,
        EVENT_DIRTY_GATE,
        EVENT_PREPARE_BUFFERS,
        EVENT_PREPARE_ROW,
        EVENT_READY_ROW,
        EVENT_VARIANT_OVERLAY,
        EVENT_AUXILIARY,
        EVENT_PREPARE_ROW,
        EVENT_UNAVAILABLE_ROW,
        EVENT_VARIANT_OVERLAY,
        EVENT_AUXILIARY,
        EVENT_PREPARE_ROW,
        EVENT_READY_ROW,
        EVENT_VARIANT_OVERLAY,
        EVENT_AUXILIARY,
        EVENT_UPLOAD
    };
    static const uint8_t expected_slots[] = {
        0xFF, 0xFF, 0xFF, 0xFF,
        0, 0, 0, 0,
        1, 1, 1, 1,
        2, 2, 2, 2,
        0xFF
    };
    Game_DatalinkFlowState state;
    size_t index;

    ResetHarness(&state, true);

    Game_UpdateDatalinkRenderer(&state);

    assert(sEventCount ==
           sizeof(expected_events) / sizeof(expected_events[0]));
    for (index = 0;
         index < sizeof(expected_events) / sizeof(expected_events[0]);
         ++index) {
        assert(sEvents[index] == expected_events[index]);
        assert(sEventSlots[index] == expected_slots[index]);
    }
}

int main(void)
{
    TestCleanFrameStopsAfterCommandsAndGate();
    TestDirtyFramePreservesFullThreeRowOrderAndCapture();
    return 0;
}
