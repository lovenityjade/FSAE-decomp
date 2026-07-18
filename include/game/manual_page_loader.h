#ifndef FSAE_GAME_MANUAL_PAGE_LOADER_H
#define FSAE_GAME_MANUAL_PAGE_LOADER_H

#include "game/manual_page_flow.h"
#include "ntmv/m2d/page.h"
#include "ntmv/m2d/pane.h"
#include "ntmv/m2d/picture.h"
#include "ntmv/m2d/text_box.h"
#include "ntmv/m2d/window.h"

#include <stddef.h>
#include <stdint.h>

typedef NtmvM2dPage GameManualPage;

/*
 * Four-word value assembled by 0x020b4f00 and passed by value through the
 * pane factory at 0x020b5318.  The final word points at Page +0x14, whose
 * count/padding/pointer layout is the resource set consumed by Picture and
 * Window constructors.
 */
typedef struct NtmvM2dPageBuildContext {
    NtmvM2dArcResourceAccessor *accessor; /* target +0x00 */
    const void *name_pool;                /* target +0x04: nap1 */
    const void *text_pool;                /* target +0x08: txp1 */
    const void *resource_set;             /* target +0x0c: Page +0x14 */
} NtmvM2dPageBuildContext;

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dPageBuildContextTargetSizeCheck[
    sizeof(NtmvM2dPageBuildContext) == 0x10 ? 1 : -1];
typedef char NtmvM2dPageBuildContextTextOffsetCheck[
    offsetof(NtmvM2dPageBuildContext, text_pool) == 0x08 ? 1 : -1];
typedef char NtmvM2dPageBuildContextResourceSetOffsetCheck[
    offsetof(NtmvM2dPageBuildContext, resource_set) == 0x0c ? 1 : -1];
#endif

GameManualPage *GameManualPage_Allocate_020bcd60(
    void *const *allocator_context);
bool GameManualPage_Load_020b4f00(
    GameManualPage *page,
    void *allocator_context,
    const void *resource,
    NtmvM2dArcResourceAccessor *accessor);

/* 0x020b51c8: count zero still allocates one pointer-sized slot. */
void **NtmvM2dPage_AllocateResourceArray_020b51c8(
    void *const *allocator_context,
    uint32_t resource_count);

/* 0x020b5318: dispatches exactly pan1, pic1, txt1 and wnd1. */
NtmvM2dPane *NtmvM2dPage_CreatePane_020b5318(
    GameManualPage *page,
    void *allocator_context,
    uint32_t block_signature,
    const void *block,
    const NtmvM2dPageBuildContext *build_context);

NtmvM2dWindow *NtmvM2dWindow_Allocate_020b5424(
    void *const *allocator_context,
    const NtmvM2dWindowResource *resource,
    const NtmvM2dPageBuildContext *build_context);
NtmvM2dTextBox *NtmvM2dTextBox_Allocate_020b547c(
    void *const *allocator_context,
    const NtmvM2dTextBoxResource *resource,
    const NtmvM2dPageBuildContext *build_context);
NtmvM2dPicture *NtmvM2dPicture_Allocate_020b54d0(
    void *const *allocator_context,
    const NtmvM2dPictureResource *resource,
    const NtmvM2dPageBuildContext *build_context);
NtmvM2dPane *NtmvM2dPane_AllocateComplete_020b5524(
    void *const *allocator_context,
    const NtmvM2dPaneResource *resource,
    const NtmvM2dPageBuildContext *build_context);

#endif
