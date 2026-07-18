#ifndef FSAE_GAME_SCROLLABLE_TILE_BUFFER_H
#define FSAE_GAME_SCROLLABLE_TILE_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct GameScrollableTileBounds {
    int16_t left;
    int16_t top;
    int16_t right;
    int16_t bottom;
} GameScrollableTileBounds;

typedef struct GameScrollableTileDrawConfig {
    GameScrollableTileBounds content_bounds; /* target +0x00 */
    GameScrollableTileBounds update_bounds;  /* target +0x08 */
    const uint16_t *rows;                    /* target +0x10 */
    int16_t row_stride;                      /* target +0x14 */
    int16_t ring_rows;                       /* target +0x16 */
    void *tilemap;                           /* target +0x18 */
    void *content;                           /* target +0x1c */
    uint32_t unknown_20;                     /* target +0x20 */
} GameScrollableTileDrawConfig;

/*
 * Scrolling state embedded at +0x40370 in the manual viewer.  The first
 * 0x24 bytes are also consumed directly by Game_UploadCircularTileRows.
 */
typedef struct GameScrollableTileBuffer {
    void *owner;                    /* target +0x00 */
    void *content;                  /* target +0x04 */
    void *tilemap;                  /* target +0x08 */
    int16_t viewport_width;         /* target +0x0c */
    int16_t viewport_height;        /* target +0x0e */
    const uint16_t *rows;           /* target +0x10 */
    int16_t row_stride;             /* target +0x14, in halfwords */
    int16_t ring_rows;              /* target +0x16, power of two */
    const int16_t *tile_step;       /* target +0x18: x/y step */
    int16_t visible_start;          /* target +0x1c */
    int16_t visible_end;            /* target +0x1e */
    int16_t previous_start;         /* target +0x20 */
    int16_t previous_end;           /* target +0x22 */
    int16_t target_start;           /* target +0x24 */
    int16_t upload_cursor;          /* target +0x26 */
    uint8_t force_upload;           /* target +0x28 */
} GameScrollableTileBuffer;

#if UINTPTR_MAX == UINT32_MAX
typedef char GameScrollableTileBufferTargetSizeCheck[
    sizeof(GameScrollableTileBuffer) == 0x2c ? 1 : -1];
typedef char GameScrollableTileBufferRowsOffsetCheck[
    offsetof(GameScrollableTileBuffer, rows) == 0x10 ? 1 : -1];
typedef char GameScrollableTileBufferVisibleStartOffsetCheck[
    offsetof(GameScrollableTileBuffer, visible_start) == 0x1c ? 1 : -1];
typedef char GameScrollableTileBufferPreviousStartOffsetCheck[
    offsetof(GameScrollableTileBuffer, previous_start) == 0x20 ? 1 : -1];
typedef char GameScrollableTileBufferForceUploadOffsetCheck[
    offsetof(GameScrollableTileBuffer, force_upload) == 0x28 ? 1 : -1];
typedef char GameScrollableTileDrawConfigTargetSizeCheck[
    sizeof(GameScrollableTileDrawConfig) == 0x24 ? 1 : -1];
typedef char GameScrollableTileDrawConfigRowsOffsetCheck[
    offsetof(GameScrollableTileDrawConfig, rows) == 0x10 ? 1 : -1];
#endif

void GameScrollableTileBuffer_Init(GameScrollableTileBuffer *buffer);
    /* 0x020be470 */
void GameScrollableTileBuffer_Destroy(GameScrollableTileBuffer *buffer);
    /* 0x020be4b8 */
void GameScrollableTileBuffer_SetOwner(
    GameScrollableTileBuffer *buffer,
    void *owner); /* 0x020be4bc */
void GameScrollableTileBuffer_SetContent(
    GameScrollableTileBuffer *buffer,
    void *content); /* 0x020be4c4 */
void GameScrollableTileBuffer_ResetViewport(GameScrollableTileBuffer *buffer);
    /* 0x020be4e8 */

bool GameScrollableTileBuffer_RequestStep(
    GameScrollableTileBuffer *buffer,
    bool upward,
    bool downward); /* 0x020be518 */
bool GameScrollableTileBuffer_ApplyDelta(
    GameScrollableTileBuffer *buffer,
    int32_t delta); /* 0x020be5a4 */
bool GameScrollableTileBuffer_Tick(
    GameScrollableTileBuffer *buffer,
    uint8_t *upload_required); /* 0x020be66c */

void GameScrollableTileBuffer_CollapsePreviousRange(
    GameScrollableTileBuffer *buffer); /* 0x020be854 */
int32_t GameScrollableTileBuffer_GetScrollExtent(
    const GameScrollableTileBuffer *buffer); /* 0x020be860 */
bool GameScrollableTileBuffer_CanScroll(
    const GameScrollableTileBuffer *buffer); /* 0x020be8c0 */
void GameScrollableTileBuffer_SetTileStep(
    GameScrollableTileBuffer *buffer,
    const int16_t *tile_step); /* 0x020be8e0 */

/* Recovered separately at 0x020be750. */
void GameScrollableTileBuffer_RefreshContent(
    GameScrollableTileBuffer *buffer); /* 0x020be750 */
void GameScrollableTileBuffer_UpdateContent(
    GameScrollableTileBuffer *buffer); /* 0x020be798 */
void *GameScrollableTileBuffer_GetContentResource(
    const GameScrollableTileBuffer *buffer); /* 0x020be9e4 */

/* Content-bound query at 0x020b52f8; kept as an external subsystem edge. */
void GameScrollableTileContent_GetBounds(
    GameScrollableTileBounds *bounds,
    const void *content);
void GameScrollableTileContent_FillBackgroundRows_020b5264(
    const void *content,
    GameScrollableTileDrawConfig *config);
void GameScrollableTileDrawConfig_Init(
    GameScrollableTileDrawConfig *config); /* 0x020b7080 */
void GameScrollableTileContent_DrawAll(
    void *content,
    GameScrollableTileDrawConfig *config); /* 0x020b5214 */
void GameScrollableTileContent_DrawRegion(
    void *content,
    GameScrollableTileDrawConfig *config); /* 0x020b523c */
void GameScrollableTileContent_DrawTile(
    GameScrollableTileDrawConfig *config,
    const int16_t position[2],
    const int16_t *const *tile_step); /* Generic content edge; target still shares 0x020b771c. */

#endif
