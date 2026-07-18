#ifndef FSAE_NTMV_M2D_ITEMS_PANEL_H
#define FSAE_NTMV_M2D_ITEMS_PANEL_H

#include "ntmv/m2d/manual_button.h"
#include "ntmv/m2d/ui_panel.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct NtmvM2dItemsPanel NtmvM2dItemsPanel;

typedef struct NtmvM2dItemsFontMetadata {
    uint8_t unknown_00[7];
    uint8_t orientation;
} NtmvM2dItemsFontMetadata;

typedef struct NtmvM2dItemsFontResource {
    uint8_t unknown_00[8];
    const NtmvM2dItemsFontMetadata *metadata;
} NtmvM2dItemsFontResource;

typedef struct NtmvM2dItemsFont {
    const NtmvM2dItemsFontResource *resource;
} NtmvM2dItemsFont;

typedef struct NtmvM2dItemsTextContext {
    void *unknown_00;
    const NtmvM2dItemsFont *font;
} NtmvM2dItemsTextContext;

/* 0x18-byte source record consumed by the callback at 0x020b9fac. */
typedef struct NtmvM2dItemsTocRecord {
    uint32_t reserved_00;
    const uint16_t *text;       /* +0x04 */
    uint8_t reserved_08[0x08];
    const uint16_t *const *child_texts; /* +0x10 */
    uint16_t child_count;       /* +0x14 */
    uint16_t reserved_16;
} NtmvM2dItemsTocRecord;

typedef struct NtmvM2dItemsTocData {
    const NtmvM2dItemsTocRecord *records; /* +0x00 */
    uint16_t record_count;                /* +0x04, not read by callback */
    uint16_t reserved_06;
} NtmvM2dItemsTocData;

/* Exact eight-byte target row record consumed at 0x020b98f8. */
typedef struct NtmvM2dItemsRow {
    const uint16_t *text;
    uint8_t use_alternate_animation;
    uint8_t reserved_05[3];
} NtmvM2dItemsRow;

typedef void (*NtmvM2dItemsRowProvider)(
    NtmvM2dItemsRow *output, void *context, int16_t item_index);

/* Exact eight-byte pointer state consumed at 0x020b94ac. */
typedef struct NtmvM2dItemsPointerState {
    NtmvM2dPoint position; /* +0x00 */
    uint8_t active;        /* +0x04 */
    uint8_t pressed;       /* +0x05 */
    uint8_t released;      /* +0x06 */
    uint8_t reserved_07;
} NtmvM2dItemsPointerState;

/* Exact five-byte result cleared and populated at 0x020b94ac. */
typedef struct NtmvM2dItemsPointerAction {
    uint8_t page_changed;      /* +0x00 */
    uint8_t selection_changed; /* +0x01 */
    uint8_t row_activated;     /* +0x02 */
    uint8_t control_hit;       /* +0x03, semantic name provisional */
    uint8_t control_pressed;   /* +0x04, semantic name provisional */
} NtmvM2dItemsPointerAction;

typedef char NtmvM2dItemsPointerStateSizeCheck[
    sizeof(NtmvM2dItemsPointerState) == 0x08 ? 1 : -1];
typedef char NtmvM2dItemsPointerActionSizeCheck[
    sizeof(NtmvM2dItemsPointerAction) == 0x05 ? 1 : -1];

/* Exact 0x12-byte configuration record consumed at 0x020b8f68. */
typedef struct NtmvM2dItemsPanelDefinition {
    NtmvM2dPoint position;                  /* +0x00 */
    uint16_t row_animation_sequence;        /* +0x04 */
    int16_t row_alternate_animation_sequence; /* +0x06, -1 disables it */
    uint16_t first_row_overlay_sequence;    /* +0x08 */
    int16_t row_center_x;                   /* +0x0a */
    int16_t row_width;                      /* +0x0c */
    int16_t visible_row_count_minus_one;    /* +0x0e */
    uint16_t auxiliary_child_count;         /* +0x10 */
} NtmvM2dItemsPanelDefinition;

typedef char NtmvM2dItemsPanelDefinitionSizeCheck[
    sizeof(NtmvM2dItemsPanelDefinition) == 0x12 ? 1 : -1];

/*
 * RTTI typeinfo 0x02126e74 encodes the original nested name
 * ntmv::m2d::detail::ItemsPanel and names UIPanel typeinfo 0x02126d5c as its
 * direct base.  The factory at 0x020ba7e0 proves the complete size 0x40.
 */
