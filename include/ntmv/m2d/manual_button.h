#ifndef FSAE_NTMV_M2D_MANUAL_BUTTON_H
#define FSAE_NTMV_M2D_MANUAL_BUTTON_H

#include "ntmv/m2d/obj_button.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct NtmvM2dManualButton NtmvM2dManualButton;

typedef struct NtmvM2dManualButtonVTable {
    void (*destroy)(
        NtmvM2dManualButton *button, NtmvM2dAllocatorContext *allocator);
    void (*render)(
        NtmvM2dManualButton *button,
        void *render_context,
        const NtmvM2dPoint *parent_position);
    void (*set_position)(
        NtmvM2dManualButton *button, const NtmvM2dPoint *position);
    void (*set_size)(
        NtmvM2dManualButton *button, const NtmvM2dSize *size);
    void (*set_visible)(NtmvM2dManualButton *button, bool visible);
} NtmvM2dManualButtonVTable;

enum NtmvM2dManualButtonVisualState {
    NTMV_M2D_MANUAL_BUTTON_NORMAL = NTMV_M2D_OBJ_BUTTON_NORMAL,
    NTMV_M2D_MANUAL_BUTTON_ACTIVE = NTMV_M2D_OBJ_BUTTON_ACTIVE,
    NTMV_M2D_MANUAL_BUTTON_LOCKED = NTMV_M2D_OBJ_BUTTON_LOCKED,
    NTMV_M2D_MANUAL_BUTTON_SELECTED = 3,
};

/*
 * RTTI typeinfo 0x02126f08 names ObjButton typeinfo 0x02126d98 as the
 * direct base.  The 0x020b9258 factory proves the complete target size 0x2c.
 */
struct NtmvM2dManualButton {
    NtmvM2dObjButton base;               /* +0x00 */
    uint32_t reserved_1c;                /* +0x1c: not touched by recovered code */
    int32_t alternate_animation_handle;  /* +0x20: -1 until configured */
    int32_t overlay_animation_handle;    /* +0x24: -1 disables the overlay */
    uint8_t use_alternate_animation;     /* +0x28 */
    uint8_t selected;                    /* +0x29: selects visual frame 3 */
    uint8_t reserved_2a[2];
};

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dManualButtonSizeCheck[
    sizeof(NtmvM2dManualButton) == 0x2c ? 1 : -1];
typedef char NtmvM2dManualButtonAlternateHandleOffsetCheck[
    offsetof(NtmvM2dManualButton, alternate_animation_handle) == 0x20 ? 1 : -1];
typedef char NtmvM2dManualButtonOverlayHandleOffsetCheck[
    offsetof(NtmvM2dManualButton, overlay_animation_handle) == 0x24 ? 1 : -1];
typedef char NtmvM2dManualButtonAlternateFlagOffsetCheck[
    offsetof(NtmvM2dManualButton, use_alternate_animation) == 0x28 ? 1 : -1];
typedef char NtmvM2dManualButtonSelectedOffsetCheck[
    offsetof(NtmvM2dManualButton, selected) == 0x29 ? 1 : -1];
#endif

/* Header 0x02126f30; typeinfo word 0x02126f34; address point 0x02126f38. */
extern const NtmvM2dManualButtonVTable gNtmvM2dManualButtonVTable;

/* 0x020bba00; calls the ObjButton base-object constructor at 0x020b826c. */
NtmvM2dManualButton *NtmvM2dManualButton_Construct(
    NtmvM2dManualButton *button);

/* 0x020bba38 */
void NtmvM2dManualButton_Configure(
    NtmvM2dManualButton *button,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size,
    int32_t animation_handle,
    int32_t alternate_animation_handle,
    int32_t overlay_animation_handle);

/* 0x020bba88, vtable slot 1; the other four slots are inherited. */
void NtmvM2dManualButton_Render(
    NtmvM2dManualButton *button,
    void *render_context,
    const NtmvM2dPoint *parent_position);

/* 0x020bbb70 */
void NtmvM2dManualButton_SetAlternateAnimation(
    NtmvM2dManualButton *button, bool use_alternate_animation);

#endif
