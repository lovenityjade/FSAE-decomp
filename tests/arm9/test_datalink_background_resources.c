#include "game/datalink_background_resources.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum HarnessEvent {
    EVENT_CHARACTER,
    EVENT_SIZE,
    EVENT_ALLOCATE,
    EVENT_ADDRESS,
    EVENT_COPY_BYTES,
    EVENT_UNPACK_PALETTE,
    EVENT_FLUSH,
    EVENT_TRANSFER_PALETTE,
    EVENT_COPY_FAST,
    EVENT_UNPACK_SCREEN,
    EVENT_TRANSFER_SCREEN,
    EVENT_FREE
} HarnessEvent;

typedef struct TestScreenData {
    uint8_t unknown_00[8];
    uint32_t data_size_08;
    uint8_t data_0c[32];
} TestScreenData;

static uint8_t sPaletteFile[64];
static uint8_t sScreenFile[80];
static uint8_t sPaletteSource[64];
static uint8_t sScreenSource[80];
static uint8_t sPaletteDataBytes[16];
static Game_DatalinkPaletteData sPaletteData;
static TestScreenData sScreenData;
static HarnessEvent sEvents[32];
static size_t sEventCount;
static uint32_t sSizeIds[2];
static int sSizeArchives[2];
static size_t sSizeCount;
static uint32_t sAddressIds[2];
static int sAddressArchives[2];
static size_t sAddressCount;
static size_t sAllocateCount;
static size_t sFreeCount;
static size_t sFlushCount;
static size_t sPaletteTransferCount;
static size_t sScreenTransferCount;
static size_t sCharacterCount;

_Static_assert(
    offsetof(TestScreenData, data_0c) == 0x0C,
    "test screen payload offset"
);

