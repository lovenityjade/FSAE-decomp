#include "game/datalink_scene_initialize.h"

#include "game/datalink_background_resources.h"
#include "game/datalink_controller.h"
#include "game/datalink_graphics_resource.h"
#include "game/datalink_phase_fifteen.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

enum {
    TEST_SCENE_SIZE = 0x26b8,
    TEST_RECORDS_OFFSET = 0x43c,
    TEST_RECORD_COUNT = 0x60,
    TEST_ACTIVE_COUNT_OFFSET = 0x253c,
    TEST_PENDING_SOUND_OFFSET = 0x253e,
    TEST_GRAPHICS_CONTEXT_OFFSET = 0x2540,
    TEST_SMALL_ALLOCATION_OFFSET = 0x2544,
    TEST_LOCAL_RESOURCES_OFFSET = 0x2548,
    TEST_FIRST_SCRATCH_OFFSET = 0x25ac,
    TEST_FIRST_SCREEN_RESOURCES_OFFSET = 0x25cc,
    TEST_SECOND_SCREEN_RESOURCES_OFFSET = 0x25ec,
    TEST_SECOND_SCRATCH_OFFSET = 0x262c,
    TEST_CHARACTER_RESOURCES_OFFSET = 0x264c,
    TEST_FIRST_SCREEN_ALLOCATION_OFFSET = 0x26ac,
    TEST_SECOND_SCREEN_ALLOCATION_OFFSET = 0x26b0,
    TEST_LARGE_ALLOCATION_OFFSET = 0x26b4,
    TEST_RECORD_SIZE = 0x58,
    TEST_SHARED_RESOURCE_OFFSET = 0x1c,
    TEST_GRAPHICS_CLEAR_OFFSET = 0x7c,
    TEST_FIRST_BACKGROUND_OFFSET = 0x9c,
    TEST_SECOND_BACKGROUND_OFFSET = 0xbc,
    TEST_SHARED_IMAGE_PROXY_OFFSET = 0x44,
    TEST_SHARED_PALETTE_PROXY_OFFSET = 0x68,
    TEST_PHASE_VALUE_OFFSET = 0x808,
    TEST_SHARED_SEQUENCE_COUNT = 51,
    TEST_ANIMATION_COUNT = 54
};

static const uint8_t sExpectedSharedSequences[TEST_SHARED_SEQUENCE_COUNT] = {
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 36, 32, 33, 32, 33,
    32, 33, 32, 33, 32, 33, 32, 33, 32, 33,
    37
};

static union {
    uint32_t alignment;
    uint8_t bytes[TEST_SCENE_SIZE];
} sSceneStorage;

volatile Game_DatalinkSceneStaticStateArena
    gGameDatalinkSceneStaticStateArena;
uint8_t gGameDatalinkSceneGraphicsArena[0xfc];
uint8_t gGameDatalinkScenePhaseObjectStorage[0x80c];
volatile uint32_t gGameDatalinkSceneSharedWord;
Game_DatalinkCompletionOwner gGameDatalinkCompletionOwner;
volatile uint32_t gGameSubDisplayControl_04001000;

static uint8_t sSmallAllocation;
static uint8_t sLargeAllocation;
static uint8_t sFirstScreenAllocation;
static uint8_t sSecondScreenAllocation;
static uint8_t sGraphicsManager[0x18];
static uint8_t sLocalCellBank;
static uint8_t sLocalAnimationBank;
static uint8_t sSharedCellBank;
static uint8_t sSharedAnimationBank;
static uint8_t sAnimations[TEST_ANIMATION_COUNT];
static uint8_t sSequences[TEST_ANIMATION_COUNT];

static struct {
    uint8_t unknown_00[8];
    uint32_t data_size_08;
    uint8_t data_0c[1];
} sFirstScreenData, sSecondScreenData;

static unsigned int sAllocationCalls;
static unsigned int sFillCalls;
static unsigned int sBackgroundCalls;
static unsigned int sCharacterCalls;
static unsigned int sScreenCalls;
static unsigned int sObjectResourceCalls;
static unsigned int sCellResourceCalls;
static unsigned int sSubScreenCalls;
static unsigned int sPoolResetCalls;
static unsigned int sAnimationAllocations;
static unsigned int sSequenceCalls;
static unsigned int sAnimationInitCalls;
static unsigned int sImageProxyCalls;
static unsigned int sPaletteProxyCalls;
static unsigned int sModeTokenCalls;
static unsigned int sManagerCalls;
static unsigned int sStateStackInitCalls;
static unsigned int sStateInterfaceCalls;
static unsigned int sEnterRootCalls;

