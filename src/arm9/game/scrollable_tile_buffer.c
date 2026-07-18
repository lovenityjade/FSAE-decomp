#include "game/scrollable_tile_buffer.h"

enum {
    GAME_SCROLLABLE_TILE_BASE_HEIGHT = 0xae,
    GAME_SCROLLABLE_TILE_STEP = 4,
    GAME_SCROLLABLE_TILE_MAX_DELTA = 0x40,
    GAME_SCROLLABLE_TILE_UPLOAD_STEP = 0x20
};

static int16_t ClampToS16(int32_t value)
{
    return (int16_t)value;
}

/* 0x020b7080 */
void GameScrollableTileDrawConfig_Init(GameScrollableTileDrawConfig *config)
{
    config->content_bounds.left = 0;
    config->content_bounds.top = 0;
    config->content_bounds.right = 0;
    config->content_bounds.bottom = 0;
    config->update_bounds.left = 0;
    config->update_bounds.top = 0;
    config->update_bounds.right = 0;
    config->update_bounds.bottom = 0;
    config->rows = 0;
    config->row_stride = 0;
    config->ring_rows = 0;
    config->tilemap = 0;
    config->content = 0;
    config->unknown_20 = 0;
}

/* 0x020be434 */
static void CopyContentBounds(
    GameScrollableTileBounds *output,
    const void *content)
{
    GameScrollableTileBounds bounds;

    GameScrollableTileContent_GetBounds(&bounds, content);
    *output = bounds;
}

/* 0x020be70c */
static void PrepareDrawConfig(
    const GameScrollableTileBuffer *buffer,
    GameScrollableTileDrawConfig *config)
{
    CopyContentBounds(&config->content_bounds, buffer->content);
    config->rows = buffer->rows;
    config->row_stride = buffer->row_stride;
    config->ring_rows = buffer->ring_rows;
    config->tilemap = buffer->tilemap;
    config->content = buffer->content;
}

/* 0x020be8e8 */
static void FillRowsBelowContent(
    const GameScrollableTileBuffer *buffer,
    GameScrollableTileDrawConfig *config)
{
    GameScrollableTileBounds bounds;
    int32_t content_height;
    int32_t y;
    const int16_t *tile_step = buffer->tile_step;

    GameScrollableTileContent_GetBounds(&bounds, buffer->content);
    content_height = ClampToS16(bounds.bottom - bounds.top);
    if (content_height < GAME_SCROLLABLE_TILE_BASE_HEIGHT) {
        content_height = GAME_SCROLLABLE_TILE_BASE_HEIGHT;
    }
    if (config->update_bounds.bottom < content_height) {
        return;
    }

    y = content_height;
    if (content_height < config->update_bounds.top) {
        y = ClampToS16(
            ((config->update_bounds.top - content_height) & ~0xf) +
            content_height);
    }
    if (y >= config->update_bounds.bottom) {
        return;
    }

    do {
        int32_t x = 0;
        if (buffer->viewport_width > 0) {
            do {
                int16_t position[2];
                position[0] = (int16_t)x;
                position[1] = (int16_t)y;
                GameScrollableTileContent_DrawTile(
                    config, position, &buffer->tile_step);
                x = ClampToS16(x + tile_step[0]);
            } while (x < buffer->viewport_width);
        }
        y = ClampToS16(y + tile_step[1]);
    } while (y < config->update_bounds.bottom);
}

/* 0x020be470 */
void GameScrollableTileBuffer_Init(GameScrollableTileBuffer *buffer)
{
    buffer->owner = 0;
    buffer->content = 0;
    buffer->tilemap = 0;
    buffer->viewport_width = 0;
    buffer->viewport_height = 0;
    buffer->rows = 0;
    buffer->row_stride = 0;
    buffer->ring_rows = 0;
    buffer->tile_step = 0;
    buffer->visible_start = 0;
    buffer->visible_end = 0;
    buffer->previous_start = 0;
    buffer->previous_end = 0;
    buffer->target_start = 0;
    buffer->upload_cursor = 0;
    buffer->force_upload = 0;
}

/* 0x020be4b8: the embedded state owns no allocation. */
void GameScrollableTileBuffer_Destroy(GameScrollableTileBuffer *buffer)
{
    (void)buffer;
}

