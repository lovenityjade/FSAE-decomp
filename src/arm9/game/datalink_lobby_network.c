#include "game/datalink_lobby_sync.h"

#include "game/datalink_post_oam.h"

#include <stdint.h>

enum {
    DATALINK_LOBBY_LINK_LOCAL_INPUT_OFFSET = 0x12,
    DATALINK_LOBBY_LINK_SLOT_INPUT_OFFSET = 0x7A,
    DATALINK_LOBBY_LINK_SLOT_STRIDE = 0x34,
    DATALINK_LOBBY_OUTPUT_PARTICIPANT_OFFSET = 0x464,
    DATALINK_LOBBY_OUTPUT_PARTICIPANT_STRIDE = 0x14,
    DATALINK_LOBBY_PARTICIPANT_NAME_OFFSET = 0x14,
    DATALINK_LOBBY_RENDER_NAME_DIRTY_OFFSET = 0x2F28,
    DATALINK_LOBBY_LINK_ACTIVE_OFFSET = 0x0E,
    DATALINK_LOBBY_LINK_OPERATION_MODE = 3
};

extern void *Game_GetDatalinkWirelessContext_020909c8(void);
extern void *Game_GetDatalinkLobbyGraphicsManager_0208da4c(void);
extern int Game_IsDatalinkLobbyRuntimeAvailable_0208de54(void);
extern void Game_RefreshDatalinkLobbyParticipantRecord_020a31ac(
    int participant,
    void *record
);
extern uint32_t STD_GetStringLength(const uint16_t *string);
extern void MI_CpuCopy8(
    const void *source,
    void *destination,
    uint32_t size
);
extern void MI_CpuFill(void *destination, uint32_t value, uint32_t size);
extern void MIi_CpuClear32(
    uint32_t value,
    void *destination,
    uint32_t size
);
extern void Game_StartDatalinkLinkOperation_02028078(int mode);

static Game_DatalinkLobbySyncState *Game_GetDatalinkLobbyNetworkState(void)
{
    return (Game_DatalinkLobbySyncState *)(uintptr_t)
        gGameDatalinkLobbyStateSlot.state_address_04;
}

static uint16_t Game_ReadDatalinkLobbyNetworkHalfword(
    const void *base,
    uint32_t offset)
{
    const uint8_t *bytes = (const uint8_t *)base + offset;

    return (uint16_t)(bytes[0] | ((uint16_t)bytes[1] << 8));
}

static void Game_WriteDatalinkLobbyNetworkHalfword(
    void *base,
    uint32_t offset,
    uint16_t value)
{
    uint8_t *bytes = (uint8_t *)base + offset;

    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
}

static uint32_t Game_ReadDatalinkLobbyNetworkWord(
    const void *base,
    uint32_t offset)
{
    const uint8_t *bytes = (const uint8_t *)base + offset;

    return (uint32_t)bytes[0] |
        ((uint32_t)bytes[1] << 8) |
        ((uint32_t)bytes[2] << 16) |
        ((uint32_t)bytes[3] << 24);
}

static void Game_WriteDatalinkLobbyNetworkWord(
    void *base,
    uint32_t offset,
    uint32_t value)
{
    uint8_t *bytes = (uint8_t *)base + offset;

    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
    bytes[2] = (uint8_t)(value >> 16);
    bytes[3] = (uint8_t)(value >> 24);
}

static uint8_t *Game_GetDatalinkLobbyNetworkOutput(void)
{
    return (uint8_t *)(void *)gGameDatalinkPostOamOutput;
}

/*
 * 0x020B2898..0x020B291B: complete 132-byte body. Its link-state, input
 * history, edge and default-input globals at 0x020B291C are excluded.
 */
void Game_UpdateDatalinkLobbyInputEdges_020b2898(void)
{
    uint8_t *link_state = (uint8_t *)(void *)&gGameLinkOperationState_0212c9a8;
    unsigned int slot;

    (void)Game_GetDatalinkWirelessContext_020909c8();
    (void)Game_GetDatalinkLobbyGraphicsManager_0208da4c();
    for (slot = 0; slot < GAME_DATALINK_LOBBY_SYNC_SLOT_COUNT; ++slot) {
        uint32_t offset = DATALINK_LOBBY_LINK_SLOT_INPUT_OFFSET +
            slot * DATALINK_LOBBY_LINK_SLOT_STRIDE;
        uint16_t current = Game_ReadDatalinkLobbyNetworkHalfword(
            link_state,
            offset
        );
        uint16_t previous = gGameDatalinkLobbyPreviousInputs_0212c584[slot];

        gGameDatalinkLobbyInputEdges_0212c58c[slot] =
            (uint16_t)(current & (current ^ previous));
        gGameDatalinkLobbyPreviousInputs_0212c584[slot] = current;
    }

    if (Game_IsDatalinkLobbyRuntimeAvailable_0208de54() == 0) {
        gGameDatalinkLobbyInputEdges_0212c58c[0] =
            gGameDatalinkLobbyDefaultInput_0212c514;
        Game_WriteDatalinkLobbyNetworkHalfword(
            link_state,
            DATALINK_LOBBY_LINK_SLOT_INPUT_OFFSET,
            gGameDatalinkLobbyDefaultInput_0212c514
        );
    }
    Game_WriteDatalinkLobbyNetworkHalfword(
        link_state,
        DATALINK_LOBBY_LINK_LOCAL_INPUT_OFFSET,
        gGameDatalinkLobbyDefaultInput_0212c514
    );
}

