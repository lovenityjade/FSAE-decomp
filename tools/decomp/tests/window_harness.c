#include "ntmv/m2d/window.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct BlitRecord {
    void *context;
    NtmvM2dPoint destination;
    NtmvM2dSize size;
    const NtmvM2dWindowBitmap *bitmap;
    NtmvM2dPoint source_origin;
    uint8_t transform_code;
} BlitRecord;

static const struct NtmvM2dTransformDescriptor sTransforms[6] = {
    {{0, 0}, { 1,  1}, 0},
    {{1, 0}, {-1,  1}, 0},
    {{0, 1}, { 1, -1}, 0},
    {{0, 1}, { 1, -1}, 1},
    {{1, 1}, {-1, -1}, 0},
    {{1, 0}, {-1,  1}, 1},
};

static BlitRecord sBlits[8];
static unsigned int sBlitCount;
static unsigned int sFillCount;
static NtmvM2dPoint sFillDestination;
static NtmvM2dSize sFillSize;
static uint16_t sFillValue;
static void *sExpectedAllocator;
static unsigned int sAllocationCount;
static unsigned int sFreeCount;
static uint32_t sLastAllocationSize;

void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size)
{
    void *memory;

    assert(allocator == sExpectedAllocator);
    ++sAllocationCount;
    sLastAllocationSize = size;
    memory = malloc(size);
    assert(memory != NULL);
    memset(memory, 0xa5, size);
    return memory;
}

void NNS_FndFreeToAllocator(void *allocator, void *memory)
{
    assert(allocator == sExpectedAllocator);
    assert(memory != NULL);
    ++sFreeCount;
    free(memory);
}

const struct NtmvM2dTransformDescriptor *NtmvM2d_GetTransformDescriptor_020b76bc(
    uint8_t transform_code)
{
    assert(transform_code < 6);
    return &sTransforms[transform_code];
}

void NtmvM2d_ComputeSourceOrigin_020b76d0(
    NtmvM2dPoint *origin,
    const NtmvM2dSize *bitmap_size,
    const struct NtmvM2dTransformDescriptor *transform)
{
    origin->x = (int16_t)(
        transform->origin_factor[0] * (bitmap_size->width - 1));
    origin->y = (int16_t)(
        transform->origin_factor[1] * (bitmap_size->height - 1));
}

void NtmvM2d_BlitTransformed_020b786c(
    void *context,
    const NtmvM2dPoint *destination,
    const NtmvM2dSize *draw_size,
    const NtmvM2dWindowBitmap *const *const *bitmap_ref,
    const NtmvM2dPoint *source_origin,
    const struct NtmvM2dTransformDescriptor *transform)
{
    BlitRecord *record;

    assert(sBlitCount < sizeof(sBlits) / sizeof(sBlits[0]));
    assert(transform >= sTransforms && transform < sTransforms + 6);
    record = &sBlits[sBlitCount++];
    record->context = context;
    record->destination = *destination;
    record->size = *draw_size;
    record->bitmap = **bitmap_ref;
    record->source_origin = *source_origin;
    record->transform_code = (uint8_t)(transform - sTransforms);
}

void NtmvM2d_FillRect_020b73b8(
    void *context,
    const NtmvM2dPoint *destination,
    const NtmvM2dSize *size,
    uint16_t value)
{
    assert(context == (void *)(uintptr_t)0x5678);
    ++sFillCount;
    sFillDestination = *destination;
    sFillSize = *size;
    sFillValue = value;
}

static void ResetDrawRecords(void)
{
    memset(sBlits, 0, sizeof(sBlits));
    sBlitCount = 0;
    sFillCount = 0;
    memset(&sFillDestination, 0, sizeof(sFillDestination));
    memset(&sFillSize, 0, sizeof(sFillSize));
    sFillValue = 0;
}

static void AssertBlit(
    unsigned int index,
    const NtmvM2dWindowBitmap *bitmap,
    int destination_x,
    int destination_y,
    int width,
    int height,
    int origin_x,
    int origin_y,
    uint8_t transform_code)
{
    const BlitRecord *record = &sBlits[index];

    assert(record->context == (void *)(uintptr_t)0x5678);
    assert(record->bitmap == bitmap);
    assert(record->destination.x == destination_x);
    assert(record->destination.y == destination_y);
    assert(record->size.width == width);
    assert(record->size.height == height);
    assert(record->source_origin.x == origin_x);
    assert(record->source_origin.y == origin_y);
    assert(record->transform_code == transform_code);
}

