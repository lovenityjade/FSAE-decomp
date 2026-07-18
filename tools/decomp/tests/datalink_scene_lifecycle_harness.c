#include "game/datalink_scene_lifecycle.h"

#include "game/datalink_controller.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

enum {
    TEST_SCENE_SIZE = 0x26b8,
    TEST_SERIALIZED_COUNT_OFFSET = 0x253c,
    TEST_SERIALIZED_FLAG_OFFSET = 0x253d,
    TEST_GRAPHICS_OWNER_ADDRESS_OFFSET = 0x2540,
    TEST_OAM_BUFFER_ADDRESS_OFFSET = 0x2544,
    TEST_INLINE_RESOURCE_SET_OFFSET = 0x2548,
    TEST_RESOURCE_GROUP_A_OFFSET = 0x25ac,
    TEST_RESOURCE_GROUP_B_OFFSET = 0x262c,
    TEST_AUXILIARY_A_ADDRESS_OFFSET = 0x26ac,
    TEST_AUXILIARY_B_ADDRESS_OFFSET = 0x26b0,
    TEST_AUXILIARY_C_ADDRESS_OFFSET = 0x26b4,
    TEST_RESOURCE_GROUP_COUNT = 4,
    TEST_RESOURCE_GROUP_STRIDE = 0x20,
    TEST_RESOURCE_ADDRESS_COUNT = 4,
    TEST_RESOURCE_ADDRESS_STRIDE = 8
};

volatile uint16_t gGameDatalinkSubBgControl_04001008[4];
Game_DatalinkSubDisplayConfigOwner gGameDatalinkSubDisplayConfigOwner;

static uint8_t sBg1Screen[0x1001];
static uint8_t sBg2Screen[0x1001];
static char sDisplayEvents[24];
static unsigned int sDisplayEventCount;
static unsigned int sClearFastCalls;

static union {
    uintptr_t alignment;
    uint8_t bytes[TEST_SCENE_SIZE];
} sSceneStorage;

static const Game_DatalinkSceneOwner *sExpectedConstScene;
static Game_DatalinkSceneOwner *sExpectedScene;
static unsigned int sSerializeCalls;
static unsigned int sDeserializeCalls;
static unsigned int sCpuCopyCalls;

static uintptr_t sFreedAllocations[40];
static uintptr_t sReleasedResourceSets[2];
static unsigned int sReleasedSlots[4];
static unsigned int sFreeCount;
static unsigned int sResourceSetCount;
static unsigned int sSlotCount;
static unsigned int sPoolResetCalls;

static void DisplayEvent(char event)
{
    assert(sDisplayEventCount + 1 < sizeof(sDisplayEvents));
    sDisplayEvents[sDisplayEventCount++] = event;
    sDisplayEvents[sDisplayEventCount] = '\0';
}

void GXS_SetGraphicsMode(uint32_t background_mode)
{
    assert(background_mode == 0);
    DisplayEvent('M');
}

void GX_SetBankForSubBG(uint32_t bank)
{
    assert(bank == 4);
    DisplayEvent('B');
}

void GX_SetBankForSubBGExtPltt(uint32_t bank)
{
    assert(bank == 0x80);
    DisplayEvent('E');
}

void *G2S_GetBG1ScrPtr(void)
{
    DisplayEvent('1');
    return sBg1Screen;
}

void *G2S_GetBG2ScrPtr(void)
{
    DisplayEvent('2');
    return sBg2Screen;
}

void MIi_CpuClearFast(uint32_t value, void *destination, uint32_t size)
{
    assert(value == 0);
    assert(sClearFastCalls < 4);
    if ((sClearFastCalls & 1U) == 0U) {
        assert(size == 0x200);
    } else {
        assert(size == 0x800);
    }
    memset(destination, 0, size);
    ++sClearFastCalls;
    DisplayEvent('C');
}

void MI_CpuCopy(const void *source, void *destination, uint32_t size)
{
    assert(size == 1);
    ++sCpuCopyCalls;
    memcpy(destination, source, size);
}

