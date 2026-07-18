#include "game/manual_frame.h"

#include "game/sound_manager.h"
#include "game/scrollable_tile_buffer.h"
#include "ntmv/m2d/manual_toc_panel.h"
#include "ntmv/m2d/scroll_controls.h"
#include "ntmv/m2d/ui_panel.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static uint8_t sViewer[0x40620];
static uint32_t sRenderContext0;
static uint32_t sRenderContext1;
static NtmvM2dUIPanel sPanels[2];
static NtmvM2dManualTocPanel sTocPanel;
static NtmvM2dScrollButton sScrollButton;
static NtmvM2dScrollIndicator sIndicators[2];
static bool sTickMoved;
static uint8_t sTickUpload;
static char sEvents[32];
static unsigned int sEventCount;
static unsigned int sRenderCount;

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
    switch (offset) {
    case 0x340:
        return &sRenderContext0;
    case 0x344:
        return &sRenderContext1;
    case 0x34c:
        return &sPanels[0];
    case 0x350:
        return &sPanels[1];
    case 0x358:
        return &sTocPanel;
    case 0x360:
        return &sScrollButton;
    case 0x364:
        return &sIndicators[0];
    case 0x368:
        return &sIndicators[1];
    default:
        assert(0);
        return 0;
    }
}

void GameSoundManager_Tick(GameSoundManager *manager)
{
    assert(manager == (GameSoundManager *)(sViewer + 0xfc));
    Event('S');
}

bool GameScrollableTileBuffer_Tick(
    GameScrollableTileBuffer *buffer,
    uint8_t *upload_required)
{
    assert(buffer == ScrollBuffer());
    *upload_required = sTickUpload;
    Event('T');
    return sTickMoved;
}

void NtmvM2dScrollButton_SetScrollOffset(
    NtmvM2dScrollButton *button,
    int16_t scroll_offset)
{
    assert(button == &sScrollButton);
    assert(scroll_offset == ScrollBuffer()->visible_start);
    Event('b');
}

void NtmvM2dScrollIndicator_SetScrollOffset(
    NtmvM2dScrollIndicator *indicator,
    int16_t scroll_offset)
{
    unsigned int index = sEvents[sEventCount - 1] == 'b' ? 0u : 1u;
    assert(indicator == &sIndicators[index]);
    assert(scroll_offset == ScrollBuffer()->visible_start);
    Event('i');
}

GameSoundEntry *GameSoundManager_PlaySequenceArc(
    GameSoundManager *manager,
    int32_t sequence_id,
    int32_t channel_priority)
{
    assert(manager == (GameSoundManager *)(sViewer + 0xfc));
    assert(sequence_id == 11);
    assert(channel_priority == 0x80);
    Event('P');
    return 0;
}

void GameScrollableTileBuffer_UpdateContent(
    GameScrollableTileBuffer *buffer)
{
    assert(buffer == ScrollBuffer());
    Event('U');
}

void DC_FlushRange(const void *address, uint32_t size)
{
    assert(address == sViewer + 0x370);
    assert(size == 0x40000);
    Event('F');
}

void NtmvM2dScrollButton_TickChildren(NtmvM2dScrollButton *button)
{
    assert(button == &sScrollButton);
    Event('C');
}

void NtmvM2dScrollIndicator_UpdateVisibility(
    NtmvM2dScrollIndicator *indicator,
    void *render_context)
{
    unsigned int index = sEvents[sEventCount - 1] == 'C' ? 0u : 1u;
    assert(indicator == &sIndicators[index]);
    assert(render_context == (index == 0 ?
        (void *)&sRenderContext1 : (void *)&sRenderContext0));
    Event('v');
}

void NtmvM2dManualTocPanel_Update(NtmvM2dManualTocPanel *panel)
{
    assert(panel == &sTocPanel);
    Event('M');
}

static void RenderPanel(
    NtmvM2dUIPanel *panel,
    void *render_context,
    const NtmvM2dPoint *parent_position)
{
    void *expected = sRenderCount == 0 ?
        (void *)&sRenderContext1 : (void *)&sRenderContext0;
    assert(panel == &sPanels[sRenderCount]);
    assert(*(void **)render_context == expected);
    assert(parent_position->x == 0 && parent_position->y == 0);
    ++sRenderCount;
    Event('r');
}

static const NtmvM2dUIPanelVTable sPanelVTable = {
    .render = RenderPanel,
};

static void Run(bool moved, bool upload, const char *events)
{
    memset(sViewer, 0, sizeof(sViewer));
    memset(sPanels, 0, sizeof(sPanels));
    sPanels[0].base.vtable =
        (const NtmvM2dUIElementVTable *)&sPanelVTable;
    sPanels[1].base.vtable =
        (const NtmvM2dUIElementVTable *)&sPanelVTable;
    ScrollBuffer()->visible_start = 37;
    sTickMoved = moved;
    sTickUpload = upload ? 1 : 0;
    sEventCount = 0;
    sRenderCount = 0;
    sEvents[0] = '\0';

    GameManualViewer_RenderFrame_020bd00c(
        (GameManualViewerPageFlow *)sViewer);
    assert(strcmp(sEvents, events) == 0);
    assert(sRenderCount == 2);
}

int main(void)
{
    Run(true, true, "STbiiPUFCvvMrr");
    Run(false, false, "STUFCvvMrr");
    return 0;
}
