#ifndef FSAE_GAME_DATA_MANAGER_H
#define FSAE_GAME_DATA_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The concrete archive owner is not reconstructed yet.  Code at 0x0208BD00
 * treats it as an object containing an NNSSndArc at offset 4.
 */
typedef struct DataManagerArchive DataManagerArchive;

/*
 * The first argument of the sequence-start helpers behaves like a C++ this
 * pointer, although these two implementations do not read it.
 */
typedef struct DataManager DataManager;

/*
 * Game-owned sequence object.  Its NNSSndHandle is embedded at offset 0x18;
 * its complete layout and final source name are still unknown.
 */
typedef struct DataManagerSequenceHandle DataManagerSequenceHandle;
typedef struct NNSSndHandle NNSSndHandle;

/* TwlSystem defines this as an opaque pointer to struct NNSSndHeap. */
typedef struct NNSSndHeap *NNSSndHeapHandle;

/* 0x0208C4D4: returns the embedded NNS sound handle at sequence handle +0x18. */
NNSSndHandle *DataManagerSequenceHandle_GetNnsHandle(
    DataManagerSequenceHandle *handle
);

/* 0x0208C63C: allocate one game-owned sequence handle from the global pool. */
DataManagerSequenceHandle *DataManager_AllocateSequenceHandle(void);

/* 0x0208C650: return one game-owned sequence handle to the global pool. */
void DataManager_ReleaseSequenceHandle(DataManagerSequenceHandle *handle);

/*
 * 0x0208BD00
 *
 * Selects the NNSSndArc embedded at archive + 4 as the current NNS archive.
 */
void DataManager_SelectArchive(DataManagerArchive *archive);

/*
 * 0x0208BD24
 *
 * Tail-calls NNS_SndArcPlayerSetup with *heap_slot.  manager is preserved in
 * the signature because r0 is a caller-supplied object pointer, but the
 * recovered body does not read it.
 */
int DataManager_SetupSequencePlayer(
    const DataManager *manager,
    const NNSSndHeapHandle *heap_slot
);

/*
 * 0x0208BD34
 *
 * Allocates a game-owned handle and starts an archive sequence with the full
 * NNS_SndArcPlayerStartSeqArcEx parameter set.  Returns NULL on either pool
 * exhaustion or an NNS start failure.
 */
DataManagerSequenceHandle *DataManager_StartSequenceArchiveEx(
    const DataManager *manager,
    int player_number,
    int bank_number,
    int player_priority,
    int sequence_archive_number,
    int archive_index
);

/*
 * 0x0208BDA8
 *
 * The r1 argument is present at known call sites but is not consumed by the
 * recovered instructions.  Its source-level meaning remains unknown.
 */
DataManagerSequenceHandle *DataManager_StartSequence(
    const DataManager *manager,
    int unused_argument,
    int sequence_number
);

/* 0x0208BDEC: initialize an archive pointer slot to NULL. */
void DataManagerArchiveSlot_Init(DataManagerArchive **archive_slot);

/* 0x0208BDF8: assign an archive pointer slot and return true (1). */
int DataManagerArchiveSlot_Set(
    DataManagerArchive **archive_slot,
    DataManagerArchive *archive
);

/*
 * 0x0208BE04 (source assertion label: DataManager.cpp:43, loadGroup)
 *
 * Both arguments named *_slot are indirect in the original instructions:
 * the function reads one pointer-sized value from each before use.
 */
void DataManager_LoadGroup(
    DataManagerArchive *const *archive_slot,
    int group_number,
    const NNSSndHeapHandle *heap_slot
);

/*
 * 0x0208BE58 (source assertion label: DataManager.cpp:50, loadSequence)
 */
void DataManager_LoadSequence(
    DataManagerArchive *const *archive_slot,
    int sequence_number,
    const NNSSndHeapHandle *heap_slot
);

#ifdef __cplusplus
}
#endif

#endif /* FSAE_GAME_DATA_MANAGER_H */