static uint32_t Test_ReadWord(const uint8_t *bytes)
{
    return (uint32_t)bytes[0] |
        ((uint32_t)bytes[1] << 8) |
        ((uint32_t)bytes[2] << 16) |
        ((uint32_t)bytes[3] << 24);
}

static uint32_t Test_TargetAddress(const void *pointer)
{
    return (uint32_t)(uintptr_t)pointer;
}

static void Test_ResetCounters(void)
{
    sAllocationCalls = 0;
    sFillCalls = 0;
    sBackgroundCalls = 0;
    sCharacterCalls = 0;
    sScreenCalls = 0;
    sObjectResourceCalls = 0;
    sCellResourceCalls = 0;
    sSubScreenCalls = 0;
    sPoolResetCalls = 0;
    sAnimationAllocations = 0;
    sSequenceCalls = 0;
    sAnimationInitCalls = 0;
    sImageProxyCalls = 0;
    sPaletteProxyCalls = 0;
    sModeTokenCalls = 0;
    sManagerCalls = 0;
    sStateStackInitCalls = 0;
    sStateInterfaceCalls = 0;
    sEnterRootCalls = 0;
}

void *EL_GetGlobalAdr(uint32_t size, int alignment)
{
    void *result;

    switch (sAllocationCalls) {
    case 0:
        assert(size == 0x200);
        assert(alignment == 4);
        result = &sSmallAllocation;
        break;
    case 1:
        assert(size == 0x8000);
        assert(alignment == 0x20);
        result = &sLargeAllocation;
        break;
    case 2:
        assert(size == 0x1234);
        assert(alignment == 4);
        result = &sFirstScreenAllocation;
        break;
    default:
        assert(sAllocationCalls == 3);
        assert(size == 0x2345);
        assert(alignment == 4);
        result = &sSecondScreenAllocation;
        break;
    }
    ++sAllocationCalls;
    return result;
}

void MI_CpuFill8(void *destination, uint8_t value, uint32_t size)
{
    static const unsigned int offsets[3] = {
        TEST_GRAPHICS_CLEAR_OFFSET,
        TEST_FIRST_SCRATCH_OFFSET,
        TEST_SECOND_SCRATCH_OFFSET
    };
    uint8_t *expected;

    assert(sFillCalls < 3);
    expected = sFillCalls == 0 ?
        gGameDatalinkSceneGraphicsArena + offsets[sFillCalls] :
        sSceneStorage.bytes + offsets[sFillCalls];
    assert(destination == expected);
    assert(value == 0);
    assert(size == 0x80);
    memset(destination, value, size);
    ++sFillCalls;
}

void Game_DatalinkStateStack_Init_020acd98(
    Game_DatalinkStateStack *stack)
{
    uint8_t *bytes = (uint8_t *)(void *)stack;

    assert(bytes == sSceneStorage.bytes);
    bytes[4] = 7;
    bytes[5] = 0;
    bytes[6] = 0;
    bytes[7] = 0;
    memset(bytes + 8, 0, 4);
    bytes[0x2c] = 0;
    ++sStateStackInitCalls;
}

void Game_InitializeDatalinkSceneTweenRecords_020ad020(
    Game_DatalinkSceneOwner *scene)
{
    uint8_t *bytes = (uint8_t *)(void *)scene;
    unsigned int index;

    assert(bytes == sSceneStorage.bytes);
    memset(
        bytes + TEST_RECORDS_OFFSET,
        0,
        TEST_RECORD_COUNT * TEST_RECORD_SIZE);
    bytes[TEST_ACTIVE_COUNT_OFFSET] = 0;
    for (index = 0; index < TEST_RECORD_COUNT; ++index) {
        uint8_t *record = bytes + TEST_RECORDS_OFFSET +
            index * TEST_RECORD_SIZE;
        record[0] = (uint8_t)index;
        record[1] = 1;
        record[3] = 1;
    }
}

