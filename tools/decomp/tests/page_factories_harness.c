#include "game/manual_page_loader.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef union TestResourceBlock {
    max_align_t alignment;
    uint8_t bytes[0x50];
} TestResourceBlock;

enum TestFactoryKind {
    TEST_FACTORY_NONE,
    TEST_FACTORY_PANE,
    TEST_FACTORY_PICTURE,
    TEST_FACTORY_TEXT_BOX,
    TEST_FACTORY_WINDOW
};

static uint32_t sAllocator;
static uint32_t sResourceSetMarker;
static uint32_t sTextPoolMarker;
static uint32_t sNamePoolMarker;
static NtmvM2dArcResourceAccessor sAccessor;
static NtmvM2dPane sPane;
static NtmvM2dPicture sPicture;
static NtmvM2dTextBox sTextBox;
static NtmvM2dWindow sWindow;
static TestResourceBlock sBlock;
static enum TestFactoryKind sExpectedKind;
static void *sExpectedObject;
static uint32_t sExpectedSize;
static bool sFailAllocation;
static unsigned int sAllocationCount;
static unsigned int sConstructorCount;
static NtmvM2dPageBuildContext sBuildContext;

void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size)
{
    assert(allocator == &sAllocator);
    assert(size == sExpectedSize);
    ++sAllocationCount;
    return sFailAllocation ? NULL : sExpectedObject;
}

NtmvM2dPane *NtmvM2dPane_ConstructComplete(
    NtmvM2dPane *pane,
    const NtmvM2dPaneResource *resource)
{
    assert(sExpectedKind == TEST_FACTORY_PANE);
    assert(pane == &sPane);
    assert(resource == (const NtmvM2dPaneResource *)sBlock.bytes);
    ++sConstructorCount;
    return pane;
}

NtmvM2dPicture *NtmvM2dPicture_Construct(
    NtmvM2dPicture *picture,
    const NtmvM2dPictureResource *resource,
    const NtmvM2dPictureBuildContext *build_context)
{
    assert(sExpectedKind == TEST_FACTORY_PICTURE);
    assert(picture == &sPicture);
    assert(resource == (const NtmvM2dPictureResource *)sBlock.bytes);
    assert(build_context->resource_set ==
        (const NtmvM2dPictureResourceSet *)sBuildContext.resource_set);
    ++sConstructorCount;
    return picture;
}

NtmvM2dTextBox *NtmvM2dTextBox_Construct(
    NtmvM2dTextBox *text_box,
    const NtmvM2dTextBoxResource *resource,
    const NtmvM2dTextBoxBuildContext *build_context)
{
    assert(sExpectedKind == TEST_FACTORY_TEXT_BOX);
    assert(text_box == &sTextBox);
    assert(resource == (const NtmvM2dTextBoxResource *)sBlock.bytes);
    assert(build_context->text_archive ==
        (const NtmvM2dTextArchive *)sBuildContext.text_pool);
    ++sConstructorCount;
    return text_box;
}

NtmvM2dWindow *NtmvM2dWindow_Construct(
    NtmvM2dWindow *window,
    NtmvM2dAllocatorContext *allocator,
    const NtmvM2dWindowResource *resource,
    const NtmvM2dWindowBuildContext *build_context)
{
    assert(sExpectedKind == TEST_FACTORY_WINDOW);
    assert(window == &sWindow);
    assert(allocator->nns_allocator == &sAllocator);
    assert(resource == (const NtmvM2dWindowResource *)sBlock.bytes);
    assert(build_context->resource_set ==
        (const NtmvM2dWindowResourceSet *)sBuildContext.resource_set);
    ++sConstructorCount;
    return window;
}

static void Prepare(
    enum TestFactoryKind kind,
    void *object,
    uint32_t size,
    bool fail_allocation)
{
    sExpectedKind = kind;
    sExpectedObject = object;
    sExpectedSize = size;
    sFailAllocation = fail_allocation;
    sAllocationCount = 0;
    sConstructorCount = 0;
}

