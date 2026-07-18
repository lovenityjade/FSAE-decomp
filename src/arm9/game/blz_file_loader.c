#include "game/blz_file_loader.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    GAME_FS_FILE_HANDLE_SIZE = 0x48,
    GAME_FS_SEEK_SET = 0,
    GAME_FS_SEEK_END = 2,
    GAME_BLZ_FOOTER_SIZE = 4,
    GAME_ALLOCATOR_BASE_ALIGNMENT = 4
};

typedef struct GameFsFileHandle {
    uint8_t storage[GAME_FS_FILE_HANDLE_SIZE];
} GameFsFileHandle;

extern void FS_InitFile(GameFsFileHandle *file);
extern bool FS_OpenFile(GameFsFileHandle *file, const char *path);
extern int32_t FS_GetFileLength(GameFsFileHandle *file);
extern bool FS_SeekFile(
    GameFsFileHandle *file,
    int32_t offset,
    int32_t origin);
extern int32_t FS_ReadFile(
    GameFsFileHandle *file,
    void *destination,
    int32_t size);
extern void FS_CloseFile(GameFsFileHandle *file);
extern void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size);
extern void NNS_FndFreeToAllocator(void *allocator, void *allocation);
extern int32_t MI_SecureUncompressBLZ(
    void *compressed_data,
    uint32_t compressed_size,
    uint32_t uncompressed_size);

static void *AlignAllocation(void *allocation, uint32_t alignment)
{
    uintptr_t mask = (uintptr_t)alignment - 1;

    return (void *)(((uintptr_t)allocation + mask) & ~mask);
}

/* 0x020b7b1c */
void *Game_LoadBlzFileToAllocator_020b7b1c(
    const char *path,
    void *allocator,
    uint32_t alignment,
    uint32_t *uncompressed_size)
{
    GameFsFileHandle file;
    int32_t compressed_size;
    uint32_t output_size;
    uint32_t alignment_reserve = 0;
    void *allocation = NULL;
    void *aligned_data;
    int32_t decompress_result = -1;

    FS_InitFile(&file);
    if (!FS_OpenFile(&file, path)) {
        return NULL;
    }

    compressed_size = FS_GetFileLength(&file);
    if (FS_SeekFile(&file, -GAME_BLZ_FOOTER_SIZE, GAME_FS_SEEK_END) &&
        FS_ReadFile(&file, &output_size, GAME_BLZ_FOOTER_SIZE) ==
            GAME_BLZ_FOOTER_SIZE) {
        output_size += (uint32_t)compressed_size;
        if (FS_SeekFile(&file, 0, GAME_FS_SEEK_SET)) {
            if (alignment != GAME_ALLOCATOR_BASE_ALIGNMENT) {
                alignment_reserve = alignment;
            }
            allocation = NNS_FndAllocFromAllocator(
                allocator, output_size + alignment_reserve);
            if (allocation != NULL) {
                aligned_data = AlignAllocation(allocation, alignment);
                if (FS_ReadFile(
                        &file,
                        aligned_data,
                        compressed_size) == compressed_size) {
                    decompress_result = MI_SecureUncompressBLZ(
                        aligned_data,
                        (uint32_t)compressed_size,
                        output_size);
                    if (decompress_result >= 0 &&
                        uncompressed_size != NULL) {
                        *uncompressed_size = output_size;
                    }
                }
                if (decompress_result < 0) {
                    NNS_FndFreeToAllocator(allocator, allocation);
                    allocation = NULL;
                }
            }
        }
    }
    FS_CloseFile(&file);
    return allocation;
}