struct NtmvM2dItemsPanel {
    NtmvM2dUIPanel base;                     /* +0x00 */
    const NtmvM2dItemsTextContext *text_context; /* +0x1c */
    uint16_t item_count;                     /* +0x20 */
    int16_t first_visible_item;              /* +0x22 */
    int16_t visible_row_count_minus_one;     /* +0x24 */
    int16_t page_index;                      /* +0x26 */
    uint16_t page_count;                     /* +0x28 */
    int16_t selected_item;                   /* +0x2a: -1 means none */
    int16_t active_control_index;            /* +0x2c: -1 means none */
    uint16_t reserved_2e;
    NtmvM2dManualButton **row_buttons;        /* +0x30 */
    int16_t *row_item_indices;               /* +0x34 */
    int16_t scroll_y;                        /* +0x38 */
    int16_t target_scroll_y;                 /* +0x3a */
    uint8_t interaction_enabled;             /* +0x3c */
    uint8_t reserved_3d[3];
};

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dItemsTocRecordSizeCheck[
    sizeof(NtmvM2dItemsTocRecord) == 0x18 ? 1 : -1];
typedef char NtmvM2dItemsTocRecordTextOffsetCheck[
    offsetof(NtmvM2dItemsTocRecord, text) == 0x04 ? 1 : -1];
typedef char NtmvM2dItemsTocRecordChildrenOffsetCheck[
    offsetof(NtmvM2dItemsTocRecord, child_count) == 0x14 ? 1 : -1];
typedef char NtmvM2dItemsTocRecordChildTextsOffsetCheck[
    offsetof(NtmvM2dItemsTocRecord, child_texts) == 0x10 ? 1 : -1];
typedef char NtmvM2dItemsRowSizeCheck[
    sizeof(NtmvM2dItemsRow) == 0x08 ? 1 : -1];
typedef char NtmvM2dItemsPanelSizeCheck[
    sizeof(NtmvM2dItemsPanel) == 0x40 ? 1 : -1];
typedef char NtmvM2dItemsPanelContextOffsetCheck[
    offsetof(NtmvM2dItemsPanel, text_context) == 0x1c ? 1 : -1];
typedef char NtmvM2dItemsPanelCountOffsetCheck[
    offsetof(NtmvM2dItemsPanel, item_count) == 0x20 ? 1 : -1];
typedef char NtmvM2dItemsPanelSelectedOffsetCheck[
    offsetof(NtmvM2dItemsPanel, selected_item) == 0x2a ? 1 : -1];
typedef char NtmvM2dItemsPanelRowsOffsetCheck[
    offsetof(NtmvM2dItemsPanel, row_buttons) == 0x30 ? 1 : -1];
typedef char NtmvM2dItemsPanelScrollOffsetCheck[
    offsetof(NtmvM2dItemsPanel, scroll_y) == 0x38 ? 1 : -1];
#endif

/* Header 0x02126eb8; typeinfo word 0x02126ebc; address point 0x02126ec0. */
extern const NtmvM2dUIPanelVTable gNtmvM2dItemsPanelVTable;

/* 0x020b8e88: signed first-hit scan used by ManualTocPanel controls. */
int32_t NtmvM2dUIElementArray_FindContainingPoint_020b8e88(
    NtmvM2dUIElement *const *elements,
    int32_t element_count,
    const NtmvM2dPoint *point);

/* 0x020b8ef8: child selection begins at -1 when record +0x00 is present. */
int32_t NtmvM2dItemsTocRecord_GetInitialChildIndex_020b8ef8(
    const NtmvM2dItemsTocRecord *record);

/* 0x020b8f0c..0x020b8f63; address-point literal 0x020b8f64 excluded. */
NtmvM2dItemsPanel *NtmvM2dItemsPanel_Construct(NtmvM2dItemsPanel *panel);

/*
 * 0x020b8f68..0x020b924f; literal pool 0x020b9250..0x020b9257 excluded.
 * Creates four fixed controls followed by every circular row button;
 * auxiliary slots are left null for the ManualTocPanel owner.
 */
void NtmvM2dItemsPanel_Configure(
    NtmvM2dItemsPanel *panel,
    NtmvM2dAllocatorContext *allocator,
    void *animation_manager,
    const NtmvM2dItemsTextContext *text_context,
    const NtmvM2dItemsPanelDefinition *definition);

/* 0x020b9380: resets pagination, selection and every circular row. */
void NtmvM2dItemsPanel_SetItemCount(
    NtmvM2dItemsPanel *panel, uint16_t item_count);

/* 0x020b9424: returns false only for an already-active identical selection. */
bool NtmvM2dItemsPanel_SetSelectedItem(
    NtmvM2dItemsPanel *panel, int16_t selected_item);

