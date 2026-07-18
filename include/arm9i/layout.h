#ifndef FSAE_ARM9I_LAYOUT_H
#define FSAE_ARM9I_LAYOUT_H

#include <stdint.h>

enum {
    FSAE_ARM9I_FILE_SIZE = 0x4794,
    FSAE_ARM9I_PREFIX_SIZE = 4,
    FSAE_ARM9I_PAYLOAD_FILE_OFFSET = 4,
    FSAE_ARM9I_PAYLOAD_SIZE = 0x4780,
    FSAE_ARM9I_AUTOLOAD_FILE_OFFSET = 0x4784,
    FSAE_ARM9I_AUTOLOAD_SIZE = 16,
    FSAE_ARM9I_START_PADDING_SIZE = 32,
    FSAE_ARM9I_END_PADDING_SIZE = 20,
    FSAE_ARM9I_LOAD_ADDRESS = 0x021870c0,
    FSAE_ARM9I_BSS_SIZE = 0x00034420
};

typedef struct FsaeArm9iAutoloadRecord {
    uint32_t destination;
    uint32_t size;
    uint32_t static_init_start;
    uint32_t bss_size;
} FsaeArm9iAutoloadRecord;

typedef char FsaeArm9iAutoloadRecordSizeCheck[
    sizeof(FsaeArm9iAutoloadRecord) == FSAE_ARM9I_AUTOLOAD_SIZE ? 1 : -1];

extern const uint8_t gFsaeArm9iFilePrefix[FSAE_ARM9I_PREFIX_SIZE];
extern const uint8_t gFsaeArm9iStartPadding[FSAE_ARM9I_START_PADDING_SIZE];
extern const uint8_t gFsaeArm9iEndPadding[FSAE_ARM9I_END_PADDING_SIZE];
extern const FsaeArm9iAutoloadRecord gFsaeArm9iAutoload;

#endif
