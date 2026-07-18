#include "game/datalink_renderer_ready_row.h"

#include <stddef.h>

enum {
    DATALINK_ROW_STRIDE = 6,
    DATALINK_ROW_METADATA_RESOURCE_BASE = 0x2328,
    DATALINK_READY_GLYPH_X = 0x13,
    DATALINK_READY_GLYPH_ROW_OFFSET = 4,
    DATALINK_READY_GLYPH_PALETTE = 3,
    DATALINK_READY_GLYPH_TILE = 0x78,
    DATALINK_READY_TILE_X = 0x15,
    DATALINK_READY_TILE_ROW_OFFSET = 5,
    DATALINK_READY_TILE_PALETTE = 1,
    DATALINK_READY_TILE_ID = 0x55,
    DATALINK_FLAG_VALUE_2 = 0x0002,
    DATALINK_FLAG_VALUE_3 = 0x0001,
    DATALINK_FLAG_VALUE_4 = 0x0200,
    DATALINK_FLAG_VALUE_5 = 0x0400,
    DATALINK_FLAG_VALUE_6 = 0x1000
};

/* Opaque 0x0209B3D8 two-by-two tile-glyph writer. */
extern void Game_SetDatalink2x2RenderGlyph(
    uint16_t *tilemap,
    int x,
    int y,
    int palette,
    uint16_t tile_id
);

/* Opaque 0x0209B394 single-tile writer. */
extern void Game_SetDatalinkRenderTile(
    uint16_t *tilemap,
    int x,
    int y,
    int palette,
    uint16_t tile_id
);

/* Host-safe representation of the BLX through descriptor field +0x04. */
extern void Game_InvokeDatalinkRenderCallback(
    uint32_t callback_address,
    uint8_t callback_argument,
    int row,
    uint32_t value,
    uint16_t *tilemap
);

_Static_assert(
    sizeof(Game_DatalinkRenderRowMetadata) == 0x34,
    "datalink render-row metadata stride"
);
_Static_assert(
    offsetof(Game_DatalinkReadyRowRecord, packed_values_05) == 0x05,
    "datalink ready-row packed-values offset"
);
_Static_assert(
    offsetof(Game_DatalinkReadyRowRecord, value_06) == 0x06,
    "datalink ready-row halfword offset"
);
_Static_assert(
    offsetof(Game_DatalinkReadyRowRecord, value_18) == 0x18,
    "datalink ready-row word offset"
);
_Static_assert(
    sizeof(Game_DatalinkReadyRowRecord) == 0x50,
    "datalink ready-row record stride"
);

/*
 * Semantic internal helper for the ready branch
 * Range: 0x020A89F0..0x020A8B30 inside catalogued FUN_020A8904.
 * The ten-value array is the zeroed parent local prepared immediately before
 * this branch.  Conditional stores therefore deliberately leave absent
 * fields untouched at zero.
 *
 * The primary tilemap binding is read separately for the two fixed glyph
 * operations, then reloaded once after all record reads and retained across
 * the ten descriptor callbacks.  Callback addresses remain raw target
 * uint32_t values and are invoked through a host-safe semantic dispatcher.
 * This helper is not promoted in Ghidra.
 */
void Game_RenderReadyDatalinkRow(
    Game_DatalinkRenderRowMetadata metadata[3],
    const volatile Game_DatalinkReadyRowRecord *record,
    const Game_DatalinkRenderDescriptor
        descriptors[GAME_DATALINK_RENDER_DESCRIPTOR_COUNT],
    volatile Game_DatalinkRenderPrimaryBinding *buffers,
    uint32_t values[GAME_DATALINK_RENDER_DESCRIPTOR_COUNT],
    uint8_t slot
)
{
    int row_base = slot * DATALINK_ROW_STRIDE;
    uint16_t *callback_tilemap;
    uint32_t extracted;
    int index;

    metadata[slot].resource_id_00 =
        DATALINK_ROW_METADATA_RESOURCE_BASE + slot;

    Game_SetDatalink2x2RenderGlyph(
        (uint16_t *)(void *)buffers->primary_buffer,
        DATALINK_READY_GLYPH_X,
        row_base + DATALINK_READY_GLYPH_ROW_OFFSET,
        DATALINK_READY_GLYPH_PALETTE,
        DATALINK_READY_GLYPH_TILE
    );
    Game_SetDatalinkRenderTile(
        (uint16_t *)(void *)buffers->primary_buffer,
        DATALINK_READY_TILE_X,
        row_base + DATALINK_READY_TILE_ROW_OFFSET,
        DATALINK_READY_TILE_PALETTE,
        DATALINK_READY_TILE_ID
    );

    values[0] = record->value_18;
    values[1] = record->value_06;
    if ((record->flags_00 & DATALINK_FLAG_VALUE_2) != 0U) {
        values[2] = 1;
    }
    if ((record->flags_00 & DATALINK_FLAG_VALUE_3) != 0U) {
        values[3] = 1;
    }
    if ((record->flags_00 & DATALINK_FLAG_VALUE_4) != 0U) {
        values[4] = 1;
    }
    if ((record->flags_00 & DATALINK_FLAG_VALUE_5) != 0U) {
        values[5] = 1;
    }
    if ((record->flags_00 & DATALINK_FLAG_VALUE_6) != 0U) {
        values[6] = 1;
    }

    extracted = (record->packed_values_05 >> 2) & 3U;
    if (extracted != 0U) {
        values[7] = extracted;
    }
    extracted = record->packed_values_05 & 3U;
    if (extracted != 0U) {
        values[8] = extracted;
    }
    extracted = (record->packed_values_05 >> 4) & 3U;
    if (extracted != 0U) {
        values[9] = extracted;
    }

    callback_tilemap = (uint16_t *)(void *)buffers->primary_buffer;
    for (index = 0;
         index < GAME_DATALINK_RENDER_DESCRIPTOR_COUNT;
         ++index) {
        Game_InvokeDatalinkRenderCallback(
            descriptors[index].callback_address_04,
            descriptors[index].callback_argument_00,
            row_base + descriptors[index].tile_offset_01,
            values[index],
            callback_tilemap
        );
    }
}
