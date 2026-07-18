#ifndef FSAE_NTMV_M2D_OBJ_BUTTON_H
#define FSAE_NTMV_M2D_OBJ_BUTTON_H

#include "ntmv/m2d/ui_element.h"

#include <stddef.h>
#include <stdint.h>

typedef struct NtmvM2dObjButton NtmvM2dObjButton;

typedef struct NtmvM2dObjButtonVTable {
    void (*destroy)(
        NtmvM2dObjButton *button, NtmvM2dAllocatorContext *allocator);
    void (*render)(
        NtmvM2dObjButton *button,
        void *render_context,
        const NtmvM2dPoint *parent_position);
    void (*set_position)(NtmvM2dObjButton *button, const NtmvM2dPoint *position);
    void (*set_size)(NtmvM2dObjButton *button, const NtmvM2dSize *size);
    void (*set_visible)(NtmvM2dObjButton *button, bool visible);
} NtmvM2dObjButtonVTable;

enum NtmvM2dObjButtonVisualState {
    NTMV_M2D_OBJ_BUTTON_NORMAL = 0,
    NTMV_M2D_OBJ_BUTTON_ACTIVE = 1,
    NTMV_M2D_OBJ_BUTTON_LOCKED = 2,
};

/*
 * RTTI typeinfo 0x02126d98 records UIElement typeinfo 0x02126ce0 as the
 * direct base.  UIPanel is not present in this object's 0x1c-byte layout.
 */
struct NtmvM2dObjButton {
    NtmvM2dUIElement base;       /* +0x00 */
    int32_t animation_handle;    /* +0x14: -1 until configured */
    uint16_t active_timer;       /* +0x18: provisional semantic name */
    uint16_t reserved_1a;
};

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dObjButtonSizeCheck[
    sizeof(NtmvM2dObjButton) == 0x1c ? 1 : -1];
typedef char NtmvM2dObjButtonHandleOffsetCheck[
    offsetof(NtmvM2dObjButton, animation_handle) == 0x14 ? 1 : -1];
typedef char NtmvM2dObjButtonTimerOffsetCheck[
    offsetof(NtmvM2dObjButton, active_timer) == 0x18 ? 1 : -1];
#endif

/* Header 0x02126dbc; typeinfo word 0x02126dc0; address point 0x02126dc4. */
extern const NtmvM2dObjButtonVTable gNtmvM2dObjButtonVTable;

/* 0x020b826c: base-object variant used by ManualButton. */
NtmvM2dObjButton *NtmvM2dObjButton_ConstructBase(NtmvM2dObjButton *button);

/* 0x020b829c: complete-object variant used by the 0x020b8528 factory. */
NtmvM2dObjButton *NtmvM2dObjButton_ConstructComplete(NtmvM2dObjButton *button);

/* 0x020b82cc */
void NtmvM2dObjButton_Configure(
    NtmvM2dObjButton *button,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size,
    int32_t animation_handle);

/* 0x020b82f4, vtable slot 1; slots 0 and 2-4 are inherited UIElement callbacks. */
void NtmvM2dObjButton_Render(
    NtmvM2dObjButton *button,
    void *render_context,
    const NtmvM2dPoint *parent_position);

/* 0x020b8384: saturating one-frame decrement of active_timer. */
void NtmvM2dObjButton_TickActiveState(NtmvM2dObjButton *button);

#endif
