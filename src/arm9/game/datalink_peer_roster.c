#include "game/datalink_peer_roster.h"
#include "game/datalink_peer_list.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_PEER_CAPACITY = 4,
    DATALINK_PEER_IDENTITY_SIZE = 6,
    DATALINK_PEER_ENTRY_SIZE = 0x20,
    DATALINK_PEER_INPUT_MODE_MASK = 0x1F00,
    DATALINK_PEER_INPUT_MODE_ACTIVE = 0x1900
};

typedef struct Game_DatalinkPeerRenderer {
    uint8_t unknown_00[0x48];
    uint32_t descriptor_callback_address_48;
} Game_DatalinkPeerRenderer;

typedef struct Game_DatalinkPeerHardwareState {
    uint8_t unknown_000[0xD2];
    int8_t connection_variant_d2;
} Game_DatalinkPeerHardwareState;

typedef struct Game_DatalinkPeerGraphicsOwner {
    uint8_t unknown_000[0x554];
    uint32_t tilemap_resource_address_554;
} Game_DatalinkPeerGraphicsOwner;

typedef struct Game_DatalinkPeerTilemapResource {
    uint8_t unknown_00[0x14];
    uint32_t data_address_14;
} Game_DatalinkPeerTilemapResource;

typedef struct Game_DatalinkPeerSubDisplayConfig {
    uint8_t unknown_000[0xFC];
    uint32_t screen_base_blocks_fc[4];
} Game_DatalinkPeerSubDisplayConfig;

extern Game_DatalinkPeerRenderer gGameDatalinkPeerRenderer;
extern Game_DatalinkPeerHardwareState gGameDatalinkPeerHardwareState;
extern Game_DatalinkPeerGraphicsOwner gGameDatalinkPeerGraphicsOwner;
extern Game_DatalinkPeerSubDisplayConfig gGameDatalinkPeerSubDisplayConfig;
extern volatile uint8_t gGameDatalinkPeerConnectionMode;
extern volatile uint32_t gGameDatalinkPeerInputState;
extern volatile uint16_t gGameDatalinkPeerSubBgControl[4];
extern volatile uint32_t gGameDatalinkPeerDisplayWordA;
extern volatile uint32_t gGameDatalinkPeerDisplayWordB;
extern volatile uint32_t gGameDatalinkPeerDisplayWordC;
extern volatile uint32_t gGameDatalinkPeerDisplayWordD;
extern uint32_t gGameDatalinkPeerFallbackResourceAddress;
extern const uint32_t gGameDatalinkPeerVariantGlyphAddresses[4];
extern int32_t gGameDatalinkPeerCellIndexOffset;
extern const int8_t gGameDatalinkPeerRosterCellMap[5];
extern uint32_t gGameDatalinkPeerDescriptorCallbackAddress;

extern void *Game_GetDatalinkWirelessContext(void);
extern int Game_GetDatalinkWirelessPeerCount(void *context);
extern uint8_t *Game_GetDatalinkWirelessPeer(void *context, int index);
extern uint32_t Game_GetDatalinkWirelessPeerAttribute(void);
extern uint32_t Game_GetDatalinkWirelessPeerDataSize(uint8_t *peer);
extern const void *Game_GetDatalinkWirelessPeerData(uint8_t *peer);
extern int Game_GetDatalinkWirelessStatus(void);
extern void Game_BeginDatalinkWirelessConnection(void *context, int mode);
extern void Game_PumpDatalinkWireless(void *context);
extern void Game_UnlockDatalinkWirelessPeerList(void *context);
extern void Game_FinalizeDatalinkWirelessConnection(
    void *context,
    int first,
    int second
);
extern void OS_WaitVBlankIntr(void);

