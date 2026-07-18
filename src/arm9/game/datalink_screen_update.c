#include "game/datalink_screen_update.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_SCREEN_MAP_ADDRESS = 0x05000400,
    DATALINK_CHARACTER_BUFFER_OFFSET = 0x400,
    DATALINK_CHARACTER_TRANSFER_SIZE = 0x3000,
    DATALINK_SCREEN_TILEMAP_WIDTH = 32,
    DATALINK_SCREEN_BUFFER_SIZE = 0x600,
    DATALINK_SCREEN_TRANSFER_TARGET = 0,
    DATALINK_SCREEN_TRANSFER_OFFSET = 0,
    DATALINK_CHARACTER_TRANSFER_OFFSET = 0x400,
    DATALINK_SCREEN_METADATA_SIZE = 0x38
};

/* Opaque 0x020144B8 cache-range flush. */
extern void Game_FlushDatalinkRenderRange(
    const void *buffer,
    uint32_t size
);

/* Opaque 0x0209DA7C background-screen transfer. */
extern void Game_TransferDatalinkRenderBuffer(
    int target,
    const void *source,
    uint32_t destination_offset,
    uint32_t size
);
extern void *G2S_GetBG0CharPtr(void);
extern void *G2S_GetBG0ScrPtr(void);
extern void MI_CpuFill(void *destination, uint32_t value, uint32_t size);
extern void *Game_GetDatalinkRenderContext(void);
extern void Game_LoadDatalinkScreenMetadata(
    uint16_t resource_id,
    void *metadata);
extern void Game_ConfigureDatalinkScreenRect(
    int8_t *clear_x,
    int8_t *clear_y,
    int8_t *clear_width,
    int8_t *clear_height,
    uint16_t resource_id,
    void *metadata);
extern void Game_PlayDatalinkScreenCue(uint16_t cue_id);
extern void Game_TransferDatalinkCharacterBuffer(
    int target,
    const void *source,
    uint32_t destination_offset,
    uint32_t size);

_Static_assert(
    offsetof(Game_DatalinkScreenUpdateState, saved_shared_value_04) == 0x04,
    "datalink screen saved-value offset"
);
_Static_assert(
    offsetof(
        Game_DatalinkScreenUpdateState,
        screen_buffer_requires_upload_08
    ) == 0x08,
    "datalink screen upload flag offset"
);
_Static_assert(
    offsetof(Game_DatalinkScreenUpdateState, clear_x_0c) == 0x0C,
    "datalink screen clear X offset"
);
_Static_assert(
    offsetof(Game_DatalinkScreenUpdateState, clear_height_0f) == 0x0F,
    "datalink screen clear height offset"
);
_Static_assert(
    offsetof(Game_DatalinkScreenUpdateState,
             character_buffer_requires_upload_10) == 0x10,
    "datalink character upload flag offset"
);
#if UINTPTR_MAX == UINT32_MAX
_Static_assert(
    offsetof(Game_DatalinkScreenUpdateState, screen_buffer_48) == 0x48,
    "datalink screen buffer offset"
);
_Static_assert(
    offsetof(Game_DatalinkScreenUpdateState, character_buffer_4c) == 0x4C,
    "datalink character buffer offset"
);
_Static_assert(
    offsetof(Game_DatalinkScreenUpdateState, display_control_58) == 0x58,
    "datalink display control offset"
);
_Static_assert(
    offsetof(Game_DatalinkScreenUpdateState, setup_active_68) == 0x68,
    "datalink setup active offset"
);
_Static_assert(
    offsetof(Game_DatalinkScreenUpdateState, participant_count_6c) == 0x6C,
    "datalink participant count offset"
);
_Static_assert(
    offsetof(Game_DatalinkScreenUpdateState, render_context_member_7c) == 0x7C,
    "datalink render context member offset"
);
#endif

static volatile uint16_t *DatalinkScreenMap(void)
{
    return (volatile uint16_t *)(uintptr_t)DATALINK_SCREEN_MAP_ADDRESS;
}

static void WriteScreenMapByteOffset(uint32_t byte_offset, uint16_t value)
{
    DatalinkScreenMap()[byte_offset / sizeof(uint16_t)] = value;
}

static void ConfigureStaticScreenTiles(uint16_t layout_flags)
{
    uint32_t layout = (uint32_t)layout_flags >> 4;

    switch (layout) {
    case 0:
        WriteScreenMapByteOffset(0x1c2, 0x0cd0);
        WriteScreenMapByteOffset(0x1c4, 0x0cd6);
        WriteScreenMapByteOffset(0x1c6, 0x0cfc);
        WriteScreenMapByteOffset(0x1c8, 0x0d1f);
        WriteScreenMapByteOffset(0x1ca, 0x2f1f);
        WriteScreenMapByteOffset(0x1d2, 0x0c91);
        WriteScreenMapByteOffset(0x1d4, 0x0c98);
        WriteScreenMapByteOffset(0x1d6, 0x0c9e);
        break;
    case 1:
        WriteScreenMapByteOffset(0x1c2, 0x4863);
        WriteScreenMapByteOffset(0x1c4, 0x6524);
        WriteScreenMapByteOffset(0x1c6, 0x7e44);
        WriteScreenMapByteOffset(0x1c8, 0x7f25);
        WriteScreenMapByteOffset(0x1ca, 0x7fb5);
        break;
    case 2:
        WriteScreenMapByteOffset(0x1c2, 0x0dc3);
        WriteScreenMapByteOffset(0x1c4, 0x0ec9);
        WriteScreenMapByteOffset(0x1c6, 0x0f6c);
        WriteScreenMapByteOffset(0x1c8, 0x0fac);
        WriteScreenMapByteOffset(0x1ca, 0x4ff8);
        break;
    default:
        break;
    }

    WriteScreenMapByteOffset(0x1dc, 0x7fff);
    WriteScreenMapByteOffset(0x1de, 0x0c63);
}

