#include "ntmv/ui_renderer.h"
#include "game/g2d_resource_loader.h"
#include "ntmv/owner_info.h"

typedef struct NtmvUiImageProxy {
    uint8_t storage[0x24];
} NtmvUiImageProxy;

typedef struct NtmvUiPaletteProxy {
    uint8_t storage[0x14];
} NtmvUiPaletteProxy;

extern void NNS_G2dGetNewOamManagerInstanceAsFastTransferMode(
    void *manager,
    uint32_t first_oam,
    uint32_t oam_count,
    uint32_t engine);
extern void NNS_G2dInitImageProxy(NtmvUiImageProxy *proxy);
extern void NNS_G2dLoadImage1DMapping(
    const void *character_data,
    uint32_t vram_offset,
    uint32_t mapping_target,
    NtmvUiImageProxy *proxy);
extern int Ntmv_FormatPath_0201dc94(
    char *output, uint32_t capacity, const char *format, ...);
extern void NNS_G2dInitImagePaletteProxy(NtmvUiPaletteProxy *proxy);
extern void NNS_G2dLoadPalette(
    const void *palette_data,
    uint32_t vram_offset,
    uint32_t mapping_target,
    NtmvUiPaletteProxy *proxy);
extern void NNS_G2dBGLoadElementsEx(
    uint32_t background,
    const void *screen_data,
    const void *character_data,
    const void *palette_data,
    uint32_t character_offset,
    uint32_t palette_offset);

extern const void *NNS_G2dGetAnimSequenceByIdx(
    const void *animation_bank, uint16_t sequence_index);
extern void NNS_G2dInitCellAnimation(
    NtmvUiCellAnimation *animation,
    const void *sequence,
    const void *cell_bank);
extern void NNS_G2dTickCellAnimation(
    NtmvUiCellAnimation *animation, int32_t frames_fx32);
extern void NNS_G2dSetCellAnimationCurrentFrame(
    NtmvUiCellAnimation *animation, uint32_t frame);
extern uint32_t NNS_G2dMakeCellToOams(
    void *output,
    uint32_t capacity,
    const void *cell,
    const void *affine,
    const NtmvUiRendererTranslation *translation,
    void *attribute_callback,
    void *callback_argument);
extern uint32_t NNS_G2dGetOamManagerOamCapacity(const void *manager);
extern void NNS_G2dEntryOamManagerOam(
    void *manager, const void *oam_entries, uint32_t count);
extern void NNS_G2dApplyOamManagerToHW(void *manager);
extern void NNS_G2dResetOamManagerBuffer(void *manager);

static const char sMainCellPath[] = "M2U:/data/obj/nlv_skin.NCER";
static const char sMainAnimationPath[] = "M2U:/data/obj/nlv_skin.NANR";
static const char sMainCharacterPath[] = "M2U:/data/obj/nlv_skin.NCGR";
static const char sSubCellPath[] = "M2U:/data/obj/nlv_skin_up_obj.NCER";
static const char sSubAnimationPath[] = "M2U:/data/obj/nlv_skin_up_obj.NANR";
static const char sSubCharacterPath[] = "M2U:/data/obj/nlv_skin_up_obj.NCGR";
static const char sPalettePathFormat[] = "M2U:/data/obj/change_color_%02u.NCLR";
static const char sBackgroundCharacterPath[] = "M2U:/data/obj/nlv_skin_bg.NCGR";
static const char sBackgroundScreen3Path[] = "M2U:/data/obj/nlv_skin_bg_03.NSCR";
static const char sBackgroundScreen2Path[] = "M2U:/data/obj/nlv_skin_bg_02.NSCR";
static const char sBackgroundScreen0Path[] = "M2U:/data/obj/nlv_skin_bg_00.NSCR";

/* 0x020bbf54 */
NtmvUiRenderer *NtmvUiRenderer_Init(NtmvUiRenderer *renderer)
{
    uint8_t *bytes = (uint8_t *)renderer;
    size_t index;

    for (index = 0; index < sizeof(*renderer); ++index) {
        bytes[index] = 0;
    }
    return renderer;
}

/* 0x020bbfd0 */
void NtmvUiRenderer_LoadBackgroundAssets(void)
{
    char palette_path[0x40];
    void *palette_data = NULL;
    void *character_data = NULL;
    void *screen_data = NULL;
    NtmvUiPaletteProxy palette_proxy;

    (void)Ntmv_FormatPath_0201dc94(
        palette_path,
        sizeof(palette_path),
        sPalettePathFormat,
        Ntmv_GetSkinColor_020b7b00());
    (void)Game_LoadG2dPaletteData(&palette_data, palette_path);
    NNS_G2dInitImagePaletteProxy(&palette_proxy);
    NNS_G2dLoadPalette(palette_data, 0, 1, &palette_proxy);
    NNS_G2dLoadPalette(palette_data, 0, 2, &palette_proxy);

    (void)Game_LoadG2dCharacterData(
        &character_data, sBackgroundCharacterPath);
    (void)Game_LoadG2dScreenData(&screen_data, sBackgroundScreen3Path);
    NNS_G2dBGLoadElementsEx(
        0, screen_data, character_data, palette_data, 0, 0);
    (void)Game_LoadG2dScreenData(&screen_data, sBackgroundScreen2Path);
    NNS_G2dBGLoadElementsEx(1, screen_data, NULL, NULL, 0, 0);
    (void)Game_LoadG2dScreenData(&screen_data, sBackgroundScreen0Path);
    NNS_G2dBGLoadElementsEx(2, screen_data, NULL, NULL, 0, 0);
}