extern uint32_t Game_GetDatalinkGlyphResourceAddress(void);
extern void Game_PrepareDatalinkPeerResourceContext(void);
extern uint32_t Game_GetDatalinkPeerResourceContext(void);
extern void Game_InitializeDatalinkPeerRenderer(
    Game_DatalinkPeerRenderer *renderer,
    uint32_t resource_context,
    void *state_control
);
extern void MI_CpuFill(void *destination, uint32_t value, uint32_t size);
extern void Game_ClearDatalinkPeerCellObjects(void);
extern void Game_ResetDatalinkPeerGraphicsState(void);
extern void Game_ClearDatalinkPeerTilemap(
    void *tilemap,
    uint32_t value,
    uint32_t size
);
extern void Game_ResetDatalinkPeerDisplaySystem(void);
extern void GXS_SetGraphicsMode(int mode);
extern void GX_SetBankForSubBG(int bank);
extern void GX_SetBankForSubBGExtPltt(int bank);
extern uint8_t *gGameDatalinkLobbySceneState;
extern uint8_t gGameDatalinkLobbySharedResourceSet[];
extern volatile uint8_t gGameDatalinkLobbyRuntimeReadyFlag;
extern void Game_FreeDatalinkLobbyAllocation(void *allocation);
extern void Game_ReleaseDatalinkLobbyResourceSlot(uint32_t slot);
extern void Game_ReleaseDatalinkLobbyResourceSet(void *resource_set);
extern void Game_ResetDatalinkLobbyCellAnimationPool(void);
extern void Game_ClearDatalinkLobbyCellObjects(void);
extern void Game_StartDatalinkLobbyExitTransition(uint32_t transition_fx);
extern void Game_PrepareDatalinkLobbyRuntime(void);
extern int Game_IsDatalinkLobbyRuntimeAvailable(void);

_Static_assert(sizeof(Game_DatalinkPeerEntry) == DATALINK_PEER_ENTRY_SIZE,
    "Datalink peer entry size");
_Static_assert(offsetof(Game_DatalinkPeerEntry, seen_0c) == 0x0C,
    "Datalink peer presence offset");
#if UINTPTR_MAX == UINT32_MAX
_Static_assert(offsetof(Game_DatalinkRosterState, selected_wireless_peer_0c4) == 0xC4,
    "Datalink selected network-record offset");
#endif

static Game_DatalinkPeerEntry *Game_GetDatalinkPeerEntryStorage(
    Game_DatalinkRosterState *state,
    uint32_t index)
{
    return (Game_DatalinkPeerEntry *)(void *)(
        (uint8_t *)state->peer_storage_034 +
            index * DATALINK_PEER_ENTRY_SIZE
    );
}

static Game_DatalinkPeerEntry *Game_GetDatalinkPeerObjectEntry(
    const Game_DatalinkRosterState *state,
    uint32_t index)
{
    return (Game_DatalinkPeerEntry *)(void *)state->ordered_peers_0b4[index];
}

static int Game_DatalinkPeerIdentityEquals(
    const uint8_t *first,
    const uint8_t *second)
{
    uint32_t index;
    for (index = 0; index < DATALINK_PEER_IDENTITY_SIZE; ++index) {
        if (first[index] != second[index]) {
            return 0;
        }
    }
    return 1;
}

/*
 * 0x020B0D7C..0x020B0E83 (264 bytes).
 * Marks current peers stale, imports every live wireless record and removes
 * roster entries that were not observed in the new discovery pass.
 */
void Game_RefreshDatalinkPeerRoster_020b0d7c(
    Game_DatalinkRosterState *state)
{
    void *context = Game_GetDatalinkWirelessContext();
    uint32_t index;
    int peer_count;

    for (index = 0; index < state->peer_count_0c8; ++index) {
        Game_GetDatalinkPeerObjectEntry(state, index)->seen_0c = 0;
    }

    peer_count = Game_GetDatalinkWirelessPeerCount(context);
    for (index = 0; index < (uint32_t)peer_count; ++index) {
        uint8_t *peer = Game_GetDatalinkWirelessPeer(context, (int)index);
        if (peer != 0) {
            uint32_t attribute =
                Game_GetDatalinkWirelessPeerAttribute();
            Game_UpsertDatalinkPeerRecord_020b0800(
                state,
                peer + 8,
                Game_GetDatalinkWirelessPeerData(peer),
                Game_GetDatalinkWirelessPeerDataSize(peer),
                (uint8_t)attribute,
                peer[0xC8]
            );
        }
    }

    index = 0;
    while (index < state->peer_count_0c8) {
        Game_DatalinkPeerEntry *entry =
            Game_GetDatalinkPeerObjectEntry(state, index);
        if (entry->seen_0c == 0) {
            Game_RemoveDatalinkPeerRecord_020b0b40(state, entry);
        }
        ++index;
    }
}

