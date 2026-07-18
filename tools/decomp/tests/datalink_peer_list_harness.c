#include "game/datalink_peer_list.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

Game_DatalinkTweenRecord gGameDatalinkControllerTweens[32];
const int32_t gGameDatalinkPeerSpriteOffsets_021263cc[4] = {
    -72, -8, 40, -8
};

static Game_DatalinkOamEntry sOams[16];
static Game_DatalinkPeerSprite sSprites[8];
static int sActivateCount;
static int sArrangeCount;
static int sSoundCount;
static uint32_t sLastSound;

uint32_t Game_ActivateDatalinkPeerSprite_020a2df4(
    uint8_t sprite_index,
    const void *parameters)
{
    assert(sprite_index < 8U);
    assert(parameters == NULL);
    ++sActivateCount;
    return 1;
}

const Game_DatalinkPeerSprite *Game_GetDatalinkPeerSprite_020a2efc(
    uint8_t sprite_index)
{
    assert(sprite_index < 8U);
    return &sSprites[sprite_index];
}

int NNS_G2dArrangeOBJ2DRect(
    Game_DatalinkOamEntry *oam_entries,
    uint32_t width,
    uint32_t height,
    int x,
    int y,
    int affine_index,
    uint32_t parameter)
{
    (void)width;
    (void)height;
    (void)x;
    (void)y;
    (void)parameter;
    assert(oam_entries >= &sOams[0]);
    assert(oam_entries < &sOams[16]);
    assert(affine_index == 0);
    ++sArrangeCount;
    return 1;
}

void MI_CpuFill8(void *destination, uint8_t value, uint32_t size)
{
    memset(destination, value, size);
}

void MI_CpuCopy8(const void *source, void *destination, uint32_t size)
{
    memcpy(destination, source, size);
}

void Game_PlayDatalinkSoundEffect(uint32_t sound_id)
{
    sLastSound = sound_id;
    ++sSoundCount;
}

static void ResetHarness(Game_DatalinkPeerMenuState *state)
{
    unsigned int index;

    memset(state, 0, sizeof(*state));
    memset(gGameDatalinkControllerTweens, 0, sizeof(gGameDatalinkControllerTweens));
    memset(sSprites, 0, sizeof(sSprites));
    for (index = 0; index < 8U; ++index) {
        sSprites[index].oam_entries_24 = sOams;
    }
    sActivateCount = 0;
    sArrangeCount = 0;
    sSoundCount = 0;
    sLastSound = 0;
}

static Game_DatalinkPeerListRecord *AddPeer(
    Game_DatalinkPeerMenuState *state,
    unsigned int storage_index,
    uint8_t sort_key,
    uint32_t age)
{
    Game_DatalinkPeerListRecord *peer =
        &state->peer_storage_034[storage_index];
    unsigned int visible_index = state->peer_count_0c8;

    peer->active_08 = 1;
    peer->seen_0c = 1;
    peer->tween_index_06 = (uint8_t)storage_index;
    peer->sprite_index_07 = (uint8_t)(storage_index * 2U);
    peer->sort_key_09 = sort_key;
    peer->discovery_age_1c = age;
    state->ordered_peers_0b4[visible_index] = peer;
    state->peer_count_0c8 = visible_index + 1U;
    return peer;
}

static void TestReadyCountStopsAtFirstYoungPeer(void)
{
    Game_DatalinkPeerMenuState state;

    ResetHarness(&state);
    AddPeer(&state, 0, 4, 12);
    AddPeer(&state, 1, 3, 9);
    AddPeer(&state, 2, 2, 8);
    AddPeer(&state, 3, 1, 20);
    assert(Game_CountDatalinkReadyPeers_020b07bc(&state) == 2U);
}

static void TestExistingPeerRefresh(void)
{
    static const uint8_t identity[6] = {1, 2, 3, 4, 5, 6};
    static const uint8_t label[3] = {'L', 'N', 'K'};
    Game_DatalinkPeerMenuState state;
    Game_DatalinkPeerListRecord *peer;

    ResetHarness(&state);
    peer = AddPeer(&state, 0, 1, 8);
    memcpy(peer->identity_00, identity, sizeof(identity));
    peer->attribute_0a = 5;
    Game_UpsertDatalinkPeerRecord_020b0800(
        &state,
        identity,
        label,
        sizeof(label),
        5,
        9
    );
    assert(peer->sort_key_09 == 9U);
    assert(peer->seen_0c == 1U);
    assert(peer->discovery_age_1c == 9U);
    assert(sActivateCount == 0);

    Game_UpsertDatalinkPeerRecord_020b0800(
        &state,
        identity,
        label,
        sizeof(label),
        6,
        10
    );
    assert(peer->attribute_0a == 6U);
    assert(peer->discovery_age_1c == 10U);
    assert(sActivateCount == 2);
    assert(sArrangeCount == 2);
}

static void TestSortRemoveAndClear(void)
{
    Game_DatalinkPeerMenuState state;
    Game_DatalinkPeerListRecord *first;
    Game_DatalinkPeerListRecord *second;

    ResetHarness(&state);
    first = AddPeer(&state, 0, 1, 9);
    second = AddPeer(&state, 1, 3, 9);
    Game_SortDatalinkPeerRecords_020b0c38(&state);
    assert(state.ordered_peers_0b4[0] == second);
    assert(state.ordered_peers_0b4[1] == first);
    assert(sArrangeCount == 4);

    Game_RemoveDatalinkPeerRecord_020b0b40(&state, second);
    assert(second->active_08 == 0U);
    assert(second->seen_0c == 0U);
    assert(state.peer_count_0c8 == 1U);
    assert(state.ordered_peers_0b4[0] == first);
    assert(sLastSound == 0x44U);

    Game_ClearDatalinkPeerList_020b0d58(&state);
    assert(state.peer_count_0c8 == 0U);
    assert(first->active_08 == 0U);
}

int main(void)
{
    TestReadyCountStopsAtFirstYoungPeer();
    TestExistingPeerRefresh();
    TestSortRemoveAndClear();
    return 0;
}
