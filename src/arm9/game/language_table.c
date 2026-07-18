#include "game/language_table.h"

enum {
    GAME_LANGUAGE_RESOURCE_TYPE = 0x6e746c69, /* "iltn" resource path */
    GAME_LANGUAGE_FILE_SIGNATURE = 0x4e544c49, /* "NTLI" */
    GAME_LANGUAGE_BLOCK_SIGNATURE = 0x6d746c31, /* "mtl1" */
    NTMV_BINARY_BYTE_ORDER = 0xfeff,
    GAME_LANGUAGE_FILE_VERSION = 0x0200
};

typedef struct NtmvM2dBinaryFileHeader {
    uint8_t signature[4];
    uint16_t byte_order;
    uint16_t version;
    uint32_t file_size;
    uint16_t header_size;
    uint16_t block_count;
} NtmvM2dBinaryFileHeader;

typedef struct GameLanguageTableBlock {
    uint8_t signature[4];
    uint32_t block_size;
    uint16_t count;
    uint16_t unknown_0a;
    uint16_t locale_ids[];
} GameLanguageTableBlock;

static uint32_t ReadTag(const uint8_t bytes[4])
{
    return ((uint32_t)bytes[0] << 24) |
        ((uint32_t)bytes[1] << 16) |
        ((uint32_t)bytes[2] << 8) |
        bytes[3];
}

/* 0x020b7378 */
bool NtmvM2dBinaryFile_IsValid(const void *file, uint32_t signature)
{
    const NtmvM2dBinaryFileHeader *header = file;
    return ReadTag(header->signature) == signature &&
        header->byte_order == NTMV_BINARY_BYTE_ORDER;
}

/* 0x020bf09c */
bool GameLanguageTable_Load(
    GameLanguageTable *table,
    const char *name,
    NtmvM2dArcResourceAccessor *accessor)
{
    const NtmvM2dBinaryFileHeader *header =
        accessor->vtable->get_file(
            accessor, GAME_LANGUAGE_RESOURCE_TYPE, name);
    const uint8_t *cursor;
    uint16_t index;

    if (header == 0 ||
        !NtmvM2dBinaryFile_IsValid(
            header, GAME_LANGUAGE_FILE_SIGNATURE) ||
        header->version != GAME_LANGUAGE_FILE_VERSION) {
        return false;
    }

    cursor = (const uint8_t *)header + header->header_size;
    for (index = 0; index < header->block_count; ++index) {
        const GameLanguageTableBlock *block =
            (const GameLanguageTableBlock *)cursor;
        if (ReadTag(block->signature) ==
            GAME_LANGUAGE_BLOCK_SIGNATURE) {
            table->count = block->count;
            table->locale_ids = block->locale_ids;
            return true;
        }
        cursor += block->block_size;
    }
    return false;
}