/*
 * 0x020B0E84..0x020B0F8B (264 bytes).
 * Finds the wireless record matching the currently selected six-byte peer
 * identity, ends the active peer-list scan and publishes that record at
 * state+0xC4.
 */
int Game_SelectDatalinkPeerNetworkRecord_020b0e84(
    Game_DatalinkRosterState *state)
{
    void *context = Game_GetDatalinkWirelessContext();
    Game_DatalinkPeerEntry *selected =
        Game_GetDatalinkPeerObjectEntry(
            state,
            (uint32_t)(uint8_t)state->selected_peer_104
        );
    int peer_count;
    int index;

    (void)Game_GetDatalinkWirelessStatus();
    peer_count = Game_GetDatalinkWirelessPeerCount(context);
    for (index = 0; index < peer_count; ++index) {
        uint8_t *peer = Game_GetDatalinkWirelessPeer(context, index);
        if (peer != 0 && Game_DatalinkPeerIdentityEquals(
                peer + 8,
                selected->identity_00
            )) {
            Game_UnlockDatalinkWirelessPeerList(context);
            state->selected_wireless_peer_0c4 = peer;
            return 1;
        }
    }
    return 0;
}

/*
 * 0x020B0F8C..0x020B0FE7 (92 bytes).
 * Starts an idle wireless context, pumps it once per VBlank until status two
 * and then finalizes connection mode one.
 */
void Game_StartDatalinkPeerConnection_020b0f8c(void)
{
    void *context = Game_GetDatalinkWirelessContext();

    if (Game_GetDatalinkWirelessStatus() == 0) {
        Game_BeginDatalinkWirelessConnection(context, 0);
        do {
            OS_WaitVBlankIntr();
            Game_PumpDatalinkWireless(context);
        } while (Game_GetDatalinkWirelessStatus() != 2);
    }
    Game_FinalizeDatalinkWirelessConnection(context, 1, 0);
}

/* 0x020B0FE8..0x020B0FF7 (16 bytes). */
void Game_UnlockDatalinkPeerNetworkList_020b0fe8(void)
{
    void *context = Game_GetDatalinkWirelessContext();
    Game_UnlockDatalinkWirelessPeerList(context);
}

/* 0x020B108C..0x020B10CB (64 bytes). */
Game_DatalinkPeerEntry *Game_FindDatalinkPeerEntryForCell_020b108c(
    Game_DatalinkRosterState *state,
    int cell_index)
{
    uint32_t target = (uint32_t)(
        cell_index + gGameDatalinkPeerCellIndexOffset
    );
    uint32_t index;

    for (index = 0; index < DATALINK_PEER_CAPACITY; ++index) {
        Game_DatalinkPeerEntry *entry =
            Game_GetDatalinkPeerEntryStorage(state, index);
        uint32_t slot = entry->sprite_index_07;
        if (target == slot || target == slot + 1U) {
            return entry;
        }
    }
    return 0;
}

/*
 * 0x020B0FF8..0x020B107F (136 bytes).
 * Resolves a six- or twelve-byte glyph slice for the peer cell and returns a
 * fallback resource when no entry owns that cell.
 */
