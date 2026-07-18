#include "game/datalink_graphics_resource.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

enum {
    SOURCE_SIZE = 256,
    ARENA_SIZE = 1024,
    BG_CHAR_DATA_ADDRESS = 0x00123000,
    OBJ_CHAR_DATA_ADDRESS = 0x00124000,
    PALETTE_DATA_ADDRESS = 0x00125000,
    SCREEN_DATA_ADDRESS = 0x00126000
};

static uint8_t sPrimarySource[SOURCE_SIZE];
static uint8_t sAlternateSource[SOURCE_SIZE];
static uint8_t sArena[ARENA_SIZE];

static Game_G2dCharacterData sBgCharacterData = {
    {0},
    0x90,
    BG_CHAR_DATA_ADDRESS
};
static Game_G2dCharacterData sObjectCharacterData = {
    {0},
    0x180,
    OBJ_CHAR_DATA_ADDRESS
};
static Game_G2dPaletteData sPaletteData = {
    {0},
    0x200,
    PALETTE_DATA_ADDRESS
};
static Game_G2dScreenData sScreenData = {
    {0},
    0x600,
    SCREEN_DATA_ADDRESS
};
static int sCellBank;
static int sAnimationBank;

static uint32_t sArenaOffset;
static uint32_t sLastSizeIndex;
static int sLastSizeAlternate;
static uint32_t sLastAddressIndex;
static int sLastAddressAlternate;
static int sSizeCalls;
static int sAddressCalls;
static int sAllocCalls;
static uint32_t sLastAllocSize;
static int sLastAllocAlignment;
static int sFastCopyCalls;
static int sByteCopyCalls;
static int sBgCharacterUnpackCalls;
static int sObjectCharacterUnpackCalls;
static int sPaletteUnpackCalls;
static int sScreenUnpackCalls;
static int sCellUnpackCalls;
static int sAnimationUnpackCalls;
static int sFlushCalls;
static const void *sLastFlushBuffer;
static uint32_t sLastFlushSize;
static int sBgTransferCalls;
static int sLastBgTransferTarget;
static const void *sLastBgTransferSource;
static uint32_t sLastBgTransferOffset;
static uint32_t sLastBgTransferSize;
static int sObjectCharacterTransferCalls;
static const void *sLastObjectCharacterTransferSource;
static uint32_t sLastObjectCharacterTransferOffset;
static uint32_t sLastObjectCharacterTransferSize;
static int sObjectPaletteTransferCalls;
static const void *sLastObjectPaletteTransferSource;
static uint32_t sLastObjectPaletteTransferOffset;
static uint32_t sLastObjectPaletteTransferSize;
static int sFreeCalls;
static void *sLastFreedBuffer;

uint32_t Game_GetDatalinkResourceSize(
    uint32_t resource_id,
    int use_alternate_archive)
{
    sLastSizeIndex = resource_id;
    sLastSizeAlternate = use_alternate_archive;
    ++sSizeCalls;
    return (use_alternate_archive != 0) ?
        0x30U + resource_id :
        0x20U + resource_id;
}

const void *Game_GetDatalinkResourceAddress(
    uint32_t resource_id,
    int use_alternate_archive)
{
    sLastAddressIndex = resource_id;
    sLastAddressAlternate = use_alternate_archive;
    ++sAddressCalls;
    if (use_alternate_archive != 0) {
        return &sAlternateSource[resource_id];
    }
    return &sPrimarySource[resource_id];
}

void *EL_GetGlobalAdr(uint32_t size, int alignment)
{
    void *buffer = &sArena[sArenaOffset];

    sLastAllocSize = size;
    sLastAllocAlignment = alignment;
    ++sAllocCalls;
    sArenaOffset += 0x80;
    assert(sArenaOffset <= ARENA_SIZE);
    return buffer;
}

void Game_FreeDatalinkResourceBuffer(void *buffer)
{
    ++sFreeCalls;
    sLastFreedBuffer = buffer;
}

