#include "game/datalink_reader.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static void WriteU16Le(uint8_t *out, uint16_t value)
{
    out[0] = (uint8_t)value;
    out[1] = (uint8_t)(value >> 8);
}

static void WriteU32Le(uint8_t *out, uint32_t value)
{
    out[0] = (uint8_t)value;
    out[1] = (uint8_t)(value >> 8);
    out[2] = (uint8_t)(value >> 16);
    out[3] = (uint8_t)(value >> 24);
}

int main(void)
{
    static const uint8_t check[] = "123456789";
    uint8_t file[37] = {0};
    DatalinkReader reader;
    uint16_t crc;

    assert(DatalinkReader_CalculateCrc16(check, 9) == 0x906e);

    memcpy(file, "DtLk", 4);
    file[4] = 1;
    file[5] = 1;
    WriteU16Le(file + 6, 2);
    WriteU32Le(file + 8, sizeof(file));
    WriteU32Le(file + 0x10, 16);
    WriteU32Le(file + 0x14, 3);
    WriteU32Le(file + 0x18, 19);
    WriteU32Le(file + 0x1c, 2);
    memcpy(file + 0x20, "abcde", 5);
    crc = DatalinkReader_CalculateCrc16(file + 0x20, 5);
    WriteU16Le(file + 0x0c, crc);

    DatalinkReader_Init(&reader);
    assert(!reader.is_valid);
    assert(DatalinkReader_ValidateBuffer(file, sizeof(file)));
    assert(DatalinkReader_Open(&reader, file, sizeof(file)));
    assert(reader.size == sizeof(file));
    assert(DatalinkReader_GetDataSize(&reader, 0) == 3);
    assert(DatalinkReader_GetDataSize(&reader, 1) == 2);
    assert(memcmp(DatalinkReader_GetDataBuffer(&reader, 0), "abc", 3) == 0);
    assert(memcmp(DatalinkReader_GetDataBuffer(&reader, 1), "de", 2) == 0);

    file[0x20] ^= 1;
    assert(!DatalinkReader_ValidateBuffer(file, sizeof(file)));
    assert(!DatalinkReader_Open(&reader, file, sizeof(file)));
    assert(!reader.is_valid);
    assert(DatalinkReader_Destroy(&reader) == &reader);
    return 0;
}