void Game_ConfigureDatalinkSubDisplay_020ad528(void)
{
    ++sSubScreenCalls;
}

void Game_LoadDatalinkBackgroundResources(
    Game_DatalinkBackgroundResources *resources,
    int16_t screen_target,
    int32_t character_resource_id,
    uint32_t palette_resource_id,
    uint32_t screen_resource_id,
    uint32_t release_flags)
{
    assert(resources == (Game_DatalinkBackgroundResources *)(void *)(
        gGameDatalinkSceneGraphicsArena +
        (sBackgroundCalls == 0 ?
            TEST_FIRST_BACKGROUND_OFFSET :
            TEST_SECOND_BACKGROUND_OFFSET)));
    assert(screen_target == (sBackgroundCalls == 0 ? 1 : 2));
    assert(character_resource_id == (sBackgroundCalls == 0 ? 0x12 : -1));
    assert(palette_resource_id ==
        (sBackgroundCalls == 0 ? UINT32_C(0x13) : UINT32_MAX));
    assert(screen_resource_id == UINT32_MAX);
    assert(release_flags == 0);
    ++sBackgroundCalls;
}

void Game_LoadDatalinkCharacterResource(
    Game_DatalinkGraphicsResourceSet *resources,
    int16_t target,
    uint32_t resource_id,
    uint32_t destination_offset,
    uint32_t flags)
{
    assert((uint8_t *)(void *)resources ==
        sSceneStorage.bytes + TEST_CHARACTER_RESOURCES_OFFSET);
    assert(target == 1);
    assert(resource_id == UINT32_C(0x80000000));
    assert(destination_offset == 0x2000);
    assert(flags == 0);
    ++sCharacterCalls;
}

void Game_LoadDatalinkScreenResourceIntoSet(
    Game_DatalinkGraphicsResourceSet *resources,
    int target,
    uint32_t resource_id)
{
    static const unsigned int offsets[4] = {
        TEST_FIRST_BACKGROUND_OFFSET,
        TEST_SECOND_BACKGROUND_OFFSET,
        TEST_FIRST_SCREEN_RESOURCES_OFFSET,
        TEST_SECOND_SCREEN_RESOURCES_OFFSET
    };
    static const uint32_t ids[4] = {
        0x21,
        0x22,
        UINT32_C(0x80000005),
        0x29
    };
    uint8_t *expected = sScreenCalls < 2 ?
        gGameDatalinkSceneGraphicsArena + offsets[sScreenCalls] :
        sSceneStorage.bytes + offsets[sScreenCalls];

    assert(sScreenCalls < 4);
    assert((uint8_t *)(void *)resources == expected);
    assert(target == (sScreenCalls == 1 || sScreenCalls == 3 ? 2 : 1));
    assert(resource_id == ids[sScreenCalls]);
    if (sScreenCalls == 0) {
        resources->screen_data_14 =
            (Game_G2dScreenData *)(void *)&sFirstScreenData;
    } else if (sScreenCalls == 1) {
        resources->screen_data_14 =
            (Game_G2dScreenData *)(void *)&sSecondScreenData;
    }
    ++sScreenCalls;
}

void Game_LoadDatalinkObjectGraphicsResources(
    Game_DatalinkGraphicsResourceSet *resources,
    uint32_t character_resource_id,
    int character_upload_size,
    uint32_t palette_resource_id,
    int palette_upload_size)
{
    uint8_t *expected = sObjectResourceCalls == 0 ?
        gGameDatalinkSceneGraphicsArena + TEST_SHARED_RESOURCE_OFFSET :
        sSceneStorage.bytes + TEST_LOCAL_RESOURCES_OFFSET;

    assert((uint8_t *)(void *)resources == expected);
    if (sObjectResourceCalls == 0) {
        assert(character_resource_id == UINT32_C(0x80000002));
        assert(character_upload_size == -1);
        assert(palette_resource_id == 0x1f);
        assert(palette_upload_size == 500);
    } else {
        assert(character_resource_id == 0x17);
        assert(character_upload_size == 0x2000);
        assert(palette_resource_id == UINT32_MAX);
        assert(palette_upload_size == 0);
    }
    ++sObjectResourceCalls;
}

