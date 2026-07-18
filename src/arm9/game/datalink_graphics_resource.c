#include "game/datalink_graphics_resource.h"

#include "game/datalink_resource_access.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_RESOURCE_ID_MASK = 0x7FFFFFFFU,
    DATALINK_CELL_RESOURCE_ID_MASK = 0x0000FFFFU,
    DATALINK_RESOURCE_ALIGNMENT = 0x20,
    DATALINK_FREE_BG_CHARACTER = 1,
    DATALINK_FREE_PALETTE = 2,
    DATALINK_FREE_SCREEN = 4
};

extern void *EL_GetGlobalAdr(uint32_t size, int alignment);
extern void Game_FreeDatalinkResourceBuffer(void *buffer);
extern void MI_CpuCopy8(const void *source, void *destination, uint32_t size);
extern void MIi_CpuCopyFast(const void *source, void *destination, uint32_t size);
extern void DC_FlushRange(const void *buffer, uint32_t size);
extern int Game_GetUnpackedDatalinkBgCharacterData(
    const void *file,
    Game_G2dCharacterData **output);
extern int NNS_G2dGetUnpackedCharacterData(
    const void *file,
    Game_G2dCharacterData **output);
extern int NNS_G2dGetUnpackedPaletteData(
    const void *file,
    Game_G2dPaletteData **output);
extern int NNS_G2dGetUnpackedScreenData(
    const void *file,
    Game_G2dScreenData **output);
extern int NNS_G2dGetUnpackedCellBank(const void *file, void **output);
extern int GetUnpackedAnimBankImpl_(const void *file, void **output);
extern void Game_TransferDatalinkCharacterBuffer(
    int target,
    const void *source,
    uint32_t destination_offset,
    uint32_t size);
extern void Game_TransferDatalinkObjectCharacterData(
    const void *source,
    uint32_t destination_offset,
    uint32_t size);
extern void Game_TransferDatalinkObjectPaletteData(
    const void *source,
    uint32_t destination_offset,
    uint32_t size);

#if UINTPTR_MAX == UINT32_MAX
_Static_assert(
    sizeof(Game_DatalinkGraphicsResourceSet) == 0x28,
    "datalink graphics resource set size"
);
_Static_assert(
    offsetof(Game_DatalinkGraphicsResourceSet, character_file_08) == 0x08,
    "datalink graphics character file offset"
);
_Static_assert(
    offsetof(Game_DatalinkGraphicsResourceSet, screen_file_10) == 0x10,
    "datalink graphics screen file offset"
);
_Static_assert(
    offsetof(Game_DatalinkGraphicsResourceSet, cell_bank_file_18) == 0x18,
    "datalink graphics cell file offset"
);
_Static_assert(
    offsetof(Game_DatalinkGraphicsResourceSet, animation_bank_file_20) == 0x20,
    "datalink graphics animation file offset"
);
#endif

static int Game_DatalinkResourceUsesAlternateArchive(uint32_t resource_id)
{
    return (int)(resource_id >> 31);
}

static uint32_t Game_DatalinkResourceIndex(uint32_t resource_id)
{
    return resource_id & DATALINK_RESOURCE_ID_MASK;
}

static void *Game_CopyDatalinkResourceFast(
    uint32_t resource_id,
    uint32_t *resource_size)
{
    uint32_t index = Game_DatalinkResourceIndex(resource_id);
    int use_alternate =
        Game_DatalinkResourceUsesAlternateArchive(resource_id);
    uint32_t size = Game_GetDatalinkResourceSize(index, use_alternate);
    void *buffer = EL_GetGlobalAdr(size, DATALINK_RESOURCE_ALIGNMENT);
    const void *source =
        Game_GetDatalinkResourceAddress(index, use_alternate);

    MIi_CpuCopyFast(source, buffer, size);
    *resource_size = size;
    return buffer;
}

static void *Game_CopyDatalinkResourceBytes(
    uint32_t resource_id,
    uint32_t *resource_size)
{
    uint32_t index = Game_DatalinkResourceIndex(resource_id);
    int use_alternate =
        Game_DatalinkResourceUsesAlternateArchive(resource_id);
    uint32_t size = Game_GetDatalinkResourceSize(index, use_alternate);
    void *buffer = EL_GetGlobalAdr(size, DATALINK_RESOURCE_ALIGNMENT);
    const void *source =
        Game_GetDatalinkResourceAddress(index, use_alternate);

    MI_CpuCopy8(source, buffer, size);
    *resource_size = size;
    return buffer;
}

/*
 * 0x020AC594
 *
 * Complete 184-byte body through 0x020AC64B. The independent screen loader at
 * 0x020AC64C is excluded.
 */
