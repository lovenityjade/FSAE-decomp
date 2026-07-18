#include "game/datalink_reader.h"

#include <assert.h>
#include <string.h>

static const uint8_t sDatalinkMagic[4] = {'D', 't', 'L', 'k'};

static uint16_t ReadU16Le(const uint8_t *data)
{
    return (uint16_t)((uint16_t)data[0] | (uint16_t)((uint16_t)data[1] << 8));
}

static uint32_t ReadU32Le(const uint8_t *data)
{
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

/*
 * 0x020a6278
 *
 * The retail routine assumes that buffer points at a readable header.  It
 * checks the signature, version bytes, total size and payload CRC.  The table
 * itself is deliberately excluded from the CRC span.
 */
bool DatalinkReader_ValidateBuffer(const void *buffer, uint32_t size)
{
    const uint8_t *bytes = (const uint8_t *)buffer;
    uint32_t data_offset;
    uint16_t entry_count;

    if (bytes == NULL) {
        return false;
    }
    if (memcmp(bytes, sDatalinkMagic, sizeof(sDatalinkMagic)) != 0) {
        return false;
    }
    if (bytes[4] != DATALINK_FORMAT_MAJOR || bytes[5] != DATALINK_FORMAT_MINOR) {
        return false;
    }
    if (ReadU32Le(bytes + 8) != size) {
        return false;
    }

    entry_count = ReadU16Le(bytes + 6);
    data_offset = DATALINK_HEADER_SIZE + (uint32_t)entry_count * DATALINK_ENTRY_SIZE;
    return ReadU16Le(bytes + 0x0c) ==
           DatalinkReader_CalculateCrc16(bytes + data_offset, size - data_offset);
}

/* 0x020a6304: reflected CRC-16/X-25, polynomial 0x8408. */
uint16_t DatalinkReader_CalculateCrc16(const void *data, uint32_t size)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t crc = 0xffff;
    uint32_t byte_index;

    for (byte_index = 0; byte_index < size; ++byte_index) {
        unsigned int bit;

        crc ^= bytes[byte_index];
        for (bit = 0; bit < 8; ++bit) {
            if ((crc & 1) != 0) {
                crc = (crc >> 1) ^ 0x8408;
            } else {
                crc >>= 1;
            }
        }
    }
    return (uint16_t)(crc ^ 0xffff);
}

/* 0x020a636c: recovered default constructor. */
void DatalinkReader_Init(DatalinkReader *reader)
{
    reader->buffer = NULL;
    reader->entry_table = NULL;
    reader->size = 0;
    reader->is_valid = 0;
}

/* 0x020a6384: thin destructor wrapper around the reset routine. */
DatalinkReader *DatalinkReader_Destroy(DatalinkReader *reader)
{
    DatalinkReader_Reset(reader);
    return reader;
}

/* 0x020a6398 */
bool DatalinkReader_Open(DatalinkReader *reader, const void *buffer, uint32_t size)
{
    DatalinkReader_Reset(reader);
    if (!DatalinkReader_ValidateBuffer(buffer, size)) {
        return false;
    }

    reader->buffer = (const uint8_t *)buffer;
    reader->entry_table = reader->buffer + DATALINK_HEADER_SIZE;
    reader->size = size;
    reader->is_valid = 1;
    return true;
}

/* 0x020a63d8 */
void DatalinkReader_Reset(DatalinkReader *reader)
{
    reader->buffer = NULL;
    reader->entry_table = NULL;
    reader->size = 0;
    reader->is_valid = 0;
}

/*
 * 0x020a63f0, assertion strings at 0x02124a71/0x02124a88/0x02124a94.
 * The original asserts the member named "validate" and performs no index
 * bounds check.  Each stored data offset is relative to entry_table.
 */
const void *DatalinkReader_GetDataBuffer(const DatalinkReader *reader, uint32_t index)
{
    const uint8_t *entry;

    assert(reader->is_valid);
    entry = reader->entry_table + index * DATALINK_ENTRY_SIZE;
    return reader->entry_table + ReadU32Le(entry);
}

/* 0x020a6438, assertion function string at 0x02124a65. */
uint32_t DatalinkReader_GetDataSize(const DatalinkReader *reader, uint32_t index)
{
    const uint8_t *entry;

    assert(reader->is_valid);
    entry = reader->entry_table + index * DATALINK_ENTRY_SIZE;
    return ReadU32Le(entry + 4);
}