int Game_BuildDatalinkPeerGlyphDescriptor_020b0ff8(
    Game_DatalinkRosterState *state,
    Game_DatalinkPeerGlyphDescriptor *descriptor,
    int cell_index)
{
    Game_DatalinkPeerEntry *entry =
        Game_FindDatalinkPeerEntryForCell_020b108c(state, cell_index);

    if (entry == 0) {
        descriptor->glyph_address = gGameDatalinkPeerFallbackResourceAddress;
        descriptor->glyph_size = 6;
    } else if (((cell_index + gGameDatalinkPeerCellIndexOffset) & 1) == 0) {
        descriptor->glyph_address =
            (uint32_t)(uintptr_t)&entry->label_0e[0];
        descriptor->glyph_size = 0x0C;
    } else {
        int variant = (int)entry->attribute_0a - 1;
        if (variant < 0) {
            variant = 0;
        }
        if (variant > 3) {
            variant = 3;
        }
        descriptor->glyph_address =
            gGameDatalinkPeerVariantGlyphAddresses[variant];
        descriptor->glyph_size = 6;
    }

    descriptor->resource_address = Game_GetDatalinkGlyphResourceAddress();
    return 1;
}

/*
 * 0x020B10D4..0x020B118B (184 bytes).
 * Binds the peer renderer, clears four entry records and publishes the
 * recovered connection-resume and input-mode fields.
 */
void Game_SetupDatalinkPeerRoster_020b10d4(
    Game_DatalinkRosterState *state)
{
    uint32_t resource_context;

    gGameDatalinkPeerRenderer.descriptor_callback_address_48 =
        gGameDatalinkPeerDescriptorCallbackAddress;
    Game_PrepareDatalinkPeerResourceContext();
    resource_context = Game_GetDatalinkPeerResourceContext();
    Game_InitializeDatalinkPeerRenderer(
        &gGameDatalinkPeerRenderer,
        resource_context,
        state != 0 ? (uint8_t *)state + 0x30 : 0
    );
    MI_CpuFill(
        state->peer_storage_034,
        0,
        sizeof(state->peer_storage_034)
    );

    if (gGameDatalinkPeerConnectionMode == 2 &&
        gGameDatalinkPeerHardwareState.connection_variant_d2 == -1) {
        state->resumed_session_107 = 1;
    }
    gGameDatalinkPeerConnectionMode = 0;
    state->completion_timer_100 = 0;
    state->phase_106 = state->resumed_session_107 != 0 ? 8 : 0;
    state->selected_peer_104 = 0;
    state->peer_count_0c8 = 0;
    state->screen_resource_0cc = UINT32_MAX;
    gGameDatalinkPeerInputState =
        (gGameDatalinkPeerInputState & ~DATALINK_PEER_INPUT_MODE_MASK) |
        DATALINK_PEER_INPUT_MODE_ACTIVE;
}

/*
 * 0x020B11A0..0x020B11C7 (40 bytes).
 * Clears roster cell objects, resets graphics state and zeroes the recovered
 * 0x2000-byte tilemap resource.
 */
void Game_ClearDatalinkPeerRosterGraphics_020b11a0(void)
{
    Game_DatalinkPeerTilemapResource *resource =
        (Game_DatalinkPeerTilemapResource *)(uintptr_t)
            gGameDatalinkPeerGraphicsOwner.tilemap_resource_address_554;

    Game_ClearDatalinkPeerCellObjects();
    Game_ResetDatalinkPeerGraphicsState();
    Game_ClearDatalinkPeerTilemap(
        (void *)(uintptr_t)resource->data_address_14,
        0,
        0x2000
    );
}

/* 0x020B11DC..0x020B121F (68 bytes). */
int Game_GetDatalinkPeerRosterCellMap_020b11dc(int index)
{
    return gGameDatalinkPeerRosterCellMap[index];
}

static uint16_t Game_ConfigureDatalinkPeerBgControl(
    uint16_t current,
    uint16_t configuration,
    uint16_t priority)
{
    uint16_t configured = (uint16_t)((current & 0x0043U) | configuration);
    return (uint16_t)((configured & 0xFFFCU) | priority);
}

/*
 * 0x020B1224..0x020B1357 (308 bytes).
 * Resets the sub display, assigns BG banks and configures four recovered BG
 * layers with priorities 0..3 and screen-base blocks 4, 0x3E, 10 and 6.
 */
