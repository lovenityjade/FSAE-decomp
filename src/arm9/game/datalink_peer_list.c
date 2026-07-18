#include "game/datalink_peer_list.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_PEER_READY_AGE = 9,
    DATALINK_PEER_LABEL_CAPACITY = 0x0E,
    DATALINK_PEER_INITIAL_X_FX = 0x80000,
    DATALINK_PEER_ROW_X = 48,
    DATALINK_PEER_ROW_SPACING = 40,
    DATALINK_PEER_FX_SHIFT = 12,
    DATALINK_PEER_REMOVED_SOUND = 0x44,
    DATALINK_PEER_ADDED_SOUND = 0x43,
    DATALINK_PEER_SPRITE_WIDTH_OFFSET = 0x38,
    DATALINK_PEER_SPRITE_HEIGHT_OFFSET = 0x3C,
    DATALINK_PEER_SPRITE_PARAMETER_OFFSET = 0x2C
};

/* ROM table 0x020DED24; the body copies all eight words before using a slot. */
static const uint32_t sGameDatalinkPeerTweenIndices[8] = {
    0x23, 0x27, 0x29, 0x2B, 0x2D, 0x2F, 0x31, 0x33
};

extern uint32_t Game_ActivateDatalinkPeerSprite_020a2df4(
    uint8_t sprite_index,
    const void *parameters
);
extern const Game_DatalinkPeerSprite *Game_GetDatalinkPeerSprite_020a2efc(
    uint8_t sprite_index
);
extern int NNS_G2dArrangeOBJ2DRect(
    Game_DatalinkOamEntry *oam_entries,
    uint32_t width,
    uint32_t height,
    int x,
    int y,
    int affine_index,
    uint32_t parameter
);
extern void MI_CpuFill8(void *destination, uint8_t value, uint32_t size);
extern void MI_CpuCopy8(
    const void *source,
    void *destination,
    uint32_t size
);
extern void Game_PlayDatalinkSoundEffect(uint32_t sound_id);

_Static_assert(
    sizeof(Game_DatalinkPeerListRecord) == 0x20,
    "datalink peer-list record size"
);
_Static_assert(
    offsetof(Game_DatalinkPeerListRecord, tween_index_06) == 0x06,
    "datalink peer-list tween-index offset"
);
_Static_assert(
    offsetof(Game_DatalinkPeerListRecord, sprite_index_07) == 0x07,
    "datalink peer-list sprite-index offset"
);
_Static_assert(
    offsetof(Game_DatalinkPeerListRecord, active_08) == 0x08,
    "datalink peer-list active offset"
);
_Static_assert(
    offsetof(Game_DatalinkPeerListRecord, sort_key_09) == 0x09,
    "datalink peer-list sort-key offset"
);
_Static_assert(
    offsetof(Game_DatalinkPeerListRecord, attribute_0a) == 0x0A,
    "datalink peer-list attribute offset"
);
_Static_assert(
    offsetof(Game_DatalinkPeerListRecord, seen_0c) == 0x0C,
    "datalink peer-list seen offset"
);
_Static_assert(
    offsetof(Game_DatalinkPeerListRecord, label_0e) == 0x0E,
    "datalink peer-list label offset"
);
_Static_assert(
    offsetof(Game_DatalinkPeerListRecord, discovery_age_1c) == 0x1C,
    "datalink peer-list age offset"
);

#if UINTPTR_MAX == UINT32_MAX
_Static_assert(
    offsetof(Game_DatalinkPeerMenuState, peer_storage_034) == 0x34,
    "datalink peer storage offset"
);
_Static_assert(
    offsetof(Game_DatalinkPeerMenuState, ordered_peers_0b4) == 0xB4,
    "datalink ordered-peer offset"
);
_Static_assert(
    offsetof(Game_DatalinkPeerMenuState, peer_count_0c8) == 0xC8,
    "datalink peer-count offset"
);
#endif

static uint32_t Game_ReadDatalinkPeerSpriteWord(
    const Game_DatalinkPeerSprite *sprite,
    uint32_t offset)
{
    const uint8_t *source = (const uint8_t *)sprite + offset;
    uint32_t value = 0;
    uint8_t *destination = (uint8_t *)&value;
    uint32_t index;

    for (index = 0; index < sizeof(value); ++index) {
        destination[index] = source[index];
    }
    return value;
}

