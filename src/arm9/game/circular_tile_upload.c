#include "game/circular_tile_upload.h"

/*
 * 0x020bc43c.  Destination space wraps every 256 rows (512 bytes each),
 * while source indices wrap by the buffer's power-of-two row count.  Calls
 * are split exactly at a destination wrap so the SDK upload callback always
 * receives one contiguous segment.
 */
void Game_UploadCircularTileRows(
    const GameCircularTileBuffer *buffer,
    const GameCircularTileUploadRange *range,
    GameCircularTileUploadCallback upload)
{
    int32_t start = buffer->visible_start;
    int32_t end = buffer->previous_start;
    int32_t cursor;
    int32_t finish;
    int32_t remaining;
    uint32_t wrapped;
    uint32_t source_mask = (uint32_t)(int32_t)buffer->ring_rows - 1u;

    if (end > start) {
        cursor = (int16_t)(start + range->leading_offset);
        finish = (int16_t)(end + range->leading_offset);
    } else {
        finish = (int16_t)(start + range->wrapped_offset);
        cursor = (int16_t)(end + range->wrapped_offset);
        if (buffer->previous_end <= cursor) {
            cursor = buffer->previous_end;
        }
        {
            int32_t leading =
                (int16_t)(start + range->leading_offset);
            if (cursor <= leading) {
                cursor = leading;
            }
        }
    }

    remaining = (int16_t)(finish - cursor);
    wrapped = (uint32_t)cursor & source_mask;
    while (remaining > 0) {
        uint32_t destination_row = wrapped & 0xffu;
        int32_t chunk_rows = (int32_t)(0x100u - destination_row);

        if (remaining <= chunk_rows) {
            chunk_rows = remaining;
        }
        upload(
            buffer->rows + wrapped * (uint32_t)(int32_t)buffer->row_stride,
            destination_row << 9,
            (uint32_t)chunk_rows << 9);
        remaining -= chunk_rows;
        wrapped = (wrapped + (uint32_t)chunk_rows) & source_mask;
    }
}
