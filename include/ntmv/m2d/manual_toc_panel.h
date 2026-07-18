#ifndef FSAE_NTMV_M2D_MANUAL_TOC_PANEL_H
#define FSAE_NTMV_M2D_MANUAL_TOC_PANEL_H

#include "ntmv/m2d/items_panel.h"
#include "ntmv/m2d/ui_panel.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct NtmvM2dManualTocPanel NtmvM2dManualTocPanel;

typedef struct NtmvM2dManualTocPanelVTable {
    void (*destroy)(
        NtmvM2dManualTocPanel *panel, NtmvM2dAllocatorContext *allocator);
    void (*render)(
        NtmvM2dManualTocPanel *panel,
        void *render_context,
        const NtmvM2dPoint *parent_position);
    void (*set_position)(
        NtmvM2dManualTocPanel *panel, const NtmvM2dPoint *position);
    void (*set_size)(
        NtmvM2dManualTocPanel *panel, const NtmvM2dSize *size);
    void (*set_visible)(NtmvM2dManualTocPanel *panel, bool visible);
} NtmvM2dManualTocPanelVTable;

enum NtmvM2dManualTocMode {
    NTMV_M2D_MANUAL_TOC_MODE_LEFT = 0,
    NTMV_M2D_MANUAL_TOC_MODE_CENTER = 1,
    NTMV_M2D_MANUAL_TOC_MODE_SPLIT = 2,
};

enum NtmvM2dManualTocActionKind {
    NTMV_M2D_MANUAL_TOC_ACTION_NONE = 0,
    NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_1 = 1,
    NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_2 = 2,
    NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_3 = 3,
    NTMV_M2D_MANUAL_TOC_ACTION_PRIMARY_ITEM = 4,
    NTMV_M2D_MANUAL_TOC_ACTION_SECONDARY_ITEM = 5,
};

/* Exact two-halfword result written at 0x020ba9a4. */
typedef struct NtmvM2dManualTocAction {
    uint16_t kind;
    uint16_t value;
} NtmvM2dManualTocAction;

typedef char NtmvM2dManualTocActionSizeCheck[
    sizeof(NtmvM2dManualTocAction) == 0x04 ? 1 : -1];

/*
 * RTTI typeinfo 0x02126e68 records UIElement, not UIPanel, as the direct
 * base.  The two UIPanel objects at +0x20 and +0x3c are embedded owners.
 * Several late field names remain semantic because no data symbols survive.
 */
struct NtmvM2dManualTocPanel {
    NtmvM2dUIElement base;               /* +0x00 */
    const NtmvM2dItemsTocData *toc_data; /* +0x14 */
    void *render_resources;              /* +0x18 */
    void *feedback_context;              /* +0x1c */
    NtmvM2dUIPanel content_panel;        /* +0x20 */
    NtmvM2dUIPanel footer_panel;         /* +0x3c */
    NtmvM2dUIPanel *body_panel;          /* +0x58 */
    NtmvM2dUIElement *item_panels[2];    /* +0x5c */
    NtmvM2dUIElement *controls[9];       /* +0x64 */
    NtmvM2dUIElement *selection_marker;  /* +0x88 */
    NtmvM2dUIElement *mode_markers[2];   /* +0x8c */
    NtmvM2dUIElement *scroll_button;     /* +0x94 */
    int16_t active_control_index;        /* +0x98: -1 means none */
    int16_t item_panel_target_x;         /* +0x9a */
    int16_t mode;                        /* +0x9c */
    int16_t target_window_x;             /* +0x9e */
    NtmvM2dPoint window_origins[3];      /* +0xa0 */
    uint8_t state_ac;                    /* +0xac, provisional */
    uint8_t secondary_state_active;      /* +0xad */
    uint8_t state_ae;                    /* +0xae, provisional */
    uint8_t reserved_af;
};

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dManualTocPanelSizeCheck[
    sizeof(NtmvM2dManualTocPanel) == 0xb0 ? 1 : -1];
typedef char NtmvM2dManualTocContentOffsetCheck[
    offsetof(NtmvM2dManualTocPanel, content_panel) == 0x20 ? 1 : -1];
typedef char NtmvM2dManualTocFooterOffsetCheck[
    offsetof(NtmvM2dManualTocPanel, footer_panel) == 0x3c ? 1 : -1];
typedef char NtmvM2dManualTocControlsOffsetCheck[
    offsetof(NtmvM2dManualTocPanel, controls) == 0x64 ? 1 : -1];
typedef char NtmvM2dManualTocActiveOffsetCheck[
    offsetof(NtmvM2dManualTocPanel, active_control_index) == 0x98 ? 1 : -1];
typedef char NtmvM2dManualTocWindowsOffsetCheck[
    offsetof(NtmvM2dManualTocPanel, window_origins) == 0xa0 ? 1 : -1];
#endif

/* Header 0x02126e80; typeinfo word 0x02126e84; address point 0x02126e88. */
extern const NtmvM2dManualTocPanelVTable gNtmvM2dManualTocPanelVTable;

/* 0x020ba004; the 0x020bcd34 factory proves the complete size 0xb0. */
NtmvM2dManualTocPanel *NtmvM2dManualTocPanel_Construct(
    NtmvM2dManualTocPanel *panel);

/*
 * 0x020ba0d0.  layout_threshold selects the compact button definitions at
 * 0x020def5c or the wide definitions at 0x020defa2.  control_texts contains
 * the three strings drawn into the OBJ text canvases described at 0x020def18.
 */