void Game_LoadDatalinkCellAnimationResources(
    Game_DatalinkGraphicsResourceSet *resources,
    uint32_t cell_bank_resource_id,
    uint32_t animation_bank_resource_id)
{
    if (sCellResourceCalls == 0) {
        assert((uint8_t *)(void *)resources ==
            gGameDatalinkSceneGraphicsArena + TEST_SHARED_RESOURCE_OFFSET);
        assert(cell_bank_resource_id == 0x1d);
        assert(animation_bank_resource_id == 0x1e);
        resources->cell_bank_1c = &sSharedCellBank;
        resources->animation_bank_24 = &sSharedAnimationBank;
    } else {
        assert((uint8_t *)(void *)resources ==
            sSceneStorage.bytes + TEST_LOCAL_RESOURCES_OFFSET);
        assert(cell_bank_resource_id == 0x1b);
        assert(animation_bank_resource_id == 0x1c);
        resources->cell_bank_1c = &sLocalCellBank;
        resources->animation_bank_24 = &sLocalAnimationBank;
    }
    ++sCellResourceCalls;
}

void NNS_G2dInitImageProxy(void *proxy)
{
    assert(proxy == gGameDatalinkSceneGraphicsArena +
        TEST_SHARED_IMAGE_PROXY_OFFSET);
    ++sImageProxyCalls;
}

void NNS_G2dInitImagePaletteProxy(void *proxy)
{
    assert(proxy == gGameDatalinkSceneGraphicsArena +
        TEST_SHARED_PALETTE_PROXY_OFFSET);
    ++sPaletteProxyCalls;
}

void Game_ResetDatalinkCellAnimationPool_0209bee4(void)
{
    ++sPoolResetCalls;
}

void *Game_AllocateDatalinkCellAnimation_0209be84(uint32_t *pool_index)
{
    uint8_t *record = sSceneStorage.bytes + TEST_RECORDS_OFFSET +
        sAnimationAllocations * TEST_RECORD_SIZE;

    assert(sAnimationAllocations < TEST_ANIMATION_COUNT);
    assert((uint8_t *)(void *)pool_index == record + 0x4c);
    *pool_index = 0x80U + sAnimationAllocations;
    return &sAnimations[sAnimationAllocations++];
}

const void *NNS_G2dGetAnimSequenceByIdx(
    const void *animation_bank,
    uint16_t sequence_index)
{
    unsigned int index = sSequenceCalls;
    uint16_t expected_sequence = index < 3 ?
        0 : sExpectedSharedSequences[index - 3];

    assert(index < TEST_ANIMATION_COUNT);
    assert(animation_bank == (index < 3 ?
        (const void *)&sLocalAnimationBank :
        (const void *)&sSharedAnimationBank));
    assert(sequence_index == expected_sequence);
    ++sSequenceCalls;
    return &sSequences[index];
}

void NNS_G2dInitCellAnimation(
    void *animation,
    const void *sequence,
    const void *cell_bank)
{
    unsigned int index = sAnimationInitCalls;

    assert(index < TEST_ANIMATION_COUNT);
    assert(animation == &sAnimations[index]);
    assert(sequence == &sSequences[index]);
    assert(cell_bank == (index < 3 ?
        (const void *)&sLocalCellBank :
        (const void *)&sSharedCellBank));
    ++sAnimationInitCalls;
}

void Game_SetDatalinkGraphicsModeToken_020a3778(uint32_t token)
{
    assert(token == 0xde);
    ++sModeTokenCalls;
}

void *Game_GetDatalinkGraphicsManager_0208da4c(void)
{
    ++sManagerCalls;
    return sGraphicsManager;
}

void Game_InitializeDatalinkStateInterface(Game_DatalinkState *state)
{
    Game_DatalinkState *primary = (Game_DatalinkState *)(void *)
        &gGameDatalinkSceneStaticStateArena.primary_state_vtable_018;
    Game_DatalinkState *secondary = (Game_DatalinkState *)(void *)
        &gGameDatalinkSceneStaticStateArena.secondary_state_vtable_080;

    assert(state == (sStateInterfaceCalls == 0 ? primary : secondary));
    ++sStateInterfaceCalls;
}

