#include "game/manual_page_flow.h"

#include "game/language_table.h"

#include <stddef.h>

enum {
    GAME_MANUAL_INDEX_RESOURCE_TYPE = 0x6e746d63,
    GAME_MANUAL_INDEX_FILE_SIGNATURE = 0x4e544d43,
    GAME_MANUAL_INDEX_FILE_VERSION = 0x0200,
    GAME_MANUAL_RECORD_BLOCK_SIGNATURE = 0x6d746331,
    GAME_MANUAL_NAME_POOL_SIGNATURE = 0x6e617031,
    GAME_MANUAL_TEXT_POOL_SIGNATURE = 0x74787031,
    GAME_MANUAL_NO_PAGE_NAME = 0xffff
};

typedef struct GameManualBinaryFileHeader {
    uint8_t signature[4];
    uint16_t byte_order;
    uint16_t version;
    uint32_t file_size;
    uint16_t header_size;
    uint16_t block_count;
} GameManualBinaryFileHeader;

typedef struct GameManualBlockHeader {
    uint8_t signature[4];
    uint32_t block_size;
} GameManualBlockHeader;

typedef struct GameManualPoolBlock {
    GameManualBlockHeader header;
    uint32_t entry_count;
    uint32_t offsets[];
} GameManualPoolBlock;

typedef struct GameManualRecordDefinition {
    uint16_t page_name_index;
    uint16_t toc_text_index;
    uint16_t title_text_index;
    uint16_t child_count;
    uint16_t first_child_name_index;
    uint16_t first_child_text_index;
} GameManualRecordDefinition;

typedef struct GameManualRecordBlock {
    GameManualBlockHeader header;
    uint16_t record_count;
    uint16_t reserved_0a;
    GameManualRecordDefinition definitions[];
} GameManualRecordBlock;

extern void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size);
extern void NNS_FndFreeToAllocator(void *allocator, void *allocation);

static uint32_t ReadTag(const uint8_t bytes[4])
{
    return ((uint32_t)bytes[0] << 24) |
        ((uint32_t)bytes[1] << 16) |
        ((uint32_t)bytes[2] << 8) |
        bytes[3];
}

static const void *PoolEntry(
    const GameManualPoolBlock *pool,
    uint32_t entry_index)
{
    const uint8_t *entries = (const uint8_t *)&pool->offsets[0];
    return entries + pool->offsets[entry_index];
}

/* 0x020b89e4 */
GameManualPageRecord *GameManualPageRecord_Construct_020b89e4(
    GameManualPageRecord *record)
{
    record->page_name = NULL;
    record->toc_text = NULL;
    record->title = NULL;
    record->subpage_names = NULL;
    record->subpage_texts = NULL;
    record->child_count = 0;
    return record;
}

/* 0x020b8a2c */
static void GameManualPageRecord_FreeTextPointers_020b8a2c(
    void *const *allocator_context,
    const uint16_t *const *pointers)
{
    if (pointers != NULL) {
        NNS_FndFreeToAllocator(
            *allocator_context, (void *)pointers);
    }
}

/* 0x020b8a44 */
static void GameManualPageRecord_FreeNamePointers_020b8a44(
    void *const *allocator_context,
    const char *const *pointers)
{
    if (pointers != NULL) {
        NNS_FndFreeToAllocator(
            *allocator_context, (void *)pointers);
    }
}

/* 0x020b8a04 */
void GameManualPageRecord_Destroy_020b8a04(
    GameManualPageRecord *record,
    void *const *allocator_context)
{
    GameManualPageRecord_FreeNamePointers_020b8a44(
        allocator_context, record->subpage_names);
    GameManualPageRecord_FreeTextPointers_020b8a2c(
        allocator_context, record->subpage_texts);
}

/* 0x020b8b68 */
static const uint16_t **GameManualPageRecord_AllocateTextPointers_020b8b68(
    void *const *allocator_context,
    uint32_t count)
{
    uint32_t allocation_count = count == 0 ? 1 : count;
    const uint16_t **pointers = NNS_FndAllocFromAllocator(
        *allocator_context,
        allocation_count * sizeof(*pointers));
    uint32_t index;

    if (pointers == NULL) {
        return NULL;
    }
    for (index = 0; index < count; ++index) {
        pointers[index] = NULL;
    }
    return pointers;
}

/* 0x020b8bb4 */
static const char **GameManualPageRecord_AllocateNamePointers_020b8bb4(
    void *const *allocator_context,
    uint32_t count)
{
    uint32_t allocation_count = count == 0 ? 1 : count;
    const char **pointers = NNS_FndAllocFromAllocator(
        *allocator_context,
        allocation_count * sizeof(*pointers));
    uint32_t index;

    if (pointers == NULL) {
        return NULL;
    }
    for (index = 0; index < count; ++index) {
        pointers[index] = NULL;
    }
    return pointers;
}

