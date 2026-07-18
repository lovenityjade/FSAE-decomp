#include "game/sound_manager.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static unsigned int sSoundInitCount;
static unsigned int sArchiveInitCount;
static unsigned int sSetupCount;
static unsigned int sMainCount;
static unsigned int sStartCount;
static unsigned int sStopCount;
static unsigned int sPriorityCount;
static int32_t sLastPriority;
static uint32_t sLastPlayerNo;
static uint32_t sLastSdkPriority;
static uint32_t sLastTempoRatio;

void NNS_SndInit(void)
{
    ++sSoundInitCount;
}

void NNS_SndArcInitOnMemory(void *archive, void *memory)
{
    assert(archive != 0);
    assert(memory == (void *)(uintptr_t)0x1234u);
    ++sArchiveInitCount;
}

void NNS_SndArcPlayerSetup(uint32_t player_no)
{
    assert(player_no == 0);
    ++sSetupCount;
}

uint32_t NNS_SndArcGetSeqArcSeqCount(uint32_t sequence_arc_no)
{
    assert(sequence_arc_no == 0);
    return 10;
}

uint32_t NNS_SndArcGetSeqCount(void)
{
    return 20;
}

void NNS_SndMain(void)
{
    ++sMainCount;
}

void NNS_SndArcPlayerStartSeqArc(
    GameSoundEntry *handle,
    uint32_t player_no,
    int32_t sequence_id)
{
    assert(player_no == 0);
    handle->sdk_handle = (uint32_t)(0x100 + sequence_id);
    ++sStartCount;
}

void NNS_SndPlayerSetTrackAllocatableChannel(
    GameSoundEntry *handle,
    uint32_t track_mask,
    int32_t channel_priority)
{
    assert(handle->sdk_handle != 0);
    assert(track_mask == 0xffffu);
    sLastPriority = channel_priority;
    ++sPriorityCount;
}

void NNSi_SndPlayerStopSeq(uintptr_t sdk_handle)
{
    (void)sdk_handle;
    ++sStopCount;
}

void SND_SetPlayerChannelPriority(uint32_t player_no, uint32_t priority)
{
    sLastPlayerNo = player_no;
    sLastSdkPriority = priority;
}

void SND_SetPlayerTempoRatio(uint32_t player_no, uint32_t ratio)
{
    sLastPlayerNo = player_no;
    sLastTempoRatio = ratio;
}

int main(void)
{
    struct TestSdkPlayer {
        uint8_t unknown_00[0x3c];
        uint8_t player_no;
    } sdk_player;
    GameSoundManager manager;
    GameSoundEntry *entry;
    unsigned int index;

    memset(&manager, 0xcc, sizeof(manager));
    GameSoundManager_InitEmpty(&manager);
    assert(manager.initialized == 0);
    assert(manager.unknown_004 == 0);
    GameSoundManager_Destroy(&manager);

    GameSoundManager_Init(&manager, (void *)(uintptr_t)0x1234u);
    assert(sSoundInitCount == 1);
    assert(sArchiveInitCount == 1);
    assert(sSetupCount == 1);
    assert(manager.initialized == 1);
    assert(manager.sequence_arc_count == 10);
    assert(manager.sequence_count == 20);
    for (index = 0; index < 16; ++index) {
        assert(manager.entries[index].sdk_handle == 0);
        assert(manager.entries[index].sequence_id == -1);
        assert(manager.entries[index].stop_pending == 0);
        assert(manager.entries[index].keep_alive == 0);
    }

    assert(GameSound_WrapIndex(16, 0, 16) == 0);
    assert(GameSound_WrapIndex(-1, 0, 16) == 15);
    assert(GameSound_WrapIndex(16, 16, 0) == 0);
    assert(GameSound_Clamp(-2, 0, 255) == 0);
    assert(GameSound_Clamp(300, 0, 255) == 255);
    assert(GameSound_IsInRange(9, 0, 10));
    assert(!GameSound_IsInRange(10, 0, 10));

    assert(GameSoundManager_PlaySequenceArc(&manager, -1, 0x80) == 0);
    assert(GameSoundManager_PlaySequenceArc(&manager, 10, 0x80) == 0);
    entry = GameSoundManager_PlaySequenceArc(&manager, 4, 0x80);
    assert(entry != 0);
    assert(entry->sequence_id == 4);
    assert(entry->sdk_handle == 0x104u);
    assert(sStartCount == 1);
    assert(sPriorityCount == 0);
    assert(GameSoundManager_FindSequenceArc(&manager, 4) == entry);

    assert(GameSoundManager_PlaySequenceArc(&manager, 4, 200) == entry);
    assert(sStartCount == 2);
    assert(sPriorityCount == 1);
    assert(sLastPriority == 72);

    entry->stop_pending = 1;
    entry->keep_alive = 1;
    GameSoundManager_Tick(&manager);
    assert(entry->keep_alive == 0);
    assert(entry->stop_pending == 1);
    GameSoundManager_Tick(&manager);
    assert(entry->sequence_id == -1);
    assert(entry->stop_pending == 0);
    assert(sMainCount == 2);
    assert(sStopCount >= 3);

    memset(&sdk_player, 0, sizeof(sdk_player));
    sdk_player.player_no = 7;
    entry->sdk_handle = (uintptr_t)&sdk_player;
    GameSoundEntry_SetChannelPriority(entry, 12);
    assert(sLastPlayerNo == 7);
    assert(sLastSdkPriority == 12);
    GameSoundEntry_SetTempoRatio(entry, 0x180);
    assert(sLastPlayerNo == 7);
    assert(sLastTempoRatio == 0x180);
    entry->sdk_handle = 0;
    GameSoundEntry_SetChannelPriority(entry, 99);
    assert(sLastSdkPriority == 12);
    return 0;
}
