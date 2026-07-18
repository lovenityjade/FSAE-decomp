#include "game/manual_page_loader.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum {
    TEST_RESOURCE_SIZE = 256,
    TEST_HEADER_SIZE = 16
};

static uint8_t sResource[TEST_RESOURCE_SIZE];
static uint32_t sAllocator;
static NtmvM2dPane sPanes[2];
static NtmvM2dPane *sRootChildren[1];
static void *sFileResults[2];
static bool sFailAllocation;
static bool sFailFactory;
static bool sCheckFullContext;
static uint32_t sLastAllocationSize;
static unsigned int sAllocationCount;
static unsigned int sGetFileCount;
static unsigned int sFactoryCount;
static unsigned int sAllocateChildrenCount;
static unsigned int sAddChildCount;
static GameManualPage *sExpectedPage;
static const void *sExpectedNamePool;
static const void *sExpectedTextPool;

static uint16_t ReadU16(const uint8_t *bytes)
{
    return (uint16_t)(bytes[0] | ((uint16_t)bytes[1] << 8));
}

static uint32_t ReadTag(const uint8_t bytes[4])
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

static void WriteBlockHeader(
    uint8_t *block,
    const char tag[4],
    uint32_t size)
{
    WriteTag(block, tag);
    WriteU32(block + 4, size);
}

static size_t BuildValidResource(void)
{
    uint8_t *name_pool;
    uint8_t *text_pool;
    uint8_t *resource_list;
    uint8_t *page_definition;
    uint8_t *pane;
    uint8_t *children_begin;
    uint8_t *picture;
    uint8_t *children_end;
    uint8_t *unknown;
    size_t cursor = TEST_HEADER_SIZE;

    memset(sResource, 0, sizeof(sResource));
    WriteTag(sResource, "NTPG");
    WriteU16(sResource + 4, 0xfeff);
    WriteU16(sResource + 6, 0x0200);
    WriteU16(sResource + 12, TEST_HEADER_SIZE);
    WriteU16(sResource + 14, 9);

    name_pool = sResource + cursor;
    WriteBlockHeader(name_pool, "nap1", 32);
    WriteU32(name_pool + 8, 2);
    WriteU32(name_pool + 12, 8);
    WriteU32(name_pool + 16, 14);
    memcpy(name_pool + 20, "fontA", 6);
    memcpy(name_pool + 26, "fontB", 6);
    cursor += 32;

    text_pool = sResource + cursor;
    WriteBlockHeader(text_pool, "txp1", 28);
    WriteU32(text_pool + 8, 2);
    WriteU32(text_pool + 12, 8);
    WriteU32(text_pool + 16, 12);
    WriteU16(text_pool + 20, 0x1111);
    WriteU16(text_pool + 24, 0x2222);
    cursor += 28;

    resource_list = sResource + cursor;
    WriteBlockHeader(resource_list, "txl1", 20);
    WriteU16(resource_list + 8, 2);
    WriteU16(resource_list + 12, 0);
    WriteU16(resource_list + 16, 1);
    cursor += 20;

    page_definition = sResource + cursor;
    WriteBlockHeader(page_definition, "pag1", 16);
    WriteU16(page_definition + 8, (uint16_t)-7);
    WriteU16(page_definition + 10, 231);
    WriteU16(page_definition + 12, 0x1234);
    WriteU16(page_definition + 14, 1);
    cursor += 16;

    pane = sResource + cursor;
    WriteBlockHeader(pane, "pan1", 16);
    cursor += 16;

    children_begin = sResource + cursor;
    WriteBlockHeader(children_begin, "pas1", 12);
    WriteU16(children_begin + 8, 1);
    cursor += 12;

    picture = sResource + cursor;
    WriteBlockHeader(picture, "pic1", 20);
    cursor += 20;

    children_end = sResource + cursor;
    WriteBlockHeader(children_end, "pae1", 8);
    cursor += 8;

    unknown = sResource + cursor;
    WriteBlockHeader(unknown, "zzz1", 8);
    cursor += 8;

    WriteU32(sResource + 8, (uint32_t)cursor);
    sExpectedNamePool = name_pool;
    sExpectedTextPool = text_pool;
    return cursor;
}

