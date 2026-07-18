#ifndef FSAE_GAME_BLZ_FILE_LOADER_H
#define FSAE_GAME_BLZ_FILE_LOADER_H

#include <stdint.h>

void *Game_LoadBlzFileToAllocator_020b7b1c(
    const char *path,
    void *allocator,
    uint32_t alignment,
    uint32_t *uncompressed_size);

#endif