/* 0x020AC038 */
void Game_ChangeDatalinkState(
    int state_id,
    uint32_t participant_count,
    uintptr_t character_buffer_address,
    uintptr_t screen_buffer_address)
{
    Game_DatalinkScreenUpdateState *state =
        &gGameDatalinkScreenUpdateState;
    uintptr_t character_buffer;
    const Game_DatalinkScreenConfigRecord *record =
        &gGameDatalinkScreenConfigRecords[state_id];
    void *metadata;

    if (character_buffer_address == 0U) {
        character_buffer =
            (uintptr_t)G2S_GetBG0CharPtr() + DATALINK_CHARACTER_BUFFER_OFFSET;
        state->character_buffer_requires_upload_10 = 0U;
    } else {
        character_buffer = character_buffer_address;
        state->character_buffer_requires_upload_10 = 1U;
    }

    if (screen_buffer_address == 0U) {
        screen_buffer_address = (uintptr_t)G2S_GetBG0ScrPtr();
        state->screen_buffer_requires_upload_08 = 0U;
    } else {
        state->screen_buffer_requires_upload_08 = 1U;
    }

    MI_CpuFill(
        &state->screen_buffer_48,
        0,
        DATALINK_SCREEN_METADATA_SIZE);
    state->screen_buffer_48 = (uint16_t *)screen_buffer_address;
    state->character_buffer_4c = (void *)character_buffer;
    state->display_control_58 = 0xe020;
    state->render_context_member_7c =
        (uint8_t *)Game_GetDatalinkRenderContext() + 0x1bc;
    state->setup_bytes_5a[0] = 0xe7;
    state->setup_bytes_5a[1] = 0x11;
    state->setup_bytes_5a[3] = 1;
    state->setup_bytes_5a[4] = 2;
    state->setup_bytes_5a[5] = 0x0f;
    state->setup_bytes_5a[6] = 6;
    state->setup_bytes_5a[7] = 10;
    state->setup_bytes_5a[8] = 0;
    state->setup_bytes_5a[9] = 0xf0;
    state->setup_active_68 = 1U;
    state->participant_count_6c = participant_count;

    ConfigureStaticScreenTiles(record->layout_flags);
    metadata = &state->screen_buffer_48;
    Game_LoadDatalinkScreenMetadata(record->resource_id, metadata);
    Game_ConfigureDatalinkScreenRect(
        &state->clear_x_0c,
        &state->clear_y_0d,
        &state->clear_width_0e,
        &state->clear_height_0f,
        record->resource_id,
        metadata);
    Game_PlayDatalinkScreenCue(record->cue_id);

    state->saved_shared_value_04 = gGameDatalinkScreenSharedValue;
    gGameDatalinkScreenSharedValue = 0U;

    if (state->character_buffer_requires_upload_10 != 0U) {
        Game_FlushDatalinkRenderRange(
            (const void *)character_buffer_address,
            DATALINK_CHARACTER_TRANSFER_SIZE);
        Game_TransferDatalinkCharacterBuffer(
            DATALINK_SCREEN_TRANSFER_TARGET,
            (const void *)character_buffer_address,
            DATALINK_CHARACTER_TRANSFER_OFFSET,
            DATALINK_CHARACTER_TRANSFER_SIZE);
    }
    if (state->screen_buffer_requires_upload_08 != 0U) {
        Game_FlushDatalinkRenderRange(
            (const void *)character_buffer_address,
            DATALINK_SCREEN_BUFFER_SIZE);
        Game_TransferDatalinkRenderBuffer(
            DATALINK_SCREEN_TRANSFER_TARGET,
            (const void *)screen_buffer_address,
            DATALINK_SCREEN_TRANSFER_OFFSET,
            DATALINK_SCREEN_BUFFER_SIZE);
    }
}

/*
 * 0x020AC308
 *
 * Complete 192-byte screen-update commit through 0x020AC3C7. The two-word
 * literal pool at 0x020AC3C8..0x020AC3CF and the independent function at
 * address 0x020AC3D0 are excluded.
 */
void Game_CommitDatalinkScreenUpdate(void)
{
    Game_DatalinkScreenUpdateState *state =
        &gGameDatalinkScreenUpdateState;
    uint16_t *screen_buffer = state->screen_buffer_48;
    uint32_t row;

    for (row = 0U;
         row < (uint32_t)(int32_t)state->clear_height_0f;
         ++row) {
        uint32_t column;

        for (column = 0U;
             column < (uint32_t)(int32_t)state->clear_width_0e;
             ++column) {
            int32_t x = (int32_t)state->clear_x_0c + (int32_t)column;
            int32_t y = (int32_t)state->clear_y_0d + (int32_t)row;

            screen_buffer[x + y * DATALINK_SCREEN_TILEMAP_WIDTH] = 0U;
        }
    }

    if (state->screen_buffer_requires_upload_08 != 0U) {
        Game_FlushDatalinkRenderRange(
            screen_buffer,
            DATALINK_SCREEN_BUFFER_SIZE
        );
        Game_TransferDatalinkRenderBuffer(
            DATALINK_SCREEN_TRANSFER_TARGET,
            screen_buffer,
            DATALINK_SCREEN_TRANSFER_OFFSET,
            DATALINK_SCREEN_BUFFER_SIZE
        );
        state->screen_buffer_requires_upload_08 = 0U;
    }

    gGameDatalinkScreenSharedValue = state->saved_shared_value_04;
}