void Game_DatalinkStateStack_Serialize(
    const Game_DatalinkStateStack *stack,
    uint8_t **cursor)
{
    unsigned int index;

    assert((const void *)stack == (const void *)sExpectedConstScene);
    ++sSerializeCalls;
    for (index = 0; index < 5; ++index) {
        (*cursor)[index] = (uint8_t)(0xc0 + index);
    }
    *cursor += 5;
}

void Game_DatalinkStateStack_Deserialize(
    Game_DatalinkStateStack *stack,
    const uint8_t **cursor)
{
    unsigned int index;

    assert((void *)stack == (void *)sExpectedScene);
    for (index = 0; index < 5; ++index) {
        assert((*cursor)[index] == (uint8_t)(0xd0 + index));
    }
    ++sDeserializeCalls;
    *cursor += 5;
}

void Game_FreeAllocation_0208e188(void *allocation)
{
    assert(sFreeCount < sizeof(sFreedAllocations) /
        sizeof(sFreedAllocations[0]));
    sFreedAllocations[sFreeCount++] = (uintptr_t)allocation;
}

void Game_ResetCellAnimationPool_0209bee4(void)
{
    ++sPoolResetCalls;
}

void Game_ReleaseGraphicsResourceSlot_0209d690(unsigned int slot)
{
    assert(sSlotCount < 4);
    sReleasedSlots[sSlotCount++] = slot;
}

void Game_ReleaseGraphicsResourceSet_0209d714(void *resource_set)
{
    assert(sResourceSetCount < 2);
    sReleasedResourceSets[sResourceSetCount++] =
        (uintptr_t)resource_set;
}

static Game_DatalinkSceneOwner *Scene(void)
{
    return (Game_DatalinkSceneOwner *)(void *)sSceneStorage.bytes;
}

static void WriteSceneWord(unsigned int offset, uint32_t value)
{
    memcpy(sSceneStorage.bytes + offset, &value, sizeof(value));
}

static uint32_t ReadSceneWord(unsigned int offset)
{
    uint32_t value;

    memcpy(&value, sSceneStorage.bytes + offset, sizeof(value));
    return value;
}

static void TestConfigureSubDisplay(void)
{
    unsigned int index;

    memset(sBg1Screen, 0xa5, sizeof(sBg1Screen));
    memset(sBg2Screen, 0xa5, sizeof(sBg2Screen));
    memset(
        &gGameDatalinkSubDisplayConfigOwner,
        0xa5,
        sizeof(gGameDatalinkSubDisplayConfigOwner));
    for (index = 0; index < 4; ++index) {
        gGameDatalinkSubBgControl_04001008[index] = 0xffff;
    }
    sDisplayEventCount = 0;
    sClearFastCalls = 0;

    Game_ConfigureDatalinkSubDisplay_020ad528();

    assert(strcmp(sDisplayEvents, "MBE1C1C2C2C") == 0);
    assert(sClearFastCalls == 4);
    assert(gGameDatalinkSubBgControl_04001008[0] == 0x154c);
    assert(gGameDatalinkSubBgControl_04001008[1] == 0x5341);
    assert(gGameDatalinkSubBgControl_04001008[2] == 0x5142);
    assert(gGameDatalinkSubBgControl_04001008[3] == 0x10d3);
    assert(sBg1Screen[0x5ff] == 0xa5);
    assert(sBg1Screen[0x600] == 0);
    assert(sBg1Screen[0xfff] == 0);
    assert(sBg1Screen[0x1000] == 0xa5);
    assert(sBg2Screen[0x5ff] == 0xa5);
    assert(sBg2Screen[0x600] == 0);
    assert(sBg2Screen[0xfff] == 0);
    assert(sBg2Screen[0x1000] == 0xa5);
    assert(gGameDatalinkSubDisplayConfigOwner.screen_base_blocks_fc[0] == 4);
    assert(gGameDatalinkSubDisplayConfigOwner.screen_base_blocks_fc[1] == 8);
    assert(gGameDatalinkSubDisplayConfigOwner.screen_base_blocks_fc[2] == 10);
    assert(gGameDatalinkSubDisplayConfigOwner.screen_base_blocks_fc[3] == 6);
}