void MI_CpuCopy8(const void *source, void *destination, uint32_t size)
{
    ++sByteCopyCalls;
    memcpy(destination, source, size);
}

void MIi_CpuCopyFast(const void *source, void *destination, uint32_t size)
{
    ++sFastCopyCalls;
    memcpy(destination, source, size);
}

void DC_FlushRange(const void *buffer, uint32_t size)
{
    ++sFlushCalls;
    sLastFlushBuffer = buffer;
    sLastFlushSize = size;
}

int Game_GetUnpackedDatalinkBgCharacterData(
    const void *file,
    Game_G2dCharacterData **output)
{
    (void)file;
    ++sBgCharacterUnpackCalls;
    *output = &sBgCharacterData;
    return 1;
}

int NNS_G2dGetUnpackedCharacterData(
    const void *file,
    Game_G2dCharacterData **output)
{
    (void)file;
    ++sObjectCharacterUnpackCalls;
    *output = &sObjectCharacterData;
    return 1;
}

int NNS_G2dGetUnpackedPaletteData(
    const void *file,
    Game_G2dPaletteData **output)
{
    (void)file;
    ++sPaletteUnpackCalls;
    *output = &sPaletteData;
    return 1;
}

int NNS_G2dGetUnpackedScreenData(
    const void *file,
    Game_G2dScreenData **output)
{
    (void)file;
    ++sScreenUnpackCalls;
    *output = &sScreenData;
    return 1;
}

int NNS_G2dGetUnpackedCellBank(const void *file, void **output)
{
    (void)file;
    ++sCellUnpackCalls;
    *output = &sCellBank;
    return 1;
}

int GetUnpackedAnimBankImpl_(const void *file, void **output)
{
    (void)file;
    ++sAnimationUnpackCalls;
    *output = &sAnimationBank;
    return 1;
}

void Game_TransferDatalinkCharacterBuffer(
    int target,
    const void *source,
    uint32_t destination_offset,
    uint32_t size)
{
    ++sBgTransferCalls;
    sLastBgTransferTarget = target;
    sLastBgTransferSource = source;
    sLastBgTransferOffset = destination_offset;
    sLastBgTransferSize = size;
}

void Game_TransferDatalinkObjectCharacterData(
    const void *source,
    uint32_t destination_offset,
    uint32_t size)
{
    ++sObjectCharacterTransferCalls;
    sLastObjectCharacterTransferSource = source;
    sLastObjectCharacterTransferOffset = destination_offset;
    sLastObjectCharacterTransferSize = size;
}

void Game_TransferDatalinkObjectPaletteData(
    const void *source,
    uint32_t destination_offset,
    uint32_t size)
{
    ++sObjectPaletteTransferCalls;
    sLastObjectPaletteTransferSource = source;
    sLastObjectPaletteTransferOffset = destination_offset;
    sLastObjectPaletteTransferSize = size;
}

static void ResetHarness(void)
{
    int i;

    for (i = 0; i < SOURCE_SIZE; ++i) {
        sPrimarySource[i] = (uint8_t)(0x20 + i);
        sAlternateSource[i] = (uint8_t)(0x80 + i);
    }
    memset(sArena, 0, sizeof(sArena));

    sArenaOffset = 0;
    sLastSizeIndex = UINT32_MAX;
    sLastSizeAlternate = -1;
    sLastAddressIndex = UINT32_MAX;
    sLastAddressAlternate = -1;
    sSizeCalls = 0;
    sAddressCalls = 0;
    sAllocCalls = 0;
    sLastAllocSize = 0;
    sLastAllocAlignment = 0;
    sFastCopyCalls = 0;
    sByteCopyCalls = 0;
    sBgCharacterUnpackCalls = 0;
    sObjectCharacterUnpackCalls = 0;
    sPaletteUnpackCalls = 0;
    sScreenUnpackCalls = 0;
    sCellUnpackCalls = 0;
    sAnimationUnpackCalls = 0;
    sFlushCalls = 0;
    sLastFlushBuffer = 0;
    sLastFlushSize = 0;
    sBgTransferCalls = 0;
    sLastBgTransferTarget = -1;
    sLastBgTransferSource = 0;
    sLastBgTransferOffset = 0;
    sLastBgTransferSize = 0;
    sObjectCharacterTransferCalls = 0;
    sLastObjectCharacterTransferSource = 0;
    sLastObjectCharacterTransferOffset = 0;
    sLastObjectCharacterTransferSize = 0;
    sObjectPaletteTransferCalls = 0;
    sLastObjectPaletteTransferSource = 0;
    sLastObjectPaletteTransferOffset = 0;
    sLastObjectPaletteTransferSize = 0;
    sFreeCalls = 0;
    sLastFreedBuffer = 0;
}

