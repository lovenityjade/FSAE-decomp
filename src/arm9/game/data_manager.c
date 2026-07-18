#include "game/data_manager.h"

#include "game/assert.h"

#include <stddef.h>
#include <stdint.h>

/*
 * The range 0x0208BCD4..0x0208BCFC contains a base-vtable initializer and three C++
 * destruction thunks (two are empty; one invokes the game's deallocator).
 * Their mechanics are proved by the table at 0x0211AAD8, but the exact class
 * identity is not.  They deliberately remain unnamed instead of being folded
 * into DataManagerArchive on insufficient type evidence.
 */

typedef struct NNSSndArc NNSSndArc;

/* TwlSystem 2.2.0 APIs identified from libnnssnd.TWL.LTD.a. */
extern NNSSndArc *NNS_SndArcGetCurrent(void);
extern NNSSndArc *NNS_SndArcSetCurrent(NNSSndArc *archive);
extern int NNS_SndArcPlayerSetup(NNSSndHeapHandle heap);
extern int NNS_SndArcPlayerStartSeq(NNSSndHandle *handle, int sequence_number);
extern int NNS_SndArcPlayerStartSeqArcEx(
    NNSSndHandle *handle,
    int player_number,
    int bank_number,
    int player_priority,
    int sequence_archive_number,
    int archive_index
);
extern int NNS_SndArcLoadGroup(int group_number, NNSSndHeapHandle heap);
extern int NNS_SndArcLoadSeq(int sequence_number, NNSSndHeapHandle heap);

enum {
    DATA_MANAGER_ARCHIVE_OFFSET = 4,
    DATA_MANAGER_SEQUENCE_ID_OFFSET = 4,
    DATA_MANAGER_SEQUENCE_NNS_HANDLE_OFFSET = 0x18,
    DATA_MANAGER_SEQUENCE_POOL_ADDRESS = 0x02171f74
};

extern DataManagerSequenceHandle *DataManagerSequencePool_Allocate_0208c250(
    void *pool);
extern void DataManagerSequencePool_Release_0208c2a4(
    void *pool,
    DataManagerSequenceHandle *handle);

static void DataManagerSequenceHandle_SetId(
    DataManagerSequenceHandle *handle,
    int sequence_id
)
{
    unsigned char *bytes = (unsigned char *)handle;

    *(int *)(void *)(bytes + DATA_MANAGER_SEQUENCE_ID_OFFSET) = sequence_id;
}

/* 0x0208C4D4 */
NNSSndHandle *DataManagerSequenceHandle_GetNnsHandle(
    DataManagerSequenceHandle *handle)
{
    unsigned char *bytes = (unsigned char *)handle;

    return (NNSSndHandle *)(void *)(
        bytes + DATA_MANAGER_SEQUENCE_NNS_HANDLE_OFFSET);
}

/* 0x0208C63C; literal pool: 0x0208C648..0x0208C64F. */
DataManagerSequenceHandle *DataManager_AllocateSequenceHandle(void)
{
    return DataManagerSequencePool_Allocate_0208c250(
        (void *)(uintptr_t)DATA_MANAGER_SEQUENCE_POOL_ADDRESS);
}

/* 0x0208C650; literal pool: 0x0208C660..0x0208C667. */
void DataManager_ReleaseSequenceHandle(DataManagerSequenceHandle *handle)
{
    DataManagerSequencePool_Release_0208c2a4(
        (void *)(uintptr_t)DATA_MANAGER_SEQUENCE_POOL_ADDRESS,
        handle);
}

/* 0x0208BD00 */
void DataManager_SelectArchive(DataManagerArchive *archive)
{
    unsigned char *bytes = (unsigned char *)archive;
    NNSSndArc *embedded_archive =
        (NNSSndArc *)(void *)(bytes + DATA_MANAGER_ARCHIVE_OFFSET);

    if (NNS_SndArcGetCurrent() != embedded_archive) {
        /*
         * The short SDK body at 0x020C05C4 also collides with a CRYPTO symbol
         * signature.  GetCurrent plus the archive+4 argument proves the NNS
         * interpretation at this call site, without claiming a global match.
         */
        (void)NNS_SndArcSetCurrent(embedded_archive);
    }
}