/* 0x020be4bc */
void GameScrollableTileBuffer_SetOwner(
    GameScrollableTileBuffer *buffer,
    void *owner)
{
    buffer->owner = owner;
}

/* 0x020be4c4 */
void GameScrollableTileBuffer_SetContent(
    GameScrollableTileBuffer *buffer,
    void *content)
{
    buffer->content = content;
    if (content != 0) {
        GameScrollableTileBuffer_ResetViewport(buffer);
        GameScrollableTileBuffer_RefreshContent(buffer);
    }
}

/* 0x020be4e8 */
void GameScrollableTileBuffer_ResetViewport(GameScrollableTileBuffer *buffer)
{
    buffer->previous_start = 0;
    GameScrollableTileBuffer_CollapsePreviousRange(buffer);
    buffer->visible_start = buffer->previous_start;
    buffer->visible_end =
        ClampToS16(buffer->previous_start + buffer->viewport_height);
    buffer->target_start = buffer->previous_start;
}

/* 0x020be518 */
bool GameScrollableTileBuffer_RequestStep(
    GameScrollableTileBuffer *buffer,
    bool upward,
    bool downward)
{
    if (buffer->content != 0 &&
        buffer->previous_start < buffer->previous_end) {
        int32_t extent = GameScrollableTileBuffer_GetScrollExtent(buffer);

        if (extent > buffer->viewport_height) {
            if (upward) {
                int32_t target = buffer->visible_start -
                    GAME_SCROLLABLE_TILE_STEP;
                if (target < 0) {
                    target = 0;
                }
                buffer->target_start = (int16_t)target;
            } else if (downward) {
                int32_t end = buffer->visible_end +
                    GAME_SCROLLABLE_TILE_STEP;
                if (end >= extent) {
                    end = extent;
                }
                buffer->target_start =
                    ClampToS16(end - buffer->viewport_height);
            }
        }
    }
    return buffer->visible_start != buffer->target_start;
}

/* 0x020be5a4 */
bool GameScrollableTileBuffer_ApplyDelta(
    GameScrollableTileBuffer *buffer,
    int32_t delta)
{
    bool changed = false;

    if (buffer->content == 0 ||
        buffer->previous_start >= buffer->previous_end) {
        return false;
    }

    {
        int32_t extent = GameScrollableTileBuffer_GetScrollExtent(buffer);
        if (extent <= buffer->viewport_height) {
            return false;
        }

        if (delta < 0) {
            int32_t applied = delta;
            int32_t start;

            if (applied < -GAME_SCROLLABLE_TILE_MAX_DELTA) {
                applied = -GAME_SCROLLABLE_TILE_MAX_DELTA;
            }
            start = buffer->previous_start + applied;
            if (start <= 0) {
                start = 0;
            }
            buffer->visible_start = (int16_t)start;
            if (buffer->visible_start != buffer->previous_start) {
                changed = true;
                buffer->visible_end = ClampToS16(
                    buffer->visible_start + buffer->viewport_height);
            }
        }

        if (delta > 0) {
            int32_t applied = delta;
            int32_t end;

            if (applied > GAME_SCROLLABLE_TILE_MAX_DELTA) {
                applied = GAME_SCROLLABLE_TILE_MAX_DELTA;
            }
            end = buffer->previous_end + applied;
            if (end >= extent) {
                end = extent;
            }
            buffer->visible_end = (int16_t)end;
            if (buffer->visible_end != buffer->previous_end) {
                changed = true;
                buffer->visible_start = ClampToS16(
                    buffer->visible_end - buffer->viewport_height);
            }
        }
    }
    return changed;
}

