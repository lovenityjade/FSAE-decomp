#ifndef FSAE_GAME_DATALINK_READER_H
#define FSAE_GAME_DATALINK_READER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_HEADER_SIZE = 0x10,
    DATALINK_ENTRY_SIZE = 0x08,
    DATALINK_FORMAT_MAJOR = 1,
    DATALINK_FORMAT_MINOR = 1
};

/*
 * Runtime view reconstructed from DatalinkReader.cpp.  On ARM9 the fields are
 * at +0x00, +0x04, +0x08 and +0x0c respectively and the object is 16 bytes.
 */
typedef struct DatalinkReader {
    const uint8_t *buffer;
    const uint8_t *entry_table;
    uint32_t size;
    uint8_t is_valid;
    uint8_t reserved[3];
} DatalinkReader;

#if UINTPTR_MAX == UINT32_MAX
typedef char DatalinkReaderTargetSizeCheck[sizeof(DatalinkReader) == 0x10 ? 1 : -1];
typedef char DatalinkReaderTableOffsetCheck[
    offsetof(DatalinkReader, entry_table) == 0x04 ? 1 : -1];
typedef char DatalinkReaderValidOffsetCheck[
    offsetof(DatalinkReader, is_valid) == 0x0c ? 1 : -1];
#endif

/* FUN_020a6278: validate a complete in-memory DtLk file. */
bool DatalinkReader_ValidateBuffer(const void *buffer, uint32_t size);

/* FUN_020a6304: CRC-16/X-25 over a byte span. */
uint16_t DatalinkReader_CalculateCrc16(const void *data, uint32_t size);

/* FUN_020a636c: default construction. */
void DatalinkReader_Init(DatalinkReader *reader);

/* FUN_020a6384: destruction wrapper; returns reader as in the recovered ABI. */
DatalinkReader *DatalinkReader_Destroy(DatalinkReader *reader);

/* FUN_020a6398: reset, validate and attach an in-memory DtLk file. */
bool DatalinkReader_Open(DatalinkReader *reader, const void *buffer, uint32_t size);

/* FUN_020a63d8: clear the runtime view without freeing the input buffer. */
void DatalinkReader_Reset(DatalinkReader *reader);

/* FUN_020a63f0 and FUN_020a6438. Offsets are relative to entry_table. */
const void *DatalinkReader_GetDataBuffer(const DatalinkReader *reader, uint32_t index);
uint32_t DatalinkReader_GetDataSize(const DatalinkReader *reader, uint32_t index);

#endif
