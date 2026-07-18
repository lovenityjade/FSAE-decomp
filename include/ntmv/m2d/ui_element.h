#ifndef FSAE_NTMV_M2D_UI_ELEMENT_H
#define FSAE_NTMV_M2D_UI_ELEMENT_H

#include "ntmv/m2d/pane.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct NtmvM2dUIElement NtmvM2dUIElement;

typedef struct NtmvM2dUIElementVTable {
    void (*destroy)(
        NtmvM2dUIElement *element, NtmvM2dAllocatorContext *allocator);
    void (*render)(
        NtmvM2dUIElement *element,
        void *render_context,
        const NtmvM2dPoint *parent_position);
    void (*set_position)(NtmvM2dUIElement *element, const NtmvM2dPoint *position);
    void (*set_size)(NtmvM2dUIElement *element, const NtmvM2dSize *size);
    void (*set_visible)(NtmvM2dUIElement *element, bool visible);
} NtmvM2dUIElementVTable;

/* RTTI typeinfo 0x02126ce0; vtable address point 0x02126d08. */
struct NtmvM2dUIElement {
    const NtmvM2dUIElementVTable *vtable; /* +0x00 */
    NtmvM2dPoint local_position;         /* +0x04 */
    NtmvM2dSize size;                    /* +0x08 */
    NtmvM2dPoint world_center;           /* +0x0c */
    uint8_t flags;                       /* +0x10 */
    uint8_t reserved_11[3];
};

enum {
    NTMV_M2D_UI_HIDDEN = 1 << 0,
    NTMV_M2D_UI_HIT_STATE_LOCKED = 1 << 1,
    NTMV_M2D_UI_HOVERED = 1 << 2,
};

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dUIElementSizeCheck[sizeof(NtmvM2dUIElement) == 0x14 ? 1 : -1];
typedef char NtmvM2dUIElementWorldOffsetCheck[
    offsetof(NtmvM2dUIElement, world_center) == 0x0c ? 1 : -1];
#endif

extern const NtmvM2dUIElementVTable gNtmvM2dUIElementVTable;

void NtmvM2dUIElement_Init(NtmvM2dUIElement *element); /* 0x020b7c50 */
void NtmvM2dUIElement_InitWithGeometry(                 /* 0x020b7c80 */
    NtmvM2dUIElement *element,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size);
void NtmvM2dUIElement_Destroy( /* 0x020b7cd0 */
    NtmvM2dUIElement *element, NtmvM2dAllocatorContext *allocator);
void NtmvM2dUIElement_Render(                             /* 0x020b7cd4 */
    NtmvM2dUIElement *element,
    void *render_context,
    const NtmvM2dPoint *parent_position);
bool NtmvM2dUIElement_IsOnScreen(const NtmvM2dUIElement *element); /* 0x020b7d08 */
bool NtmvM2dUIElement_UpdateWorldCenter(                           /* 0x020b7d74 */
    NtmvM2dUIElement *element, const NtmvM2dPoint *parent_position);
bool NtmvM2dUIElement_ContainsPoint(                               /* 0x020b7db4 */
    const NtmvM2dUIElement *element, const NtmvM2dPoint *point);
void NtmvM2dUIElement_SetPosition(                                 /* 0x020b7e48 */
    NtmvM2dUIElement *element, const NtmvM2dPoint *position);
void NtmvM2dUIElement_SetSize(                                     /* 0x020b7e5c */
    NtmvM2dUIElement *element, const NtmvM2dSize *size);
void NtmvM2dUIElement_SetVisible(                                  /* 0x020b7e70 */
    NtmvM2dUIElement *element, bool visible);
void NtmvM2dUIElement_UpdateHitState(                               /* 0x020b7e94 */
    NtmvM2dUIElement *element, const NtmvM2dPoint *point);
void NtmvM2dUIElement_GetTopLeft(                                  /* 0x020b7ef8 */
    NtmvM2dPoint *output, const NtmvM2dUIElement *element);

#endif
