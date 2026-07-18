#define _GNU_SOURCE

#include "game/datalink_screen_update.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>

enum {
    SCREEN_WIDTH = 32,
    SCREEN_HEIGHT = 24,
    SCREEN_TILE_COUNT = SCREEN_WIDTH * SCREEN_HEIGHT,
    SCREEN_MAP_PAGE = 0x05000000,
    SCREEN_MAP_ADDRESS = 0x05000400,
    SCREEN_MAP_SIZE = 0x2000
};

Game_DatalinkScreenUpdateState
    gGameDatalinkScreenUpdateState;
volatile uint32_t gGameDatalinkScreenSharedValue;
const Game_DatalinkScreenConfigRecord
    gGameDatalinkScreenConfigRecords[] = {
        {0x0000, 0x1111, 0x2222},
        {0x0010, 0x3333, 0x4444},
        {0x0020, 0x5555, 0x6666},
    };

static uint16_t sScreenBuffer[SCREEN_TILE_COUNT];
static uint16_t sReplacementBuffer[SCREEN_TILE_COUNT];
static uint8_t sDefaultCharacterBase[0x4000];
static uint16_t sDefaultScreenBuffer[SCREEN_TILE_COUNT];
static uint8_t sExternalCharacterBuffer[0x4000];
static uint16_t sExternalScreenBuffer[SCREEN_TILE_COUNT];
static uint8_t sRenderContext[0x300];
static int sFlushCount;
static int sTransferCount;
static int sEventCount;
static int sMutateBindings;
static int sChangeScenario;
static uint16_t sLoadedResourceId;
static uint16_t sConfiguredResourceId;
static uint16_t sPlayedCueId;
static void *sMetadataPointer;
static const void *sFlushedBuffers[4];
static uint32_t sFlushedSizes[4];
static const void *sCharacterTransferSource;
static uint32_t sCharacterTransferOffset;
static uint32_t sCharacterTransferSize;
static const void *sScreenTransferSource;
static uint32_t sScreenTransferOffset;
static uint32_t sScreenTransferSize;

void Game_FlushDatalinkRenderRange(
    const void *buffer,
    uint32_t size
)
{
    if (sChangeScenario != 0) {
        assert(sFlushCount < 4);
        sFlushedBuffers[sFlushCount] = buffer;
        sFlushedSizes[sFlushCount] = size;
        ++sFlushCount;
        return;
    }

    assert(sEventCount == 0);
    assert(buffer == sScreenBuffer);
    assert(size == 0x600U);
    assert(gGameDatalinkScreenUpdateState
               .screen_buffer_requires_upload_08 == 1U);
    ++sFlushCount;
    ++sEventCount;

    if (sMutateBindings != 0) {
        gGameDatalinkScreenUpdateState.screen_buffer_48 =
            sReplacementBuffer;
    }
}

void Game_TransferDatalinkRenderBuffer(
    int target,
    const void *source,
    uint32_t destination_offset,
    uint32_t size
)
{
    if (sChangeScenario != 0) {
        assert(target == 0);
        sScreenTransferSource = source;
        sScreenTransferOffset = destination_offset;
        sScreenTransferSize = size;
        ++sTransferCount;
        return;
    }

    assert(sEventCount == 1);
    assert(target == 0);
    assert(source == sScreenBuffer);
    assert(destination_offset == 0U);
    assert(size == 0x600U);
    assert(gGameDatalinkScreenUpdateState
               .screen_buffer_requires_upload_08 == 1U);
    ++sTransferCount;
    ++sEventCount;

    if (sMutateBindings != 0) {
        gGameDatalinkScreenUpdateState.saved_shared_value_04 =
            UINT32_C(0xCAFEBABE);
    }
}

void *G2S_GetBG0CharPtr(void)
{
    return sDefaultCharacterBase;
}

void *G2S_GetBG0ScrPtr(void)
{
    return sDefaultScreenBuffer;
}

void MI_CpuFill(void *destination, uint32_t value, uint32_t size)
{
    assert(destination == &gGameDatalinkScreenUpdateState.screen_buffer_48);
    assert(value == 0U);
    assert(size == 0x38U);
    memset(destination, 0, size);
}