void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size)
{
    void *allocation;

    assert(allocator == &sAllocator);
    assert(size != 0);
    ++sAllocationCount;
    sLastAllocationSize = size;
    if (sFailAllocation) {
        return NULL;
    }
    allocation = malloc(size);
    assert(allocation != NULL);
    memset(allocation, 0xa5, size);
    return allocation;
}

bool NtmvM2dBinaryFile_IsValid(
    const void *file,
    uint32_t signature)
{
    const uint8_t *bytes = file;
    return ReadTag(bytes) == signature &&
        ReadU16(bytes + 4) == 0xfeff;
}

static void *GetFile(
    NtmvM2dArcResourceAccessor *accessor,
    uint32_t resource_type,
    const char *name)
{
    static const char *const expected_names[2] = {"fontA", "fontB"};

    assert(accessor != NULL);
    assert(resource_type == 0x6e747466);
    assert(sGetFileCount < 2);
    assert(strcmp(name, expected_names[sGetFileCount]) == 0);
    return sFileResults[sGetFileCount++];
}

NtmvM2dPane *NtmvM2dPage_CreatePane_020b5318(
    GameManualPage *page,
    void *allocator_context,
    uint32_t block_signature,
    const void *block,
    const NtmvM2dPageBuildContext *build_context)
{
    assert(page == sExpectedPage);
    assert(*(void **)allocator_context == &sAllocator);
    assert(ReadTag(block) == block_signature);
    if (sCheckFullContext) {
        assert(build_context->accessor != NULL);
        assert(build_context->name_pool == sExpectedNamePool);
        assert(build_context->text_pool == sExpectedTextPool);
        assert(build_context->resource_set == &page->resource_count);
    }
    ++sFactoryCount;
    if (sFailFactory) {
        return NULL;
    }
    if (block_signature == 0x70616e31) {
        return &sPanes[0];
    }
    assert(block_signature == 0x70696331);
    return &sPanes[1];
}

void NtmvM2dPane_AllocateChildren_020b56b8(
    NtmvM2dPane *pane,
    NtmvM2dAllocatorContext *allocator_context,
    uint16_t child_capacity)
{
    assert(pane == &sPanes[0]);
    assert(*(void **)allocator_context == &sAllocator);
    assert(child_capacity == 1);
    pane->children = sRootChildren;
    pane->child_capacity = child_capacity;
    ++sAllocateChildrenCount;
}

void NtmvM2dPane_AddChild_020b5728(
    NtmvM2dPane *parent,
    NtmvM2dPane *child)
{
    uint16_t index;

    assert(parent == &sPanes[0]);
    assert(child == &sPanes[1]);
    index = parent->child_count++;
    parent->children[index] = child;
    child->parent = parent;
    ++sAddChildCount;
}

static void ResetFixture(void)
{
    memset(sPanes, 0, sizeof(sPanes));
    memset(sRootChildren, 0, sizeof(sRootChildren));
    sFileResults[0] = (void *)(uintptr_t)0x11110000u;
    sFileResults[1] = (void *)(uintptr_t)0x22220000u;
    sFailAllocation = false;
    sFailFactory = false;
    sCheckFullContext = false;
    sLastAllocationSize = 0;
    sAllocationCount = 0;
    sGetFileCount = 0;
    sFactoryCount = 0;
    sAllocateChildrenCount = 0;
    sAddChildCount = 0;
}

static void TestCompleteParse(void)
{
    static const NtmvM2dArcResourceAccessorVTable vtable = {
        .get_file = GetFile,
    };
    NtmvM2dArcResourceAccessor accessor = {.vtable = &vtable};
    GameManualPage page;
    void *allocator_context = &sAllocator;
    size_t file_size;

    ResetFixture();
    file_size = BuildValidResource();
    memset(&page, 0, sizeof(page));
    sExpectedPage = &page;
    sCheckFullContext = true;

    assert(GameManualPage_Load_020b4f00(
        &page, &allocator_context, sResource, &accessor));
    assert(file_size == 176);
    assert(page.content_width == (uint16_t)-7);
    assert(page.content_extent == 231);
    assert(page.palette_color == 0x1234);
    assert(page.content_resource == sResource + TEST_HEADER_SIZE + 32 + 24);
    assert(page.resource_count == 2);
    assert(page.resource_files != NULL);
    assert(page.resource_files[0] == sFileResults[0]);
    assert(page.resource_files[1] == sFileResults[1]);
    assert(page.root_pane == &sPanes[0]);
    assert(sPanes[0].children == sRootChildren);
    assert(sPanes[0].child_capacity == 1);
    assert(sPanes[0].child_count == 1);
    assert(sPanes[0].children[0] == &sPanes[1]);
    assert(sPanes[1].parent == &sPanes[0]);
    assert(sAllocationCount == 1);
    assert(sLastAllocationSize == 2 * sizeof(void *));
    assert(sGetFileCount == 2);
    assert(sFactoryCount == 2);
    assert(sAllocateChildrenCount == 1);
    assert(sAddChildCount == 1);
    free(page.resource_files);
}

