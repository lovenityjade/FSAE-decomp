#ifndef FSAE_NTMV_UI_RENDERER_H
#define FSAE_NTMV_UI_RENDERER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    NTMV_UI_RENDERER_ANIMATION_STORAGE = 128,
    NTMV_UI_RENDERER_USABLE_ANIMATIONS = 127,
    NTMV_UI_RENDERER_OAM_CAPACITY = 128,
};

typedef struct NtmvUiCellAnimation {
    uint8_t unknown_00[0x30];
    const void *current_cell; /* target +0x30 */
    uint8_t unknown_34[0x24];
} NtmvUiCellAnimation;

typedef struct NtmvUiRenderer {
    void *owner;                                    /* target +0x0000 */
    const void *cell_resource_file;                 /* target +0x0004 */
    const void *animation_resource_file;            /* target +0x0008 */
    const void *cell_bank;                         /* target +0x000c */
    const void *animation_bank;                    /* target +0x0010 */
    NtmvUiCellAnimation animations[128];           /* target +0x0014 */
    int32_t animation_count;                       /* target +0x2c14 */
    uint8_t oam_manager[0x1c];                     /* target +0x2c18 */
    uint8_t oam_buffer[0x400];                     /* target +0x2c34 */
} NtmvUiRenderer;

typedef struct NtmvUiRendererTranslation {
    int32_t x;
    int32_t y;
} NtmvUiRendererTranslation;

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvUiCellAnimationTargetSizeCheck[
    sizeof(NtmvUiCellAnimation) == 0x58 ? 1 : -1];
typedef char NtmvUiRendererAnimationsOffsetCheck[
    offsetof(NtmvUiRenderer, animations) == 0x14 ? 1 : -1];
typedef char NtmvUiRendererCellBankOffsetCheck[
    offsetof(NtmvUiRenderer, cell_bank) == 0x0c ? 1 : -1];
typedef char NtmvUiRendererAnimationCountOffsetCheck[
    offsetof(NtmvUiRenderer, animation_count) == 0x2c14 ? 1 : -1];
typedef char NtmvUiRendererOamManagerOffsetCheck[
    offsetof(NtmvUiRenderer, oam_manager) == 0x2c18 ? 1 : -1];
typedef char NtmvUiRendererOamBufferOffsetCheck[
    offsetof(NtmvUiRenderer, oam_buffer) == 0x2c34 ? 1 : -1];
typedef char NtmvUiRendererTargetSizeCheck[
    sizeof(NtmvUiRenderer) == 0x3034 ? 1 : -1];
#endif

NtmvUiRenderer *NtmvUiRenderer_Init( /* 0x020bbf54 */
    NtmvUiRenderer *renderer);
void NtmvUiRenderer_LoadBackgroundAssets(void); /* 0x020bbfd0 */
void NtmvUiRenderer_SetupMain( /* 0x020bc0e4 */
    NtmvUiRenderer *renderer, void *owner);
void NtmvUiRenderer_SetupSub( /* 0x020bc170 */
    NtmvUiRenderer *renderer, void *owner);
void NtmvUiRenderer_Finalize(NtmvUiRenderer *renderer); /* 0x020bc1fc */
int32_t NtmvUiRenderer_CreateAnimation( /* 0x020bc200 */
    NtmvUiRenderer *renderer, uint16_t sequence_index);
NtmvUiCellAnimation *NtmvUiRenderer_GetAnimation( /* 0x020bc260 */
    NtmvUiRenderer *renderer, int32_t animation_handle);
bool NtmvUiRenderer_TickAnimation( /* 0x020bc280 */
    NtmvUiRenderer *renderer,
    int32_t animation_handle,
    int32_t frames_fx32);
bool NtmvUiRenderer_SetAnimationFrame( /* 0x020bc2a8 */
    NtmvUiRenderer *renderer,
    int32_t animation_handle,
    uint32_t frame);
bool NtmvUiRenderer_SubmitCell( /* 0x020bc2d0 */
    NtmvUiRenderer *renderer,
    int32_t animation_handle,
    int32_t x_fx32,
    int32_t y_fx32);
void NtmvUiRenderer_FlushOam( /* 0x020bc36c */
    NtmvUiRenderer *renderer);

#endif
