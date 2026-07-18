#ifndef FSAE_NTMV_M2D_PAGE_HEADER_PANEL_H
#define FSAE_NTMV_M2D_PAGE_HEADER_PANEL_H

#include "ntmv/m2d/font.h"
#include "ntmv/m2d/ui_static.h"

#include <stddef.h>
#include <stdint.h>

typedef struct NtmvM2dPageHeaderFont {
    const NtmvM2dFontResource *resource;
} NtmvM2dPageHeaderFont;

typedef struct NtmvM2dPageHeaderContext {
    void *unknown_00;
    const NtmvM2dPageHeaderFont *font;
} NtmvM2dPageHeaderContext;

typedef struct NtmvM2dPageHeaderPanel {
    NtmvM2dUIStatic base;                    /* target +0x00 */
    uint32_t reserved_18;                    /* target +0x18 */
    const NtmvM2dPageHeaderContext *context; /* target +0x1c */
    const uint16_t *text;                    /* target +0x20 */
} NtmvM2dPageHeaderPanel;

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dPageHeaderPanelTargetSizeCheck[
    sizeof(NtmvM2dPageHeaderPanel) == 0x24 ? 1 : -1];
typedef char NtmvM2dPageHeaderPanelContextOffsetCheck[
    offsetof(NtmvM2dPageHeaderPanel, context) == 0x1c ? 1 : -1];
typedef char NtmvM2dPageHeaderPanelTextOffsetCheck[
    offsetof(NtmvM2dPageHeaderPanel, text) == 0x20 ? 1 : -1];
#endif

/* RTTI typeinfo 0x02126f4c; callable vtable address point 0x02126f60. */
extern const NtmvM2dUIStaticVTable gNtmvM2dPageHeaderPanelVTable;

NtmvM2dPageHeaderPanel *NtmvM2dPageHeaderPanel_Construct( /* 0x020bbb78 */
    NtmvM2dPageHeaderPanel *panel);
void NtmvM2dPageHeaderPanel_Configure(                    /* 0x020bbba0 */
    NtmvM2dPageHeaderPanel *panel,
    void *animation_manager,
    const NtmvM2dPageHeaderContext *context);
void NtmvM2dPageHeaderPanel_SetText(                      /* 0x020bbbfc */
    NtmvM2dPageHeaderPanel *panel,
    const uint16_t *text,
    uint16_t page_index);

#endif
