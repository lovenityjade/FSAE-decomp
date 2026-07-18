#include "game/manual_viewer_lifecycle.h"

#include "game/manual_locale.h"
#include "game/manual_page_loader.h"
#include "game/manual_viewer_factories.h"
#include "game/scrollable_tile_buffer.h"
#include "game/sound_manager.h"
#include "ntmv/m2d/text_box.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static uint8_t sViewer[0x40628];
static uint32_t sSharedArchiveMemory;
static uint32_t sLocalizedArchiveMemory;
static uint32_t sSoundData;
static int16_t sTileStep[2] = {1, 2};
static uint32_t sUiResource;
static NtmvUiRenderer sRenderer0;
static NtmvUiRenderer sRenderer1;
static NtmvM2dPageHeaderPanel sHeader;
static NtmvM2dScrollIndicator sSubIndicator;
static NtmvM2dScrollIndicator sMainIndicator;
static NtmvM2dUIPanel sSubRoot;
static NtmvM2dUIPanel sMainRoot;
static GameManualPage sUiPage;
static NtmvM2dPane sPageRoot;
static NtmvM2dTextBox sTextBoxes[3];
static NtmvM2dPane *sPageChildren[3];
static uint16_t sControlText0[] = {'A', 0};
static uint16_t sControlText1[] = {'B', 0};
static uint16_t sControlText2[] = {'C', 0};
static NtmvM2dManualTocPanel sTocPanel;
static NtmvM2dUIElement sScrollButton;
static void *sStoredPointers[11];
static char sEvents[64];
static unsigned int sEventCount;
static unsigned int sMountCount;
static unsigned int sRootPathCount;
static unsigned int sArchiveFileCount;
static unsigned int sRendererFactoryCount;
static unsigned int sIndicatorFactoryCount;
static unsigned int sPanelFactoryCount;
static unsigned int sIndicatorConfigureCount;
static unsigned int sPanelConfigureCount;

static void Event(char event)
{
    assert(sEventCount + 1 < sizeof(sEvents));
    sEvents[sEventCount++] = event;
    sEvents[sEventCount] = '\0';
}

static unsigned int PointerIndex(uint32_t offset)
{
    switch (offset) {
    case 0x340: return 0;
    case 0x344: return 1;
    case 0x34c: return 2;
    case 0x350: return 3;
    case 0x354: return 4;
    case 0x358: return 5;
    case 0x35c: return 6;
    case 0x360: return 7;
    case 0x364: return 8;
    case 0x368: return 9;
    default: assert(0); return 10;
    }
}

void GameManualViewer_StoreHostPointer(
    void *object,
    uint32_t offset,
    void *value)
{
    assert(object == sViewer);
    sStoredPointers[PointerIndex(offset)] = value;
}

void GameManualViewer_SetupGraphics_020bdb34(void)
{
    Event('G');
}

bool NNS_FndMountArchive(
    void *archive,
    const char *archive_name,
    void *archive_memory)
{
    Event('M');
    if (sMountCount++ == 0) {
        assert(archive == sViewer + 0x18);
        assert(strcmp(archive_name, "M2M") == 0);
        assert(archive_memory == &sSharedArchiveMemory);
        return true;
    }
    assert(archive == sViewer + 0x84);
    assert(strcmp(archive_name, "M2U") == 0);
    assert(archive_memory == &sLocalizedArchiveMemory);
    return false;
}

bool NtmvM2dArcResourceAccessor_SetRootPath(
    NtmvM2dArcResourceAccessor *accessor,
    const char *root_path)
{
    Event('R');
    if (sRootPathCount++ == 0) {
        assert(accessor ==
            (NtmvM2dArcResourceAccessor *)(sViewer + 0x40500));
        assert(strcmp(root_path, "M2M:/arc") == 0);
    } else {
        assert(accessor ==
            (NtmvM2dArcResourceAccessor *)(sViewer + 0x403f8));
        assert(strcmp(root_path, "M2M:/gpArc") == 0);
    }
    return true;
}

void GameManualViewer_SetTilemapContext(
    GameManualViewerPageFlow *viewer,
    const uint32_t context_words[3])
{
    static const uint32_t expected[3] = {3, 4, 5};

    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    assert(memcmp(context_words, expected, sizeof(expected)) == 0);
    Event('C');
}

const void *NNS_FndGetArchiveFileByName(const char *path)
{
    Event('F');
    if (sArchiveFileCount++ == 0) {
        assert(strcmp(path, "M2U:/data/snd/sound_data.sdat") == 0);
        return &sSoundData;
    }
    assert(strcmp(path, "M2U:/data/nttf/wallpaper") == 0);
    return sTileStep;
}

void GameSoundManager_Init(GameSoundManager *manager, void *memory)
{
    assert(manager == (GameSoundManager *)(sViewer + 0xfc));
    assert(memory == &sSoundData);
    Event('S');
}

void GameScrollableTileBuffer_SetOwner(
    GameScrollableTileBuffer *buffer,
    void *owner)
{
    assert(buffer == (GameScrollableTileBuffer *)(sViewer + 0x40370));
    assert(owner == sViewer);
    Event('O');
}

