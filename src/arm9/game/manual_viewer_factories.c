#include "game/manual_viewer_factories.h"

#include <stddef.h>
#include <stdint.h>

enum {
    GAME_MANUAL_SCROLL_BUTTON_TARGET_SIZE = 0x24,
    GAME_MANUAL_UI_PANEL_TARGET_SIZE = 0x1c,
    GAME_MANUAL_UI_ELEMENT_TARGET_SIZE = 0x14,
    GAME_MANUAL_ITEMS_PANEL_TARGET_SIZE = 0x40,
    GAME_MANUAL_TOC_PANEL_TARGET_SIZE = 0xb0,
    GAME_MANUAL_SCROLL_INDICATOR_TARGET_SIZE = 0x24,
    GAME_MANUAL_PAGE_HEADER_TARGET_SIZE = 0x24,
    GAME_MANUAL_RENDERER_TARGET_SIZE = 0x3034
};

extern void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size);

/* 0x020ba71c..0x020ba747 */
NtmvM2dScrollButton *GameManualScrollButton_Allocate_020ba71c(
    void *const *allocator_context)
{
    NtmvM2dScrollButton *button = NNS_FndAllocFromAllocator(
        *allocator_context, GAME_MANUAL_SCROLL_BUTTON_TARGET_SIZE);

    return button != NULL ? NtmvM2dScrollButton_Construct(button) : NULL;
}

/* 0x020ba748 */
NtmvM2dUIPanel *GameManualUIPanel_Allocate_020ba748(
    void *const *allocator_context)
{
    NtmvM2dUIPanel *panel = NNS_FndAllocFromAllocator(
        *allocator_context, GAME_MANUAL_UI_PANEL_TARGET_SIZE);

    return panel != NULL ? NtmvM2dUIPanel_ConstructComplete(panel) : NULL;
}

/* 0x020ba774..0x020ba7df */
NtmvM2dUIElement *GameManualUIElement_AllocateWithGeometry_020ba774(
    void *const *allocator_context,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size)
{
    NtmvM2dUIElement *element = NNS_FndAllocFromAllocator(
        *allocator_context, GAME_MANUAL_UI_ELEMENT_TARGET_SIZE);

    if (element == NULL) {
        return NULL;
    }
    NtmvM2dUIElement_InitWithGeometry(element, position, size);
    return element;
}

/* 0x020ba7e0..0x020ba80b */
NtmvM2dItemsPanel *GameManualItemsPanel_Allocate_020ba7e0(
    void *const *allocator_context)
{
    NtmvM2dItemsPanel *panel = NNS_FndAllocFromAllocator(
        *allocator_context, GAME_MANUAL_ITEMS_PANEL_TARGET_SIZE);

    return panel != NULL ? NtmvM2dItemsPanel_Construct(panel) : NULL;
}

/* 0x020bcd34 */
NtmvM2dManualTocPanel *GameManualTocPanel_Allocate_020bcd34(
    void *const *allocator_context)
{
    NtmvM2dManualTocPanel *panel = NNS_FndAllocFromAllocator(
        *allocator_context, GAME_MANUAL_TOC_PANEL_TARGET_SIZE);

    return panel != NULL ? NtmvM2dManualTocPanel_Construct(panel) : NULL;
}

/* 0x020bcd8c */
NtmvM2dScrollIndicator *GameManualScrollIndicator_Allocate_020bcd8c(
    void *const *allocator_context)
{
    NtmvM2dScrollIndicator *indicator = NNS_FndAllocFromAllocator(
        *allocator_context, GAME_MANUAL_SCROLL_INDICATOR_TARGET_SIZE);

    return indicator != NULL ?
        NtmvM2dScrollIndicator_Construct(indicator) : NULL;
}

/* 0x020bcdb8 */
NtmvM2dPageHeaderPanel *GameManualPageHeaderPanel_Allocate_020bcdb8(
    void *const *allocator_context)
{
    NtmvM2dPageHeaderPanel *panel = NNS_FndAllocFromAllocator(
        *allocator_context, GAME_MANUAL_PAGE_HEADER_TARGET_SIZE);

    return panel != NULL ? NtmvM2dPageHeaderPanel_Construct(panel) : NULL;
}

/* 0x020bcde4 */
NtmvUiRenderer *GameManualRenderer_Allocate_020bcde4(
    void *const *allocator_context)
{
    NtmvUiRenderer *renderer = NNS_FndAllocFromAllocator(
        *allocator_context, GAME_MANUAL_RENDERER_TARGET_SIZE);

    return renderer != NULL ? NtmvUiRenderer_Init(renderer) : NULL;
}
