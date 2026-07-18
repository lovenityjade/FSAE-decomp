#include "game/sound_manager.h"

enum {
    GAME_SOUND_ENTRY_COUNT = 16,
    GAME_SOUND_DEFAULT_CHANNEL_PRIORITY = 0x80
};

/* 0x020bec6c */
void GameSoundManager_InitEmpty(GameSoundManager *manager)
{
    manager->initialized = 0;
    manager->unknown_004 = 0;
}

/* 0x020bec7c */
void GameSoundManager_Destroy(GameSoundManager *manager)
{
    (void)manager;
}

/* 0x020bf954 */
void GameSoundEntry_Init(GameSoundEntry *entry)
{
    entry->sdk_handle = 0;
}

/* 0x020bf89c */
void GameSoundEntry_Stop(GameSoundEntry *entry)
{
    NNSi_SndPlayerStopSeq(entry->sdk_handle);
}

typedef struct GameSoundSdkPlayer {
    uint8_t unknown_00[0x3c];
    uint8_t player_no;
} GameSoundSdkPlayer;

/* 0x020bf9bc */
void GameSoundEntry_SetChannelPriority(
    GameSoundEntry *entry,
    uint32_t priority)
{
    if (entry->sdk_handle != 0) {
        const GameSoundSdkPlayer *player =
            (const GameSoundSdkPlayer *)entry->sdk_handle;
        SND_SetPlayerChannelPriority(player->player_no, priority);
    }
}

/* 0x020bf9fc */
void GameSoundEntry_SetTempoRatio(
    GameSoundEntry *entry,
    uint32_t ratio)
{
    if (entry->sdk_handle != 0) {
        const GameSoundSdkPlayer *player =
            (const GameSoundSdkPlayer *)entry->sdk_handle;
        SND_SetPlayerTempoRatio(player->player_no, ratio);
    }
}

/* 0x020bec80 */
void GameSoundManager_Init(
    GameSoundManager *manager,
    void *sound_archive_memory)
{
    uint32_t index;

    NNS_SndInit();
    NNS_SndArcInitOnMemory(manager->sound_archive, sound_archive_memory);
    NNS_SndArcPlayerSetup(0);
    for (index = 0; index < GAME_SOUND_ENTRY_COUNT; ++index) {
        GameSoundEntry *entry = &manager->entries[index];
        GameSoundEntry_Init(entry);
        entry->sequence_id = -1;
        entry->stop_pending = 0;
        entry->keep_alive = 0;
    }
    manager->special_handle = 0;
    manager->special_sequence_id = -1;
    manager->special_pending = 0;
    manager->round_robin_index = 0;
    manager->sequence_arc_count = NNS_SndArcGetSeqArcSeqCount(0);
    manager->sequence_count = NNS_SndArcGetSeqCount();
    manager->initialized = 1;
}

/* 0x020bed14 */
void GameSoundManager_Tick(GameSoundManager *manager)
{
    uint32_t index;

    if (manager->initialized == 0) {
        return;
    }
    for (index = 0; index < GAME_SOUND_ENTRY_COUNT; ++index) {
        GameSoundEntry *entry = &manager->entries[index];
        if (entry->sdk_handle != 0) {
            if (entry->keep_alive == 0) {
                if (entry->stop_pending != 0) {
                    GameSoundEntry_Stop(entry);
                    entry->sequence_id = -1;
                    entry->stop_pending = 0;
                }
            } else {
                entry->keep_alive = 0;
            }
        }
    }
    NNS_SndMain();
}

/* 0x020befb4 */
bool GameSound_IsInRange(int32_t value, int32_t start, int32_t end)
{
    return value >= start && value < end;
}

/* 0x020bef98 */
int32_t GameSound_Clamp(int32_t value, int32_t minimum, int32_t maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

/* 0x020bef68 */
int32_t GameSound_WrapIndex(int32_t value, int32_t start, int32_t end)
{
    int32_t width = end - start;
    if (width < 0) {
        int32_t temporary = start;
        start = end;
        end = temporary;
        width = end - start;
    }
    if (value < start) {
        value += width;
    } else if (value >= end) {
        value -= width;
    }
    return value;
}

/* 0x020bee6c */
GameSoundEntry *GameSoundManager_FindSequenceArc(
    GameSoundManager *manager,
    int32_t sequence_id)
{
    uint32_t index;

    if (!GameSound_IsInRange(
            sequence_id, 0, (int32_t)manager->sequence_arc_count)) {
        return 0;
    }
    for (index = 0; index < GAME_SOUND_ENTRY_COUNT; ++index) {
        GameSoundEntry *entry = &manager->entries[index];
        if (entry->sequence_id == sequence_id && entry->sdk_handle != 0) {
            return entry;
        }
    }
    return 0;
}

/* 0x020beed4 */
GameSoundEntry *GameSoundManager_AllocateEntry(
    GameSoundManager *manager,
    bool reuse_if_full)
{
    uint32_t offset;

    manager->round_robin_index = (uint32_t)GameSound_WrapIndex(
        (int32_t)manager->round_robin_index + 1, 0,
        GAME_SOUND_ENTRY_COUNT);
    for (offset = 0; offset < GAME_SOUND_ENTRY_COUNT; ++offset) {
        int32_t index = GameSound_WrapIndex(
            (int32_t)manager->round_robin_index + (int32_t)offset,
            0, GAME_SOUND_ENTRY_COUNT);
        GameSoundEntry *entry = &manager->entries[index];
        if (entry->sdk_handle == 0) {
            manager->round_robin_index = (uint32_t)index;
            return entry;
        }
    }
    if (reuse_if_full) {
        return &manager->entries[manager->round_robin_index];
    }
    return 0;
}

/* 0x020beda0 */
GameSoundEntry *GameSoundManager_PlaySequenceArc(
    GameSoundManager *manager,
    int32_t sequence_id,
    int32_t channel_priority)
{
    GameSoundEntry *entry;

    if (manager->initialized == 0 ||
        !GameSound_IsInRange(
            sequence_id, 0, (int32_t)manager->sequence_arc_count)) {
        return 0;
    }

    entry = GameSoundManager_FindSequenceArc(manager, sequence_id);
    if (entry == 0) {
        entry = GameSoundManager_AllocateEntry(manager, true);
    }
    if (entry == 0) {
        return 0;
    }

    GameSoundEntry_Stop(entry);
    NNS_SndArcPlayerStartSeqArc(entry, 0, sequence_id);
    if (channel_priority != GAME_SOUND_DEFAULT_CHANNEL_PRIORITY) {
        int32_t clamped = GameSound_Clamp(channel_priority, 0, 0xff);
        NNS_SndPlayerSetTrackAllocatableChannel(
            entry, 0xffff, clamped - GAME_SOUND_DEFAULT_CHANNEL_PRIORITY);
    }
    entry->sequence_id = sequence_id;
    entry->stop_pending = 0;
    return entry;
}