static void RecordEvent(HarnessEvent event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

void Game_LoadDatalinkCharacterResource(
    Game_DatalinkBackgroundResources *resources,
    int16_t screen_target,
    uint32_t encoded_resource_id,
    uint32_t destination_offset,
    uint32_t release_flags
)
{
    RecordEvent(EVENT_CHARACTER);
    assert(resources != NULL);
    assert(screen_target == -2);
    assert(encoded_resource_id == 3U);
    assert(destination_offset == 0U);
    assert(release_flags == 0U);
    ++sCharacterCount;
}

uint32_t Game_GetDatalinkResourceSize(
    uint32_t resource_id,
    int use_alternate_archive
)
{
    RecordEvent(EVENT_SIZE);
    assert(sSizeCount < 2U);
    sSizeIds[sSizeCount] = resource_id;
    sSizeArchives[sSizeCount] = use_alternate_archive;
    ++sSizeCount;
    return resource_id == 4U || resource_id == 6U ? 64U : 80U;
}

const void *Game_GetDatalinkResourceAddress(
    uint32_t resource_id,
    int use_alternate_archive
)
{
    RecordEvent(EVENT_ADDRESS);
    assert(sAddressCount < 2U);
    sAddressIds[sAddressCount] = resource_id;
    sAddressArchives[sAddressCount] = use_alternate_archive;
    ++sAddressCount;
    if (resource_id == 4U || resource_id == 6U) {
        return sPaletteSource;
    }
    return sScreenSource;
}

void *Game_AllocateDatalinkResource(
    uint32_t size,
    uint32_t alignment
)
{
    void *allocation;

    RecordEvent(EVENT_ALLOCATE);
    assert(alignment == 0x20U);
    if (sAllocateCount == 0U) {
        assert(size == 64U);
        allocation = sPaletteFile;
    } else {
        assert(sAllocateCount == 1U);
        assert(size == 80U);
        allocation = sScreenFile;
    }
    ++sAllocateCount;
    return allocation;
}

void Game_FreeDatalinkResource(void *allocation)
{
    RecordEvent(EVENT_FREE);
    if (sFreeCount == 0U) {
        assert(allocation == sPaletteFile);
    } else {
        assert(sFreeCount == 1U);
        assert(allocation == sScreenFile);
    }
    ++sFreeCount;
}

void Game_CopyDatalinkResourceBytes(
    const void *source,
    void *destination,
    uint32_t size
)
{
    RecordEvent(EVENT_COPY_BYTES);
    assert(source == sPaletteSource);
    assert(destination == sPaletteFile);
    assert(size == 64U);
}

void Game_CopyDatalinkResourceFast(
    const void *source,
    void *destination,
    uint32_t size
)
{
    RecordEvent(EVENT_COPY_FAST);
    assert(source == sScreenSource);
    assert(destination == sScreenFile);
    assert(size == 80U);
}

int Game_GetUnpackedDatalinkPaletteData(
    const void *file,
    Game_DatalinkPaletteData **palette_data
)
{
    RecordEvent(EVENT_UNPACK_PALETTE);
    assert(file == sPaletteFile);
    *palette_data = &sPaletteData;
    return 1;
}

int Game_GetUnpackedDatalinkScreenData(
    const void *file,
    Game_DatalinkScreenData **screen_data
)
{
    RecordEvent(EVENT_UNPACK_SCREEN);
    assert(file == sScreenFile);
    *screen_data = (Game_DatalinkScreenData *)(void *)&sScreenData;
    return 1;
}

void Game_FlushDatalinkRenderRange(
    const void *buffer,
    uint32_t size
)
{
    RecordEvent(EVENT_FLUSH);
    if (sFlushCount == 0U) {
        assert(buffer == sPaletteDataBytes);
        assert(size == sizeof(sPaletteDataBytes));
    } else {
        assert(sFlushCount == 1U);
        assert(buffer == sScreenData.data_0c);
        assert(size == sizeof(sScreenData.data_0c));
    }
    ++sFlushCount;
}

void Game_TransferDatalinkPalette(
    const void *source,
    uint32_t destination_offset,
    uint32_t size
)
{
    RecordEvent(EVENT_TRANSFER_PALETTE);
    assert(source == sPaletteDataBytes);
    assert(destination_offset == 0U);
    assert(size == sizeof(sPaletteDataBytes));
    ++sPaletteTransferCount;
}

void Game_TransferDatalinkScreen(
    int target,
    const void *source,
    uint32_t destination_offset,
    uint32_t size
)
{
    RecordEvent(EVENT_TRANSFER_SCREEN);
    assert(target == -2);
    assert(source == sScreenData.data_0c);
    assert(destination_offset == 0U);
    assert(size == sizeof(sScreenData.data_0c));
    ++sScreenTransferCount;
}

static void ResetHarness(Game_DatalinkBackgroundResources *resources)
{
    memset(resources, 0, sizeof(*resources));
    memset(sEvents, 0, sizeof(sEvents));
    memset(sSizeIds, 0, sizeof(sSizeIds));
    memset(sSizeArchives, 0, sizeof(sSizeArchives));
    memset(sAddressIds, 0, sizeof(sAddressIds));
    memset(sAddressArchives, 0, sizeof(sAddressArchives));
    memset(&sPaletteData, 0, sizeof(sPaletteData));
    memset(&sScreenData, 0, sizeof(sScreenData));
    sPaletteData.data_size_08 = sizeof(sPaletteDataBytes);
    sPaletteData.data_0c = sPaletteDataBytes;
    sScreenData.data_size_08 = sizeof(sScreenData.data_0c);
    sEventCount = 0U;
    sSizeCount = 0U;
    sAddressCount = 0U;
    sAllocateCount = 0U;
    sFreeCount = 0U;
    sFlushCount = 0U;
    sPaletteTransferCount = 0U;
    sScreenTransferCount = 0U;
    sCharacterCount = 0U;
}

static void TestMissingResourcesDoNothing(void)
{
    Game_DatalinkBackgroundResources resources;

    ResetHarness(&resources);
    Game_LoadDatalinkBackgroundResources(
        &resources,
        1,
        -1,
        UINT32_MAX,
        UINT32_MAX,
        0U
    );
    assert(sEventCount == 0U);
}

static void TestLoadsPrimaryResourcesInOrder(void)
{
    static const HarnessEvent expected_events[] = {
        EVENT_CHARACTER,
        EVENT_SIZE,
        EVENT_ALLOCATE,
        EVENT_ADDRESS,
        EVENT_COPY_BYTES,
        EVENT_UNPACK_PALETTE,
        EVENT_FLUSH,
        EVENT_TRANSFER_PALETTE,
        EVENT_SIZE,
        EVENT_ALLOCATE,
        EVENT_ADDRESS,
        EVENT_COPY_FAST,
        EVENT_UNPACK_SCREEN,
        EVENT_FLUSH,
        EVENT_TRANSFER_SCREEN
    };
    Game_DatalinkBackgroundResources resources;
    size_t index;

    ResetHarness(&resources);
    Game_LoadDatalinkBackgroundResources(
        &resources,
        -2,
        3,
        4U,
        5U,
        0U
    );

    assert(sEventCount ==
           sizeof(expected_events) / sizeof(expected_events[0]));
    for (index = 0U; index < sEventCount; ++index) {
        assert(sEvents[index] == expected_events[index]);
    }
    assert(sSizeIds[0] == 4U && sSizeArchives[0] == 0);
    assert(sSizeIds[1] == 5U && sSizeArchives[1] == 0);
    assert(sAddressIds[0] == 4U && sAddressArchives[0] == 0);
    assert(sAddressIds[1] == 5U && sAddressArchives[1] == 0);
    assert(resources.palette_file_00 == sPaletteFile);
    assert(resources.palette_data_04 == &sPaletteData);
    assert(resources.screen_file_10 == sScreenFile);
    assert(resources.screen_data_14 ==
           (Game_DatalinkScreenData *)(void *)&sScreenData);
    assert(sCharacterCount == 1U);
    assert(sPaletteTransferCount == 1U);
    assert(sScreenTransferCount == 1U);
    assert(sFreeCount == 0U);
}

static void TestAlternateResourcesAreReleased(void)
{
    Game_DatalinkBackgroundResources resources;

    ResetHarness(&resources);
    Game_LoadDatalinkBackgroundResources(
        &resources,
        -2,
        -1,
        UINT32_C(0x80000006),
        UINT32_C(0x80000007),
        6U
    );

    assert(sSizeIds[0] == 6U && sSizeArchives[0] == 1);
    assert(sSizeIds[1] == 7U && sSizeArchives[1] == 1);
    assert(sAddressIds[0] == 6U && sAddressArchives[0] == 1);
    assert(sAddressIds[1] == 7U && sAddressArchives[1] == 1);
    assert(sFreeCount == 2U);
    assert(resources.palette_file_00 == NULL);
    assert(resources.palette_data_04 == NULL);
    assert(resources.screen_file_10 == NULL);
    assert(resources.screen_data_14 == NULL);
}

int main(void)
{
    TestMissingResourcesDoNothing();
    TestLoadsPrimaryResourcesInOrder();
    TestAlternateResourcesAreReleased();
    return 0;
}