/* 0x020b8a5c */
void GameManualPageRecord_LoadDefinition_020b8a5c(
    GameManualPageRecord *record,
    const uint16_t raw_definition[6],
    const void *raw_name_pool,
    const void *raw_text_pool,
    void *const *allocator_context)
{
    const GameManualRecordDefinition *definition =
        (const GameManualRecordDefinition *)raw_definition;
    const GameManualPoolBlock *name_pool = raw_name_pool;
    const GameManualPoolBlock *text_pool = raw_text_pool;
    const char **subpage_names;
    const uint16_t **subpage_texts;
    uint32_t index;

    record->page_name = definition->page_name_index ==
        GAME_MANUAL_NO_PAGE_NAME ? NULL :
        PoolEntry(name_pool, definition->page_name_index);
    record->toc_text = PoolEntry(
        text_pool, definition->toc_text_index);
    record->title = PoolEntry(
        text_pool, definition->title_text_index);
    record->child_count = definition->child_count;
    subpage_names = GameManualPageRecord_AllocateNamePointers_020b8bb4(
        allocator_context, record->child_count);
    subpage_texts = GameManualPageRecord_AllocateTextPointers_020b8b68(
        allocator_context, record->child_count);

    for (index = 0; index < record->child_count; ++index) {
        subpage_names[index] = PoolEntry(
            name_pool,
            definition->first_child_name_index + index);
        subpage_texts[index] = PoolEntry(
            text_pool,
            definition->first_child_text_index + index);
    }
    record->subpage_names = subpage_names;
    record->subpage_texts = subpage_texts;
}

/* 0x020b8c00 */
GameManualPageIndex *GameManualPageIndex_Construct(
    GameManualPageIndex *page_index)
{
    page_index->records = NULL;
    page_index->record_count = 0;
    return page_index;
}

/* 0x020b8c10 */
void GameManualPageIndex_Destroy_020b8c10(
    GameManualPageIndex *page_index,
    void *const *allocator_context)
{
    GameManualPageIndex_FreeRecords_020b8c2c(
        allocator_context,
        (GameManualPageRecord *)page_index->records,
        page_index->record_count);
}

/* 0x020b8c2c */
void GameManualPageIndex_FreeRecords_020b8c2c(
    void *const *allocator_context,
    GameManualPageRecord *records,
    uint32_t record_count)
{
    uint32_t index;

    if (records == NULL) {
        return;
    }
    for (index = 0; index < record_count; ++index) {
        GameManualPageRecord_Destroy_020b8a04(
            &records[index], allocator_context);
    }
    NNS_FndFreeToAllocator(*allocator_context, records);
}

/* 0x020b8e28 */
GameManualPageRecord *GameManualPageIndex_AllocateRecords_020b8e28(
    void *const *allocator_context,
    uint32_t record_count)
{
    uint32_t allocation_count = record_count == 0 ? 1 : record_count;
    GameManualPageRecord *records = NNS_FndAllocFromAllocator(
        *allocator_context,
        allocation_count * sizeof(*records));
    uint32_t index;

    if (records == NULL) {
        return NULL;
    }
    for (index = 0; index < record_count; ++index) {
        GameManualPageRecord_Construct_020b89e4(&records[index]);
    }
    return records;
}

/* 0x020b8cbc */
bool GameManualPageIndex_Parse_020b8cbc(
    GameManualPageIndex *page_index,
    const void *resource,
    void *const *allocator_context)
{
    const GameManualBinaryFileHeader *header = resource;
    const GameManualPoolBlock *name_pool = NULL;
    const GameManualPoolBlock *text_pool = NULL;
    const uint8_t *cursor;
    uint16_t block_index;

    if (!NtmvM2dBinaryFile_IsValid(
            header, GAME_MANUAL_INDEX_FILE_SIGNATURE) ||
        header->version != GAME_MANUAL_INDEX_FILE_VERSION) {
        return false;
    }

    cursor = (const uint8_t *)header + header->header_size;
    for (block_index = 0;
         block_index < header->block_count;
         ++block_index) {
        const GameManualBlockHeader *block =
            (const GameManualBlockHeader *)cursor;
        uint32_t signature = ReadTag(block->signature);

        if (signature == GAME_MANUAL_RECORD_BLOCK_SIGNATURE) {
            const GameManualRecordBlock *record_block =
                (const GameManualRecordBlock *)block;
            uint16_t record_index;

            page_index->record_count = record_block->record_count;
            page_index->records =
                GameManualPageIndex_AllocateRecords_020b8e28(
                    allocator_context, page_index->record_count);
            for (record_index = 0;
                 record_index < page_index->record_count;
                 ++record_index) {
                GameManualPageRecord_LoadDefinition_020b8a5c(
                    (GameManualPageRecord *)&page_index->records[record_index],
                    (const uint16_t *)&record_block->definitions[record_index],
                    name_pool,
                    text_pool,
                    allocator_context);
            }
        } else if (signature == GAME_MANUAL_NAME_POOL_SIGNATURE) {
            name_pool = (const GameManualPoolBlock *)block;
        } else if (signature == GAME_MANUAL_TEXT_POOL_SIGNATURE) {
            text_pool = (const GameManualPoolBlock *)block;
        }
        cursor += block->block_size;
    }
    return true;
}

/* 0x020b8c78 */
bool GameManualPageIndex_Load_020b8c78(
    GameManualPageIndex *page_index,
    const char *resource_name,
    NtmvM2dArcResourceAccessor *accessor,
    void *allocator_context)
{
    const void *resource = accessor->vtable->get_localized_file(
        accessor,
        GAME_MANUAL_INDEX_RESOURCE_TYPE,
        resource_name,
        allocator_context);

    if (resource == NULL) {
        return false;
    }
    return GameManualPageIndex_Parse_020b8cbc(
        page_index, resource, allocator_context);
}