void GameScrollableTileBuffer_SetTileStep(
    GameScrollableTileBuffer *buffer,
    const int16_t *tile_step)
{
    assert(buffer == (GameScrollableTileBuffer *)(sViewer + 0x40370));
    assert(tile_step == sTileStep);
    buffer->tile_step = tile_step;
    Event('T');
}

void NNS_G2dInitOamManagerModule(void)
{
    Event('A');
}

NtmvUiRenderer *GameManualRenderer_Allocate_020bcde4(
    void *const *allocator_context)
{
    assert(*allocator_context == sViewer);
    Event('r');
    return sRendererFactoryCount++ == 0 ? &sRenderer0 : &sRenderer1;
}

void NtmvUiRenderer_SetupMain(NtmvUiRenderer *renderer, void *owner)
{
    assert(renderer == &sRenderer0 && owner == sViewer);
    Event('0');
}

void NtmvUiRenderer_SetupSub(NtmvUiRenderer *renderer, void *owner)
{
    assert(renderer == &sRenderer1 && owner == sViewer);
    Event('1');
}

void NtmvUiRenderer_LoadBackgroundAssets(void)
{
    Event('B');
}

NtmvM2dPageHeaderPanel *GameManualPageHeaderPanel_Allocate_020bcdb8(
    void *const *allocator_context)
{
    assert(*allocator_context == sViewer);
    Event('H');
    return &sHeader;
}

void NtmvM2dPageHeaderPanel_Configure(
    NtmvM2dPageHeaderPanel *panel,
    void *animation_manager,
    const NtmvM2dPageHeaderContext *context)
{
    assert(panel == &sHeader);
    assert(animation_manager == &sRenderer1);
    assert(context == (const NtmvM2dPageHeaderContext *)(sViewer + 0xf0));
    Event('h');
}

NtmvM2dScrollIndicator *GameManualScrollIndicator_Allocate_020bcd8c(
    void *const *allocator_context)
{
    assert(*allocator_context == sViewer);
    Event('I');
    return sIndicatorFactoryCount++ == 0 ?
        &sSubIndicator : &sMainIndicator;
}

void NtmvM2dScrollIndicator_Configure(
    NtmvM2dScrollIndicator *indicator,
    NtmvM2dAllocatorContext *allocator,
    void *animation_manager,
    const NtmvM2dPoint *position,
    int16_t height,
    int16_t page_size,
    const uint16_t animation_sequences[2])
{
    assert(allocator->nns_allocator == sViewer);
    Event('i');
    if (sIndicatorConfigureCount++ == 0) {
        assert(indicator == &sSubIndicator);
        assert(animation_manager == &sRenderer1);
        assert(position->x == 0xf9 && position->y == 0x69);
        assert(height == 0xae && page_size == 0xae);
        assert(animation_sequences[0] == 1 && animation_sequences[1] == 2);
    } else {
        assert(indicator == &sMainIndicator);
        assert(animation_manager == &sRenderer0);
        assert(position->x == 0xf9 && position->y == -0x0d);
        assert(height == 0x142 && page_size == 0xae);
        assert(animation_sequences[0] == 0x28);
        assert(animation_sequences[1] == 0x28);
    }
}

NtmvM2dUIPanel *GameManualUIPanel_Allocate_020ba748(
    void *const *allocator_context)
{
    assert(*allocator_context == sViewer);
    Event('P');
    return sPanelFactoryCount++ == 0 ? &sSubRoot : &sMainRoot;
}

void NtmvM2dUIPanel_Configure(
    NtmvM2dUIPanel *panel,
    NtmvM2dAllocatorContext *allocator,
    NtmvM2dUIElement *const *children,
    uint16_t child_count,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size)
{
    assert(allocator->nns_allocator == sViewer);
    assert(child_count == 2);
    assert(position->x == 0 && position->y == 0);
    assert(size->width == 0x100 && size->height == 0xc0);
    Event('p');
    if (sPanelConfigureCount++ == 0) {
        assert(panel == &sSubRoot);
        assert(children[0] == (NtmvM2dUIElement *)&sHeader);
        assert(children[1] == (NtmvM2dUIElement *)&sSubIndicator);
    } else {
        assert(panel == &sMainRoot);
        assert(children[0] == (NtmvM2dUIElement *)&sTocPanel);
        assert(children[1] == (NtmvM2dUIElement *)&sMainIndicator);
    }
}

void GameManualViewer_SetLocale(
    GameManualViewerLocaleContext *viewer,
    uint32_t requested_locale)
{
    assert(viewer == (GameManualViewerLocaleContext *)sViewer);
    assert(requested_locale == 0x656e);
    Event('L');
}

static void *GetLocalizedFile(
    NtmvM2dArcResourceAccessor *accessor,
    uint32_t resource_type,
    const char *name,
    void *unused_result)
{
    assert(accessor ==
        (NtmvM2dArcResourceAccessor *)(sViewer + 0x403f8));
    assert(resource_type == 0x6e747067);
    assert(strcmp(name, "000uitext") == 0);
    assert(unused_result == 0);
    Event('X');
    return &sUiResource;
}

