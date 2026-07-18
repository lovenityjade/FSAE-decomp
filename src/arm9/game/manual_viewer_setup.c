#include "game/manual_viewer_lifecycle.h"

#include "game/graphics_state.h"
#include "game/manual_graphics.h"
#include "game/manual_locale.h"
#include "game/manual_page_loader.h"
#include "game/manual_viewer_factories.h"
#include "game/scrollable_tile_buffer.h"
#include "game/sound_manager.h"
#include "ntmv/m2d/text_box.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    GAME_MANUAL_SHARED_ARCHIVE_OFFSET = 0x18,
    GAME_MANUAL_LOCALIZED_ARCHIVE_OFFSET = 0x84,
    GAME_MANUAL_SOUND_MANAGER_OFFSET = 0x0fc,
    GAME_MANUAL_TILEMAP_ROWS_OFFSET = 0x370,
    GAME_MANUAL_RENDERER_0_OFFSET = 0x340,
    GAME_MANUAL_RENDERER_1_OFFSET = 0x344,
    GAME_MANUAL_ROOT_PANEL_0_OFFSET = 0x34c,
    GAME_MANUAL_ROOT_PANEL_1_OFFSET = 0x350,
    GAME_MANUAL_HEADER_PANEL_OFFSET = 0x354,
    GAME_MANUAL_TOC_PANEL_OFFSET = 0x358,
    GAME_MANUAL_PAGE_RESOURCE_OFFSET = 0x35c,
    GAME_MANUAL_SCROLL_BUTTON_OFFSET = 0x360,
    GAME_MANUAL_SCROLL_INDICATOR_0_OFFSET = 0x364,
    GAME_MANUAL_SCROLL_INDICATOR_1_OFFSET = 0x368,
    GAME_MANUAL_SCROLL_BUFFER_OFFSET = 0x40370,
    GAME_MANUAL_SHARED_ACCESSOR_OFFSET = 0x403f8,
    GAME_MANUAL_LOCALIZED_ACCESSOR_OFFSET = 0x40500,
    GAME_MANUAL_RESOURCE_TYPE_UI_PAGE = 0x6e747067,
    GAME_MANUAL_VIEWPORT_WIDTH = 0x100,
    GAME_MANUAL_COMPACT_HEIGHT = 0xae,
    GAME_MANUAL_EXPANDED_HEIGHT = 0x142,
    GAME_MANUAL_RING_ROWS = 0x200
};

static const char sSharedArchiveName[] = "M2M";
static const char sSharedRootPath[] = "M2M:/arc";
static const char sLocalizedRootPath[] = "M2M:/gpArc";
static const char sLocalizedArchiveName[] = "M2U";
static const char sSoundArchivePath[] =
    "M2U:/data/snd/sound_data.sdat";
static const char sTileStepPath[] = "M2U:/data/nttf/wallpaper";
static const char sUiTextPageName[] = "000uitext";
static const uint16_t sSubIndicatorSequences[2] = {1, 2};
static const uint16_t sMainIndicatorSequences[2] = {0x28, 0x28};

extern bool NNS_FndMountArchive(
    void *archive,
    const char *archive_name,
    void *archive_memory);
extern const void *NNS_FndGetArchiveFileByName(const char *path);
extern void NNS_G2dInitOamManagerModule(void);
extern void OS_WaitVBlankIntr(void);

#if UINTPTR_MAX > UINT32_MAX
extern void GameManualViewer_StoreHostPointer(
    void *object,
    uint32_t offset,
    void *value);
#endif

static void StoreEmbeddedPointer(void *object, uint32_t offset, void *value)
{
#if UINTPTR_MAX > UINT32_MAX
    GameManualViewer_StoreHostPointer(object, offset, value);
#else
    uint8_t *bytes = (uint8_t *)object + offset;
    uintptr_t encoded = (uintptr_t)value;
    uint32_t index;

    for (index = 0; index < sizeof(void *); ++index) {
        bytes[index] = (uint8_t)(encoded >> (index * 8));
    }
#endif
}

static void ConfigureRootPanel(
    NtmvM2dUIPanel *panel,
    NtmvM2dAllocatorContext *allocator,
    NtmvM2dUIElement *first,
    NtmvM2dUIElement *second)
{
    NtmvM2dUIElement *children[2] = {first, second};
    const NtmvM2dPoint position = {0, 0};
    const NtmvM2dSize size = {
        GAME_MANUAL_VIEWPORT_WIDTH,
        0xc0
    };

    NtmvM2dUIPanel_Configure(
        panel, allocator, children, 2, &position, &size);
}