void Game_ConfigureDatalinkLobbySubDisplay_020b1224(void)
{
    gGameDatalinkPeerDisplayWordA = 0;
    gGameDatalinkPeerDisplayWordB = 0;
    Game_ResetDatalinkPeerDisplaySystem();
    gGameDatalinkPeerDisplayWordC &= UINT32_C(0xFFFFFF00);
    gGameDatalinkPeerDisplayWordD = 0;

    GXS_SetGraphicsMode(0);
    GX_SetBankForSubBG(4);
    GX_SetBankForSubBGExtPltt(0x80);

    gGameDatalinkPeerSubBgControl[0] =
        Game_ConfigureDatalinkPeerBgControl(
            gGameDatalinkPeerSubBgControl[0], 0x150CU, 0
        );
    gGameDatalinkPeerSubBgControl[1] =
        Game_ConfigureDatalinkPeerBgControl(
            gGameDatalinkPeerSubBgControl[1], 0x5300U, 1
        );
    gGameDatalinkPeerSubBgControl[2] =
        Game_ConfigureDatalinkPeerBgControl(
            gGameDatalinkPeerSubBgControl[2], 0x5100U, 2
        );
    gGameDatalinkPeerSubBgControl[3] =
        Game_ConfigureDatalinkPeerBgControl(
            gGameDatalinkPeerSubBgControl[3], 0x1090U, 3
        );

    gGameDatalinkPeerSubDisplayConfig.screen_base_blocks_fc[0] = 4;
    gGameDatalinkPeerSubDisplayConfig.screen_base_blocks_fc[1] = 0x3E;
    gGameDatalinkPeerSubDisplayConfig.screen_base_blocks_fc[2] = 10;
    gGameDatalinkPeerSubDisplayConfig.screen_base_blocks_fc[3] = 6;
}

static uint32_t Game_ReadDatalinkLobbySceneWord(uint32_t offset)
{
    const uint8_t *bytes = gGameDatalinkLobbySceneState + offset;
    return (uint32_t)bytes[0] |
        ((uint32_t)bytes[1] << 8) |
        ((uint32_t)bytes[2] << 16) |
        ((uint32_t)bytes[3] << 24);
}

/*
 * 0x020B1920..0x020B19CB (172 bytes).
 * Releases the lobby allocations and graphics resource sets, resets the cell
 * pool and starts the exit transition unless the scene reason is seven/eight.
 */
void Game_ReleaseDatalinkLobbyScene_020b1920(void)
{
    uint32_t slot;
    uint8_t exit_reason;

    Game_ClearDatalinkLobbyCellObjects();
    Game_FreeDatalinkLobbyAllocation(
        (void *)(uintptr_t)Game_ReadDatalinkLobbySceneWord(0x34)
    );
    Game_FreeDatalinkLobbyAllocation(
        (void *)(uintptr_t)Game_ReadDatalinkLobbySceneWord(0xB0)
    );
    Game_FreeDatalinkLobbyAllocation(
        (void *)(uintptr_t)Game_ReadDatalinkLobbySceneWord(0xB4)
    );
    Game_FreeDatalinkLobbyAllocation(
        (void *)(uintptr_t)Game_ReadDatalinkLobbySceneWord(0xB8)
    );

    for (slot = 0; slot < 4U; ++slot) {
        Game_ReleaseDatalinkLobbyResourceSlot(slot);
    }
    Game_ReleaseDatalinkLobbyResourceSet(
        gGameDatalinkLobbySharedResourceSet
    );
    Game_ReleaseDatalinkLobbyResourceSet(
        gGameDatalinkLobbySceneState + 0x3C
    );
    Game_ResetDatalinkLobbyCellAnimationPool();
    Game_ClearDatalinkPeerCellObjects();

    exit_reason = gGameDatalinkLobbySceneState[0x28];
    if (exit_reason != 8U && exit_reason != 7U) {
        Game_StartDatalinkLobbyExitTransition(0x1000U);
    }
    Game_PrepareDatalinkLobbyRuntime();
    if (Game_IsDatalinkLobbyRuntimeAvailable() != 0) {
        gGameDatalinkLobbyRuntimeReadyFlag = 0;
    }
}