static void TestFourBorders(void)
{
    struct BitmapStorage {
        NtmvM2dWindowBitmap bitmap;
        uint8_t data[1];
    } images[4];
    const NtmvM2dWindowBitmap *bitmap_table[4];
    NtmvM2dWindowResourceSet resource_set;
    NtmvM2dWindowBuildContext build_context;
    union ResourceStorage {
        uint32_t alignment;
        uint8_t bytes[0x14 + 4 * sizeof(NtmvM2dWindowResourceEntry)];
    } storage;
    NtmvM2dWindowResource *resource =
        (NtmvM2dWindowResource *)(void *)storage.bytes;
    NtmvM2dAllocatorContext allocator;
    NtmvM2dWindow window;
    static int allocator_token;
    const uint16_t widths[4] = {10, 3, 4, 13};
    const uint16_t heights[4] = {2, 11, 12, 5};
    const uint8_t transforms[4] = {0, 1, 2, 4};
    unsigned int index;

    memset(images, 0, sizeof(images));
    for (index = 0; index < 4; ++index) {
        images[index].bitmap.width = widths[index];
        images[index].bitmap.height = heights[index];
        bitmap_table[index] = &images[index].bitmap;
    }
    resource_set.unknown_00 = 0;
    resource_set.bitmaps = bitmap_table;
    memset(&build_context, 0, sizeof(build_context));
    build_context.resource_set = &resource_set;
    memset(&storage, 0, sizeof(storage));
    resource->pane.position.x = 1;
    resource->pane.position.y = 2;
    resource->pane.size.width = 40;
    resource->pane.size.height = 30;
    resource->fill_value = 0x8123;
    resource->border_count = 4;
    for (index = 0; index < 4; ++index) {
        resource->borders[index].bitmap_index = (uint16_t)index;
        resource->borders[index].transform_code = transforms[index];
    }
    allocator.nns_allocator = &allocator_token;
    sExpectedAllocator = &allocator_token;

    assert(NtmvM2dWindow_Construct(
               &window, &allocator, resource, &build_context) == &window);
    assert(window.pane.vtable ==
           (const NtmvM2dPaneVTable *)&gNtmvM2dWindowVTable);
    assert(gNtmvM2dWindowVTable.update_position == NtmvM2dPane_UpdatePosition);
    assert(window.border_count == 4);
    assert(sLastAllocationSize == 4 * sizeof(NtmvM2dWindowBorder));
    assert(window.pane.reserved_1d[1] == 0x23);
    assert(window.pane.reserved_1d[2] == 0x81);
    for (index = 0; index < 4; ++index) {
        assert(window.borders[index].bitmap_ref == &bitmap_table[index]);
        assert(window.borders[index].transform_code == transforms[index]);
        assert(window.borders[index].reserved_05[0] == 0);
        assert(window.borders[index].reserved_05[1] == 0);
        assert(window.borders[index].reserved_05[2] == 0);
    }
    assert(strcmp(
               (const char *)window.pane.vtable->get_runtime_type(&window.pane),
               "N4ntmv3m2d6WindowE") == 0);

    window.pane.world_position.x = 7;
    window.pane.world_position.y = 9;
    ResetDrawRecords();
    window.pane.vtable->draw(&window.pane, (void *)(uintptr_t)0x5678);

    assert(sFillCount == 1);
    assert(sFillDestination.x == 11 && sFillDestination.y == 11);
    assert(sFillSize.width == 33 && sFillSize.height == 23);
    assert(sFillValue == 0x8123);
    assert(sBlitCount == 4);
    AssertBlit(0, &images[0].bitmap, 7, 9, 37, 2, 0, 0, 0);
    AssertBlit(1, &images[1].bitmap, 44, 9, 3, 25, 2, 0, 1);
    AssertBlit(2, &images[3].bitmap, 11, 34, 36, 5, 35, 4, 4);
    AssertBlit(3, &images[2].bitmap, 7, 11, 4, 28, 0, 27, 2);

    window.pane.vtable->dispose(&window.pane, &allocator);
    assert(sFreeCount == 1);
}

static void TestSingleBorder(void)
{
    struct BitmapStorage {
        NtmvM2dWindowBitmap bitmap;
        uint8_t data[1];
    } image;
    const NtmvM2dWindowBitmap *bitmap_table[1];
    NtmvM2dWindowResourceSet resource_set;
    NtmvM2dWindowBuildContext build_context;
    union ResourceStorage {
        uint32_t alignment;
        uint8_t bytes[0x14 + sizeof(NtmvM2dWindowResourceEntry)];
    } storage;
    NtmvM2dWindowResource *resource =
        (NtmvM2dWindowResource *)(void *)storage.bytes;
    NtmvM2dAllocatorContext allocator;
    NtmvM2dWindow window;
    static int allocator_token;

    memset(&image, 0, sizeof(image));
    image.bitmap.width = 3;
    image.bitmap.height = 4;
    bitmap_table[0] = &image.bitmap;
    resource_set.unknown_00 = 0;
    resource_set.bitmaps = bitmap_table;
    memset(&build_context, 0, sizeof(build_context));
    build_context.resource_set = &resource_set;
    memset(&storage, 0, sizeof(storage));
    resource->pane.size.width = 10;
    resource->pane.size.height = 10;
    resource->fill_value = 0x0123;
    resource->border_count = 1;
    resource->borders[0].bitmap_index = 0;
    resource->borders[0].transform_code = 5;
    allocator.nns_allocator = &allocator_token;
    sExpectedAllocator = &allocator_token;

    NtmvM2dWindow_Construct(&window, &allocator, resource, &build_context);
    window.pane.world_position.x = 1;
    window.pane.world_position.y = 2;
    ResetDrawRecords();
    window.pane.vtable->draw(&window.pane, (void *)(uintptr_t)0x5678);

    assert(sFillCount == 0);
    assert(sBlitCount == 4);
    /* Slot 5 uses fixed transforms 0,1,4,2; stored code 5 is ignored. */
    AssertBlit(0, &image.bitmap, 1, 2, 7, 4, 0, 0, 0);
    AssertBlit(1, &image.bitmap, 8, 2, 3, 6, 2, 0, 1);
    AssertBlit(2, &image.bitmap, 4, 8, 7, 4, 6, 3, 4);
    AssertBlit(3, &image.bitmap, 1, 6, 3, 6, 0, 5, 2);

    window.pane.vtable->dispose(&window.pane, &allocator);
    assert(sFreeCount == 2);
}

int main(void)
{
    TestFourBorders();
    TestSingleBorder();
    assert(sAllocationCount == 2);
    return 0;
}
