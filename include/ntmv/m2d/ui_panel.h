#ifndef FSAE_NTMV_M2D_UI_PANEL_H
#define FSAE_NTMV_M2D_UI_PANEL_H

#include "ntmv/m2d/ui_element.h"

#include <stdint.h>

typedef struct NtmvM2dUIPanel NtmvM2dUIPanel;

typedef struct NtmvM2dUIPanelVTable {
    void (*destroy)(NtmvM2dUIPanel *panel, NtmvM2dAllocatorContext *allocator);
    void (*render)(
        NtmvM2dUIPanel *panel,
        void *render_context,
        const NtmvM2dPoint *parent_position);
    void (*set_position)(NtmvM2dUIPanel *panel, const NtmvM2dPoint *position);
    void (*set_size)(NtmvM2dUIPanel *panel, const NtmvM2dSize *size);
    void (*set_visible)(NtmvM2dUIPanel *panel, bool visible);
} NtmvM2dUIPanelVTable;

/*
 * RTTI typeinfo 0x02126d5c; base UIElement typeinfo 0x02126ce0;
 * callable vtable address point 0x02126d84.
 */
struct NtmvM2dUIPanel {
    NtmvM2dUIElement base;       /* +0x00 */
    NtmvM2dUIElement **children; /* +0x14 */
    uint16_t child_count;        /* +0x18 */
    uint16_t reserved_1a;
};

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dUIPanelSizeCheck[sizeof(NtmvM2dUIPanel) == 0x1c ? 1 : -1];
#endif

extern const NtmvM2dUIPanelVTable gNtmvM2dUIPanelVTable;

NtmvM2dUIPanel *NtmvM2dUIPanel_ConstructBase( /* 0x020b8020 */
    NtmvM2dUIPanel *panel);
NtmvM2dUIPanel *NtmvM2dUIPanel_ConstructComplete( /* 0x020b804c */
    NtmvM2dUIPanel *panel);
void NtmvM2dUIPanel_Configure( /* 0x020b8078 */
    NtmvM2dUIPanel *panel,
    NtmvM2dAllocatorContext *allocator,
    NtmvM2dUIElement *const *children,
    uint16_t child_count,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size);
void NtmvM2dUIPanel_Destroy( /* 0x020b8138 */
    NtmvM2dUIPanel *panel, NtmvM2dAllocatorContext *allocator);
void NtmvM2dUIPanel_Render( /* 0x020b81dc */
    NtmvM2dUIPanel *panel,
    void *render_context,
    const NtmvM2dPoint *parent_position);

#endif
