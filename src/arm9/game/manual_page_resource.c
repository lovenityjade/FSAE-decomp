#include "game/manual_page_loader.h"

#include "game/language_table.h"

#include <stddef.h>

enum {
    NTMV_M2D_PAGE_FILE_SIGNATURE = 0x4e545047, /* "NTPG" */
    NTMV_M2D_PAGE_FILE_VERSION = 0x0200,
    NTMV_M2D_PAGE_NAME_POOL_SIGNATURE = 0x6e617031, /* "nap1" */
    NTMV_M2D_PAGE_TEXT_POOL_SIGNATURE = 0x74787031, /* "txp1" */
    NTMV_M2D_PAGE_RESOURCE_LIST_SIGNATURE = 0x74786c31, /* "txl1" */
    NTMV_M2D_PAGE_DEFINITION_SIGNATURE = 0x70616731, /* "pag1" */
    NTMV_M2D_PANE_SIGNATURE = 0x70616e31, /* "pan1" */
    NTMV_M2D_PICTURE_SIGNATURE = 0x70696331, /* "pic1" */
    NTMV_M2D_TEXT_BOX_SIGNATURE = 0x74787431, /* "txt1" */
    NTMV_M2D_WINDOW_SIGNATURE = 0x776e6431, /* "wnd1" */
    NTMV_M2D_CHILDREN_BEGIN_SIGNATURE = 0x70617331, /* "pas1" */
    NTMV_M2D_CHILDREN_END_SIGNATURE = 0x70616531, /* "pae1" */
    NTMV_M2D_FILE_RESOURCE_TYPE = 0x6e747466 /* "ntff" */
};

typedef struct NtmvM2dPageFileHeader {
    uint8_t signature[4];
    uint16_t byte_order;
    uint16_t version;
    uint32_t file_size;
    uint16_t header_size;
    uint16_t block_count;
} NtmvM2dPageFileHeader;

typedef struct NtmvM2dPageBlockHeader {
    uint8_t signature[4];
    uint32_t block_size;
} NtmvM2dPageBlockHeader;

typedef struct NtmvM2dPagePoolBlock {
    NtmvM2dPageBlockHeader header;
    uint32_t entry_count;
    uint32_t offsets[];
} NtmvM2dPagePoolBlock;

typedef struct NtmvM2dPageResourceListEntry {
    uint16_t name_index;
    uint16_t reserved_02;
} NtmvM2dPageResourceListEntry;

typedef struct NtmvM2dPageResourceListBlock {
    NtmvM2dPageBlockHeader header;
    uint16_t resource_count;
    uint16_t reserved_0a;
    NtmvM2dPageResourceListEntry entries[];
} NtmvM2dPageResourceListBlock;

typedef struct NtmvM2dPageDefinitionBlock {
    NtmvM2dPageBlockHeader header;
    int16_t content_width;
    int16_t content_extent;
    uint16_t palette_color;
    uint16_t content_resource_index;
} NtmvM2dPageDefinitionBlock;

typedef struct NtmvM2dChildrenBeginBlock {
    NtmvM2dPageBlockHeader header;
    uint16_t child_capacity;
    uint16_t reserved_0a;
} NtmvM2dChildrenBeginBlock;

extern void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size);

static uint32_t ReadTag(const uint8_t bytes[4])
{
    return ((uint32_t)bytes[0] << 24) |
        ((uint32_t)bytes[1] << 16) |
        ((uint32_t)bytes[2] << 8) |
        bytes[3];
}

static const void *PoolEntry(
    const NtmvM2dPagePoolBlock *pool,
    uint32_t entry_index)
{
    const uint8_t *offset_table = (const uint8_t *)&pool->offsets[0];
    return offset_table + pool->offsets[entry_index];
}

/* 0x020b51c8 */
void **NtmvM2dPage_AllocateResourceArray_020b51c8(
    void *const *allocator_context,
    uint32_t resource_count)
{
    uint32_t allocation_count = resource_count == 0 ? 1 : resource_count;
    void **resources = NNS_FndAllocFromAllocator(
        *allocator_context, allocation_count * sizeof(*resources));
    uint32_t index;

    if (resources == NULL) {
        return NULL;
    }
    for (index = 0; index < resource_count; ++index) {
        resources[index] = NULL;
    }
    return resources;
}

