#include "ntmv/ui_renderer.h"
#include "ntmv/owner_info.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static uint32_t sSequences[8];
static uint32_t sCell;
static unsigned int sInitCalls;
static unsigned int sTickCalls;
static unsigned int sFrameCalls;
static unsigned int sMakeCalls;
static unsigned int sEntryCalls;
static unsigned int sFlushStep;
static int32_t sTickValue;
static uint32_t sFrame;
static uint32_t sGenerated;
static uint32_t sAvailable;
static NtmvUiRendererTranslation sTranslation;
static NtmvUiRenderer *sRenderer;
static uint32_t sSetupEngine;
static uint32_t sMappingTarget;
static unsigned int sSetupCalls;
static uint32_t sCellFile;
static uint32_t sAnimationFile;
static uint32_t sCellBank;
static uint32_t sAnimationBank;
static uint32_t sCharacterData;
static uint32_t sPaletteData;
static uint32_t sScreenData[3];
static unsigned int sPaletteLoadCalls;
static unsigned int sScreenLoadCalls;
static unsigned int sBgLoadCalls;

void NNS_G2dGetNewOamManagerInstanceAsFastTransferMode(
    void *manager,
    uint32_t first_oam,
    uint32_t oam_count,
    uint32_t engine)
{
    assert(manager == sRenderer->oam_manager);
    assert(first_oam == 0 && oam_count == 0x80);
    sSetupEngine = engine;
    ++sSetupCalls;
}

const void *Game_LoadG2dCellBank(void **output, const char *path)
{
    assert(strstr(path, ".NCER") != NULL);
    *output = &sCellBank;
    return &sCellFile;
}

const void *Game_LoadG2dAnimationBank(void **output, const char *path)
{
    assert(strstr(path, ".NANR") != NULL);
    *output = &sAnimationBank;
    return &sAnimationFile;
}

const void *Game_LoadG2dCharacterData(void **output, const char *path)
{
    assert(strstr(path, ".NCGR") != NULL);
    *output = &sCharacterData;
    return &sCharacterData;
}

const void *Game_LoadG2dPaletteData(void **output, const char *path)
{
    assert(strcmp(path, "M2U:/data/obj/change_color_07.NCLR") == 0);
    *output = &sPaletteData;
    return &sPaletteData;
}

const void *Game_LoadG2dScreenData(void **output, const char *path)
{
    static const char *const expected[3] = {
        "M2U:/data/obj/nlv_skin_bg_03.NSCR",
        "M2U:/data/obj/nlv_skin_bg_02.NSCR",
        "M2U:/data/obj/nlv_skin_bg_00.NSCR",
    };
    assert(sScreenLoadCalls < 3);
    assert(strcmp(path, expected[sScreenLoadCalls]) == 0);
    *output = &sScreenData[sScreenLoadCalls];
    return &sScreenData[sScreenLoadCalls++];
}

void OS_GetOwnerInfo(NtmvOwnerInfo *owner_info)
{
    memset(owner_info, 0, sizeof(*owner_info));
    owner_info->language = 1;
    owner_info->favorite_color = 7;
}

int Ntmv_FormatPath_0201dc94(
    char *output, uint32_t capacity, const char *format, ...)
{
    assert(capacity == 0x40);
    assert(strcmp(format, "M2U:/data/obj/change_color_%02u.NCLR") == 0);
    strcpy(output, "M2U:/data/obj/change_color_07.NCLR");
    return 37;
}

void NNS_G2dInitImagePaletteProxy(void *proxy)
{
    memset(proxy, 0, 0x14);
}

void NNS_G2dLoadPalette(
    const void *palette_data,
    uint32_t vram_offset,
    uint32_t mapping_target,
    void *proxy)
{
    assert(palette_data == &sPaletteData);
    assert(vram_offset == 0 && proxy != NULL);
    assert(mapping_target == sPaletteLoadCalls + 1);
    ++sPaletteLoadCalls;
}