/* 0x020be66c */
bool GameScrollableTileBuffer_Tick(
    GameScrollableTileBuffer *buffer,
    uint8_t *upload_required)
{
    bool changed = false;

    if (buffer->target_start != buffer->visible_start) {
        int16_t delta = ClampToS16(
            buffer->target_start - buffer->visible_start);
        changed = GameScrollableTileBuffer_ApplyDelta(buffer, delta);
        if (changed) {
            if (buffer->force_upload != 0) {
                buffer->force_upload = 0;
                buffer->upload_cursor = buffer->visible_start;
                *upload_required = 1;
            } else {
                int32_t distance =
                    buffer->visible_start - buffer->upload_cursor;
                if (distance < 0) {
                    distance = -distance;
                }
                if (distance >= GAME_SCROLLABLE_TILE_UPLOAD_STEP) {
                    if (buffer->visible_start - buffer->upload_cursor > 0) {
                        buffer->upload_cursor = ClampToS16(
                            buffer->upload_cursor +
                            GAME_SCROLLABLE_TILE_UPLOAD_STEP);
                    } else {
                        buffer->upload_cursor = ClampToS16(
                            buffer->upload_cursor -
                            GAME_SCROLLABLE_TILE_UPLOAD_STEP);
                    }
                    *upload_required = 1;
                }
            }
        }
    }
    return changed;
}

/* 0x020be854 */
void GameScrollableTileBuffer_CollapsePreviousRange(
    GameScrollableTileBuffer *buffer)
{
    buffer->previous_end = buffer->previous_start;
}

/* 0x020be860 */
int32_t GameScrollableTileBuffer_GetScrollExtent(
    const GameScrollableTileBuffer *buffer)
{
    GameScrollableTileBounds bounds;
    int32_t height;

    if (buffer->content == 0) {
        return 0;
    }

    GameScrollableTileContent_GetBounds(&bounds, buffer->content);
    height = ClampToS16(bounds.bottom - bounds.top);
    if (height < GAME_SCROLLABLE_TILE_BASE_HEIGHT) {
        height = GAME_SCROLLABLE_TILE_BASE_HEIGHT;
    }
    return ClampToS16(
        buffer->viewport_height + height -
        GAME_SCROLLABLE_TILE_BASE_HEIGHT);
}

/* 0x020be8c0 */
bool GameScrollableTileBuffer_CanScroll(
    const GameScrollableTileBuffer *buffer)
{
    return GameScrollableTileBuffer_GetScrollExtent(buffer) >
        buffer->viewport_height;
}

/* 0x020be8e0 */
void GameScrollableTileBuffer_SetTileStep(
    GameScrollableTileBuffer *buffer,
    const int16_t *tile_step)
{
    buffer->tile_step = tile_step;
}

/* 0x020be750 */
void GameScrollableTileBuffer_RefreshContent(
    GameScrollableTileBuffer *buffer)
{
    GameScrollableTileDrawConfig config;

    if (buffer->content == 0) {
        return;
    }
    GameScrollableTileDrawConfig_Init(&config);
    PrepareDrawConfig(buffer, &config);
    GameScrollableTileContent_DrawAll(buffer->content, &config);
}

/* 0x020be798 */
void GameScrollableTileBuffer_UpdateContent(
    GameScrollableTileBuffer *buffer)
{
    GameScrollableTileDrawConfig config;
    int16_t update_top = 0;
    int16_t update_right = 0;
    int16_t update_bottom = 0;

    if (buffer->content == 0) {
        return;
    }

    GameScrollableTileDrawConfig_Init(&config);
    PrepareDrawConfig(buffer, &config);

    if (buffer->previous_start == buffer->visible_start) {
        if (buffer->previous_end < buffer->visible_end) {
            update_top = buffer->previous_end;
            update_right = buffer->viewport_width;
            update_bottom = buffer->visible_end;
        }
    } else if (buffer->previous_start < buffer->visible_start) {
        update_top = buffer->previous_end;
        update_right = buffer->viewport_width;
        update_bottom = buffer->visible_end;
    } else {
        update_top = buffer->visible_start;
        update_right = buffer->viewport_width;
        update_bottom = buffer->previous_start;
    }

    config.update_bounds.left = 0;
    config.update_bounds.top = update_top;
    config.update_bounds.right = update_right;
    config.update_bounds.bottom = update_bottom;
    GameScrollableTileContent_DrawRegion(buffer->content, &config);
    FillRowsBelowContent(buffer, &config);
}

typedef struct GameScrollableTileContentPrefix {
    uint8_t unknown_00[0x10];
    void *resource;
} GameScrollableTileContentPrefix;

/* 0x020be9e4 */
void *GameScrollableTileBuffer_GetContentResource(
    const GameScrollableTileBuffer *buffer)
{
    const GameScrollableTileContentPrefix *content =
        (const GameScrollableTileContentPrefix *)buffer->content;
    return content->resource;
}