/* 0x0208BD24 */
int DataManager_SetupSequencePlayer(
    const DataManager *manager,
    const NNSSndHeapHandle *heap_slot
)
{
    (void)manager;
    return NNS_SndArcPlayerSetup(*heap_slot);
}

/* 0x0208BD34 */
DataManagerSequenceHandle *DataManager_StartSequenceArchiveEx(
    const DataManager *manager,
    int player_number,
    int bank_number,
    int player_priority,
    int sequence_archive_number,
    int archive_index
)
{
    DataManagerSequenceHandle *handle;
    NNSSndHandle *nns_handle;

    (void)manager;
    handle = DataManager_AllocateSequenceHandle();
    if (handle == NULL) {
        return NULL;
    }

    nns_handle = DataManagerSequenceHandle_GetNnsHandle(handle);
    if (!NNS_SndArcPlayerStartSeqArcEx(
            nns_handle,
            player_number,
            bank_number,
            player_priority,
            sequence_archive_number,
            archive_index)) {
        DataManager_ReleaseSequenceHandle(handle);
        return NULL;
    }

    DataManagerSequenceHandle_SetId(handle, archive_index);
    return handle;
}

/* 0x0208BDA8 */
DataManagerSequenceHandle *DataManager_StartSequence(
    const DataManager *manager,
    int unused_argument,
    int sequence_number
)
{
    DataManagerSequenceHandle *handle;
    NNSSndHandle *nns_handle;

    (void)manager;
    (void)unused_argument;
    handle = DataManager_AllocateSequenceHandle();
    if (handle == NULL) {
        return NULL;
    }

    nns_handle = DataManagerSequenceHandle_GetNnsHandle(handle);
    if (!NNS_SndArcPlayerStartSeq(nns_handle, sequence_number)) {
        DataManager_ReleaseSequenceHandle(handle);
        return NULL;
    }

    DataManagerSequenceHandle_SetId(handle, sequence_number);
    return handle;
}

/* 0x0208BDEC */
void DataManagerArchiveSlot_Init(DataManagerArchive **archive_slot)
{
    *archive_slot = NULL;
}

/* 0x0208BDF8 */
int DataManagerArchiveSlot_Set(
    DataManagerArchive **archive_slot,
    DataManagerArchive *archive
)
{
    *archive_slot = archive;
    return 1;
}

/* 0x0208BE04 */
void DataManager_LoadGroup(
    DataManagerArchive *const *archive_slot,
    int group_number,
    const NNSSndHeapHandle *heap_slot
)
{
    DataManagerArchive *archive = *archive_slot;

    if (archive == NULL) {
        Game_AssertFail("archive", "DataManager.cpp", "loadGroup", 43);
    }

    DataManager_SelectArchive(archive);
    (void)NNS_SndArcLoadGroup(group_number, *heap_slot);
}

/* 0x0208BE58 */
void DataManager_LoadSequence(
    DataManagerArchive *const *archive_slot,
    int sequence_number,
    const NNSSndHeapHandle *heap_slot
)
{
    DataManagerArchive *archive = *archive_slot;

    if (archive == NULL) {
        Game_AssertFail("archive", "DataManager.cpp", "loadSequence", 50);
    }

    DataManager_SelectArchive(archive);

    /*
     * Several NNS loader wrappers have identical instruction bodies.  The
     * recovered source label "loadSequence" is the evidence selecting LoadSeq
     * here instead of LoadBank/LoadSeqArc/LoadWaveArc.
     */
    (void)NNS_SndArcLoadSeq(sequence_number, *heap_slot);
}