static void TestSerializeSceneState(void)
{
    uint8_t buffer[16];
    uint8_t *cursor = buffer;
    unsigned int index;

    memset(&sSceneStorage, 0xa5, sizeof(sSceneStorage));
    memset(buffer, 0, sizeof(buffer));
    sSceneStorage.bytes[TEST_SERIALIZED_COUNT_OFFSET] = 0x3a;
    sSceneStorage.bytes[TEST_SERIALIZED_FLAG_OFFSET] = 0x7b;
    sExpectedConstScene = Scene();
    sSerializeCalls = 0;
    sCpuCopyCalls = 0;

    Game_SerializeDatalinkSceneState_020ad668(Scene(), &cursor);

    assert(sSerializeCalls == 1);
    assert(sCpuCopyCalls == 2);
    assert(cursor == buffer + 7);
    for (index = 0; index < 5; ++index) {
        assert(buffer[index] == (uint8_t)(0xc0 + index));
    }
    assert(buffer[5] == 0x3a);
    assert(buffer[6] == 0x7b);
}

static void TestDeserializeSceneState(void)
{
    uint8_t buffer[16];
    const uint8_t *cursor = buffer;
    unsigned int index;

    memset(&sSceneStorage, 0xa5, sizeof(sSceneStorage));
    memset(buffer, 0, sizeof(buffer));
    for (index = 0; index < 5; ++index) {
        buffer[index] = (uint8_t)(0xd0 + index);
    }
    buffer[5] = 0x42;
    buffer[6] = 0x24;
    sExpectedScene = Scene();
    sDeserializeCalls = 0;
    sCpuCopyCalls = 0;

    Game_DeserializeDatalinkSceneState_020ad6bc(Scene(), &cursor);

    assert(sDeserializeCalls == 1);
    assert(sCpuCopyCalls == 2);
    assert(cursor == buffer + 7);
    assert(sSceneStorage.bytes[TEST_SERIALIZED_COUNT_OFFSET] == 0x42);
    assert(sSceneStorage.bytes[TEST_SERIALIZED_FLAG_OFFSET] == 0x24);
}