void *Game_GetDatalinkRenderContext(void)
{
    return sRenderContext;
}

void Game_LoadDatalinkScreenMetadata(
    uint16_t resource_id,
    void *metadata)
{
    sLoadedResourceId = resource_id;
    sMetadataPointer = metadata;
}

void Game_ConfigureDatalinkScreenRect(
    int8_t *clear_x,
    int8_t *clear_y,
    int8_t *clear_width,
    int8_t *clear_height,
    uint16_t resource_id,
    void *metadata)
{
    assert(metadata == sMetadataPointer);
    sConfiguredResourceId = resource_id;
    *clear_x = 1;
    *clear_y = 2;
    *clear_width = 3;
    *clear_height = 4;
}

void Game_PlayDatalinkScreenCue(uint16_t cue_id)
{
    sPlayedCueId = cue_id;
}

void Game_TransferDatalinkCharacterBuffer(
    int target,
    const void *source,
    uint32_t destination_offset,
    uint32_t size)
{
    assert(target == 0);
    sCharacterTransferSource = source;
    sCharacterTransferOffset = destination_offset;
    sCharacterTransferSize = size;
    ++sTransferCount;
}

static void ResetHarness(void)
{
    memset(&gGameDatalinkScreenUpdateState, 0,
           sizeof(gGameDatalinkScreenUpdateState));
    memset(sScreenBuffer, 0xA5, sizeof(sScreenBuffer));
    memset(sReplacementBuffer, 0x5A, sizeof(sReplacementBuffer));
    gGameDatalinkScreenUpdateState.screen_buffer_48 = sScreenBuffer;
    gGameDatalinkScreenUpdateState.saved_shared_value_04 =
        UINT32_C(0x12345678);
    gGameDatalinkScreenSharedValue = UINT32_C(0xFFFFFFFF);
    sFlushCount = 0;
    sTransferCount = 0;
    sEventCount = 0;
    sMutateBindings = 0;
    sChangeScenario = 0;
    sLoadedResourceId = 0;
    sConfiguredResourceId = 0;
    sPlayedCueId = 0;
    sMetadataPointer = NULL;
    memset(sFlushedBuffers, 0, sizeof(sFlushedBuffers));
    memset(sFlushedSizes, 0, sizeof(sFlushedSizes));
    sCharacterTransferSource = NULL;
    sCharacterTransferOffset = 0;
    sCharacterTransferSize = 0;
    sScreenTransferSource = NULL;
    sScreenTransferOffset = 0;
    sScreenTransferSize = 0;
}

static volatile uint16_t *MapScreenMap(void)
{
    void *mapped = mmap(
        (void *)(uintptr_t)SCREEN_MAP_PAGE,
        SCREEN_MAP_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
        -1,
        0);

    assert(mapped == (void *)(uintptr_t)SCREEN_MAP_PAGE);
    memset((void *)(uintptr_t)SCREEN_MAP_PAGE, 0, SCREEN_MAP_SIZE);
    return (volatile uint16_t *)(uintptr_t)SCREEN_MAP_ADDRESS;
}

