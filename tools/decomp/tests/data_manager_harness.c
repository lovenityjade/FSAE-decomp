#include "game/data_manager.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

struct DataManagerSequenceHandle {
    unsigned char storage[0x40];
};

struct DataManagerArchive {
    unsigned char storage[0x20];
};

struct NNSSndArc {
    int unused;
};

struct NNSSndHeap {
    int unused;
};

static struct DataManagerSequenceHandle sHandle;
static unsigned int sAllocateCount;
static unsigned int sReleaseCount;
static unsigned int sStartSeqCount;
static int sStartSeqResult = 1;

static const uintptr_t kExpectedSequencePool = 0x02171f74u;

DataManagerSequenceHandle *DataManagerSequencePool_Allocate_0208c250(
    void *pool)
{
    assert((uintptr_t)pool == kExpectedSequencePool);
    ++sAllocateCount;
    memset(&sHandle, 0, sizeof(sHandle));
    return &sHandle;
}

void DataManagerSequencePool_Release_0208c2a4(
    void *pool,
    DataManagerSequenceHandle *handle)
{
    assert((uintptr_t)pool == kExpectedSequencePool);
    assert(handle == &sHandle);
    ++sReleaseCount;
}

void Game_AssertFail(
    const char *condition,
    const char *file,
    const char *function,
    int line)
{
    (void)condition;
    (void)file;
    (void)function;
    (void)line;
    assert(0);
}

struct NNSSndArc *NNS_SndArcGetCurrent(void)
{
    return NULL;
}

struct NNSSndArc *NNS_SndArcSetCurrent(struct NNSSndArc *archive)
{
    return archive;
}

int NNS_SndArcPlayerSetup(NNSSndHeapHandle heap)
{
    (void)heap;
    return 1;
}

int NNS_SndArcPlayerStartSeq(NNSSndHandle *handle, int sequence_number)
{
    assert(handle == DataManagerSequenceHandle_GetNnsHandle(&sHandle));
    assert(sequence_number == 77);
    ++sStartSeqCount;
    return sStartSeqResult;
}

int NNS_SndArcPlayerStartSeqArcEx(
    NNSSndHandle *handle,
    int player_number,
    int bank_number,
    int player_priority,
    int sequence_archive_number,
    int archive_index)
{
    assert(handle == DataManagerSequenceHandle_GetNnsHandle(&sHandle));
    assert(player_number == 1);
    assert(bank_number == 2);
    assert(player_priority == 3);
    assert(sequence_archive_number == 4);
    assert(archive_index == 5);
    return 1;
}

int NNS_SndArcLoadGroup(int group_number, NNSSndHeapHandle heap)
{
    (void)group_number;
    (void)heap;
    return 1;
}

int NNS_SndArcLoadSeq(int sequence_number, NNSSndHeapHandle heap)
{
    (void)sequence_number;
    (void)heap;
    return 1;
}

static int SequenceId(const DataManagerSequenceHandle *handle)
{
    int value;

    memcpy(&value, ((const unsigned char *)handle) + 4, sizeof(value));
    return value;
}

int main(void)
{
    DataManagerSequenceHandle *allocated;

    allocated = DataManager_AllocateSequenceHandle();
    assert(allocated == &sHandle);
    assert(sAllocateCount == 1);
    assert(
        DataManagerSequenceHandle_GetNnsHandle(allocated) ==
        (NNSSndHandle *)(void *)(sHandle.storage + 0x18));

    DataManager_ReleaseSequenceHandle(allocated);
    assert(sReleaseCount == 1);

    sStartSeqResult = 1;
    allocated = DataManager_StartSequence(NULL, 123, 77);
    assert(allocated == &sHandle);
    assert(sAllocateCount == 2);
    assert(sStartSeqCount == 1);
    assert(sReleaseCount == 1);
    assert(SequenceId(allocated) == 77);

    allocated = DataManager_StartSequenceArchiveEx(NULL, 1, 2, 3, 4, 5);
    assert(allocated == &sHandle);
    assert(sAllocateCount == 3);
    assert(SequenceId(allocated) == 5);

    sStartSeqResult = 0;
    allocated = DataManager_StartSequence(NULL, 123, 77);
    assert(allocated == NULL);
    assert(sAllocateCount == 4);
    assert(sReleaseCount == 2);

    return 0;
}
