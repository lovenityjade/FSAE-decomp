#include "game/blz_file_loader.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct GameFsFileHandle {
    uint8_t storage[0x48];
} GameFsFileHandle;

enum FailureStage {
    FAIL_NONE,
    FAIL_OPEN,
    FAIL_SEEK_END,
    FAIL_FOOTER_READ,
    FAIL_SEEK_SET,
    FAIL_ALLOCATE,
    FAIL_DATA_READ,
    FAIL_DECOMPRESS
};

static enum FailureStage sFailure;
static GameFsFileHandle *sFile;
static char sEvents[24];
static unsigned int sEventCount;
static unsigned int sReadCount;
static uint32_t sRequestedSize;
static void *sAllocationPointer;
static void *sAlignedData;
static _Alignas(32) uint8_t sAllocationStorage[256];
static uint32_t sAllocator;

static void Event(char event)
{
    assert(sEventCount + 1 < sizeof(sEvents));
    sEvents[sEventCount++] = event;
    sEvents[sEventCount] = '\0';
}

void FS_InitFile(GameFsFileHandle *file)
{
    sFile = file;
    Event('I');
}

bool FS_OpenFile(GameFsFileHandle *file, const char *path)
{
    assert(file == sFile);
    assert(strcmp(path, "manual.blz") == 0);
    Event('O');
    return sFailure != FAIL_OPEN;
}

int32_t FS_GetFileLength(GameFsFileHandle *file)
{
    assert(file == sFile);
    Event('L');
    return 100;
}

bool FS_SeekFile(GameFsFileHandle *file, int32_t offset, int32_t origin)
{
    assert(file == sFile);
    if (origin == 2) {
        assert(offset == -4);
        Event('E');
        return sFailure != FAIL_SEEK_END;
    }
    assert(origin == 0);
    assert(offset == 0);
    Event('S');
    return sFailure != FAIL_SEEK_SET;
}

int32_t FS_ReadFile(
    GameFsFileHandle *file,
    void *destination,
    int32_t size)
{
    assert(file == sFile);
    if (sReadCount++ == 0) {
        assert(size == 4);
        Event('F');
        *(uint32_t *)destination = 64;
        return sFailure == FAIL_FOOTER_READ ? 3 : 4;
    }
    assert(size == 100);
    assert(destination == sAlignedData);
    Event('R');
    return sFailure == FAIL_DATA_READ ? 99 : 100;
}

void FS_CloseFile(GameFsFileHandle *file)
{
    assert(file == sFile);
    Event('C');
}

void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size)
{
    assert(allocator == &sAllocator);
    assert(size == sRequestedSize);
    Event('A');
    return sFailure == FAIL_ALLOCATE ? 0 : sAllocationPointer;
}

void NNS_FndFreeToAllocator(void *allocator, void *allocation)
{
    assert(allocator == &sAllocator);
    assert(allocation == sAllocationPointer);
    Event('X');
}

int32_t MI_SecureUncompressBLZ(
    void *compressed_data,
    uint32_t compressed_size,
    uint32_t uncompressed_size)
{
    assert(compressed_data == sAlignedData);
    assert(compressed_size == 100);
    assert(uncompressed_size == 164);
    Event('D');
    return sFailure == FAIL_DECOMPRESS ? -1 : 0;
}

static void Reset(enum FailureStage failure, uint32_t alignment)
{
    uintptr_t mask = (uintptr_t)alignment - 1;

    sFailure = failure;
    sFile = 0;
    sEventCount = 0;
    sEvents[0] = '\0';
    sReadCount = 0;
    sAllocationPointer = alignment == 4 ?
        (void *)sAllocationStorage : (void *)(sAllocationStorage + 3);
    sAlignedData = (void *)(((uintptr_t)sAllocationPointer + mask) & ~mask);
    sRequestedSize = 164 + (alignment == 4 ? 0 : alignment);
}

static void TestFailure(
    enum FailureStage failure,
    const char *expected_events)
{
    uint32_t output_size = 0xdeadbeef;

    Reset(failure, 0x20);
    assert(Game_LoadBlzFileToAllocator_020b7b1c(
        "manual.blz", &sAllocator, 0x20, &output_size) == 0);
    assert(strcmp(sEvents, expected_events) == 0);
    assert(output_size == 0xdeadbeef);
}

int main(void)
{
    uint32_t output_size;

    TestFailure(FAIL_OPEN, "IO");
    TestFailure(FAIL_SEEK_END, "IOLEC");
    TestFailure(FAIL_FOOTER_READ, "IOLEFC");
    TestFailure(FAIL_SEEK_SET, "IOLEFSC");
    TestFailure(FAIL_ALLOCATE, "IOLEFSAC");
    TestFailure(FAIL_DATA_READ, "IOLEFSARXC");
    TestFailure(FAIL_DECOMPRESS, "IOLEFSARDXC");

    Reset(FAIL_NONE, 4);
    output_size = 0;
    assert(Game_LoadBlzFileToAllocator_020b7b1c(
        "manual.blz", &sAllocator, 4, &output_size) == sAllocationPointer);
    assert(strcmp(sEvents, "IOLEFSARDC") == 0);
    assert(sAlignedData == sAllocationPointer);
    assert(output_size == 164);

    Reset(FAIL_NONE, 0x20);
    output_size = 0;
    assert(Game_LoadBlzFileToAllocator_020b7b1c(
        "manual.blz", &sAllocator, 0x20, &output_size) ==
        sAllocationPointer);
    assert(strcmp(sEvents, "IOLEFSARDC") == 0);
    assert(sAlignedData == sAllocationStorage + 0x20);
    assert(output_size == 164);
    return 0;
}
