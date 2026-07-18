#ifndef FSAE_GAME_CIRCULAR_TILE_UPLOAD_H
#define FSAE_GAME_CIRCULAR_TILE_UPLOAD_H

#include "game/scrollable_tile_buffer.h"

typedef struct GameCircularTileUploadRange {
    int16_t leading_offset;
    int16_t wrapped_offset;
} GameCircularTileUploadRange;

typedef GameScrollableTileBuffer GameCircularTileBuffer;

typedef void (*GameCircularTileUploadCallback)(
    const uint16_t *source,
    uint32_t destination_offset,
    uint32_t byte_count);

void Game_UploadCircularTileRows( /* 0x020bc43c */
    const GameCircularTileBuffer *buffer,
    const GameCircularTileUploadRange *range,
    GameCircularTileUploadCallback upload);

#endif