static void LoadResourceList(
    GameManualPage *page,
    void *allocator_context,
    const NtmvM2dPageResourceListBlock *block,
    NtmvM2dPageBuildContext *build_context)
{
    const NtmvM2dPagePoolBlock *name_pool = build_context->name_pool;
    uint32_t index;

    page->resource_count = block->resource_count;
    page->resource_files = NtmvM2dPage_AllocateResourceArray_020b51c8(
        (void *const *)allocator_context, page->resource_count);
    for (index = 0; index < page->resource_count; ++index) {
        const char *name = PoolEntry(
            name_pool, block->entries[index].name_index);
        page->resource_files[index] =
            build_context->accessor->vtable->get_file(
                build_context->accessor,
                NTMV_M2D_FILE_RESOURCE_TYPE,
                name);
    }
    build_context->resource_set = &page->resource_count;
}

static void LoadPageDefinition(
    GameManualPage *page,
    const NtmvM2dPageDefinitionBlock *block,
    const NtmvM2dPageBuildContext *build_context)
{
    const NtmvM2dPagePoolBlock *text_pool = build_context->text_pool;

    page->content_width = (uint16_t)block->content_width;
    page->content_extent = block->content_extent;
    page->palette_color = block->palette_color;
    page->content_resource = (void *)PoolEntry(
        text_pool, block->content_resource_index);
}

static bool IsPaneSignature(uint32_t signature)
{
    return signature == NTMV_M2D_PANE_SIGNATURE ||
        signature == NTMV_M2D_PICTURE_SIGNATURE ||
        signature == NTMV_M2D_TEXT_BOX_SIGNATURE ||
        signature == NTMV_M2D_WINDOW_SIGNATURE;
}

/*
 * 0x020b4f00..0x020b51af (688 instruction bytes).
 *
 * The original intentionally validates only signature/BOM and version.  It
 * trusts block ordering, sizes, indices, allocation results and pane factory
 * results; this recovery preserves those observable failure semantics.
 */
bool GameManualPage_Load_020b4f00(
    GameManualPage *page,
    void *allocator_context,
    const void *resource,
    NtmvM2dArcResourceAccessor *accessor)
{
    const NtmvM2dPageFileHeader *header = resource;
    NtmvM2dPageBuildContext build_context = {
        accessor, NULL, NULL, NULL
    };
    const uint8_t *cursor;
    NtmvM2dPane *current_parent = NULL;
    NtmvM2dPane *current_pane = NULL;
    uint32_t block_index;

    if (!NtmvM2dBinaryFile_IsValid(
            header, NTMV_M2D_PAGE_FILE_SIGNATURE) ||
        header->version != NTMV_M2D_PAGE_FILE_VERSION) {
        return false;
    }

    cursor = (const uint8_t *)header + header->header_size;
    for (block_index = 0;
         block_index < header->block_count;
         ++block_index) {
        const NtmvM2dPageBlockHeader *block =
            (const NtmvM2dPageBlockHeader *)cursor;
        uint32_t signature = ReadTag(block->signature);

        if (signature == NTMV_M2D_PAGE_NAME_POOL_SIGNATURE) {
            build_context.name_pool = block;
        } else if (signature == NTMV_M2D_PAGE_TEXT_POOL_SIGNATURE) {
            build_context.text_pool = block;
        } else if (signature == NTMV_M2D_PAGE_RESOURCE_LIST_SIGNATURE) {
            LoadResourceList(
                page,
                allocator_context,
                (const NtmvM2dPageResourceListBlock *)block,
                &build_context);
        } else if (signature == NTMV_M2D_PAGE_DEFINITION_SIGNATURE) {
            LoadPageDefinition(
                page,
                (const NtmvM2dPageDefinitionBlock *)block,
                &build_context);
        } else if (IsPaneSignature(signature)) {
            current_pane = NtmvM2dPage_CreatePane_020b5318(
                page,
                allocator_context,
                signature,
                block,
                &build_context);
            if (page->root_pane == NULL) {
                page->root_pane = current_pane;
            }
            if (current_parent != NULL) {
                NtmvM2dPane_AddChild_020b5728(
                    current_parent, current_pane);
            }
        } else if (signature == NTMV_M2D_CHILDREN_BEGIN_SIGNATURE) {
            const NtmvM2dChildrenBeginBlock *children =
                (const NtmvM2dChildrenBeginBlock *)block;
            NtmvM2dPane_AllocateChildren_020b56b8(
                current_pane,
                allocator_context,
                children->child_capacity);
            current_parent = current_pane;
        } else if (signature == NTMV_M2D_CHILDREN_END_SIGNATURE) {
            current_pane = current_parent;
            current_parent = current_parent->parent;
        }
        cursor += block->block_size;
    }
    return true;
}