static void TestValidationFailures(void)
{
    static const NtmvM2dArcResourceAccessorVTable vtable = {
        .get_file = GetFile,
    };
    NtmvM2dArcResourceAccessor accessor = {.vtable = &vtable};
    GameManualPage page;
    void *allocator_context = &sAllocator;

    ResetFixture();
    BuildValidResource();
    memset(&page, 0, sizeof(page));
    sExpectedPage = &page;

    sResource[0] = 'X';
    assert(!GameManualPage_Load_020b4f00(
        &page, &allocator_context, sResource, &accessor));
    sResource[0] = 'N';
    WriteU16(sResource + 4, 0xfffe);
    assert(!GameManualPage_Load_020b4f00(
        &page, &allocator_context, sResource, &accessor));
    WriteU16(sResource + 4, 0xfeff);
    WriteU16(sResource + 6, 0x0201);
    assert(!GameManualPage_Load_020b4f00(
        &page, &allocator_context, sResource, &accessor));
    assert(sAllocationCount == 0);
    assert(sFactoryCount == 0);
}

static void TestTrustedFailureSemantics(void)
{
    static const NtmvM2dArcResourceAccessorVTable vtable = {
        .get_file = GetFile,
    };
    NtmvM2dArcResourceAccessor accessor = {.vtable = &vtable};
    GameManualPage page;
    GameManualPage before;
    void *allocator_context = &sAllocator;

    ResetFixture();
    memset(sResource, 0, sizeof(sResource));
    WriteTag(sResource, "NTPG");
    WriteU16(sResource + 4, 0xfeff);
    WriteU16(sResource + 6, 0x0200);
    WriteU32(sResource + 8, TEST_HEADER_SIZE);
    WriteU16(sResource + 12, TEST_HEADER_SIZE);
    memset(&page, 0x5a, sizeof(page));
    before = page;
    sExpectedPage = &page;
    assert(GameManualPage_Load_020b4f00(
        &page, &allocator_context, sResource, &accessor));
    assert(memcmp(&page, &before, sizeof(page)) == 0);

    memset(&page, 0, sizeof(page));
    WriteU16(sResource + 14, 1);
    WriteU32(sResource + 8, TEST_HEADER_SIZE + 16);
    WriteBlockHeader(sResource + TEST_HEADER_SIZE, "pan1", 16);
    sFailFactory = true;
    assert(GameManualPage_Load_020b4f00(
        &page, &allocator_context, sResource, &accessor));
    assert(page.root_pane == NULL);
    assert(sFactoryCount == 1);
}

static void TestResourceArrayAllocation(void)
{
    void *allocator_context = &sAllocator;
    void **resources;

    ResetFixture();
    resources = NtmvM2dPage_AllocateResourceArray_020b51c8(
        &allocator_context, 2);
    assert(resources != NULL);
    assert(resources[0] == NULL);
    assert(resources[1] == NULL);
    assert(sLastAllocationSize == 2 * sizeof(void *));
    free(resources);

    resources = NtmvM2dPage_AllocateResourceArray_020b51c8(
        &allocator_context, 0);
    assert(resources != NULL);
    assert(sLastAllocationSize == sizeof(void *));
    free(resources);

    sFailAllocation = true;
    assert(NtmvM2dPage_AllocateResourceArray_020b51c8(
        &allocator_context, 3) == NULL);
}

int main(void)
{
    TestCompleteParse();
    TestValidationFailures();
    TestTrustedFailureSemantics();
    TestResourceArrayAllocation();
    return 0;
}
