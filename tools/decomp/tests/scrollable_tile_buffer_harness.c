#include "game/scrollable_tile_buffer.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static GameScrollableTileBounds sBounds;
static GameScrollableTileDrawConfig sLastConfig;
static unsigned int sDrawAllCount;
static unsigned int sDrawRegionCount;
static unsigned int sDrawTileCount;

void GameScrollableTileContent_GetBounds(
    GameScrollableTileBounds *bounds,
    const void *content)
{
    assert(content == (const void *)(uintptr_t)0x1234u);
    *bounds = sBounds;
}

void GameScrollableTileContent_DrawAll(
    void *content,
    GameScrollableTileDrawConfig *config)
{
    assert(content == (void *)(uintptr_t)0x1234u);
    sLastConfig = *config;
    ++sDrawAllCount;
}

void GameScrollableTileContent_DrawRegion(
    void *content,
    GameScrollableTileDrawConfig *config)
{
    assert(content == (void *)(uintptr_t)0x1234u);
    sLastConfig = *config;
    ++sDrawRegionCount;
}

void GameScrollableTileContent_DrawTile(
    GameScrollableTileDrawConfig *config,
    const int16_t position[2],
    const int16_t *const *tile_step)
{
    assert(config != 0);
    assert(position[0] == 0 || position[0] == 8);
    assert(*tile_step != 0);
    ++sDrawTileCount;
}

static void SeedScrollableRange(GameScrollableTileBuffer *buffer)
{
    buffer->content = (void *)(uintptr_t)0x1234u;
    buffer->viewport_height = 0xae;
    buffer->previous_start = 100;
    buffer->previous_end = 274;
    buffer->visible_start = 100;
    buffer->visible_end = 274;
    buffer->target_start = 100;
    buffer->upload_cursor = 100;
    sBounds.top = 10;
    sBounds.bottom = 510;
}

int main(void)
{
    struct FakeContent {
        uint8_t unknown_00[0x10];
        void *resource;
    } fake_content;
    GameScrollableTileBuffer buffer;
    GameScrollableTileDrawConfig config;
    uint8_t upload_required;
    int16_t tile_step[2] = {8, 16};

    memset(&config, 0xff, sizeof(config));
    GameScrollableTileDrawConfig_Init(&config);
    assert(config.content_bounds.left == 0);
    assert(config.content_bounds.top == 0);
    assert(config.content_bounds.right == 0);
    assert(config.content_bounds.bottom == 0);
    assert(config.update_bounds.left == 0);
    assert(config.update_bounds.top == 0);
    assert(config.update_bounds.right == 0);
    assert(config.update_bounds.bottom == 0);
    assert(config.rows == 0);
    assert(config.row_stride == 0);
    assert(config.ring_rows == 0);
    assert(config.tilemap == 0);
    assert(config.content == 0);
    assert(config.unknown_20 == 0);

    memset(&buffer, 0xff, sizeof(buffer));
    GameScrollableTileBuffer_Init(&buffer);
    assert(buffer.owner == 0);
    assert(buffer.content == 0);
    assert(buffer.rows == 0);
    assert(buffer.force_upload == 0);

    GameScrollableTileBuffer_SetOwner(
        &buffer, (void *)(uintptr_t)0x5678u);
    assert(buffer.owner == (void *)(uintptr_t)0x5678u);
    buffer.viewport_height = 0xae;
    sBounds.top = 0;
    sBounds.bottom = 500;
    GameScrollableTileBuffer_SetContent(
        &buffer, (void *)(uintptr_t)0x1234u);
    assert(sDrawAllCount == 1);
    assert(sLastConfig.content == (void *)(uintptr_t)0x1234u);
    assert(buffer.visible_start == 0);
    assert(buffer.visible_end == 0xae);
    assert(buffer.previous_start == 0);
    assert(buffer.previous_end == 0);

    GameScrollableTileBuffer_SetTileStep(&buffer, tile_step);
    assert(buffer.tile_step == tile_step);
    assert(GameScrollableTileBuffer_GetScrollExtent(&buffer) == 500);
    assert(GameScrollableTileBuffer_CanScroll(&buffer));

    SeedScrollableRange(&buffer);
    assert(GameScrollableTileBuffer_RequestStep(&buffer, true, false));
    assert(buffer.target_start == 96);
    assert(GameScrollableTileBuffer_RequestStep(&buffer, false, true));
    assert(buffer.target_start == 104);

    assert(GameScrollableTileBuffer_ApplyDelta(&buffer, -80));
    assert(buffer.visible_start == 36);
    assert(buffer.visible_end == 210);
    SeedScrollableRange(&buffer);
    assert(GameScrollableTileBuffer_ApplyDelta(&buffer, 80));
    assert(buffer.visible_end == 338);
    assert(buffer.visible_start == 164);

    SeedScrollableRange(&buffer);
    buffer.target_start = 164;
    upload_required = 0;
    assert(GameScrollableTileBuffer_Tick(&buffer, &upload_required));
    assert(buffer.visible_start == 164);
    assert(buffer.upload_cursor == 132);
    assert(upload_required == 1);

    SeedScrollableRange(&buffer);
    buffer.target_start = 96;
    buffer.force_upload = 1;
    upload_required = 0;
    assert(GameScrollableTileBuffer_Tick(&buffer, &upload_required));
    assert(buffer.visible_start == 96);
    assert(buffer.upload_cursor == 96);
    assert(buffer.force_upload == 0);
    assert(upload_required == 1);

    buffer.previous_start = 77;
    buffer.previous_end = 99;
    GameScrollableTileBuffer_CollapsePreviousRange(&buffer);
    assert(buffer.previous_end == 77);

    SeedScrollableRange(&buffer);
    buffer.viewport_width = 16;
    buffer.tile_step = tile_step;
    buffer.previous_start = 0;
    buffer.previous_end = 174;
    buffer.visible_start = 0;
    buffer.visible_end = 206;
    sBounds.top = 0;
    sBounds.bottom = 180;
    sDrawTileCount = 0;
    GameScrollableTileBuffer_UpdateContent(&buffer);
    assert(sDrawRegionCount == 1);
    assert(sLastConfig.update_bounds.left == 0);
    assert(sLastConfig.update_bounds.top == 174);
    assert(sLastConfig.update_bounds.right == 16);
    assert(sLastConfig.update_bounds.bottom == 206);
    assert(sDrawTileCount == 4);

    memset(&fake_content, 0, sizeof(fake_content));
    fake_content.resource = (void *)(uintptr_t)0xabcdefu;
    buffer.content = &fake_content;
    assert(GameScrollableTileBuffer_GetContentResource(&buffer) ==
        (void *)(uintptr_t)0xabcdefu);
    GameScrollableTileBuffer_Destroy(&buffer);
    return 0;
}
