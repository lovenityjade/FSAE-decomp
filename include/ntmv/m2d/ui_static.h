#ifndef FSAE_NTMV_M2D_UI_STATIC_H
#define FSAE_NTMV_M2D_UI_STATIC_H

#include "ntmv/m2d/ui_element.h"

#include <stdint.h>

typedef struct NtmvM2dUIStatic NtmvM2dUIStatic;

typedef struct NtmvM2dUIStaticVTable {
    void (*destroy)(
        NtmvM2dUIStatic *element, NtmvM2dAllocatorContext *allocator);
    void (*render)(
        NtmvM2dUIStatic *element,
        void *render_context,
        const NtmvM2dPoint *parent_position);
    void (*set_position)(NtmvM2dUIStatic *element, const NtmvM2dPoint *position);
    void (*set_size)(NtmvM2dUIStatic *element, const NtmvM2dSize *size);
    void (*set_visible)(NtmvM2dUIStatic *element, bool visible);
} NtmvM2dUIStaticVTable;

/*
 * RTTI typeinfo 0x02126d1c; base UIElement typeinfo 0x02126ce0;
 * callable vtable address point 0x02126d48.
 */
struct NtmvM2dUIStatic {
    NtmvM2dUIElement base; /* +0x00 */
    int32_t resource_id;   /* +0x14 */
};

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dUIStaticSizeCheck[sizeof(NtmvM2dUIStatic) == 0x18 ? 1 : -1];
#endif

extern const NtmvM2dUIStaticVTable gNtmvM2dUIStaticVTable;

NtmvM2dUIStatic *NtmvM2dUIStatic_ConstructBase( /* 0x020b7f28 */
    NtmvM2dUIStatic *element);
NtmvM2dUIStatic *NtmvM2dUIStatic_ConstructComplete( /* 0x020b7f50 */
    NtmvM2dUIStatic *element);
void NtmvM2dUIStatic_Configure( /* 0x020b7f78 */
    NtmvM2dUIStatic *element,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size,
    int32_t resource_id);
void NtmvM2dUIStatic_Render( /* 0x020b7fa0 */
    NtmvM2dUIStatic *element,
    void *render_context,
    const NtmvM2dPoint *parent_position);
void NtmvM2dUIStatic_SetAnimationFrame( /* 0x020b7fec */
    NtmvM2dUIStatic *element, void *render_context, uint32_t frame);
void NtmvM2dUIStatic_TickAnimation( /* 0x020b8004 */
    NtmvM2dUIStatic *element, void *render_context);

#endif