static void AssertCommonStateAfterChange(
    uint32_t participant_count,
    uint16_t resource_id,
    uint16_t cue_id)
{
    assert(gGameDatalinkScreenUpdateState.display_control_58 == 0xe020);
    assert(gGameDatalinkScreenUpdateState.setup_bytes_5a[0] == 0xe7);
    assert(gGameDatalinkScreenUpdateState.setup_bytes_5a[1] == 0x11);
    assert(gGameDatalinkScreenUpdateState.setup_bytes_5a[3] == 1);
    assert(gGameDatalinkScreenUpdateState.setup_bytes_5a[4] == 2);
    assert(gGameDatalinkScreenUpdateState.setup_bytes_5a[5] == 0x0f);
    assert(gGameDatalinkScreenUpdateState.setup_bytes_5a[6] == 6);
    assert(gGameDatalinkScreenUpdateState.setup_bytes_5a[7] == 10);
    assert(gGameDatalinkScreenUpdateState.setup_bytes_5a[8] == 0);
    assert(gGameDatalinkScreenUpdateState.setup_bytes_5a[9] == 0xf0);
    assert(gGameDatalinkScreenUpdateState.setup_active_68 == 1U);
    assert(gGameDatalinkScreenUpdateState.participant_count_6c ==
           participant_count);
    assert(gGameDatalinkScreenUpdateState.render_context_member_7c ==
           sRenderContext + 0x1bc);
    assert(sMetadataPointer ==
           &gGameDatalinkScreenUpdateState.screen_buffer_48);
    assert(sLoadedResourceId == resource_id);
    assert(sConfiguredResourceId == resource_id);
    assert(sPlayedCueId == cue_id);
    assert(gGameDatalinkScreenUpdateState.clear_x_0c == 1);
    assert(gGameDatalinkScreenUpdateState.clear_y_0d == 2);
    assert(gGameDatalinkScreenUpdateState.clear_width_0e == 3);
    assert(gGameDatalinkScreenUpdateState.clear_height_0f == 4);
    assert(gGameDatalinkScreenUpdateState.saved_shared_value_04 ==
           UINT32_C(0xAABBCCDD));
    assert(gGameDatalinkScreenSharedValue == 0U);
}

static void TestChangeStateUsesInternalBuffersWithoutUpload(void)
{
    volatile uint16_t *screen = MapScreenMap();

    ResetHarness();
    sChangeScenario = 1;
    gGameDatalinkScreenSharedValue = UINT32_C(0xAABBCCDD);

    Game_ChangeDatalinkState(0, 4, 0, 0);

    assert(gGameDatalinkScreenUpdateState.character_buffer_requires_upload_10
           == 0U);
    assert(gGameDatalinkScreenUpdateState.screen_buffer_requires_upload_08
           == 0U);
    assert(gGameDatalinkScreenUpdateState.character_buffer_4c ==
           sDefaultCharacterBase + 0x400);
    assert(gGameDatalinkScreenUpdateState.screen_buffer_48 ==
           sDefaultScreenBuffer);
    AssertCommonStateAfterChange(4, 0x1111, 0x2222);
    assert(screen[0x1c2 / 2] == 0x0cd0);
    assert(screen[0x1c4 / 2] == 0x0cd6);
    assert(screen[0x1c6 / 2] == 0x0cfc);
    assert(screen[0x1c8 / 2] == 0x0d1f);
    assert(screen[0x1ca / 2] == 0x2f1f);
    assert(screen[0x1d2 / 2] == 0x0c91);
    assert(screen[0x1d4 / 2] == 0x0c98);
    assert(screen[0x1d6 / 2] == 0x0c9e);
    assert(screen[0x1dc / 2] == 0x7fff);
    assert(screen[0x1de / 2] == 0x0c63);
    assert(sFlushCount == 0);
    assert(sTransferCount == 0);
}

static void TestChangeStateUploadsExternalBuffers(void)
{
    volatile uint16_t *screen = MapScreenMap();

    ResetHarness();
    sChangeScenario = 1;
    gGameDatalinkScreenSharedValue = UINT32_C(0xAABBCCDD);

    Game_ChangeDatalinkState(
        2,
        7,
        (uintptr_t)sExternalCharacterBuffer,
        (uintptr_t)sExternalScreenBuffer);

    assert(gGameDatalinkScreenUpdateState.character_buffer_requires_upload_10
           == 1U);
    assert(gGameDatalinkScreenUpdateState.screen_buffer_requires_upload_08
           == 1U);
    assert(gGameDatalinkScreenUpdateState.character_buffer_4c ==
           sExternalCharacterBuffer);
    assert(gGameDatalinkScreenUpdateState.screen_buffer_48 ==
           sExternalScreenBuffer);
    AssertCommonStateAfterChange(7, 0x5555, 0x6666);
    assert(screen[0x1c2 / 2] == 0x0dc3);
    assert(screen[0x1c4 / 2] == 0x0ec9);
    assert(screen[0x1c6 / 2] == 0x0f6c);
    assert(screen[0x1c8 / 2] == 0x0fac);
    assert(screen[0x1ca / 2] == 0x4ff8);
    assert(screen[0x1dc / 2] == 0x7fff);
    assert(screen[0x1de / 2] == 0x0c63);
    assert(sFlushCount == 2);
    assert(sFlushedBuffers[0] == sExternalCharacterBuffer);
    assert(sFlushedSizes[0] == 0x3000U);
    assert(sFlushedBuffers[1] == sExternalCharacterBuffer);
    assert(sFlushedSizes[1] == 0x600U);
    assert(sTransferCount == 2);
    assert(sCharacterTransferSource == sExternalCharacterBuffer);
    assert(sCharacterTransferOffset == 0x400U);
    assert(sCharacterTransferSize == 0x3000U);
    assert(sScreenTransferSource == sExternalScreenBuffer);
    assert(sScreenTransferOffset == 0U);
    assert(sScreenTransferSize == 0x600U);
}