static Game_DatalinkTweenRecord *Game_GetDatalinkPeerPosition(
    const Game_DatalinkPeerListRecord *peer)
{
    return &gGameDatalinkControllerTweens[peer->tween_index_06];
}

static int32_t Game_GetDatalinkPeerRowYFx(unsigned int visible_index)
{
    return (int32_t)(
        (visible_index * DATALINK_PEER_ROW_SPACING +
         DATALINK_PEER_ROW_X) <<
        DATALINK_PEER_FX_SHIFT
    );
}

static void Game_ResetDatalinkPeerPosition(
    Game_DatalinkPeerListRecord *peer,
    unsigned int visible_index)
{
    Game_DatalinkTweenRecord *position =
        Game_GetDatalinkPeerPosition(peer);

    position->duration_fx_28 = 0;
    position->current_x_fx_30 = DATALINK_PEER_INITIAL_X_FX;
    position->current_y_fx_34 =
        Game_GetDatalinkPeerRowYFx(visible_index);
}

/* 0x020B07BC..0x020B07FF: complete 68-byte body with no pool. */
unsigned int Game_CountDatalinkReadyPeers_020b07bc(
    const Game_DatalinkPeerMenuState *state)
{
    unsigned int count = 0;

    while (count < state->peer_count_0c8) {
        if (state->ordered_peers_0b4[count]->discovery_age_1c <
            DATALINK_PEER_READY_AGE) {
            break;
        }
        ++count;
    }
    return count;
}

static int Game_DatalinkPeerIdentityMatches(
    const Game_DatalinkPeerListRecord *peer,
    const uint8_t identity[6])
{
    unsigned int index;

    for (index = 0; index < sizeof(peer->identity_00); ++index) {
        if (peer->identity_00[index] != identity[index]) {
            return 0;
        }
    }
    return 1;
}

static void Game_UpdateExistingDatalinkPeer(
    Game_DatalinkPeerMenuState *state,
    Game_DatalinkPeerListRecord *peer,
    unsigned int storage_index,
    uint8_t attribute,
    uint8_t sort_key)
{
    peer->sort_key_09 = sort_key;
    peer->seen_0c = 1;
    ++peer->discovery_age_1c;
    if (peer->attribute_0a == attribute) {
        return;
    }

    peer->attribute_0a = attribute;
    Game_InitializeDatalinkPeerVisuals_020b09f4(
        state,
        peer,
        storage_index
    );
}

static void Game_InitializeNewDatalinkPeer(
    Game_DatalinkPeerMenuState *state,
    Game_DatalinkPeerListRecord *peer,
    unsigned int storage_index,
    const uint8_t identity[6],
    const void *label,
    uint32_t label_size,
    uint8_t attribute,
    uint8_t sort_key)
{
    unsigned int identity_index;
    unsigned int visible_index = state->peer_count_0c8;

    peer->active_08 = 1;
    for (identity_index = 0;
         identity_index < sizeof(peer->identity_00);
         ++identity_index) {
        peer->identity_00[identity_index] = identity[identity_index];
    }
    MI_CpuFill8(peer->label_0e, 0, DATALINK_PEER_LABEL_CAPACITY);
    MI_CpuCopy8(label, peer->label_0e, label_size);
    peer->attribute_0a = attribute;
    peer->sort_key_09 = sort_key;
    peer->tween_index_06 =
        (uint8_t)sGameDatalinkPeerTweenIndices[storage_index];
    peer->unknown_0b = 0;
    peer->sprite_index_07 = (uint8_t)(storage_index << 1);
    peer->seen_0c = 1;
    peer->discovery_age_1c = 0;

    Game_InitializeDatalinkPeerVisuals_020b09f4(
        state,
        peer,
        visible_index
    );
    state->ordered_peers_0b4[visible_index] = peer;
    state->peer_count_0c8 = visible_index + 1U;
    Game_PlayDatalinkSoundEffect(DATALINK_PEER_ADDED_SOUND);
    Game_SortDatalinkPeerRecords_020b0c38(state);
}