/* 0x020b9478: maps selected_item to an overlapping page. */
bool NtmvM2dItemsPanel_SyncPageToSelection(
    NtmvM2dItemsPanel *panel);

/* 0x020b94ac..0x020b9687: pointer capture, hover and release actions. */
bool NtmvM2dItemsPanel_HandlePointer(
    NtmvM2dItemsPanel *panel,
    NtmvM2dItemsPointerAction *action,
    const NtmvM2dItemsPointerState *pointer);

/* 0x020b9688: the four page controls exist only when page_count >= 2. */
void NtmvM2dItemsPanel_UpdatePageControlVisibility(
    NtmvM2dItemsPanel *panel);

/* 0x020b9720: pages advance by visible_row_count_minus_one overlapping rows. */
void NtmvM2dItemsPanel_SetPageIndex(
    NtmvM2dItemsPanel *panel, int16_t page_index);

/* 0x020b97d4: updates only lock bit 1 on previous/next page controls. */
void NtmvM2dItemsPanel_UpdatePageButtonLocks_020b97d4(
    NtmvM2dItemsPanel *panel);

/* 0x020b9740: updates target_scroll_y only when page_index changes. */
bool NtmvM2dItemsPanel_SetPageIndexIfChanged(
    NtmvM2dItemsPanel *panel, int16_t page_index);

/* 0x020b9774: updates +0x29 only for the selectable visible row range. */
void NtmvM2dItemsPanel_SetSelectedRowVisual(
    NtmvM2dItemsPanel *panel, bool selected);

/* 0x020b931c, vtable slot 0. */
void NtmvM2dItemsPanel_Destroy(
    NtmvM2dItemsPanel *panel, NtmvM2dAllocatorContext *allocator);

/* 0x020b9350..0x020b9367: null-safe row-button array release. */
void NtmvM2dItemsPanel_FreeRowButtons(
    NtmvM2dAllocatorContext *allocator,
    NtmvM2dManualButton **row_buttons);

/* 0x020b9368..0x020b937f: null-safe row-index array release. */
void NtmvM2dItemsPanel_FreeRowItemIndices(
    NtmvM2dAllocatorContext *allocator,
    int16_t *row_item_indices);

/*
 * Vtable slot 1 is inherited verbatim as NtmvM2dUIPanel_Render at
 * 0x020b81dc; slots 2-4 are the inherited UIElement setters.
 */

/* 0x020b9d30: auxiliary children follow four controls and all row buttons. */
void NtmvM2dItemsPanel_SetAuxiliaryChild(
    NtmvM2dItemsPanel *panel,
    int32_t auxiliary_index,
    NtmvM2dUIElement *child);

/* 0x020b9d58: hit-tests the four controls and every visible row button. */
int32_t NtmvM2dItemsPanel_FindControlAtPoint(
    const NtmvM2dItemsPanel *panel, const NtmvM2dPoint *point);

/* 0x020b9844: updates a bounded run and hides every remaining row button. */
void NtmvM2dItemsPanel_UpdateRows(
    NtmvM2dItemsPanel *panel,
    const NtmvM2dItemsRow *rows,
    uint16_t populated_count,
    int16_t first_item_index);

/* 0x020b98f8: redraws and exposes the row selected by item_index modulo rows. */
void NtmvM2dItemsPanel_UpdateTextRow(
    NtmvM2dItemsPanel *panel,
    const NtmvM2dItemsRow *row,
    int16_t item_index);

/*
 * 0x020b9b58: sibling layout used by the secondary list; it draws text only
 * in a 0x18-by-4 tile canvas whose row area begins at tile Y 0x12.
 */
void NtmvM2dItemsPanel_UpdateSimpleRows(
    NtmvM2dItemsPanel *panel,
    const NtmvM2dItemsRow *rows,
    uint16_t populated_count,
    int16_t first_item_index);

/*
 * 0x020b9df0: advances scroll_y toward target_scroll_y by at most 12 pixels,
 * repositions the circular row buttons and refreshes only changed slots.
 */
bool NtmvM2dItemsPanel_AnimateScroll(
    NtmvM2dItemsPanel *panel,
    NtmvM2dItemsRowProvider row_provider,
    void *provider_context);

/*
 * 0x020b9fac: row-provider callback used with AnimateScroll.  Empty source
 * text becomes the ROM's wide "(---)" placeholder; child_count selects the
 * alternate ManualButton animation.
 */
void NtmvM2dItemsPanel_BuildTocRow(
    NtmvM2dItemsRow *output, void *context, int16_t item_index);

#endif