static void TestClearsConfiguredRectangleWithoutUpload(void)
{
    int y;
    int x;

    ResetHarness();
    gGameDatalinkScreenUpdateState.clear_x_0c = 3;
    gGameDatalinkScreenUpdateState.clear_y_0d = 2;
    gGameDatalinkScreenUpdateState.clear_width_0e = 4;
    gGameDatalinkScreenUpdateState.clear_height_0f = 3;

    Game_CommitDatalinkScreenUpdate();

    for (y = 0; y < SCREEN_HEIGHT; ++y) {
        for (x = 0; x < SCREEN_WIDTH; ++x) {
            uint16_t expected = UINT16_C(0xA5A5);

            if (x >= 3 && x < 7 && y >= 2 && y < 5) {
                expected = 0U;
            }
            assert(sScreenBuffer[x + y * SCREEN_WIDTH] == expected);
        }
    }
    assert(sFlushCount == 0);
    assert(sTransferCount == 0);
    assert(gGameDatalinkScreenSharedValue == UINT32_C(0x12345678));
}

static void TestExternalBufferUploadsThenRestoresLatestValue(void)
{
    size_t index;

    ResetHarness();
    gGameDatalinkScreenUpdateState.clear_width_0e = 0;
    gGameDatalinkScreenUpdateState.clear_height_0f = 8;
    gGameDatalinkScreenUpdateState.screen_buffer_requires_upload_08 = 1U;
    sMutateBindings = 1;

    Game_CommitDatalinkScreenUpdate();

    for (index = 0U; index < SCREEN_TILE_COUNT; ++index) {
        assert(sScreenBuffer[index] == UINT16_C(0xA5A5));
    }
    assert(sFlushCount == 1);
    assert(sTransferCount == 1);
    assert(sEventCount == 2);
    assert(gGameDatalinkScreenUpdateState
               .screen_buffer_requires_upload_08 == 0U);
    assert(gGameDatalinkScreenUpdateState.screen_buffer_48 ==
           sReplacementBuffer);
    assert(gGameDatalinkScreenSharedValue == UINT32_C(0xCAFEBABE));
}

static void TestZeroHeightStillRestoresSharedValue(void)
{
    ResetHarness();
    gGameDatalinkScreenUpdateState.clear_width_0e = 5;
    gGameDatalinkScreenUpdateState.clear_height_0f = 0;
    gGameDatalinkScreenUpdateState.saved_shared_value_04 =
        UINT32_C(0x0BADF00D);

    Game_CommitDatalinkScreenUpdate();

    assert(sScreenBuffer[0] == UINT16_C(0xA5A5));
    assert(sFlushCount == 0);
    assert(sTransferCount == 0);
    assert(gGameDatalinkScreenSharedValue == UINT32_C(0x0BADF00D));
}

int main(void)
{
    TestChangeStateUsesInternalBuffersWithoutUpload();
    TestChangeStateUploadsExternalBuffers();
    TestClearsConfiguredRectangleWithoutUpload();
    TestExternalBufferUploadsThenRestoresLatestValue();
    TestZeroHeightStillRestoresSharedValue();
    return 0;
}
