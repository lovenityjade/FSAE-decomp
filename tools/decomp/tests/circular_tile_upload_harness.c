#include "game/circular_tile_upload.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct UploadRecord {
    ptrdiff_t source_halfword;
    uint32_t destination_offset;
    uint32_t byte_count;
} UploadRecord;

static uint16_t sRows[2048];
static UploadRecord sRecords[4];
static unsigned int sRecordCount;

static void Upload(
    const uint16_t *source,
    uint32_t destination_offset,
    uint32_t byte_count)
{
    UploadRecord *record;
    assert(sRecordCount < 4);
    record = &sRecords[sRecordCount++];
    record->source_halfword = source - sRows;
    record->destination_offset = destination_offset;
    record->byte_count = byte_count;
}

static void Reset(void)
{
    memset(sRecords, 0, sizeof(sRecords));
    sRecordCount = 0;
}

int main(void)
{
    GameCircularTileBuffer buffer;
    GameCircularTileUploadRange range;

    memset(&buffer, 0, sizeof(buffer));
    buffer.rows = sRows;
    buffer.row_stride = 2;
    buffer.ring_rows = 512;
    buffer.previous_end = 511;

    /* Linear interval: [start-2, end-2). */
    range.leading_offset = -2;
    range.wrapped_offset = 3;
    buffer.visible_start = 10;
    buffer.previous_start = 20;
    Game_UploadCircularTileRows(&buffer, &range, Upload);
    assert(sRecordCount == 1);
    assert(sRecords[0].source_halfword == 16);
    assert(sRecords[0].destination_offset == 8u * 0x200u);
    assert(sRecords[0].byte_count == 10u * 0x200u);

    /* One interval crossing destination row 255 is split into 6 + 14. */
    Reset();
    buffer.visible_start = 252;
    buffer.previous_start = 272;
    Game_UploadCircularTileRows(&buffer, &range, Upload);
    assert(sRecordCount == 2);
    assert(sRecords[0].source_halfword == 500);
    assert(sRecords[0].destination_offset == 250u * 0x200u);
    assert(sRecords[0].byte_count == 6u * 0x200u);
    assert(sRecords[1].source_halfword == 512);
    assert(sRecords[1].destination_offset == 0);
    assert(sRecords[1].byte_count == 14u * 0x200u);

    /* Wrapped branch applies row_limit, then the leading lower bound. */
    Reset();
    buffer.visible_start = 50;
    buffer.previous_start = 5;
    buffer.previous_end = 12;
    Game_UploadCircularTileRows(&buffer, &range, Upload);
    assert(sRecordCount == 1);
    assert(sRecords[0].source_halfword == 96);
    assert(sRecords[0].destination_offset == 48u * 0x200u);
    assert(sRecords[0].byte_count == 5u * 0x200u);

    /* A non-positive signed 16-bit interval performs no upload. */
    Reset();
    range.leading_offset = 4;
    buffer.visible_start = 20;
    buffer.previous_start = 20;
    buffer.previous_end = 0;
    range.wrapped_offset = 0;
    Game_UploadCircularTileRows(&buffer, &range, Upload);
    assert(sRecordCount == 0);
    return 0;
}