void NtmvM2dManualTocPanel_Configure(
    NtmvM2dManualTocPanel *panel,
    NtmvM2dAllocatorContext *allocator,
    void *animation_manager,
    const NtmvM2dItemsTextContext *text_context,
    int16_t layout_threshold,
    const uint16_t *const control_texts[3],
    void *feedback_context);

/* 0x020ba80c, vtable slot 0. */
void NtmvM2dManualTocPanel_Destroy(
    NtmvM2dManualTocPanel *panel, NtmvM2dAllocatorContext *allocator);

/* 0x020ba84c, vtable slot 1. */
void NtmvM2dManualTocPanel_Render(
    NtmvM2dManualTocPanel *panel,
    void *render_context,
    const NtmvM2dPoint *parent_position);

/* 0x020ba948: installs TOC records and resets both list/navigation states. */
void NtmvM2dManualTocPanel_SetTocData(
    NtmvM2dManualTocPanel *panel,
    const NtmvM2dItemsTocData *toc_data);

/* 0x020ba9a4..0x020bac1f: controls and both item lists share one event pass. */
bool NtmvM2dManualTocPanel_HandlePointer(
    NtmvM2dManualTocPanel *panel,
    NtmvM2dManualTocAction *action,
    const NtmvM2dItemsPointerState *pointer);

/* 0x020bac20: sign-extends the primary ItemsPanel selection. */
int32_t NtmvM2dManualTocPanel_GetPrimarySelection_020bac20(
    const NtmvM2dManualTocPanel *panel);

/* 0x020bac2c: updates the primary selection, page and secondary reset. */
void NtmvM2dManualTocPanel_SelectPrimaryItem_020bac2c(
    NtmvM2dManualTocPanel *panel, int16_t item_index);

/* 0x020bac58: sign-extends the secondary ItemsPanel selection. */
int32_t NtmvM2dManualTocPanel_GetSecondarySelection_020bac58(
    const NtmvM2dManualTocPanel *panel);

/* 0x020bac64: refreshes rows only when selection moves to another page. */
void NtmvM2dManualTocPanel_SelectSecondaryItem_020bac64(
    NtmvM2dManualTocPanel *panel, int16_t item_index);

/* 0x020bac98: stores +0xad and selects control 2's inverse animation. */
void NtmvM2dManualTocPanel_SetSecondaryState_020bac98(
    NtmvM2dManualTocPanel *panel, bool active);

/* 0x020bacb8..0x020baef3: advances every TOC layout/input animation phase. */
void NtmvM2dManualTocPanel_Update(NtmvM2dManualTocPanel *panel);

/* 0x020baef4..0x020bafb3: publishes visibility and three window origins. */
void NtmvM2dManualTocPanel_PublishWindowRegisters_020baef4(
    const NtmvM2dManualTocPanel *panel);

/* 0x020bafb4..0x020bb07f: rebuilds the visible primary TOC rows. */
void NtmvM2dManualTocPanel_RefreshPrimaryRows_020bafb4(
    NtmvM2dManualTocPanel *panel);

/* 0x020bb080..0x020bb147: rebuilds visible rows of the selected TOC entry. */
void NtmvM2dManualTocPanel_RefreshSecondaryRows_020bb080(
    NtmvM2dManualTocPanel *panel);

/* 0x020bb148..0x020bb1bb: resets and rebuilds the secondary selection. */
void NtmvM2dManualTocPanel_InitializeEmptySelection_020bb148(
    NtmvM2dManualTocPanel *panel);

/* 0x020bb1bc..0x020bb207: controls 3 and 4 are navigation buttons. */
void NtmvM2dManualTocPanel_SetNavigationAvailability(
    NtmvM2dManualTocPanel *panel,
    bool previous_available,
    bool next_available);

/* 0x020bb208..0x020bb28f: derives navigation from both TOC selections. */
void NtmvM2dManualTocPanel_UpdateNavigation_020bb208(
    NtmvM2dManualTocPanel *panel);

/* 0x020bb290..0x020bb44f: captures, tracks and dispatches nine controls. */
bool NtmvM2dManualTocPanel_HandleControls_020bb290(
    NtmvM2dManualTocPanel *panel,
    NtmvM2dManualTocAction *action,
    const NtmvM2dItemsPointerState *pointer);

/* 0x020bb730..0x020bb787: combines panel, horizontal and scroll motion. */
bool NtmvM2dManualTocPanel_IsAnimating_020bb730(
    const NtmvM2dManualTocPanel *panel);

/* 0x020bb788..0x020bb85b: sets layout target and two mode markers. */
void NtmvM2dManualTocPanel_SetMode_020bb788(
    NtmvM2dManualTocPanel *panel, int32_t mode);

/* 0x020bb85c..0x020bb897: maps modes 0/1/2 to target X -96/0/-48. */
void NtmvM2dManualTocPanel_UpdateTargetWindowX(
    NtmvM2dManualTocPanel *panel);

/* 0x020bb898..0x020bb97f: positions and shows the secondary-list marker. */
void NtmvM2dManualTocPanel_UpdateSecondaryMode_020bb898(
    NtmvM2dManualTocPanel *panel, bool force_update);

/* 0x020bb980..0x020bb9ff: toggles both lists' selected-row interaction. */
void NtmvM2dManualTocPanel_SetInteractionMode_020bb980(
    NtmvM2dManualTocPanel *panel, bool inactive);

#endif
