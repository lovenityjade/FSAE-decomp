#include "game/manual_page_flow.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static GameManualPageIndex sIndex;
static uint32_t sAllocator;
static void *sAllocatorContext = &sAllocator;
static uint8_t sResource[128];
static const void *sReturnedResource;
static unsigned int sAllocationCount;
static unsigned int sFreeCount;
static unsigned int sGetCount;

static uint16_t ReadU16(const uint8_t *bytes)
{
    return (uint16_t)(bytes[0] | (uint16_t)(bytes[1] << 8));
}

static uint32_t ReadTag(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24) |
        ((uint32_t)bytes[1] << 16) |
        ((uint32_t)bytes[2] << 8) |
        bytes[3];
}

static void WriteU16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
}

static void WriteU32(uint8_t *bytes, uint32_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
    bytes[2] = (uint8_t)(value >> 16);
    bytes[3] = (uint8_t)(value >> 24);
}

static void WriteTag(uint8_t *bytes, const char tag[4])
{
    memcpy(bytes, tag, 4);
}

static void BuildValidResource(void)
{
    uint8_t *name = sResource + 16;
    uint8_t *text = name + 32;
    uint8_t *records = text + 36;

    memset(sResource, 0, sizeof(sResource));
    WriteTag(sResource, "NTMC");
    WriteU16(sResource + 4, 0xfeff);
    WriteU16(sResource + 6, 0x0200);
    WriteU32(sResource + 8, 120);
    WriteU16(sResource + 12, 16);
    WriteU16(sResource + 14, 3);

    WriteTag(name, "nap1");
    WriteU32(name + 4, 32);
    WriteU32(name + 8, 2);
    WriteU32(name + 12, 8);
    WriteU32(name + 16, 13);
    memcpy(name + 20, "page", 5);
    memcpy(name + 25, "child", 6);

    WriteTag(text, "txp1");
    WriteU32(text + 4, 36);
    WriteU32(text + 8, 3);
    WriteU32(text + 12, 12);
    WriteU32(text + 16, 16);
    WriteU32(text + 20, 20);
    WriteU16(text + 24, 0x1111);
    WriteU16(text + 28, 0x2222);
    WriteU16(text + 32, 0x3333);

    WriteTag(records, "mtc1");
    WriteU32(records + 4, 36);
    WriteU16(records + 8, 2);
    WriteU16(records + 12, 0);
    WriteU16(records + 14, 0);
    WriteU16(records + 16, 1);
    WriteU16(records + 18, 1);
    WriteU16(records + 20, 1);
    WriteU16(records + 22, 2);
    WriteU16(records + 24, 0xffff);
    WriteU16(records + 26, 0);
    WriteU16(records + 28, 1);
    WriteU16(records + 30, 0);
    WriteU16(records + 32, 0);
    WriteU16(records + 34, 0);
}

void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size)
{
    void *allocation;

    assert(allocator == &sAllocator);
    assert(size != 0);
    allocation = calloc(1, size);
    assert(allocation != NULL);
    ++sAllocationCount;
    return allocation;
}

void NNS_FndFreeToAllocator(void *allocator, void *allocation)
{
    assert(allocator == &sAllocator);
    assert(allocation != NULL);
    ++sFreeCount;
    free(allocation);
}

bool NtmvM2dBinaryFile_IsValid(
    const void *file,
    uint32_t signature)
{
    const uint8_t *bytes = file;
    return ReadTag(bytes) == signature &&
        ReadU16(bytes + 4) == 0xfeff;
}

static void *GetLocalizedFile(
    NtmvM2dArcResourceAccessor *accessor,
    uint32_t resource_type,
    const char *name,
    void *unused_result)
{
    assert(accessor != NULL);
    assert(resource_type == 0x6e746d63);
    assert(strcmp(name, "manual") == 0);
    assert(unused_result == &sAllocatorContext);
    ++sGetCount;
    return (void *)sReturnedResource;
}

static const NtmvM2dArcResourceAccessorVTable sAccessorVTable = {
    .get_localized_file = GetLocalizedFile,
};

int main(void)
{
    NtmvM2dArcResourceAccessor accessor = {
        .vtable = &sAccessorVTable,
    };
    GameManualPageRecord *records;

    memset(&sIndex, 0xff, sizeof(sIndex));
    assert(GameManualPageIndex_Construct(&sIndex) == &sIndex);
    assert(sIndex.records == NULL);
    assert(sIndex.record_count == 0);

    GameManualPageIndex_Destroy_020b8c10(
        &sIndex, &sAllocatorContext);
    assert(sFreeCount == 0);

    records = GameManualPageIndex_AllocateRecords_020b8e28(
        &sAllocatorContext, 2);
    assert(records[0].page_name == NULL);
    assert(records[0].child_count == 0);
    assert(records[1].subpage_names == NULL);
    sIndex.records = records;
    sIndex.record_count = 2;
    GameManualPageIndex_Destroy_020b8c10(
        &sIndex, &sAllocatorContext);
    assert(sAllocationCount == 1);
    assert(sFreeCount == 1);

    sReturnedResource = NULL;
    assert(!GameManualPageIndex_Load_020b8c78(
        &sIndex, "manual", &accessor, &sAllocatorContext));
    assert(sGetCount == 1);

    BuildValidResource();
    WriteU16(sResource + 6, 0x0100);
    sReturnedResource = sResource;
    assert(!GameManualPageIndex_Load_020b8c78(
        &sIndex, "manual", &accessor, &sAllocatorContext));
    assert(sGetCount == 2);

    WriteU16(sResource + 6, 0x0200);
    GameManualPageIndex_Construct(&sIndex);
    assert(GameManualPageIndex_Load_020b8c78(
        &sIndex, "manual", &accessor, &sAllocatorContext));
    assert(sGetCount == 3);
    assert(sIndex.record_count == 2);
    assert(strcmp(sIndex.records[0].page_name, "page") == 0);
    assert(sIndex.records[0].toc_text[0] == 0x1111);
    assert(sIndex.records[0].title[0] == 0x2222);
    assert(sIndex.records[0].child_count == 1);
    assert(strcmp(sIndex.records[0].subpage_names[0], "child") == 0);
    assert(sIndex.records[0].subpage_texts[0][0] == 0x3333);
    assert(sIndex.records[1].page_name == NULL);
    assert(sIndex.records[1].child_count == 0);
    assert(sAllocationCount == 6);

    GameManualPageIndex_Destroy_020b8c10(
        &sIndex, &sAllocatorContext);
    assert(sFreeCount == 6);
    return 0;
}
