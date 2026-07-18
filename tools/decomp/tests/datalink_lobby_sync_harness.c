#include "game/datalink_lobby_sync.h"

#include "game/datalink_post_oam.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

Game_DatalinkLobbyStateSlot gGameDatalinkLobbyStateSlot;
Game_DatalinkPostOamHardwareState gGameDatalinkPostOamHardwareState;
Game_DatalinkPostOamSession gGameDatalinkPostOamSession;
Game_DatalinkPostOamOutput *gGameDatalinkPostOamOutput;
Game_DatalinkPostOamParticipant
    gGameDatalinkPostOamParticipants[
        GAME_DATALINK_POST_OAM_PARTICIPANT_CAPACITY
    ];
Game_DatalinkOamManager gGameDatalinkOamManager;
GameLinkOperationState gGameLinkOperationState_0212c9a8;
uint8_t gGameDatalinkLobbyTransitionObject_02171c84[1];
const uint8_t gGameDatalinkLobbyTransitionTransfer_0216f020[1] = {0};
uint16_t gGameDatalinkLobbyPreviousInputs_0212c584[4];
uint16_t gGameDatalinkLobbyInputEdges_0212c58c[4];
const uint16_t gGameDatalinkLobbyDefaultInput_0212c514 = 0;
uint16_t
    gGameDatalinkLobbyPlayerNameCache_02181868
        [GAME_DATALINK_LOBBY_SYNC_SLOT_COUNT]
        [GAME_DATALINK_LOBBY_PLAYER_NAME_UNITS];
uint8_t
    gGameDatalinkLobbyLinkScratch_0212c7f0
        [GAME_DATALINK_LOBBY_LINK_SCRATCH_SIZE];

static uint8_t sOutputStorage[0x500];
static Game_DatalinkLobbySyncState sState;
static int sRuntimeAvailable;

void *Game_GetDatalinkWirelessContext_020909c8(void)
{
    return NULL;
}

int Game_GetDatalinkWirelessState_02090ea4(void *context)
{
    (void)context;
    return 8;
}

void *Game_GetDatalinkLobbyGraphicsManager_0208da4c(void)
{
    return NULL;
}

int Game_IsDatalinkLobbyRuntimeAvailable_0208de54(void)
{
    return sRuntimeAvailable;
}

int Game_GetDatalinkLobbyWirelessParticipantCount_020922d0(void *context)
{
    (void)context;
    return 4;
}

int Game_BeginDatalinkHostShutdown_02091738(void *context)
{
    (void)context;
    return 1;
}

void Game_BeginDatalinkLobbyErrorTransition_0202811c(void)
{
}

int Game_IsDatalinkLobbyTransitionObjectActive_020a1d08(
    const void *object)
{
    assert(object == gGameDatalinkLobbyTransitionObject_02171c84);
    return 0;
}

void Game_ResetDatalinkLobbyTransitionObject_020a20e0(void *object)
{
    (void)object;
}

void Game_TransferDatalinkLobbyErrorResource_0209da7c(
    int target,
    const void *source,
    int destination_offset,
    uint32_t size)
{
    (void)target;
    (void)source;
    (void)destination_offset;
    (void)size;
}

void Game_PlayDatalinkSoundEffect(uint32_t sound_id)
{
    (void)sound_id;
}

void Game_LoadDatalinkScreenResource(uint16_t resource_id)
{
    (void)resource_id;
}

void Game_CommitDatalinkScreenUpdate(void)
{
}

void Game_RefreshDatalinkLobbyParticipantRecord_020a31ac(
    int participant,
    void *record)
{
    (void)participant;
    (void)record;
}

uint32_t STD_GetStringLength(const uint16_t *string)
{
    uint32_t length = 0;

    while (string[length] != 0U) {
        ++length;
    }
    return length;
}

void MI_CpuCopy8(const void *source, void *destination, uint32_t size)
{
    memcpy(destination, source, size);
}

void MI_CpuFill(void *destination, uint32_t value, uint32_t size)
{
    memset(destination, (int)(uint8_t)value, size);
}

void MIi_CpuClear32(uint32_t value, void *destination, uint32_t size)
{
    assert(value == 0U);
    memset(destination, 0, size);
}

void Game_StartDatalinkLinkOperation_02028078(int mode)
{
    assert(mode == 3);
}

static void TestRecoveredStateLayout(void)
{
    memset(&sState, 0, sizeof(sState));
    memset(sOutputStorage, 0, sizeof(sOutputStorage));
    gGameDatalinkPostOamOutput =
        (Game_DatalinkPostOamOutput *)(void *)sOutputStorage;

    assert(sizeof(sState) == 0xBCU);
    sState.phase_28 = 5;
    sState.participant_count_3b = 4;
    sState.remote_variants_ac[3] = 2;
    assert(sState.phase_28 == 5U);
    assert(sState.participant_count_3b == 4);
    assert(sState.remote_variants_ac[3] == 2U);
}

static void TestExistingLinkPredicate(void)
{
    gGameLinkOperationState_0212c9a8.result = 0;
    assert(!Game_LinkOperation_HasSucceeded_020b2a90());
    gGameLinkOperationState_0212c9a8.result = 1;
    assert(Game_LinkOperation_HasSucceeded_020b2a90());
}

int main(void)
{
    TestRecoveredStateLayout();
    TestExistingLinkPredicate();
    return 0;
}