void NNS_G2dBGLoadElementsEx(
    uint32_t background,
    const void *screen_data,
    const void *character_data,
    const void *palette_data,
    uint32_t character_offset,
    uint32_t palette_offset)
{
    assert(background == sBgLoadCalls);
    assert(screen_data == &sScreenData[sBgLoadCalls]);
    assert(character_offset == 0 && palette_offset == 0);
    if (background == 0) {
        assert(character_data == &sCharacterData);
        assert(palette_data == &sPaletteData);
    } else {
        assert(character_data == NULL && palette_data == NULL);
    }
    ++sBgLoadCalls;
}

void NNS_G2dInitImageProxy(void *proxy)
{
    memset(proxy, 0, 0x24);
}

void NNS_G2dLoadImage1DMapping(
    const void *character_data,
    uint32_t vram_offset,
    uint32_t mapping_target,
    void *proxy)
{
    assert(character_data == &sCharacterData);
    assert(vram_offset == 0 && proxy != NULL);
    sMappingTarget = mapping_target;
}

const void *NNS_G2dGetAnimSequenceByIdx(
    const void *animation_bank, uint16_t sequence_index)
{
    assert(animation_bank == sSequences);
    if (sequence_index >= 8 || sSequences[sequence_index] == 0) {
        return NULL;
    }
    return &sSequences[sequence_index];
}

void NNS_G2dInitCellAnimation(
    NtmvUiCellAnimation *animation,
    const void *sequence,
    const void *cell_bank)
{
    assert(sequence == &sSequences[3]);
    assert(cell_bank == &sCell);
    animation->current_cell = &sCell;
    ++sInitCalls;
}

void NNS_G2dTickCellAnimation(
    NtmvUiCellAnimation *animation, int32_t frames_fx32)
{
    assert(animation->current_cell == &sCell);
    sTickValue = frames_fx32;
    ++sTickCalls;
}

void NNS_G2dSetCellAnimationCurrentFrame(
    NtmvUiCellAnimation *animation, uint32_t frame)
{
    assert(animation->current_cell == &sCell);
    sFrame = frame;
    ++sFrameCalls;
}

uint32_t NNS_G2dMakeCellToOams(
    void *output,
    uint32_t capacity,
    const void *cell,
    const void *affine,
    const NtmvUiRendererTranslation *translation,
    void *attribute_callback,
    void *callback_argument)
{
    assert(output == sRenderer->oam_buffer);
    assert(capacity == NTMV_UI_RENDERER_OAM_CAPACITY);
    assert(cell == &sCell);
    assert(affine == NULL);
    assert(attribute_callback == NULL && callback_argument == NULL);
    sTranslation = *translation;
    ++sMakeCalls;
    return sGenerated;
}

uint32_t NNS_G2dGetOamManagerOamCapacity(const void *manager)
{
    assert(manager == sRenderer->oam_manager);
    return sAvailable;
}

void NNS_G2dEntryOamManagerOam(
    void *manager, const void *oam_entries, uint32_t count)
{
    assert(manager == sRenderer->oam_manager);
    assert(oam_entries == sRenderer->oam_buffer);
    assert(count == sGenerated);
    ++sEntryCalls;
}

void NNS_G2dApplyOamManagerToHW(void *manager)
{
    assert(manager == sRenderer->oam_manager);
    assert(sFlushStep == 0);
    sFlushStep = 1;
}

void NNS_G2dResetOamManagerBuffer(void *manager)
{
    assert(manager == sRenderer->oam_manager);
    assert(sFlushStep == 1);
    sFlushStep = 2;
}

