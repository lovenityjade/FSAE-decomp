#include "game/manual_page_loader.h"

#include <stddef.h>

enum {
    NTMV_M2D_PANE_SIGNATURE = 0x70616e31, /* "pan1" */
    NTMV_M2D_PICTURE_SIGNATURE = 0x70696331, /* "pic1" */
    NTMV_M2D_TEXT_BOX_SIGNATURE = 0x74787431, /* "txt1" */
    NTMV_M2D_WINDOW_SIGNATURE = 0x776e6431, /* "wnd1" */
    NTMV_M2D_PANE_TARGET_SIZE = 0x20,
    NTMV_M2D_PICTURE_TARGET_SIZE = 0x28,
    NTMV_M2D_TEXT_BOX_TARGET_SIZE = 0x48,
    NTMV_M2D_WINDOW_TARGET_SIZE = 0x28
};

extern void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size);

/* 0x020b5424 */
NtmvM2dWindow *NtmvM2dWindow_Allocate_020b5424(
    void *const *allocator_context,
    const NtmvM2dWindowResource *resource,
    const NtmvM2dPageBuildContext *build_context)
{
    NtmvM2dWindow *window = NNS_FndAllocFromAllocator(
        *allocator_context, NTMV_M2D_WINDOW_TARGET_SIZE);

    if (window == NULL) {
        return NULL;
    }
#if UINTPTR_MAX > UINT32_MAX
    const NtmvM2dWindowBuildContext host_context = {
        {0}, (const NtmvM2dWindowResourceSet *)build_context->resource_set
    };
    const NtmvM2dWindowBuildContext *constructor_context = &host_context;
#else
    const NtmvM2dWindowBuildContext *constructor_context =
        (const NtmvM2dWindowBuildContext *)build_context;
#endif

    return NtmvM2dWindow_Construct(
        window,
        (NtmvM2dAllocatorContext *)allocator_context,
        resource,
        constructor_context);
}

/* 0x020b547c */
NtmvM2dTextBox *NtmvM2dTextBox_Allocate_020b547c(
    void *const *allocator_context,
    const NtmvM2dTextBoxResource *resource,
    const NtmvM2dPageBuildContext *build_context)
{
    NtmvM2dTextBox *text_box = NNS_FndAllocFromAllocator(
        *allocator_context, NTMV_M2D_TEXT_BOX_TARGET_SIZE);

    if (text_box == NULL) {
        return NULL;
    }
#if UINTPTR_MAX > UINT32_MAX
    const NtmvM2dTextBoxBuildContext host_context = {
        {0}, (const NtmvM2dTextArchive *)build_context->text_pool
    };
    const NtmvM2dTextBoxBuildContext *constructor_context = &host_context;
#else
    const NtmvM2dTextBoxBuildContext *constructor_context =
        (const NtmvM2dTextBoxBuildContext *)build_context;
#endif

    return NtmvM2dTextBox_Construct(
        text_box,
        resource,
        constructor_context);
}

/* 0x020b54d0 */
NtmvM2dPicture *NtmvM2dPicture_Allocate_020b54d0(
    void *const *allocator_context,
    const NtmvM2dPictureResource *resource,
    const NtmvM2dPageBuildContext *build_context)
{
    NtmvM2dPicture *picture = NNS_FndAllocFromAllocator(
        *allocator_context, NTMV_M2D_PICTURE_TARGET_SIZE);

    if (picture == NULL) {
        return NULL;
    }
#if UINTPTR_MAX > UINT32_MAX
    const NtmvM2dPictureBuildContext host_context = {
        {0}, (const NtmvM2dPictureResourceSet *)build_context->resource_set
    };
    const NtmvM2dPictureBuildContext *constructor_context = &host_context;
#else
    const NtmvM2dPictureBuildContext *constructor_context =
        (const NtmvM2dPictureBuildContext *)build_context;
#endif

    return NtmvM2dPicture_Construct(
        picture,
        resource,
        constructor_context);
}

/* 0x020b5524 */
NtmvM2dPane *NtmvM2dPane_AllocateComplete_020b5524(
    void *const *allocator_context,
    const NtmvM2dPaneResource *resource,
    const NtmvM2dPageBuildContext *build_context)
{
    NtmvM2dPane *pane;

    (void)build_context;
    pane = NNS_FndAllocFromAllocator(
        *allocator_context, NTMV_M2D_PANE_TARGET_SIZE);
    if (pane == NULL) {
        return NULL;
    }
    return NtmvM2dPane_ConstructComplete(pane, resource);
}

/* 0x020b5318..0x020b5413; literal tag pool 0x020b5414..0x020b5420 excluded. */
NtmvM2dPane *NtmvM2dPage_CreatePane_020b5318(
    GameManualPage *page,
    void *allocator_context,
    uint32_t block_signature,
    const void *block,
    const NtmvM2dPageBuildContext *build_context)
{
    void *const *allocator = (void *const *)allocator_context;

    (void)page;
    switch (block_signature) {
    case NTMV_M2D_PANE_SIGNATURE:
        return NtmvM2dPane_AllocateComplete_020b5524(
            allocator,
            (const NtmvM2dPaneResource *)block,
            build_context);
    case NTMV_M2D_PICTURE_SIGNATURE:
        return (NtmvM2dPane *)NtmvM2dPicture_Allocate_020b54d0(
            allocator,
            (const NtmvM2dPictureResource *)block,
            build_context);
    case NTMV_M2D_TEXT_BOX_SIGNATURE:
        return (NtmvM2dPane *)NtmvM2dTextBox_Allocate_020b547c(
            allocator,
            (const NtmvM2dTextBoxResource *)block,
            build_context);
    case NTMV_M2D_WINDOW_SIGNATURE:
        return (NtmvM2dPane *)NtmvM2dWindow_Allocate_020b5424(
            allocator,
            (const NtmvM2dWindowResource *)block,
            build_context);
    default:
        return NULL;
    }
}