/*
 * 0x020B0800..0x020B09EF: complete 496-byte body. Its pointer to the
 * eight-word ROM tween-index table is the sole pool word at 0x020B09F0.
 */
void Game_UpsertDatalinkPeerRecord_020b0800(
    Game_DatalinkPeerMenuState *state,
    const uint8_t identity[6],
    const void *label,
    uint32_t label_size,
    uint8_t attribute,
    uint8_t sort_key)
{
    unsigned int index;

    for (index = 0; index < state->peer_count_0c8; ++index) {
        Game_DatalinkPeerListRecord *peer = state->ordered_peers_0b4[index];

        if (Game_DatalinkPeerIdentityMatches(peer, identity)) {
            Game_UpdateExistingDatalinkPeer(
                state,
                peer,
                index,
                attribute,
                sort_key
            );
            return;
        }
    }

    for (index = 0; index < GAME_DATALINK_PEER_MENU_MAX_PEERS; ++index) {
        Game_DatalinkPeerListRecord *peer = &state->peer_storage_034[index];

        if (peer->active_08 == 0U) {
            Game_InitializeNewDatalinkPeer(
                state,
                peer,
                index,
                identity,
                label,
                label_size,
                attribute,
                sort_key
            );
            return;
        }
    }
}

/*
 * 0x020B09F4..0x020B0A67: complete 116-byte body. The positioned-object
 * global at 0x0217EB2C is its sole pool word at 0x020B0A68.
 */
void Game_InitializeDatalinkPeerVisuals_020b09f4(
    Game_DatalinkPeerMenuState *state,
    Game_DatalinkPeerListRecord *peer,
    unsigned int visible_index)
{
    (void)Game_ActivateDatalinkPeerSprite_020a2df4(
        peer->sprite_index_07,
        NULL
    );
    (void)Game_ActivateDatalinkPeerSprite_020a2df4(
        (uint8_t)(peer->sprite_index_07 + 1U),
        NULL
    );
    Game_ResetDatalinkPeerPosition(peer, visible_index);
    Game_ArrangeDatalinkPeerSprites_020b0a6c(
        state,
        peer,
        visible_index
    );
}

static void Game_ArrangeOneDatalinkPeerSprite(
    const Game_DatalinkPeerSprite *sprite,
    const Game_DatalinkTweenRecord *position,
    int x_offset,
    int y_offset)
{
    Game_DatalinkOamEntry *output =
        (Game_DatalinkOamEntry *)(uintptr_t)(
            (uintptr_t)sprite->oam_entries_24 +
            (uint32_t)sprite->oam_start_index_44 *
                sizeof(Game_DatalinkOamEntry)
        );

    (void)NNS_G2dArrangeOBJ2DRect(
        output,
        Game_ReadDatalinkPeerSpriteWord(
            sprite,
            DATALINK_PEER_SPRITE_WIDTH_OFFSET
        ),
        Game_ReadDatalinkPeerSpriteWord(
            sprite,
            DATALINK_PEER_SPRITE_HEIGHT_OFFSET
        ),
        x_offset + (position->current_x_fx_30 >> DATALINK_PEER_FX_SHIFT),
        y_offset + (position->current_y_fx_34 >> DATALINK_PEER_FX_SHIFT),
        0,
        Game_ReadDatalinkPeerSpriteWord(
            sprite,
            DATALINK_PEER_SPRITE_PARAMETER_OFFSET
        )
    );
}

/*
 * 0x020B0A6C..0x020B0B37: complete 204-byte body. Its two-word pool at
 * 0x020B0B38 selects the positioned-object array and signed offset quartet.
 */
