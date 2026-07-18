#include "ntmv/m2d/ui_static.h"
#include "ntmv/ui_renderer.h"

const NtmvM2dUIStaticVTable gNtmvM2dUIStaticVTable = {
    (void (*)(NtmvM2dUIStatic *, NtmvM2dAllocatorContext *))NtmvM2dUIElement_Destroy,
    NtmvM2dUIStatic_Render,
    (void (*)(NtmvM2dUIStatic *, const NtmvM2dPoint *))NtmvM2dUIElement_SetPosition,
    (void (*)(NtmvM2dUIStatic *, const NtmvM2dSize *))NtmvM2dUIElement_SetSize,
    (void (*)(NtmvM2dUIStatic *, bool))NtmvM2dUIElement_SetVisible,
};

static NtmvM2dUIStatic *ConstructCommon(NtmvM2dUIStatic *element)
{
    NtmvM2dUIElement_Init(&element->base);
    element->base.vtable = (const NtmvM2dUIElementVTable *)&gNtmvM2dUIStaticVTable;
    element->resource_id = -1;
    return element;
}

/* 0x020b7f28: base-object constructor variant. */
NtmvM2dUIStatic *NtmvM2dUIStatic_ConstructBase(NtmvM2dUIStatic *element)
{
    return ConstructCommon(element);
}

/* 0x020b7f50: instruction-identical complete-object constructor variant. */
NtmvM2dUIStatic *NtmvM2dUIStatic_ConstructComplete(NtmvM2dUIStatic *element)
{
    return ConstructCommon(element);
}

/* 0x020b7f78 */
void NtmvM2dUIStatic_Configure(
    NtmvM2dUIStatic *element,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size,
    int32_t resource_id)
{
    element->base.local_position = *position;
    element->base.size = *size;
    element->resource_id = resource_id;
}

/* 0x020b7fa0, UIStatic vtable slot 1. */
void NtmvM2dUIStatic_Render(
    NtmvM2dUIStatic *element,
    void *render_context,
    const NtmvM2dPoint *parent_position)
{
    NtmvUiRenderer *renderer;

    if (!NtmvM2dUIElement_UpdateWorldCenter(&element->base, parent_position)) {
        return;
    }
    renderer = *(NtmvUiRenderer **)render_context;
    (void)NtmvUiRenderer_SubmitCell(
        renderer,
        element->resource_id,
        (int32_t)element->base.world_center.x << 12,
        (int32_t)element->base.world_center.y << 12);
}

/*
 * 0x020b7fec is a tail call to 0x020bc2a8.  Its third argument is preserved
 * in r2; recovered call sites pass frame zero or one.
 */
void NtmvM2dUIStatic_SetAnimationFrame(
    NtmvM2dUIStatic *element, void *render_context, uint32_t frame)
{
    (void)NtmvUiRenderer_SetAnimationFrame(
        render_context, element->resource_id, frame);
}

/* 0x020b8004; advances the cell animation by fixed-point unit 0x1000. */
void NtmvM2dUIStatic_TickAnimation(
    NtmvM2dUIStatic *element, void *render_context)
{
    (void)NtmvUiRenderer_TickAnimation(
        render_context, element->resource_id, 0x1000);
}
