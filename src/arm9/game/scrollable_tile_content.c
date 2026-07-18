#include "game/scrollable_tile_buffer.h"

#include "ntmv/m2d/page.h"
#include "ntmv/m2d/pane.h"
#include "ntmv/m2d/render_surface.h"

static int16_t ClampToS16(int32_t value)
{
    return (int16_t)value;
}

static uint16_t PageBackgroundFillValue(const NtmvM2dPage *page)
{
    return (uint16_t)(page->palette_color | 0x8000u);
}

static uint16_t *ScrollableTileRowAddress(
    const GameScrollableTileDrawConfig *config,
    int32_t y)
{
    return NtmvM2d_RenderSurfaceRowAddress_020b7508(
        (const NtmvM2dRenderSurface *)(const void *)config,
        (uint32_t)(int16_t)y);
}

/* 0x020b5214 */
void GameScrollableTileContent_DrawAll(
    void *content,
    GameScrollableTileDrawConfig *config)
{
    NtmvM2dPage *page = (NtmvM2dPage *)content;
    NtmvM2dPoint origin = {0, 0};

    if (page->root_pane != 0) {
        NtmvM2dPane_UpdateTree_020b5744(page->root_pane, config, &origin);
    }
}

/* 0x020b523c */
void GameScrollableTileContent_DrawRegion(
    void *content,
    GameScrollableTileDrawConfig *config)
{
    NtmvM2dPage *page = (NtmvM2dPage *)content;

    GameScrollableTileContent_FillBackgroundRows_020b5264(page, config);
    if (page->root_pane != 0) {
        NtmvM2dPane_DrawTree_020b57f4(page->root_pane, config);
    }
}

/* 0x020b5264 */
void GameScrollableTileContent_FillBackgroundRows_020b5264(
    const void *content,
    GameScrollableTileDrawConfig *config)
{
    const NtmvM2dPage *page = (const NtmvM2dPage *)content;
    int32_t bottom = page->viewport_extent;
    int32_t y = config->update_bounds.top;
    uint16_t fill_value = PageBackgroundFillValue(page);

    if (bottom <= page->content_extent) {
        bottom = page->content_extent;
    }
    if (config->update_bounds.bottom <= bottom) {
        bottom = config->update_bounds.bottom;
    }
    if (bottom <= y) {
        return;
    }

    do {
        uint16_t *row = ScrollableTileRowAddress(config, y);
        int32_t x = config->update_bounds.left;

        if (x < config->update_bounds.right) {
            do {
                row[x] = fill_value;
                x = ClampToS16(x + 1);
            } while (x < config->update_bounds.right);
        }
        y = ClampToS16(y + 1);
    } while (y < bottom);
}

/* 0x020b52f8 */
void GameScrollableTileContent_GetBounds(
    GameScrollableTileBounds *bounds,
    const void *content)
{
    const NtmvM2dPage *page = (const NtmvM2dPage *)content;

    bounds->left = 0;
    bounds->top = 0;
    bounds->right = (int16_t)page->content_width;
    bounds->bottom = page->content_extent;
}