static void Setup(
    NtmvUiRenderer *renderer,
    void *owner,
    uint32_t engine,
    uint32_t mapping_target,
    const char *cell_path,
    const char *animation_path,
    const char *character_path)
{
    void *character_data = NULL;
    NtmvUiImageProxy image_proxy;

    renderer->owner = owner;
    NNS_G2dGetNewOamManagerInstanceAsFastTransferMode(
        renderer->oam_manager, 0, NTMV_UI_RENDERER_OAM_CAPACITY, engine);
    renderer->cell_resource_file =
        Game_LoadG2dCellBank((void **)&renderer->cell_bank, cell_path);
    renderer->animation_resource_file = Game_LoadG2dAnimationBank(
        (void **)&renderer->animation_bank, animation_path);
    (void)Game_LoadG2dCharacterData(&character_data, character_path);
    NNS_G2dInitImageProxy(&image_proxy);
    NNS_G2dLoadImage1DMapping(
        character_data, 0, mapping_target, &image_proxy);
}

/* 0x020bc0e4 */
void NtmvUiRenderer_SetupMain(NtmvUiRenderer *renderer, void *owner)
{
    Setup(
        renderer,
        owner,
        0,
        1,
        sMainCellPath,
        sMainAnimationPath,
        sMainCharacterPath);
}

/* 0x020bc170 */
void NtmvUiRenderer_SetupSub(NtmvUiRenderer *renderer, void *owner)
{
    Setup(
        renderer,
        owner,
        1,
        2,
        sSubCellPath,
        sSubAnimationPath,
        sSubCharacterPath);
}

/* 0x020bc1fc: retained empty lifecycle hook. */
void NtmvUiRenderer_Finalize(NtmvUiRenderer *renderer)
{
    (void)renderer;
}

/* 0x020bc200 */
int32_t NtmvUiRenderer_CreateAnimation(
    NtmvUiRenderer *renderer, uint16_t sequence_index)
{
    int32_t handle = -1;
    const void *sequence;

    if (renderer->animation_count + 1 >= NTMV_UI_RENDERER_ANIMATION_STORAGE) {
        return handle;
    }
    sequence = NNS_G2dGetAnimSequenceByIdx(
        renderer->animation_bank, sequence_index);
    if (sequence != NULL) {
        NNS_G2dInitCellAnimation(
            &renderer->animations[renderer->animation_count],
            sequence,
            renderer->cell_bank);
        handle = renderer->animation_count;
        renderer->animation_count = handle + 1;
    }
    return handle;
}

/*
 * 0x020bc260.  The original signed comparison assumes a non-negative handle;
 * all recovered callers enforce that invariant by retaining -1 as invalid.
 */
NtmvUiCellAnimation *NtmvUiRenderer_GetAnimation(
    NtmvUiRenderer *renderer, int32_t animation_handle)
{
    if (animation_handle < renderer->animation_count) {
        return &renderer->animations[animation_handle];
    }
    return NULL;
}

/* 0x020bc280 */
bool NtmvUiRenderer_TickAnimation(
    NtmvUiRenderer *renderer,
    int32_t animation_handle,
    int32_t frames_fx32)
{
    NtmvUiCellAnimation *animation =
        NtmvUiRenderer_GetAnimation(renderer, animation_handle);

    if (animation == NULL) {
        return false;
    }
    NNS_G2dTickCellAnimation(animation, frames_fx32);
    return true;
}

/* 0x020bc2a8 */
bool NtmvUiRenderer_SetAnimationFrame(
    NtmvUiRenderer *renderer,
    int32_t animation_handle,
    uint32_t frame)
{
    NtmvUiCellAnimation *animation =
        NtmvUiRenderer_GetAnimation(renderer, animation_handle);

    if (animation == NULL) {
        return false;
    }
    NNS_G2dSetCellAnimationCurrentFrame(animation, frame);
    return true;
}

/* 0x020bc2d0 */
bool NtmvUiRenderer_SubmitCell(
    NtmvUiRenderer *renderer,
    int32_t animation_handle,
    int32_t x_fx32,
    int32_t y_fx32)
{
    NtmvUiCellAnimation *animation =
        NtmvUiRenderer_GetAnimation(renderer, animation_handle);
    NtmvUiRendererTranslation translation = {x_fx32, y_fx32};
    uint32_t generated;
    uint32_t available;

    if (animation == NULL || animation->current_cell == NULL) {
        return false;
    }
    generated = NNS_G2dMakeCellToOams(
        renderer->oam_buffer,
        NTMV_UI_RENDERER_OAM_CAPACITY,
        animation->current_cell,
        NULL,
        &translation,
        NULL,
        NULL);
    available = NNS_G2dGetOamManagerOamCapacity(renderer->oam_manager);
    if (generated >= available) {
        return false;
    }
    NNS_G2dEntryOamManagerOam(
        renderer->oam_manager, renderer->oam_buffer, generated);
    return true;
}

/* 0x020bc36c */
void NtmvUiRenderer_FlushOam(NtmvUiRenderer *renderer)
{
    NNS_G2dApplyOamManagerToHW(renderer->oam_manager);
    NNS_G2dResetOamManagerBuffer(renderer->oam_manager);
}