static void TestReleaseSceneResources(void)
{
    uintptr_t expected_frees[40];
    unsigned int expected_free_count = 0;
    unsigned int group_index;
    unsigned int resource_index;

    memset(&sSceneStorage, 0, sizeof(sSceneStorage));
    sFreeCount = 0;
    sResourceSetCount = 0;
    sSlotCount = 0;
    sPoolResetCalls = 0;
    WriteSceneWord(TEST_GRAPHICS_OWNER_ADDRESS_OFFSET, 0x1000);
    WriteSceneWord(TEST_OAM_BUFFER_ADDRESS_OFFSET, 0x2000);
    expected_frees[expected_free_count++] = 0x2000;
    WriteSceneWord(TEST_AUXILIARY_A_ADDRESS_OFFSET, 0);
    WriteSceneWord(TEST_AUXILIARY_B_ADDRESS_OFFSET, 0x3000);
    WriteSceneWord(TEST_AUXILIARY_C_ADDRESS_OFFSET, 0x4000);
    expected_frees[expected_free_count++] = 0x3000;
    expected_frees[expected_free_count++] = 0x4000;

    for (group_index = 0;
         group_index < TEST_RESOURCE_GROUP_COUNT;
         ++group_index) {
        for (resource_index = 0;
             resource_index < TEST_RESOURCE_ADDRESS_COUNT;
             ++resource_index) {
            unsigned int a_offset = TEST_RESOURCE_GROUP_A_OFFSET +
                group_index * TEST_RESOURCE_GROUP_STRIDE +
                resource_index * TEST_RESOURCE_ADDRESS_STRIDE;
            unsigned int b_offset = TEST_RESOURCE_GROUP_B_OFFSET +
                group_index * TEST_RESOURCE_GROUP_STRIDE +
                resource_index * TEST_RESOURCE_ADDRESS_STRIDE;
            uint32_t a_address =
                0xa000 + group_index * 0x100 + resource_index;
            uint32_t b_address = (resource_index & 1U) == 0U ?
                0xb000 + group_index * 0x100 + resource_index : 0;

            WriteSceneWord(a_offset, a_address);
            WriteSceneWord(a_offset + 4, 0xdead0000 + a_offset);
            WriteSceneWord(b_offset, b_address);
            WriteSceneWord(b_offset + 4, 0xbeef0000 + b_offset);
        }
    }

    Game_ReleaseDatalinkSceneResources_020ad710(Scene());

    assert(sResourceSetCount == 2);
    assert(sReleasedResourceSets[0] == 0x101c);
    assert(sReleasedResourceSets[1] ==
        (uintptr_t)(sSceneStorage.bytes + TEST_INLINE_RESOURCE_SET_OFFSET));
    assert(sSlotCount == 4);
    for (group_index = 0;
         group_index < TEST_RESOURCE_GROUP_COUNT;
         ++group_index) {
        assert(sReleasedSlots[group_index] == group_index);
        for (resource_index = 0;
             resource_index < TEST_RESOURCE_ADDRESS_COUNT;
             ++resource_index) {
            expected_frees[expected_free_count++] =
                0xa000 + group_index * 0x100 + resource_index;
        }
        for (resource_index = 0;
             resource_index < TEST_RESOURCE_ADDRESS_COUNT;
             resource_index += 2) {
            expected_frees[expected_free_count++] =
                0xb000 + group_index * 0x100 + resource_index;
        }
        for (resource_index = 0;
             resource_index < TEST_RESOURCE_ADDRESS_COUNT;
             ++resource_index) {
            unsigned int a_offset = TEST_RESOURCE_GROUP_A_OFFSET +
                group_index * TEST_RESOURCE_GROUP_STRIDE +
                resource_index * TEST_RESOURCE_ADDRESS_STRIDE;
            unsigned int b_offset = TEST_RESOURCE_GROUP_B_OFFSET +
                group_index * TEST_RESOURCE_GROUP_STRIDE +
                resource_index * TEST_RESOURCE_ADDRESS_STRIDE;

            assert(ReadSceneWord(a_offset) == 0);
            assert(ReadSceneWord(b_offset) == 0);
            assert(ReadSceneWord(a_offset + 4) == 0xdead0000 + a_offset);
            assert(ReadSceneWord(b_offset + 4) == 0xbeef0000 + b_offset);
        }
    }
    assert(sFreeCount == expected_free_count);
    for (group_index = 0; group_index < sFreeCount; ++group_index) {
        assert(sFreedAllocations[group_index] ==
            expected_frees[group_index]);
    }
    assert(ReadSceneWord(TEST_GRAPHICS_OWNER_ADDRESS_OFFSET) == 0x1000);
    assert(ReadSceneWord(TEST_OAM_BUFFER_ADDRESS_OFFSET) == 0);
    assert(ReadSceneWord(TEST_AUXILIARY_A_ADDRESS_OFFSET) == 0);
    assert(ReadSceneWord(TEST_AUXILIARY_B_ADDRESS_OFFSET) == 0);
    assert(ReadSceneWord(TEST_AUXILIARY_C_ADDRESS_OFFSET) == 0);
    assert(sPoolResetCalls == 1);
}

int main(void)
{
    assert(sizeof(Game_DatalinkSceneResourceGroup) == 0x20);
    TestConfigureSubDisplay();
    TestSerializeSceneState();
    TestDeserializeSceneState();
    TestReleaseSceneResources();
    return 0;
}
