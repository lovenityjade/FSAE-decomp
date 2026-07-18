#ifndef FSAE_NTMV_M2D_SCROLL_CONTROLS_H
#define FSAE_NTMV_M2D_SCROLL_CONTROLS_H

#include "ntmv/m2d/obj_button.h"
#include "ntmv/m2d/ui_panel.h"
#include "ntmv/m2d/ui_static.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct NtmvM2dScrollButton NtmvM2dScrollButton;
typedef struct NtmvM2dScrollIndicator NtmvM2dScrollIndicator;

/* Exact eight-byte input prefix consumed by 0x020b8554; event names are provisional. */
typedef struct NtmvM2dScrollPointerState {
    NtmvM2dPoint position; /* +0x00 */
    uint8_t active;        /* +0x04 */
    uint8_t pressed;       /* +0x05 */
    uint8_t released;      /* +0x06 */
    uint8_t reserved_07;
} NtmvM2dScrollPointerState;

typedef struct NtmvM2dScrollButtonAction {
    uint16_t activated; /* +0x00 */
    uint8_t decrement;  /* +0x02: selected child index 1 */
    uint8_t increment;  /* +0x03: selected child index 0 */
} NtmvM2dScrollButtonAction;

/*
 * RTTI typeinfo 0x02126dd8 records UIPanel typeinfo 0x02126d5c as base.
 * base.reserved_1a is interpreted as a signed selected-child index (-1/0/1).
 */
struct NtmvM2dScrollButton {
    NtmvM2dUIPanel base; /* +0x00, selected child overlays base +0x1a */
    int16_t item_count;  /* +0x1c */
    int16_t page_size;   /* +0x1e */
    int16_t scroll_offset; /* +0x20 */
    uint16_t reserved_22;
};

/*
 * RTTI typeinfo 0x02126e1c also derives directly from UIPanel.  Here the
 * inherited word at +0x1a is the total item count rather than a selection.
 */
struct NtmvM2dScrollIndicator {
    NtmvM2dUIPanel base; /* +0x00, total count overlays base +0x1a */
    int16_t page_size;   /* +0x1c */
    int16_t scroll_offset; /* +0x1e */
    int16_t child_extent[2]; /* +0x20, initialized to 8; later use uncertain */
};

typedef char NtmvM2dScrollPointerStateSizeCheck[
    sizeof(NtmvM2dScrollPointerState) == 0x08 ? 1 : -1];
typedef char NtmvM2dScrollButtonActionSizeCheck[
    sizeof(NtmvM2dScrollButtonAction) == 0x04 ? 1 : -1];

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dScrollButtonSizeCheck[
    sizeof(NtmvM2dScrollButton) == 0x24 ? 1 : -1];
typedef char NtmvM2dScrollButtonCountOffsetCheck[
    offsetof(NtmvM2dScrollButton, item_count) == 0x1c ? 1 : -1];
typedef char NtmvM2dScrollButtonOffsetOffsetCheck[
    offsetof(NtmvM2dScrollButton, scroll_offset) == 0x20 ? 1 : -1];
typedef char NtmvM2dScrollIndicatorSizeCheck[
    sizeof(NtmvM2dScrollIndicator) == 0x24 ? 1 : -1];
typedef char NtmvM2dScrollIndicatorPageOffsetCheck[
    offsetof(NtmvM2dScrollIndicator, page_size) == 0x1c ? 1 : -1];
typedef char NtmvM2dScrollIndicatorExtentOffsetCheck[
    offsetof(NtmvM2dScrollIndicator, child_extent) == 0x20 ? 1 : -1];
#endif

/* ScrollButton header 0x02126e00; typeinfo word +0x04; address point +0x08. */
extern const NtmvM2dUIPanelVTable gNtmvM2dScrollButtonVTable;
/* ScrollIndicator header 0x02126e28; typeinfo word +0x04; address point +0x08. */
extern const NtmvM2dUIPanelVTable gNtmvM2dScrollIndicatorVTable;

NtmvM2dScrollButton *NtmvM2dScrollButton_Construct( /* 0x020b8408 */
    NtmvM2dScrollButton *button);
void NtmvM2dScrollButton_Configure( /* 0x020b8440 */
    NtmvM2dScrollButton *button,
    NtmvM2dAllocatorContext *allocator,
    void *animation_manager,
    const NtmvM2dPoint *position,
    int16_t page_size);
bool NtmvM2dScrollButton_HandlePointer( /* 0x020b8554 */
    NtmvM2dScrollButton *button,
    NtmvM2dScrollButtonAction *action,
    const NtmvM2dScrollPointerState *pointer);
void NtmvM2dScrollButton_SetDirectionalHover( /* 0x020b86b8 */
    NtmvM2dScrollButton *button, bool decrement, bool increment);
void NtmvM2dScrollButton_SetScrollOffset( /* 0x020b86f4 */
    NtmvM2dScrollButton *button, int16_t scroll_offset);
void NtmvM2dScrollButton_TickChildren( /* 0x020b8760 */
    NtmvM2dScrollButton *button);

NtmvM2dScrollIndicator *NtmvM2dScrollIndicator_Construct( /* 0x020b8784 */
    NtmvM2dScrollIndicator *indicator);
void NtmvM2dScrollIndicator_Configure( /* 0x020b87b4 */
    NtmvM2dScrollIndicator *indicator,
    NtmvM2dAllocatorContext *allocator,
    void *animation_manager,
    const NtmvM2dPoint *position,
    int16_t height,
    int16_t page_size,
    const uint16_t animation_sequences[2]);
void NtmvM2dScrollIndicator_RepositionChildren( /* 0x020b88dc */
    NtmvM2dScrollIndicator *indicator, int16_t height);
void NtmvM2dScrollIndicator_SetScrollOffset( /* 0x020b895c */
    NtmvM2dScrollIndicator *indicator, int16_t scroll_offset);
void NtmvM2dScrollIndicator_UpdateVisibility( /* 0x020b8964 */
    NtmvM2dScrollIndicator *indicator, void *render_context);

#endif