int main(void)
{
    NtmvUiRenderer renderer;
    int32_t handle;

    memset(&renderer, 0, sizeof(renderer));
    sRenderer = &renderer;
    memset(&renderer, 0xa5, sizeof(renderer));
    assert(NtmvUiRenderer_Init(&renderer) == &renderer);
    {
        const uint8_t *bytes = (const uint8_t *)&renderer;
        size_t index;
        for (index = 0; index < sizeof(renderer); ++index) {
            assert(bytes[index] == 0);
        }
    }
    NtmvUiRenderer_SetupMain(&renderer, (void *)(uintptr_t)0x1111);
    assert(renderer.owner == (void *)(uintptr_t)0x1111);
    assert(renderer.cell_resource_file == &sCellFile);
    assert(renderer.animation_resource_file == &sAnimationFile);
    assert(renderer.cell_bank == &sCellBank);
    assert(renderer.animation_bank == &sAnimationBank);
    assert(sSetupEngine == 0 && sMappingTarget == 1 && sSetupCalls == 1);
    NtmvUiRenderer_SetupSub(&renderer, (void *)(uintptr_t)0x2222);
    assert(renderer.owner == (void *)(uintptr_t)0x2222);
    assert(sSetupEngine == 1 && sMappingTarget == 2 && sSetupCalls == 2);
    NtmvUiRenderer_Finalize(&renderer);
    NtmvUiRenderer_LoadBackgroundAssets();
    assert(sPaletteLoadCalls == 2);
    assert(sScreenLoadCalls == 3 && sBgLoadCalls == 3);

    renderer.cell_bank = &sCell;
    renderer.animation_bank = sSequences;
    sSequences[3] = 0x1234;

    handle = NtmvUiRenderer_CreateAnimation(&renderer, 3);
    assert(handle == 0 && renderer.animation_count == 1);
    assert(sInitCalls == 1);
    assert(NtmvUiRenderer_GetAnimation(&renderer, 0) ==
        &renderer.animations[0]);
    assert(NtmvUiRenderer_GetAnimation(&renderer, 1) == NULL);

    assert(NtmvUiRenderer_CreateAnimation(&renderer, 4) == -1);
    assert(renderer.animation_count == 1 && sInitCalls == 1);

    assert(NtmvUiRenderer_TickAnimation(&renderer, 0, 0x1000));
    assert(sTickCalls == 1 && sTickValue == 0x1000);
    assert(!NtmvUiRenderer_TickAnimation(&renderer, 9, 0x1000));
    assert(sTickCalls == 1);
    assert(NtmvUiRenderer_SetAnimationFrame(&renderer, 0, 7));
    assert(sFrameCalls == 1 && sFrame == 7);
    assert(!NtmvUiRenderer_SetAnimationFrame(&renderer, 9, 8));

    sGenerated = 3;
    sAvailable = 4;
    assert(NtmvUiRenderer_SubmitCell(&renderer, 0, 12, -34));
    assert(sMakeCalls == 1 && sEntryCalls == 1);
    assert(sTranslation.x == 12 && sTranslation.y == -34);

    /* Equality is deliberately rejected by the target's unsigned bcs. */
    sAvailable = 3;
    assert(!NtmvUiRenderer_SubmitCell(&renderer, 0, 1, 2));
    assert(sMakeCalls == 2 && sEntryCalls == 1);
    renderer.animations[0].current_cell = NULL;
    assert(!NtmvUiRenderer_SubmitCell(&renderer, 0, 1, 2));
    assert(sMakeCalls == 2);

    NtmvUiRenderer_FlushOam(&renderer);
    assert(sFlushStep == 2);

    renderer.animation_count = NTMV_UI_RENDERER_USABLE_ANIMATIONS - 1;
    handle = NtmvUiRenderer_CreateAnimation(&renderer, 3);
    assert(handle == NTMV_UI_RENDERER_USABLE_ANIMATIONS - 1);
    assert(renderer.animation_count == NTMV_UI_RENDERER_USABLE_ANIMATIONS);
    assert(NtmvUiRenderer_CreateAnimation(&renderer, 3) == -1);
    assert(renderer.animation_count == NTMV_UI_RENDERER_USABLE_ANIMATIONS);
    return 0;
}