GameManualPage *GameManualPage_Allocate_020bcd60(
    void *const *allocator_context)
{
    assert(*allocator_context == sViewer);
    Event('Q');
    return &sUiPage;
}

bool GameManualPage_Load_020b4f00(
    GameManualPage *page,
    void *allocator_context,
    const void *resource,
    NtmvM2dArcResourceAccessor *accessor)
{
    assert(page == &sUiPage);
    assert(*(void **)allocator_context == sViewer);
    assert(resource == &sUiResource);
    assert(accessor ==
        (NtmvM2dArcResourceAccessor *)(sViewer + 0x403f8));
    Event('q');
    return true;
}

NtmvM2dManualTocPanel *GameManualTocPanel_Allocate_020bcd34(
    void *const *allocator_context)
{
    assert(*allocator_context == sViewer);
    Event('U');
    return &sTocPanel;
}

void NtmvM2dManualTocPanel_Configure(
    NtmvM2dManualTocPanel *panel,
    NtmvM2dAllocatorContext *allocator,
    void *animation_manager,
    const NtmvM2dItemsTextContext *text_context,
    int16_t layout_threshold,
    const uint16_t *const control_texts[3],
    void *feedback_context)
{
    assert(panel == &sTocPanel);
    assert(allocator->nns_allocator == sViewer);
    assert(animation_manager == &sRenderer0);
    assert(text_context == (const NtmvM2dItemsTextContext *)(sViewer + 0xf0));
    assert(layout_threshold == 77);
    assert(control_texts[0] == sControlText0);
    assert(control_texts[1] == sControlText1);
    assert(control_texts[2] == sControlText2);
    assert(feedback_context == sViewer + 0xfc);
    Event('u');
}

void Game_SetBgPaletteColor0Both(uint16_t color)
{
    assert(color == 0x7fff);
    Event('K');
}

void GameManualViewer_InitializePageIndex(GameManualViewerPageFlow *viewer)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    Event('J');
}

void OS_WaitVBlankIntr(void)
{
    Event('W');
}

int main(void)
{
    static const NtmvM2dArcResourceAccessorVTable accessor_vtable = {
        0, 0, 0, GetLocalizedFile, 0, 0,
    };
    static const uint32_t tilemap_context[3] = {3, 4, 5};
    GameScrollableTileBuffer *scroll_buffer =
        (GameScrollableTileBuffer *)(sViewer + 0x40370);
    NtmvM2dArcResourceAccessor *shared_accessor =
        (NtmvM2dArcResourceAccessor *)(sViewer + 0x403f8);

    memset(sViewer, 0, sizeof(sViewer));
    memset(sStoredPointers, 0, sizeof(sStoredPointers));
    sEventCount = 0;
    sEvents[0] = '\0';
    sTocPanel.scroll_button = &sScrollButton;
    sUiPage.root_pane = &sPageRoot;
    sPageRoot.children = sPageChildren;
    sTextBoxes[0].text = (const uint8_t *)sControlText0;
    sTextBoxes[1].text = (const uint8_t *)sControlText1;
    sTextBoxes[2].text = (const uint8_t *)sControlText2;
    sPageChildren[0] = &sTextBoxes[0].pane;
    sPageChildren[1] = &sTextBoxes[1].pane;
    sPageChildren[2] = &sTextBoxes[2].pane;
    sPageChildren[1]->size.width = 77;
    shared_accessor->vtable = &accessor_vtable;

    GameManualViewer_Setup_020bc99c(
        (GameManualViewerPageFlow *)sViewer,
        &sSharedArchiveMemory,
        &sLocalizedArchiveMemory,
        tilemap_context,
        0x656e);

    assert(strcmp(
        sEvents,
        "GMRRMCFSOFTAr0r1BHhIiPpLXQqUuIiPpKJW") == 0);
    assert(sViewer[0x80] == 1);
    assert(sViewer[0xec] == 0);
    assert(scroll_buffer->rows == (const uint16_t *)(sViewer + 0x370));
    assert(scroll_buffer->viewport_width == 0x100);
    assert(scroll_buffer->viewport_height == 0x142);
    assert(scroll_buffer->row_stride == 0x100);
    assert(scroll_buffer->ring_rows == 0x200);
    assert(scroll_buffer->tile_step == sTileStep);
    assert(sStoredPointers[0] == &sRenderer0);
    assert(sStoredPointers[1] == &sRenderer1);
    assert(sStoredPointers[2] == &sSubRoot);
    assert(sStoredPointers[3] == &sMainRoot);
    assert(sStoredPointers[4] == &sHeader);
    assert(sStoredPointers[5] == &sTocPanel);
    assert(sStoredPointers[6] == &sUiPage);
    assert(sStoredPointers[7] == &sScrollButton);
    assert(sStoredPointers[8] == &sSubIndicator);
    assert(sStoredPointers[9] == &sMainIndicator);
    return 0;
}