void Game_DatalinkStateStack_EnterRootState(
    Game_DatalinkStateStack *stack,
    Game_DatalinkState *state)
{
    Game_DatalinkState *secondary = (Game_DatalinkState *)(void *)
        &gGameDatalinkSceneStaticStateArena.secondary_state_vtable_080;

    assert((uint8_t *)(void *)stack == sSceneStorage.bytes);
    assert(state == secondary);
    ++sEnterRootCalls;
}

static void Test_PrepareRun(int skip_state_interfaces)
{
    memset(&sSceneStorage, 0xa5, sizeof(sSceneStorage));
    memset(gGameDatalinkSceneGraphicsArena, 0xa5,
        sizeof(gGameDatalinkSceneGraphicsArena));
    memset(gGameDatalinkScenePhaseObjectStorage, 0xa5,
        sizeof(gGameDatalinkScenePhaseObjectStorage));
    memset(sGraphicsManager, 0xa5, sizeof(sGraphicsManager));
    sFirstScreenData.data_size_08 = 0x1234;
    sSecondScreenData.data_size_08 = 0x2345;
    gGameDatalinkSceneSharedWord = UINT32_C(0xdeadbeef);
    gGameSubDisplayControl_04001000 = UINT32_MAX;
    gGameDatalinkCompletionOwner.completion_flag_d0 =
        skip_state_interfaces ? 1 : 0;
    gGameDatalinkCompletionOwner.completion_flag_d1 =
        skip_state_interfaces ? 3 : 0;
    Test_ResetCounters();
}

static void Test_CheckRun(int skip_state_interfaces)
{
    unsigned int index;
    uint32_t expected_display =
        (UINT32_C(0xffcfffef) & UINT32_C(0xffffe0ff)) |
        UINT32_C(0x1900);

    assert(sAllocationCalls == 4);
    assert(sFillCalls == 3);
    assert(sBackgroundCalls == 2);
    assert(sCharacterCalls == 1);
    assert(sScreenCalls == 4);
    assert(sObjectResourceCalls == 2);
    assert(sCellResourceCalls == 2);
    assert(sSubScreenCalls == 1);
    assert(sPoolResetCalls == 1);
    assert(sAnimationAllocations == TEST_ANIMATION_COUNT);
    assert(sSequenceCalls == TEST_ANIMATION_COUNT);
    assert(sAnimationInitCalls == TEST_ANIMATION_COUNT);
    assert(sImageProxyCalls == 1);
    assert(sPaletteProxyCalls == 1);
    assert(sModeTokenCalls == 1);
    assert(sManagerCalls == 1);
    assert(sStateStackInitCalls == 1);
    assert(sStateInterfaceCalls == (skip_state_interfaces ? 0U : 2U));
    assert(sEnterRootCalls == 1);

    assert(sSceneStorage.bytes[TEST_PENDING_SOUND_OFFSET] == 0);
    assert(sSceneStorage.bytes[TEST_ACTIVE_COUNT_OFFSET] == 58);
    assert(Test_ReadWord(sSceneStorage.bytes + TEST_GRAPHICS_CONTEXT_OFFSET) ==
        Test_TargetAddress(gGameDatalinkSceneGraphicsArena));
    assert(Test_ReadWord(sSceneStorage.bytes + TEST_SMALL_ALLOCATION_OFFSET) ==
        Test_TargetAddress(&sSmallAllocation));
    assert(Test_ReadWord(sSceneStorage.bytes + TEST_LARGE_ALLOCATION_OFFSET) ==
        Test_TargetAddress(&sLargeAllocation));
    assert(Test_ReadWord(
        sSceneStorage.bytes + TEST_FIRST_SCREEN_ALLOCATION_OFFSET) ==
        Test_TargetAddress(&sFirstScreenAllocation));
    assert(Test_ReadWord(
        sSceneStorage.bytes + TEST_SECOND_SCREEN_ALLOCATION_OFFSET) ==
        Test_TargetAddress(&sSecondScreenAllocation));
    assert(Test_ReadWord(
        gGameDatalinkScenePhaseObjectStorage + TEST_PHASE_VALUE_OFFSET) ==
        UINT32_C(0x40000));
    assert(Test_ReadWord(sGraphicsManager + 0x10) == 0);
    assert(Test_ReadWord(sGraphicsManager + 0x14) == 0);
    assert(gGameSubDisplayControl_04001000 == expected_display);
    assert(gGameDatalinkSceneSharedWord == 0);

    assert(Test_ReadWord(sSceneStorage.bytes + 0x430) ==
        Test_TargetAddress(sSceneStorage.bytes + 0x2d));
    assert(Test_ReadWord(sSceneStorage.bytes + 0x434) ==
        Test_TargetAddress(
            (const void *)&gGameDatalinkSceneStaticStateArena
                .primary_state_vtable_018));
    assert(Test_ReadWord(sSceneStorage.bytes + 0x438) ==
        Test_TargetAddress(
            (const void *)&gGameDatalinkSceneStaticStateArena
                .secondary_state_vtable_080));

    for (index = 0; index < TEST_ANIMATION_COUNT; ++index) {
        const uint8_t *record = sSceneStorage.bytes + TEST_RECORDS_OFFSET +
            index * TEST_RECORD_SIZE;
        uint16_t expected_sequence = index < 3 ?
            0 : sExpectedSharedSequences[index - 3];

        assert(record[0x48] == (uint8_t)expected_sequence);
        assert(record[0x49] == 0);
        assert(Test_ReadWord(record + 0x4c) == 0x80U + index);
        assert(Test_ReadWord(record + 0x50) == 0x1000);
        assert(Test_ReadWord(record + 0x54) ==
            Test_TargetAddress(&sAnimations[index]));
    }
    for (index = 0; index < 4; ++index) {
        const uint8_t *record = sSceneStorage.bytes + TEST_RECORDS_OFFSET +
            (0x36U + index) * TEST_RECORD_SIZE;
        assert(record[2] == index + 1U);
    }
}

