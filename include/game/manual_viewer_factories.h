#ifndef FSAE_GAME_MANUAL_VIEWER_FACTORIES_H
#define FSAE_GAME_MANUAL_VIEWER_FACTORIES_H

#include "ntmv/m2d/manual_toc_panel.h"
#include "ntmv/m2d/items_panel.h"
#include "ntmv/m2d/page_header_panel.h"
#include "ntmv/m2d/scroll_controls.h"
#include "ntmv/m2d/ui_panel.h"
#include "ntmv/m2d/ui_element.h"
#include "ntmv/ui_renderer.h"

NtmvM2dScrollButton *GameManualScrollButton_Allocate_020ba71c(
    void *const *allocator_context);
NtmvM2dUIPanel *GameManualUIPanel_Allocate_020ba748(
    void *const *allocator_context);
NtmvM2dUIElement *GameManualUIElement_AllocateWithGeometry_020ba774(
    void *const *allocator_context,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size);
NtmvM2dItemsPanel *GameManualItemsPanel_Allocate_020ba7e0(
    void *const *allocator_context);
NtmvM2dManualTocPanel *GameManualTocPanel_Allocate_020bcd34(
    void *const *allocator_context);
NtmvM2dScrollIndicator *GameManualScrollIndicator_Allocate_020bcd8c(
    void *const *allocator_context);
NtmvM2dPageHeaderPanel *GameManualPageHeaderPanel_Allocate_020bcdb8(
    void *const *allocator_context);
NtmvUiRenderer *GameManualRenderer_Allocate_020bcde4(
    void *const *allocator_context);

#endif