/* 0x020bc99c */
void GameManualViewer_Setup_020bc99c(
    GameManualViewerPageFlow *viewer,
    void *shared_archive,
    void *localized_archive,
    const uint32_t tilemap_context[3],
    uint16_t language)
{
    uint8_t *bytes = (uint8_t *)viewer;
    void *allocator_handle = viewer;
    void *const *allocator_context = &allocator_handle;
    NtmvM2dAllocatorContext *panel_allocator =
        (NtmvM2dAllocatorContext *)&allocator_handle;
    GameScrollableTileBuffer *scroll_buffer =
        (GameScrollableTileBuffer *)(
            bytes + GAME_MANUAL_SCROLL_BUFFER_OFFSET);
    NtmvM2dArcResourceAccessor *shared_accessor =
        (NtmvM2dArcResourceAccessor *)(
            bytes + GAME_MANUAL_SHARED_ACCESSOR_OFFSET);
    NtmvM2dArcResourceAccessor *localized_accessor =
        (NtmvM2dArcResourceAccessor *)(
            bytes + GAME_MANUAL_LOCALIZED_ACCESSOR_OFFSET);
    NtmvUiRenderer *renderer0;
    NtmvUiRenderer *renderer1;
    NtmvM2dPageHeaderPanel *header;
    NtmvM2dScrollIndicator *sub_indicator;
    NtmvM2dScrollIndicator *main_indicator;
    NtmvM2dUIPanel *sub_root;
    NtmvM2dUIPanel *main_root;
    GameManualPage *ui_page;
    NtmvM2dManualTocPanel *toc_panel;
    const void *ui_page_resource;
    const uint16_t *control_texts[3];
    const NtmvM2dPoint sub_indicator_position = {0xf9, 0x69};
    const NtmvM2dPoint main_indicator_position = {0xf9, -0x0d};
    uint32_t index;

    GameManualViewer_SetupGraphics_020bdb34();
    bytes[0x80] = NNS_FndMountArchive(
        bytes + GAME_MANUAL_SHARED_ARCHIVE_OFFSET,
        sSharedArchiveName,
        shared_archive) ? 1 : 0;
    (void)NtmvM2dArcResourceAccessor_SetRootPath(
        localized_accessor, sSharedRootPath);
    (void)NtmvM2dArcResourceAccessor_SetRootPath(
        shared_accessor, sLocalizedRootPath);
    bytes[0xec] = NNS_FndMountArchive(
        bytes + GAME_MANUAL_LOCALIZED_ARCHIVE_OFFSET,
        sLocalizedArchiveName,
        localized_archive) ? 1 : 0;

    GameManualViewer_SetTilemapContext(viewer, tilemap_context);
    GameSoundManager_Init(
        (GameSoundManager *)(bytes + GAME_MANUAL_SOUND_MANAGER_OFFSET),
        (void *)NNS_FndGetArchiveFileByName(sSoundArchivePath));
    GameScrollableTileBuffer_SetOwner(scroll_buffer, viewer);
    GameScrollableTileBuffer_SetTileStep(
        scroll_buffer,
        (const int16_t *)NNS_FndGetArchiveFileByName(sTileStepPath));
    scroll_buffer->rows = (const uint16_t *)(
        bytes + GAME_MANUAL_TILEMAP_ROWS_OFFSET);
    scroll_buffer->viewport_width = GAME_MANUAL_VIEWPORT_WIDTH;
    scroll_buffer->viewport_height = GAME_MANUAL_EXPANDED_HEIGHT;
    scroll_buffer->row_stride = GAME_MANUAL_VIEWPORT_WIDTH;
    scroll_buffer->ring_rows = GAME_MANUAL_RING_ROWS;

    NNS_G2dInitOamManagerModule();
    renderer0 = GameManualRenderer_Allocate_020bcde4(allocator_context);
    StoreEmbeddedPointer(
        viewer, GAME_MANUAL_RENDERER_0_OFFSET, renderer0);
    NtmvUiRenderer_SetupMain(renderer0, viewer);
    renderer1 = GameManualRenderer_Allocate_020bcde4(allocator_context);
    StoreEmbeddedPointer(
        viewer, GAME_MANUAL_RENDERER_1_OFFSET, renderer1);
    NtmvUiRenderer_SetupSub(renderer1, viewer);
    NtmvUiRenderer_LoadBackgroundAssets();

    header = GameManualPageHeaderPanel_Allocate_020bcdb8(
        allocator_context);
    StoreEmbeddedPointer(viewer, GAME_MANUAL_HEADER_PANEL_OFFSET, header);
    NtmvM2dPageHeaderPanel_Configure(
        header,
        renderer1,
        (const NtmvM2dPageHeaderContext *)(bytes + 0xf0));

    sub_indicator = GameManualScrollIndicator_Allocate_020bcd8c(
        allocator_context);
    StoreEmbeddedPointer(
        viewer, GAME_MANUAL_SCROLL_INDICATOR_0_OFFSET, sub_indicator);
    NtmvM2dScrollIndicator_Configure(
        sub_indicator,
        panel_allocator,
        renderer1,
        &sub_indicator_position,
        GAME_MANUAL_COMPACT_HEIGHT,
        GAME_MANUAL_COMPACT_HEIGHT,
        sSubIndicatorSequences);
    sub_root = GameManualUIPanel_Allocate_020ba748(allocator_context);
    StoreEmbeddedPointer(viewer, GAME_MANUAL_ROOT_PANEL_0_OFFSET, sub_root);
    ConfigureRootPanel(
        sub_root,
        panel_allocator,
        (NtmvM2dUIElement *)header,
        (NtmvM2dUIElement *)sub_indicator);

    GameManualViewer_SetLocale(
        (GameManualViewerLocaleContext *)viewer, language);
    ui_page_resource = shared_accessor->vtable->get_localized_file(
        shared_accessor,
        GAME_MANUAL_RESOURCE_TYPE_UI_PAGE,
        sUiTextPageName,
        NULL);
    ui_page = GameManualPage_Allocate_020bcd60(allocator_context);
    StoreEmbeddedPointer(
        viewer, GAME_MANUAL_PAGE_RESOURCE_OFFSET, ui_page);
    (void)GameManualPage_Load_020b4f00(
        ui_page,
        (void *)allocator_context,
        ui_page_resource,
        shared_accessor);
    for (index = 0; index < 3; ++index) {
        control_texts[index] = (const uint16_t *)
            ((NtmvM2dTextBox *)ui_page->root_pane->children[index])->text;
    }

    toc_panel = GameManualTocPanel_Allocate_020bcd34(allocator_context);
    StoreEmbeddedPointer(viewer, GAME_MANUAL_TOC_PANEL_OFFSET, toc_panel);
    NtmvM2dManualTocPanel_Configure(
        toc_panel,
        panel_allocator,
        renderer0,
        (const NtmvM2dItemsTextContext *)(bytes + 0xf0),
        ui_page->root_pane->children[1]->size.width,
        control_texts,
        (GameSoundManager *)(bytes + GAME_MANUAL_SOUND_MANAGER_OFFSET));
    StoreEmbeddedPointer(
        viewer,
        GAME_MANUAL_SCROLL_BUTTON_OFFSET,
        toc_panel->scroll_button);

    main_indicator = GameManualScrollIndicator_Allocate_020bcd8c(
        allocator_context);
    StoreEmbeddedPointer(
        viewer, GAME_MANUAL_SCROLL_INDICATOR_1_OFFSET, main_indicator);
    NtmvM2dScrollIndicator_Configure(
        main_indicator,
        panel_allocator,
        renderer0,
        &main_indicator_position,
        GAME_MANUAL_EXPANDED_HEIGHT,
        GAME_MANUAL_COMPACT_HEIGHT,
        sMainIndicatorSequences);
    main_root = GameManualUIPanel_Allocate_020ba748(allocator_context);
    StoreEmbeddedPointer(viewer, GAME_MANUAL_ROOT_PANEL_1_OFFSET, main_root);
    ConfigureRootPanel(
        main_root,
        panel_allocator,
        (NtmvM2dUIElement *)toc_panel,
        (NtmvM2dUIElement *)main_indicator);

    Game_SetBgPaletteColor0Both(0x7fff);
    GameManualViewer_InitializePageIndex(viewer);
    OS_WaitVBlankIntr();
}