int main(void)
{
    memset((void *)&gGameDatalinkSceneStaticStateArena, 0,
        sizeof(gGameDatalinkSceneStaticStateArena));
    gGameDatalinkSceneStaticStateArena.primary_guard_014 = 0x10;
    gGameDatalinkSceneStaticStateArena.secondary_guard_000 = 0x20;
    Test_PrepareRun(0);
    Game_InitializeDatalinkScene_020ad070(
        (Game_DatalinkSceneOwner *)(void *)sSceneStorage.bytes);
    Test_CheckRun(0);
    assert(gGameDatalinkSceneStaticStateArena.primary_guard_014 == 0x11);
    assert(gGameDatalinkSceneStaticStateArena.secondary_guard_000 == 0x21);
    assert(gGameDatalinkSceneStaticStateArena.primary_state_vtable_018 ==
        UINT32_C(0x02124b40));
    assert(gGameDatalinkSceneStaticStateArena.secondary_state_vtable_080 ==
        UINT32_C(0x02124b88));
    assert(gGameDatalinkSceneStaticStateArena.secondary_state_data_0b0 ==
        UINT32_C(0x02124bb0));

    gGameDatalinkSceneStaticStateArena.primary_guard_014 = 0x31;
    gGameDatalinkSceneStaticStateArena.secondary_guard_000 = 0x41;
    gGameDatalinkSceneStaticStateArena.primary_state_vtable_018 =
        UINT32_C(0x12345678);
    gGameDatalinkSceneStaticStateArena.secondary_state_vtable_080 =
        UINT32_C(0x23456789);
    gGameDatalinkSceneStaticStateArena.secondary_state_data_0b0 =
        UINT32_C(0x3456789a);
    Test_PrepareRun(1);
    Game_InitializeDatalinkScene_020ad070(
        (Game_DatalinkSceneOwner *)(void *)sSceneStorage.bytes);
    Test_CheckRun(1);
    assert(gGameDatalinkSceneStaticStateArena.primary_guard_014 == 0x31);
    assert(gGameDatalinkSceneStaticStateArena.secondary_guard_000 == 0x41);
    assert(gGameDatalinkSceneStaticStateArena.primary_state_vtable_018 ==
        UINT32_C(0x12345678));
    assert(gGameDatalinkSceneStaticStateArena.secondary_state_vtable_080 ==
        UINT32_C(0x23456789));
    assert(gGameDatalinkSceneStaticStateArena.secondary_state_data_0b0 ==
        UINT32_C(0x3456789a));
    return 0;
}
