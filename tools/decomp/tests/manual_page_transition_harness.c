#include "game/manual_page_transition.h"

#include "game/circular_tile_upload.h"
#include "game/manual_display.h"
#include "ntmv/m2d/manual_toc_panel.h"
#include "ntmv/ui_renderer.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static uint8_t sViewer[0x40620];
static NtmvUiRenderer sRenderers[2];
static NtmvM2dManualTocPanel sTocPanel;
static char sEvents[24];
static unsigned int sEventCount;
static unsigned int sFlushCount;
static unsigned int sAffineCount;
static unsigned int sUploadCount;
static int32_t sExpectedOffset;

volatile uint32_t gGameMainDisplayControl_04000000;
volatile uint32_t gGameSubDisplayControl_04001000;

static void Event(char event)
{
    assert(sEventCount + 1 < sizeof(sEvents));
    sEvents[sEventCount++] = event;
    sEvents[sEventCount] = '\0';
}

static GameScrollableTileBuffer *ScrollBuffer(void)
{
    return (GameScrollableTileBuffer *)(sViewer + 0x40370);
}

void *GameManualViewer_ResolveHostPointer(
    const void *object,
    uint32_t offset)
{
    assert(object == sViewer);
    if (offset == 0x340) {
        return &sRenderers[0];
    }
    if (offset == 0x344) {
        return &sRenderers[1];
    }
    assert(offset == 0x358);
    return &sTocPanel;
}

void NtmvUiRenderer_FlushOam(NtmvUiRenderer *renderer)
{
    assert(sFlushCount < 2);
    assert(renderer == &sRenderers[sFlushCount]);
    ++sFlushCount;
    Event('F');
}

void NtmvM2dManualTocPanel_PublishWindowRegisters_020baef4(
    const NtmvM2dManualTocPanel *panel)
{
    assert(panel == &sTocPanel);
    Event('P');
}

void MTX_Identity22_(int32_t matrix[4])
{
    matrix[0] = 0x1000;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 0x1000;
    Event('I');
}

void G2x_SetBGyAffine_(
    uintptr_t affine_register,
    const int32_t matrix[4],
    int32_t center_x,
    int32_t center_y,
    int32_t x,
    int32_t y)
{
    assert(sAffineCount < 2);
    assert(affine_register == (sAffineCount == 0 ?
        0x04001030u : 0x04000030u));
    assert(matrix[0] == 0x1000 && matrix[1] == 0);
    assert(matrix[2] == 0 && matrix[3] == 0x1000);
    assert(center_x == 0 && center_y == 0);
    assert(x == sExpectedOffset);
    assert(y == (sAffineCount == 0 ? 19 : 211));
    ++sAffineCount;
    Event('A');
}

void GameManualViewer_SetRevealWindowOffset(
    GameManualViewerDisplay *viewer,
    int32_t horizontal_offset)
{
    assert(viewer == (GameManualViewerDisplay *)sViewer);
    assert(horizontal_offset == sExpectedOffset);
    Event('R');
}

void Game_UploadSubBg3Screen_020bd2c0(
    const uint16_t *source,
    uint32_t destination_offset,
    uint32_t byte_count)
{
    (void)source;
    (void)destination_offset;
    (void)byte_count;
    assert(0);
}

void Game_UploadMainBg3Screen_020bd2cc(
    const uint16_t *source,
    uint32_t destination_offset,
    uint32_t byte_count)
{
    (void)source;
    (void)destination_offset;
    (void)byte_count;
    assert(0);
}

void Game_UploadCircularTileRows(
    const GameCircularTileBuffer *buffer,
    const GameCircularTileUploadRange *range,
    GameCircularTileUploadCallback upload)
{
    assert(buffer == ScrollBuffer());
    assert(sUploadCount < 2);
    if (sUploadCount == 0) {
        assert(range->leading_offset == 0);
        assert(range->wrapped_offset == 0xae);
        assert(upload == Game_UploadSubBg3Screen_020bd2c0);
    } else {
        assert(range->leading_offset == 0xae);
        assert(range->wrapped_offset == 0x142);
        assert(upload == Game_UploadMainBg3Screen_020bd2cc);
    }
    ++sUploadCount;
    Event('U');
}

static void Run(
    int32_t horizontal_offset,
    uint32_t main_control,
    uint32_t sub_control,
    uint8_t sub_screen_only,
    uint32_t expected_main,
    uint32_t expected_sub,
    const char *events)
{
    GameScrollableTileBuffer *buffer;

    memset(sViewer, 0, sizeof(sViewer));
    memset(&sTocPanel, 0, sizeof(sTocPanel));
    ((uint8_t *)&sTocPanel)[0xad] = sub_screen_only;
    buffer = ScrollBuffer();
    buffer->visible_start = 37;
    buffer->visible_end = 211;
    buffer->previous_start = -1;
    buffer->previous_end = -1;
    gGameMainDisplayControl_04000000 = main_control;
    gGameSubDisplayControl_04001000 = sub_control;
    sExpectedOffset = horizontal_offset;
    sEventCount = 0;
    sFlushCount = 0;
    sAffineCount = 0;
    sUploadCount = 0;
    sEvents[0] = '\0';

    GameManualViewer_SetPageTransitionOffset_020bd128(
        (GameManualViewerPageFlow *)sViewer,
        horizontal_offset);

    assert(strcmp(sEvents, events) == 0);
    assert(sFlushCount == 2);
    assert(sAffineCount == 2);
    assert(sUploadCount == 2);
    assert(gGameMainDisplayControl_04000000 == expected_main);
    assert(gGameSubDisplayControl_04001000 == expected_sub);
    assert(buffer->previous_start == buffer->visible_start);
    assert(buffer->previous_end == buffer->visible_end);
}

int main(void)
{
    Run(0, 0x1234e777, 0xabcd6005, 0, 0x12340777, 0xabcd0005,
        "FFPIAAUU");
    Run(0, 0x1234e777, 0xabcd0005, 0, 0x1234e777, 0xabcd0005,
        "FFPIAAUU");
    Run(32, 0x1234e777, 0xabcd0005, 0, 0x12342777, 0xabcd2005,
        "FFPIAARUU");
    Run(-32, 0x1234e777, 0xabcd4005, 1, 0x1234e777, 0xabcd4005,
        "FFPIAARUU");
    return 0;
}
