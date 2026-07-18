#ifndef FSAE_GAME_SOUND_MANAGER_H
#define FSAE_GAME_SOUND_MANAGER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct GameSoundEntry {
    uintptr_t sdk_handle;      /* target +0x00 */
    int32_t sequence_id;       /* target +0x04 */
    uint32_t stop_pending;     /* target +0x08 */
    uint32_t keep_alive;       /* target +0x0c */
    uint32_t unknown_10;
    uint32_t unknown_14;
} GameSoundEntry;

typedef struct GameSoundManager {
    uint32_t initialized;      /* target +0x000 */
    uint32_t unknown_004;
    uint32_t sequence_arc_count;
    uint32_t sequence_count;
    uint8_t sound_archive[0xa4];
    GameSoundEntry entries[16]; /* target +0x0b4 */
    uint32_t special_handle;    /* target +0x234 */
    int32_t special_sequence_id;
    uint32_t special_pending;
    uint32_t round_robin_index; /* target +0x240 */
} GameSoundManager;

#if UINTPTR_MAX == UINT32_MAX
typedef char GameSoundEntryTargetSizeCheck[
    sizeof(GameSoundEntry) == 0x18 ? 1 : -1];
typedef char GameSoundManagerTargetSizeCheck[
    sizeof(GameSoundManager) == 0x244 ? 1 : -1];
typedef char GameSoundManagerEntriesOffsetCheck[
    offsetof(GameSoundManager, entries) == 0xb4 ? 1 : -1];
typedef char GameSoundManagerCursorOffsetCheck[
    offsetof(GameSoundManager, round_robin_index) == 0x240 ? 1 : -1];
#endif

void GameSoundManager_InitEmpty(GameSoundManager *manager); /* 0x020bec6c */
void GameSoundManager_Destroy(GameSoundManager *manager); /* 0x020bec7c */
void GameSoundManager_Init(
    GameSoundManager *manager,
    void *sound_archive_memory); /* 0x020bec80 */
void GameSoundManager_Tick(GameSoundManager *manager); /* 0x020bed14 */
GameSoundEntry *GameSoundManager_PlaySequenceArc(
    GameSoundManager *manager,
    int32_t sequence_id,
    int32_t channel_priority); /* 0x020beda0 */
GameSoundEntry *GameSoundManager_FindSequenceArc(
    GameSoundManager *manager,
    int32_t sequence_id); /* 0x020bee6c */
GameSoundEntry *GameSoundManager_AllocateEntry(
    GameSoundManager *manager,
    bool reuse_if_full); /* 0x020beed4 */

int32_t GameSound_WrapIndex(
    int32_t value,
    int32_t start,
    int32_t end); /* 0x020bef68 */
int32_t GameSound_Clamp(
    int32_t value,
    int32_t minimum,
    int32_t maximum); /* 0x020bef98 */
bool GameSound_IsInRange(
    int32_t value,
    int32_t start,
    int32_t end); /* 0x020befb4 */

void GameSoundEntry_Stop(GameSoundEntry *entry); /* 0x020bf89c */
void GameSoundEntry_Init(GameSoundEntry *entry); /* 0x020bf954 */
void GameSoundEntry_SetChannelPriority(
    GameSoundEntry *entry,
    uint32_t priority); /* 0x020bf9bc */
void GameSoundEntry_SetTempoRatio(
    GameSoundEntry *entry,
    uint32_t ratio); /* 0x020bf9fc */

/* Sound SDK edges. */
void NNS_SndInit(void);
void NNS_SndArcInitOnMemory(void *archive, void *memory);
void NNS_SndArcPlayerSetup(uint32_t player_no);
uint32_t NNS_SndArcGetSeqArcSeqCount(uint32_t sequence_arc_no);
uint32_t NNS_SndArcGetSeqCount(void);
void NNS_SndMain(void);
void NNS_SndArcPlayerStartSeqArc(
    GameSoundEntry *handle,
    uint32_t player_no,
    int32_t sequence_id);
void NNS_SndPlayerSetTrackAllocatableChannel(
    GameSoundEntry *handle,
    uint32_t track_mask,
    int32_t channel_priority);
void NNSi_SndPlayerStopSeq(uintptr_t sdk_handle);
void SND_SetPlayerChannelPriority(uint32_t player_no, uint32_t priority);
void SND_SetPlayerTempoRatio(uint32_t player_no, uint32_t ratio);

#endif