static void TestBgCharacterResourceUploadAndKeep(void)
{
    Game_DatalinkGraphicsResourceSet resources = {0};

    ResetHarness();
    Game_LoadDatalinkCharacterResource(
        &resources,
        3,
        UINT32_C(0x80000002),
        0x2000,
        0);

    assert(sSizeCalls == 1);
    assert(sLastSizeIndex == 2U);
    assert(sLastSizeAlternate == 1);
    assert(sAddressCalls == 1);
    assert(sLastAddressIndex == 2U);
    assert(sLastAddressAlternate == 1);
    assert(sAllocCalls == 1);
    assert(sLastAllocSize == 0x32U);
    assert(sLastAllocAlignment == 0x20);
    assert(sFastCopyCalls == 1);
    assert(sByteCopyCalls == 0);
    assert(resources.character_file_08 == &sArena[0]);
    assert(resources.character_data_0c == &sBgCharacterData);
    assert(sBgCharacterUnpackCalls == 1);
    assert(sFlushCalls == 1);
    assert(sLastFlushBuffer == (const void *)(uintptr_t)BG_CHAR_DATA_ADDRESS);
    assert(sLastFlushSize == 0x90U);
    assert(sBgTransferCalls == 1);
    assert(sLastBgTransferTarget == 3);
    assert(sLastBgTransferSource ==
           (const void *)(uintptr_t)BG_CHAR_DATA_ADDRESS);
    assert(sLastBgTransferOffset == 0x2000U);
    assert(sLastBgTransferSize == 0x90U);
    assert(sFreeCalls == 0);
}

static void TestBgCharacterResourceFreeFlag(void)
{
    Game_DatalinkGraphicsResourceSet resources = {0};

    ResetHarness();
    Game_LoadDatalinkCharacterResource(
        &resources,
        1,
        0x12,
        0x400,
        1);

    assert(sFreeCalls == 1);
    assert(sLastFreedBuffer == &sArena[0]);
    assert(resources.character_file_08 == 0);
    assert(resources.character_data_0c == 0);
}

static void TestScreenResourceLoadsWithoutUpload(void)
{
    Game_DatalinkGraphicsResourceSet resources = {0};

    ResetHarness();
    Game_LoadDatalinkScreenResourceIntoSet(&resources, 2, 0x21);

    assert(sSizeCalls == 1);
    assert(sLastSizeIndex == 0x21U);
    assert(sLastSizeAlternate == 0);
    assert(sFastCopyCalls == 1);
    assert(sByteCopyCalls == 0);
    assert(resources.screen_file_10 == &sArena[0]);
    assert(resources.screen_data_14 == &sScreenData);
    assert(sScreenUnpackCalls == 1);
    assert(sFlushCalls == 0);
    assert(sBgTransferCalls == 0);
}