void Game_LoadDatalinkCharacterResource(
    Game_DatalinkGraphicsResourceSet *resources,
    int16_t target,
    uint32_t resource_id,
    uint32_t destination_offset,
    uint32_t flags)
{
    uint32_t resource_size;
    const void *character_data;
    uint32_t character_size;

    if (resource_id == UINT32_MAX) {
        return;
    }

    resources->character_file_08 =
        Game_CopyDatalinkResourceFast(resource_id, &resource_size);
    (void)Game_GetUnpackedDatalinkBgCharacterData(
        resources->character_file_08,
        &resources->character_data_0c);

    character_data = (const void *)(uintptr_t)
        resources->character_data_0c->data_address_14;
    character_size = resources->character_data_0c->byte_size_10;

    DC_FlushRange(character_data, character_size);
    Game_TransferDatalinkCharacterBuffer(
        (int)target,
        character_data,
        destination_offset,
        character_size);

    if ((flags & DATALINK_FREE_BG_CHARACTER) != 0U) {
        Game_FreeDatalinkResourceBuffer(resources->character_file_08);
        resources->character_file_08 = 0;
        resources->character_data_0c = 0;
    }
}

/*
 * 0x020AC64C
 *
 * Complete 100-byte body through 0x020AC6AF. The independent object graphics
 * resource loader at 0x020AC6B0 is excluded.
 */
void Game_LoadDatalinkScreenResourceIntoSet(
    Game_DatalinkGraphicsResourceSet *resources,
    int target,
    uint32_t resource_id)
{
    uint32_t resource_size;

    (void)target;

    if (resource_id == UINT32_MAX) {
        return;
    }

    resources->screen_file_10 =
        Game_CopyDatalinkResourceFast(resource_id, &resource_size);
    (void)NNS_G2dGetUnpackedScreenData(
        resources->screen_file_10,
        &resources->screen_data_14);
}

/*
 * 0x020AC6B0
 *
 * Complete 300-byte body through 0x020AC7DB. The independent cell/animation
 * resource loader at 0x020AC7DC is excluded.
 */
void Game_LoadDatalinkObjectGraphicsResources(
    Game_DatalinkGraphicsResourceSet *resources,
    uint32_t character_resource_id,
    int character_upload_size,
    uint32_t palette_resource_id,
    int palette_upload_size)
{
    if (character_resource_id != UINT32_MAX) {
        uint32_t resource_size;
        const void *character_data;
        uint32_t upload_size;

        resources->character_file_08 =
            Game_CopyDatalinkResourceFast(
                character_resource_id,
                &resource_size);
        (void)NNS_G2dGetUnpackedCharacterData(
            resources->character_file_08,
            &resources->character_data_0c);

        upload_size = (character_upload_size == -1) ?
            resources->character_data_0c->byte_size_10 :
            (uint32_t)character_upload_size;
        character_data = (const void *)(uintptr_t)
            resources->character_data_0c->data_address_14;

        DC_FlushRange(character_data, upload_size);
        Game_TransferDatalinkObjectCharacterData(
            character_data,
            0,
            upload_size);
    }

    if (palette_resource_id != UINT32_MAX) {
        uint32_t resource_size;
        const void *palette_data;
        uint32_t upload_size;

        resources->palette_file_00 =
            Game_CopyDatalinkResourceBytes(
                palette_resource_id,
                &resource_size);
        (void)NNS_G2dGetUnpackedPaletteData(
            resources->palette_file_00,
            &resources->palette_data_04);

        upload_size = (palette_upload_size == -1) ?
            resources->palette_data_04->byte_size_08 :
            (uint32_t)palette_upload_size;
        palette_data = (const void *)(uintptr_t)
            resources->palette_data_04->data_address_0c;

        DC_FlushRange(palette_data, upload_size);
        Game_TransferDatalinkObjectPaletteData(
            palette_data,
            0,
            upload_size);
    }
}

/*
 * 0x020AC7DC
 *
 * Complete 188-byte body through 0x020AC897. The clamp helper at 0x020AC898
 * is excluded.
 */
void Game_LoadDatalinkCellAnimationResources(
    Game_DatalinkGraphicsResourceSet *resources,
    uint32_t cell_bank_resource_id,
    uint32_t animation_bank_resource_id)
{
    if (cell_bank_resource_id != UINT32_MAX) {
        uint32_t index =
            cell_bank_resource_id & DATALINK_CELL_RESOURCE_ID_MASK;
        int use_alternate =
            Game_DatalinkResourceUsesAlternateArchive(cell_bank_resource_id);
        uint32_t size =
            Game_GetDatalinkResourceSize(index, use_alternate);
        void *buffer = EL_GetGlobalAdr(size, DATALINK_RESOURCE_ALIGNMENT);
        const void *source =
            Game_GetDatalinkResourceAddress(index, use_alternate);

        MI_CpuCopy8(source, buffer, size);
        resources->cell_bank_file_18 = buffer;
        (void)NNS_G2dGetUnpackedCellBank(
            resources->cell_bank_file_18,
            &resources->cell_bank_1c);
    }

    if (animation_bank_resource_id != UINT32_MAX) {
        uint32_t resource_size;

        resources->animation_bank_file_20 =
            Game_CopyDatalinkResourceBytes(
                animation_bank_resource_id,
                &resource_size);
        (void)GetUnpackedAnimBankImpl_(
            resources->animation_bank_file_20,
            &resources->animation_bank_24);
    }
}