static void TestDispatcherSuccess(void)
{
    static const struct {
        uint32_t signature;
        enum TestFactoryKind kind;
        void *object;
        uint32_t size;
    } cases[] = {
        {0x70616e31, TEST_FACTORY_PANE, &sPane, 0x20},
        {0x70696331, TEST_FACTORY_PICTURE, &sPicture, 0x28},
        {0x74787431, TEST_FACTORY_TEXT_BOX, &sTextBox, 0x48},
        {0x776e6431, TEST_FACTORY_WINDOW, &sWindow, 0x28},
    };
    void *allocator_context = &sAllocator;
    size_t index;

    for (index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        NtmvM2dPane *result;

        Prepare(
            cases[index].kind,
            cases[index].object,
            cases[index].size,
            false);
        result = NtmvM2dPage_CreatePane_020b5318(
            NULL,
            &allocator_context,
            cases[index].signature,
            sBlock.bytes,
            &sBuildContext);
        assert((void *)result == cases[index].object);
        assert(sAllocationCount == 1);
        assert(sConstructorCount == 1);
    }

    Prepare(TEST_FACTORY_NONE, NULL, 0, false);
    assert(NtmvM2dPage_CreatePane_020b5318(
        NULL,
        &allocator_context,
        0x7a7a7a31,
        sBlock.bytes,
        &sBuildContext) == NULL);
    assert(sAllocationCount == 0);
    assert(sConstructorCount == 0);
}

static void TestEveryAllocationFailure(void)
{
    static const struct {
        uint32_t signature;
        enum TestFactoryKind kind;
        void *object;
        uint32_t size;
    } cases[] = {
        {0x70616e31, TEST_FACTORY_PANE, &sPane, 0x20},
        {0x70696331, TEST_FACTORY_PICTURE, &sPicture, 0x28},
        {0x74787431, TEST_FACTORY_TEXT_BOX, &sTextBox, 0x48},
        {0x776e6431, TEST_FACTORY_WINDOW, &sWindow, 0x28},
    };
    void *allocator_context = &sAllocator;
    size_t index;

    for (index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        Prepare(
            cases[index].kind,
            cases[index].object,
            cases[index].size,
            true);
        assert(NtmvM2dPage_CreatePane_020b5318(
            NULL,
            &allocator_context,
            cases[index].signature,
            sBlock.bytes,
            &sBuildContext) == NULL);
        assert(sAllocationCount == 1);
        assert(sConstructorCount == 0);
    }
}

static void TestDirectFactories(void)
{
    void *allocator_context = &sAllocator;

    Prepare(TEST_FACTORY_PANE, &sPane, 0x20, false);
    assert(NtmvM2dPane_AllocateComplete_020b5524(
        &allocator_context,
        (const NtmvM2dPaneResource *)sBlock.bytes,
        &sBuildContext) == &sPane);

    Prepare(TEST_FACTORY_PICTURE, &sPicture, 0x28, false);
    assert(NtmvM2dPicture_Allocate_020b54d0(
        &allocator_context,
        (const NtmvM2dPictureResource *)sBlock.bytes,
        &sBuildContext) == &sPicture);

    Prepare(TEST_FACTORY_TEXT_BOX, &sTextBox, 0x48, false);
    assert(NtmvM2dTextBox_Allocate_020b547c(
        &allocator_context,
        (const NtmvM2dTextBoxResource *)sBlock.bytes,
        &sBuildContext) == &sTextBox);

    Prepare(TEST_FACTORY_WINDOW, &sWindow, 0x28, false);
    assert(NtmvM2dWindow_Allocate_020b5424(
        &allocator_context,
        (const NtmvM2dWindowResource *)sBlock.bytes,
        &sBuildContext) == &sWindow);
}

int main(void)
{
    memset(&sBlock, 0, sizeof(sBlock));
    sBuildContext.accessor = &sAccessor;
    sBuildContext.name_pool = &sNamePoolMarker;
    sBuildContext.text_pool = &sTextPoolMarker;
    sBuildContext.resource_set = &sResourceSetMarker;

    TestDispatcherSuccess();
    TestEveryAllocationFailure();
    TestDirectFactories();
    return 0;
}