void Game_ArrangeDatalinkPeerSprites_020b0a6c(
    Game_DatalinkPeerMenuState *state,
    Game_DatalinkPeerListRecord *peer,
    unsigned int visible_index)
{
    const Game_DatalinkPeerSprite *first;
    const Game_DatalinkPeerSprite *second;
    Game_DatalinkTweenRecord *position =
        Game_GetDatalinkPeerPosition(peer);

    (void)state;
    (void)visible_index;
    first = Game_GetDatalinkPeerSprite_020a2efc(peer->sprite_index_07);
    Game_ArrangeOneDatalinkPeerSprite(
        first,
        position,
        gGameDatalinkPeerSpriteOffsets_021263cc[0],
        gGameDatalinkPeerSpriteOffsets_021263cc[1]
    );
    second = Game_GetDatalinkPeerSprite_020a2efc(
        (uint8_t)(peer->sprite_index_07 + 1U)
    );
    Game_ArrangeOneDatalinkPeerSprite(
        second,
        position,
        gGameDatalinkPeerSpriteOffsets_021263cc[2],
        gGameDatalinkPeerSpriteOffsets_021263cc[3]
    );
}

/*
 * 0x020B0B40..0x020B0C33: complete 244-byte body. Its positioned-object
 * global pointer at 0x020B0C34 is excluded.
 */
void Game_RemoveDatalinkPeerRecord_020b0b40(
    Game_DatalinkPeerMenuState *state,
    Game_DatalinkPeerListRecord *peer)
{
    unsigned int removed_index = UINT32_MAX;
    unsigned int index;

    for (index = 0; index < state->peer_count_0c8; ++index) {
        if (state->ordered_peers_0b4[index] == peer) {
            peer->active_08 = 0;
            peer->seen_0c = 0;
            Game_PlayDatalinkSoundEffect(DATALINK_PEER_REMOVED_SOUND);
            removed_index = index;
            break;
        }
    }

    if (removed_index == UINT32_MAX) {
        return;
    }
    while (removed_index < state->peer_count_0c8 - 1U) {
        Game_DatalinkPeerListRecord *shifted =
            state->ordered_peers_0b4[removed_index + 1U];

        state->ordered_peers_0b4[removed_index] = shifted;
        Game_ResetDatalinkPeerPosition(shifted, removed_index);
        Game_ArrangeDatalinkPeerSprites_020b0a6c(
            state,
            shifted,
            removed_index
        );
        ++removed_index;
    }
    --state->peer_count_0c8;
}

static int Game_SortDatalinkReadyPeerPass(
    Game_DatalinkPeerMenuState *state,
    unsigned int first_index)
{
    unsigned int index = Game_CountDatalinkReadyPeers_020b07bc(state);
    int swapped = 0;

    while (--index > first_index) {
        Game_DatalinkPeerListRecord *left =
            state->ordered_peers_0b4[index - 1U];
        Game_DatalinkPeerListRecord *right =
            state->ordered_peers_0b4[index];

        if (right->sort_key_09 > left->sort_key_09) {
            state->ordered_peers_0b4[index] = left;
            state->ordered_peers_0b4[index - 1U] = right;
            swapped = 1;
        }
    }
    return swapped;
}

/*
 * 0x020B0C38..0x020B0D53: complete 284-byte body. Its positioned-object
 * global pointer at 0x020B0D54 is excluded.
 */
void Game_SortDatalinkPeerRecords_020b0c38(
    Game_DatalinkPeerMenuState *state)
{
    unsigned int index;
    int swapped = 0;

    if (Game_CountDatalinkReadyPeers_020b07bc(state) < 2U) {
        return;
    }

    index = 0;
    while (index < Game_CountDatalinkReadyPeers_020b07bc(state)) {
        if (Game_SortDatalinkReadyPeerPass(state, index) != 0) {
            swapped = 1;
        }
        ++index;
    }

    if (swapped == 0) {
        return;
    }
    index = 0;
    while (index < Game_CountDatalinkReadyPeers_020b07bc(state)) {
        Game_DatalinkPeerListRecord *peer = state->ordered_peers_0b4[index];

        Game_ResetDatalinkPeerPosition(peer, index);
        Game_ArrangeDatalinkPeerSprites_020b0a6c(state, peer, index);
        ++index;
    }
}

/* 0x020B0D58..0x020B0D7B: complete 36-byte body with no pool. */
void Game_ClearDatalinkPeerList_020b0d58(
    Game_DatalinkPeerMenuState *state)
{
    unsigned int index;

    state->peer_count_0c8 = 0;
    for (index = 0; index < GAME_DATALINK_PEER_MENU_MAX_PEERS; ++index) {
        state->peer_storage_034[index].active_08 = 0;
    }
}