static void TestObjectGraphicsResourcesUseDefaultSizes(void)
{
    Game_DatalinkGraphicsResourceSet resources = {0};

    ResetHarness();
    Game_LoadDatalinkObjectGraphicsResources(
        &resources,
        UINT32_C(0x80000002),
        -1,
        0x1f,
        -1);

    assert(sSizeCalls == 2);
    assert(sAddressCalls == 2);
    assert(sAllocCalls == 2);
    assert(sFastCopyCalls == 1);
    assert(sByteCopyCalls == 1);
    assert(resources.character_file_08 == &sArena[0]);
    assert(resources.character_data_0c == &sObjectCharacterData);
    assert(resources.palette_file_00 == &sArena[0x80]);
    assert(resources.palette_data_04 == &sPaletteData);
    assert(sObjectCharacterUnpackCalls == 1);
    assert(sPaletteUnpackCalls == 1);
    assert(sFlushCalls == 2);
    assert(sObjectCharacterTransferCalls == 1);
    assert(sLastObjectCharacterTransferSource ==
           (const void *)(uintptr_t)OBJ_CHAR_DATA_ADDRESS);
    assert(sLastObjectCharacterTransferOffset == 0U);
    assert(sLastObjectCharacterTransferSize == 0x180U);
    assert(sObjectPaletteTransferCalls == 1);
    assert(sLastObjectPaletteTransferSource ==
           (const void *)(uintptr_t)PALETTE_DATA_ADDRESS);
    assert(sLastObjectPaletteTransferOffset == 0U);
    assert(sLastObjectPaletteTransferSize == 0x200U);
}

static void TestObjectGraphicsExplicitSizes(void)
{
    Game_DatalinkGraphicsResourceSet resources = {0};

    ResetHarness();
    Game_LoadDatalinkObjectGraphicsResources(
        &resources,
        0x17,
        0x2000,
        0x8000001fU,
        500);

    assert(sObjectCharacterTransferCalls == 1);
    assert(sLastObjectCharacterTransferSize == 0x2000U);
    assert(sObjectPaletteTransferCalls == 1);
    assert(sLastObjectPaletteTransferSize == 500U);
}

static void TestCellAndAnimationResources(void)
{
    Game_DatalinkGraphicsResourceSet resources = {0};

    ResetHarness();
    Game_LoadDatalinkCellAnimationResources(
        &resources,
        UINT32_C(0x80010005),
        UINT32_C(0x80000006));

    assert(sSizeCalls == 2);
    assert(sAddressCalls == 2);
    assert(sFastCopyCalls == 0);
    assert(sByteCopyCalls == 2);
    assert(resources.cell_bank_file_18 == &sArena[0]);
    assert(resources.cell_bank_1c == &sCellBank);
    assert(resources.animation_bank_file_20 == &sArena[0x80]);
    assert(resources.animation_bank_24 == &sAnimationBank);
    assert(sCellUnpackCalls == 1);
    assert(sAnimationUnpackCalls == 1);
    assert(sLastSizeIndex == 6U);
    assert(sLastSizeAlternate == 1);
}

static void TestSentinelResourcesAreNoops(void)
{
    Game_DatalinkGraphicsResourceSet resources = {0};

    ResetHarness();
    Game_LoadDatalinkCharacterResource(
        &resources,
        0,
        UINT32_MAX,
        0,
        1);
    Game_LoadDatalinkScreenResourceIntoSet(&resources, 0, UINT32_MAX);
    Game_LoadDatalinkObjectGraphicsResources(
        &resources,
        UINT32_MAX,
        -1,
        UINT32_MAX,
        -1);
    Game_LoadDatalinkCellAnimationResources(
        &resources,
        UINT32_MAX,
        UINT32_MAX);

    assert(sSizeCalls == 0);
    assert(sAddressCalls == 0);
    assert(sAllocCalls == 0);
    assert(sFastCopyCalls == 0);
    assert(sByteCopyCalls == 0);
    assert(sFlushCalls == 0);
    assert(sBgTransferCalls == 0);
    assert(sObjectCharacterTransferCalls == 0);
    assert(sObjectPaletteTransferCalls == 0);
    assert(sFreeCalls == 0);
}

int main(void)
{
    TestBgCharacterResourceUploadAndKeep();
    TestBgCharacterResourceFreeFlag();
    TestScreenResourceLoadsWithoutUpload();
    TestObjectGraphicsResourcesUseDefaultSizes();
    TestObjectGraphicsExplicitSizes();
    TestCellAndAnimationResources();
    TestSentinelResourcesAreNoops();
    return 0;
}