static int Game_DatalinkLobbyPlayerNamesEqual(
    const uint16_t *first,
    const uint16_t *second,
    uint32_t length)
{
    uint32_t index;

    for (index = 0; index < length; ++index) {
        if (first[index] != second[index]) {
            return 0;
        }
    }
    return 1;
}

static const uint16_t *Game_GetDatalinkLobbyParticipantName(
    unsigned int participant)
{
    return (const uint16_t *)(uintptr_t)(
        (uintptr_t)&gGameDatalinkPostOamParticipants[participant] +
        DATALINK_LOBBY_PARTICIPANT_NAME_OFFSET
    );
}

static void Game_ClearDatalinkLobbyPlayerNameDirtyFlag(
    unsigned int participant)
{
    Game_WriteDatalinkLobbyNetworkWord(
        &gGameDatalinkOamManager,
        DATALINK_LOBBY_RENDER_NAME_DIRTY_OFFSET + participant * 4U,
        0
    );
}

/*
 * 0x020B292C..0x020B2A47: complete 284-byte body. Its output, participant,
 * name-cache, render-state and lobby-state globals at 0x020B2A48 are excluded.
 */
void Game_SynchronizeDatalinkLobbyPlayerNames_020b292c(void)
{
    Game_DatalinkLobbySyncState *state =
        Game_GetDatalinkLobbyNetworkState();
    unsigned int participant;

    for (participant = 0;
         participant < GAME_DATALINK_LOBBY_SYNC_SLOT_COUNT;
         ++participant) {
        uint8_t *output = Game_GetDatalinkLobbyNetworkOutput();
        uint32_t output_offset = DATALINK_LOBBY_OUTPUT_PARTICIPANT_OFFSET +
            participant * DATALINK_LOBBY_OUTPUT_PARTICIPANT_STRIDE;
        uint16_t *cached_name =
            &gGameDatalinkLobbyPlayerNameCache_02181868[participant][0];

        Game_RefreshDatalinkLobbyParticipantRecord_020a31ac(
            (int)participant,
            output + output_offset
        );
        if ((Game_ReadDatalinkLobbyNetworkWord(output, output_offset) & 1U) !=
            0U) {
            const uint16_t *name =
                Game_GetDatalinkLobbyParticipantName(participant);
            uint32_t cached_length = STD_GetStringLength(cached_name);
            uint32_t name_length = STD_GetStringLength(name);

            if (cached_length != name_length ||
                Game_DatalinkLobbyPlayerNamesEqual(
                    name,
                    cached_name,
                    name_length
                ) == 0) {
                Game_ClearDatalinkLobbyPlayerNameDirtyFlag(participant);
                MI_CpuCopy8(
                    name,
                    cached_name,
                    name_length * sizeof(uint16_t)
                );
            }
        } else if (state->participant_present_14[participant] != 0U) {
            MI_CpuFill(
                cached_name,
                0,
                GAME_DATALINK_LOBBY_PLAYER_NAME_UNITS * sizeof(uint16_t)
            );
            Game_ClearDatalinkLobbyPlayerNameDirtyFlag(participant);
        }
    }
}

/*
 * 0x020B2A5C..0x020B2A87: complete 44-byte body. Its scratch-buffer and
 * link-operation-state globals at 0x020B2A88 are excluded.
 */
void Game_StartDatalinkLobbyLinkOperation_020b2a5c(void)
{
    uint8_t *link_state = (uint8_t *)(void *)&gGameLinkOperationState_0212c9a8;

    MIi_CpuClear32(
        0,
        gGameDatalinkLobbyLinkScratch_0212c7f0,
        GAME_DATALINK_LOBBY_LINK_SCRATCH_SIZE
    );
    Game_StartDatalinkLinkOperation_02028078(
        DATALINK_LOBBY_LINK_OPERATION_MODE
    );
    link_state[DATALINK_LOBBY_LINK_ACTIVE_OFFSET] = 1;
}
