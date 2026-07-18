#include "game/scrollable_tile_buffer.h"

#include "ntmv/m2d/page.h"
#include "ntmv/m2d/pane.h"
#include "ntmv/m2d/render_surface.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static NtmvM2dPane sPane;
static unsigned int sUpdateTreeCount;
static unsigned int sDrawTreeCount;
static void *sUpdateContext;
static void *sDrawContext;
static NtmvM2dPoint sUpdateOrigin;

uint16_t *NtmvM2d_RenderSurfaceRowAddress_020b7508(
    const NtmvM2dRenderSurface *surface,
    uint32_t y)
{
    uint32_t wrapped_y = y & (uint32_t)(surface->row_wrap - 1);

    return surface->pixels + surface->row_stride * wrapped_y;
}

void NtmvM2dPane_UpdateTree_020b5744(
    NtmvM2dPane *pane,
    void *context,
    const NtmvM2dPoint *parent_position)
{
    assert(pane == &sPane);
    ++sUpdateTreeCount;
    sUpdateContext = context;
    sUpdateOrigin = *parent_position;
}

void NtmvM2dPane_DrawTree_020b57f4(
    NtmvM2dPane *pane,
    void *context)
{
    assert(pane == &sPane);
    ++sDrawTreeCount;
    sDrawContext = context;
}

static void ResetPixels(uint16_t *pixels, uint16_t value, unsigned int count)
{
    unsigned int index;

    for (index = 0; index < count; ++index) {
        pixels[index] = value;
    }
}

static NtmvM2dPage MakePage(void)
{
    NtmvM2dPage page;

    (void)memset(&page, 0, sizeof(page));
    page.root_pane = &sPane;
    page.content_width = 77;
    page.content_extent = 4;
    page.palette_color = 0x1234;
    page.viewport_extent = 6;
    return page;
}

static GameScrollableTileDrawConfig MakeConfig(uint16_t *pixels)
{
    GameScrollableTileDrawConfig config;

    (void)memset(&config, 0, sizeof(config));
    config.update_bounds.left = 1;
    config.update_bounds.top = 2;
    config.update_bounds.right = 4;
    config.update_bounds.bottom = 8;
    config.rows = pixels;
    config.row_stride = 8;
    config.ring_rows = 8;
    return config;
}

static void TestBoundsFromPageContentSize(void)
{
    NtmvM2dPage page = MakePage();
    GameScrollableTileBounds bounds;

    page.content_width = 123;
    page.content_extent = 231;
    GameScrollableTileContent_GetBounds(&bounds, &page);
    assert(bounds.left == 0);
    assert(bounds.top == 0);
    assert(bounds.right == 123);
    assert(bounds.bottom == 231);
}

static void TestDrawAllUpdatesRootFromZeroOrigin(void)
{
    NtmvM2dPage page = MakePage();
    GameScrollableTileDrawConfig config;

    (void)memset(&config, 0, sizeof(config));
    sUpdateTreeCount = 0;
    sUpdateContext = 0;
    sUpdateOrigin.x = -1;
    sUpdateOrigin.y = -1;
    GameScrollableTileContent_DrawAll(&page, &config);
    assert(sUpdateTreeCount == 1);
    assert(sUpdateContext == &config);
    assert(sUpdateOrigin.x == 0);
    assert(sUpdateOrigin.y == 0);

    page.root_pane = 0;
    GameScrollableTileContent_DrawAll(&page, &config);
    assert(sUpdateTreeCount == 1);
}

static void TestFillBackgroundRows(void)
{
    NtmvM2dPage page = MakePage();
    uint16_t pixels[8 * 8];
    GameScrollableTileDrawConfig config = MakeConfig(pixels);
    unsigned int y;
    unsigned int x;

    ResetPixels(pixels, 0x1111, 8 * 8);
    GameScrollableTileContent_FillBackgroundRows_020b5264(&page, &config);

    for (y = 0; y < 8; ++y) {
        for (x = 0; x < 8; ++x) {
            uint16_t expected = 0x1111;

            if (y >= 2 && y < 6 && x >= 1 && x < 4) {
                expected = 0x9234;
            }
            assert(pixels[y * 8 + x] == expected);
        }
    }

    ResetPixels(pixels, 0x2222, 8 * 8);
    config.update_bounds.top = 6;
    GameScrollableTileContent_FillBackgroundRows_020b5264(&page, &config);
    for (y = 0; y < 8 * 8; ++y) {
        assert(pixels[y] == 0x2222);
    }
}

static void TestDrawRegionFillsThenDrawsRoot(void)
{
    NtmvM2dPage page = MakePage();
    uint16_t pixels[8 * 8];
    GameScrollableTileDrawConfig config = MakeConfig(pixels);

    ResetPixels(pixels, 0x3333, 8 * 8);
    sDrawTreeCount = 0;
    sDrawContext = 0;
    GameScrollableTileContent_DrawRegion(&page, &config);
    assert(pixels[2 * 8 + 1] == 0x9234);
    assert(sDrawTreeCount == 1);
    assert(sDrawContext == &config);

    page.root_pane = 0;
    GameScrollableTileContent_DrawRegion(&page, &config);
    assert(sDrawTreeCount == 1);
}

int main(void)
{
    TestBoundsFromPageContentSize();
    TestDrawAllUpdatesRootFromZeroOrigin();
    TestFillBackgroundRows();
    TestDrawRegionFillsThenDrawsRoot();
    return 0;
}
