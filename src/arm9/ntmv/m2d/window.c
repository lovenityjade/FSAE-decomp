#include "ntmv/m2d/window.h"

/* External NNS allocation API used at 0x020b5c84 and 0x020b5d00. */
extern void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size);
extern void NNS_FndFreeToAllocator(void *allocator, void *memory);

extern void NtmvM2d_FillRect_020b73b8(
    void *context,
    const NtmvM2dPoint *destination,
    const NtmvM2dSize *size,
    uint16_t value);

/*
 * GetRuntimeType returns 0x02126ad4, immediately before ABI typeinfo at
 * 0x02126ad8.  The typeinfo names N4ntmv3m2d6WindowE and records Pane's
 * 0x02126a64 typeinfo as its base.  This string is a semantic local tag, not
 * an attempted reconstruction of the CodeWarrior RTTI object.
 */
static const char sNtmvM2dWindowRuntimeType[] = "N4ntmv3m2d6WindowE";

const NtmvM2dWindowVTable gNtmvM2dWindowVTable = {
    NtmvM2dWindow_GetRuntimeType,  /* slot 0: 0x020b64f0 */
    NtmvM2dWindow_Dispose,         /* slot 1: 0x020b5cd4 */
    NtmvM2dPane_UpdatePosition,    /* slot 2: 0x020b57c8 */
    NtmvM2dWindow_Draw,            /* slot 3: 0x020b5d48 */
    NtmvM2dWindow_DrawInterior,    /* slot 4: 0x020b5e48 */
    NtmvM2dWindow_DrawSingleBorder,/* slot 5: 0x020b5ec4 */
    NtmvM2dWindow_DrawFourBorders, /* slot 6: 0x020b6158 */
};

/* Window overlays its 16-bit fill value onto Pane bytes +0x1e and +0x1f. */
static void SetFillValue(NtmvM2dWindow *window, uint16_t value)
{
    window->pane.reserved_1d[1] = (uint8_t)value;
    window->pane.reserved_1d[2] = (uint8_t)(value >> 8);
}

static uint16_t GetFillValue(const NtmvM2dWindow *window)
{
    return (uint16_t)(
        window->pane.reserved_1d[1] |
        ((uint16_t)window->pane.reserved_1d[2] << 8));
}

/* 0x020b5ba4 */
static void NtmvM2dWindow_DestroyBorder_020b5ba4(
    NtmvM2dWindowBorder *border,
    NtmvM2dAllocatorContext *allocator)
{
    (void)border;
    (void)allocator;
}

/* 0x020b5c84: allocation plus value-initialization of eight-byte entries. */
static NtmvM2dWindowBorder *AllocateBorders(
    NtmvM2dAllocatorContext *allocator, uint8_t count)
{
    uint32_t index;
    uint32_t allocation_count = count == 0 ? 1u : count;
    NtmvM2dWindowBorder *borders = NNS_FndAllocFromAllocator(
        allocator->nns_allocator,
        allocation_count * (uint32_t)sizeof(NtmvM2dWindowBorder));

    if (borders == NULL) {
        return NULL;
    }
    for (index = 0; index < count; ++index) {
        borders[index].bitmap_ref = NULL;
        borders[index].transform_code = 0;
        borders[index].reserved_05[0] = 0;
        borders[index].reserved_05[1] = 0;
        borders[index].reserved_05[2] = 0;
    }
    return borders;
}

/* 0x020b5c58 wraps the allocator while publishing pointer and count. */
static void InitializeBorders(
    NtmvM2dWindow *window,
    NtmvM2dAllocatorContext *allocator,
    uint8_t count)
{
    window->border_count = 0;
    window->borders = AllocateBorders(allocator, count);
    window->border_count = count;
}

/* 0x020b5ba8. The original assumes a successful internal allocation. */
NtmvM2dWindow *NtmvM2dWindow_Construct(
    NtmvM2dWindow *window,
    NtmvM2dAllocatorContext *allocator,
    const NtmvM2dWindowResource *resource,
    const NtmvM2dWindowBuildContext *build_context)
{
    uint32_t index;

    NtmvM2dPane_ConstructBase(&window->pane, &resource->pane);
    SetFillValue(window, resource->fill_value);
    window->pane.vtable = (const NtmvM2dPaneVTable *)&gNtmvM2dWindowVTable;
    window->border_count = 0;
    window->borders = NULL;

    if (resource->border_count != 0) {
        InitializeBorders(window, allocator, resource->border_count);
        for (index = 0; index < window->border_count; ++index) {
            window->borders[index].bitmap_ref =
                &build_context->resource_set->bitmaps[
                    resource->borders[index].bitmap_index];
            window->borders[index].transform_code =
                resource->borders[index].transform_code;
        }
    }
    return window;
}

/* 0x020b64f0, vtable slot 0. */
const void *NtmvM2dWindow_GetRuntimeType(const NtmvM2dPane *pane)
{
    (void)pane;
    return sNtmvM2dWindowRuntimeType;
}

/* 0x020b5d00. The per-border destructor at 0x020b5ba4 is a no-op. */
static void FreeBorders(
    NtmvM2dAllocatorContext *allocator,
    NtmvM2dWindowBorder *borders,
    uint8_t count)
{
    uint32_t index;

    if (borders == NULL) {
        return;
    }
    for (index = 0; index < count; ++index) {
        NtmvM2dWindow_DestroyBorder_020b5ba4(&borders[index], allocator);
    }
    NNS_FndFreeToAllocator(allocator->nns_allocator, borders);
}

/* 0x020b5cd4, vtable slot 1. */
void NtmvM2dWindow_Dispose(
    NtmvM2dPane *pane, NtmvM2dAllocatorContext *allocator)
{
    NtmvM2dWindow *window = (NtmvM2dWindow *)pane;

    FreeBorders(allocator, window->borders, window->border_count);
    NtmvM2dPane_Dispose(pane, allocator);
}

/* 0x020b6430: derive left/right/top/bottom thickness from bitmap headers. */
static void GetMargins(
    NtmvM2dWindowMargins *margins,
    uint8_t border_count,
    const NtmvM2dWindowBorder *borders)
{
    margins->left = 0;
    margins->right = 0;
    margins->top = 0;
    margins->bottom = 0;

    if (border_count == 1) {
        const NtmvM2dWindowBitmap *bitmap = *borders[0].bitmap_ref;
        margins->left = (int16_t)bitmap->width;
        margins->right = (int16_t)bitmap->width;
        margins->top = (int16_t)bitmap->height;
        margins->bottom = (int16_t)bitmap->height;
    } else if (border_count == 4) {
        margins->left = (int16_t)(*borders[2].bitmap_ref)->width;
        margins->right = (int16_t)(*borders[1].bitmap_ref)->width;
        margins->top = (int16_t)(*borders[0].bitmap_ref)->height;
        margins->bottom = (int16_t)(*borders[3].bitmap_ref)->height;
    }
}

/* 0x020b5d48, vtable slot 3. */
void NtmvM2dWindow_Draw(NtmvM2dPane *pane, void *context)
{
    NtmvM2dWindow *window = (NtmvM2dWindow *)pane;
    const NtmvM2dWindowVTable *vtable =
        (const NtmvM2dWindowVTable *)window->pane.vtable;
    NtmvM2dWindowMargins margins;
    NtmvM2dPoint position = window->pane.world_position;

    GetMargins(&margins, window->border_count, window->borders);
    vtable->draw_interior(window, context, &position, &margins);
    if (window->border_count == 1) {
        vtable->draw_single_border(
            window, context, &position, window->borders, &margins);
    } else if (window->border_count == 4) {
        vtable->draw_four_borders(
            window, context, &position, window->borders, &margins);
    }
}

/* 0x020b5e48, vtable slot 4. Bit 15 gates the interior fill. */
void NtmvM2dWindow_DrawInterior(
    NtmvM2dWindow *window,
    void *context,
    const NtmvM2dPoint *position,
    const NtmvM2dWindowMargins *margins)
{
    uint16_t fill_value = GetFillValue(window);

    if ((fill_value & 0x8000u) != 0) {
        NtmvM2dPoint destination;
        NtmvM2dSize size;

        destination.x = (int16_t)(position->x + margins->left);
        destination.y = (int16_t)(position->y + margins->top);
        size.width = (int16_t)(
            window->pane.size.width - margins->left - margins->right);
        size.height = (int16_t)(
            window->pane.size.height - margins->top - margins->bottom);
        NtmvM2d_FillRect_020b73b8(context, &destination, &size, fill_value);
    }
}

/* 0x020b5950 */
static void NtmvM2dWindow_GetTopRect_020b5950(
    NtmvM2dPoint *destination,
    NtmvM2dSize *size,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *window_size,
    const NtmvM2dWindowMargins *margins)
{
    *destination = *position;
    size->width = (int16_t)(window_size->width - margins->right);
    size->height = margins->top;
}

/* 0x020b5998 */
static void NtmvM2dWindow_GetRightRect_020b5998(
    NtmvM2dPoint *destination,
    NtmvM2dSize *size,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *window_size,
    const NtmvM2dWindowMargins *margins)
{
    destination->x = (int16_t)(
        position->x + window_size->width - margins->right);
    destination->y = position->y;
    size->width = margins->right;
    size->height = (int16_t)(window_size->height - margins->bottom);
}

/* 0x020b5ae4 */
static void NtmvM2dWindow_GetBottomRect_020b5ae4(
    NtmvM2dPoint *destination,
    NtmvM2dSize *size,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *window_size,
    const NtmvM2dWindowMargins *margins)
{
    destination->x = (int16_t)(position->x + margins->left);
    destination->y = (int16_t)(
        position->y + window_size->height - margins->bottom);
    size->width = (int16_t)(window_size->width - margins->left);
    size->height = margins->bottom;
}

/* 0x020b5a34 */
static void NtmvM2dWindow_GetLeftRect_020b5a34(
    NtmvM2dPoint *destination,
    NtmvM2dSize *size,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *window_size,
    const NtmvM2dWindowMargins *margins)
{
    destination->x = position->x;
    destination->y = (int16_t)(position->y + margins->top);
    size->width = margins->left;
    size->height = (int16_t)(window_size->height - margins->top);
}

static int16_t GetBitmapAxis(const NtmvM2dSize *size, uint32_t axis)
{
    return axis == 0 ? size->width : size->height;
}

static int8_t GetTransformAxis(const int8_t values[2], uint32_t axis)
{
    return values[axis == 0 ? 0 : 1];
}

static void SetOriginAxis(NtmvM2dPoint *origin, uint32_t axis, int16_t value)
{
    if (axis == 0) {
        origin->x = value;
    } else {
        origin->y = value;
    }
}

/* 0x020b5984: PC-literal tail-call wrapper around 0x020b76d0. */
static void NtmvM2dWindow_ComputeTopSourceOrigin_020b5984(
    NtmvM2dPoint *origin,
    const NtmvM2dSize *bitmap_size,
    const NtmvM2dTransformDescriptor *transform)
{
    NtmvM2d_ComputeSourceOrigin_020b76d0(origin, bitmap_size, transform);
}

/* 0x020b59dc: generic right-edge origin helper, duplicated in this unit. */
static void NtmvM2dWindow_ComputeRightSourceOrigin_020b59dc(
    NtmvM2dPoint *origin,
    const NtmvM2dSize *bitmap_size,
    const NtmvM2dTransformDescriptor *transform)
{
    uint32_t axis = transform->transpose_axes != 0 ? 1u : 0u;
    uint32_t other = axis ^ 1u;
    int32_t origin_axis = GetTransformAxis(transform->origin_factor, axis);
    int32_t origin_other = GetTransformAxis(transform->origin_factor, other);

    SetOriginAxis(
        origin,
        axis,
        (int16_t)(origin_axis * (GetBitmapAxis(bitmap_size, axis) - 1)));
    SetOriginAxis(
        origin,
        other,
        (int16_t)(origin_other * (GetBitmapAxis(bitmap_size, other) - 1)));
}

/* 0x020b5b30: align a reversed/repeated bottom border to its right edge. */
static void ComputeBottomSourceOrigin(
    NtmvM2dPoint *origin,
    const NtmvM2dSize *draw_size,
    const NtmvM2dSize *bitmap_size,
    const NtmvM2dTransformDescriptor *transform)
{
    uint32_t axis = transform->transpose_axes != 0 ? 1u : 0u;
    uint32_t other = axis ^ 1u;
    int32_t origin_axis = GetTransformAxis(transform->origin_factor, axis);
    int32_t step_axis = GetTransformAxis(transform->step, axis);
    int32_t origin_other = GetTransformAxis(transform->origin_factor, other);

    SetOriginAxis(
        origin,
        axis,
        (int16_t)(
            (origin_axis + step_axis) * (GetBitmapAxis(bitmap_size, axis) - 1) -
            (draw_size->width - 1) * step_axis));
    SetOriginAxis(
        origin,
        other,
        (int16_t)(origin_other * (GetBitmapAxis(bitmap_size, other) - 1)));
}

/* 0x020b5a70: align a reversed/repeated left border to its bottom edge. */
static void ComputeLeftSourceOrigin(
    NtmvM2dPoint *origin,
    const NtmvM2dSize *draw_size,
    const NtmvM2dSize *bitmap_size,
    const NtmvM2dTransformDescriptor *transform)
{
    uint32_t axis = transform->transpose_axes != 0 ? 1u : 0u;
    uint32_t other = axis ^ 1u;
    int32_t origin_axis = GetTransformAxis(transform->origin_factor, axis);
    int32_t origin_other = GetTransformAxis(transform->origin_factor, other);
    int32_t step_other = GetTransformAxis(transform->step, other);

    SetOriginAxis(
        origin,
        axis,
        (int16_t)(origin_axis * (GetBitmapAxis(bitmap_size, axis) - 1)));
    SetOriginAxis(
        origin,
        other,
        (int16_t)(
            (origin_other + step_other) * (GetBitmapAxis(bitmap_size, other) - 1) -
            (draw_size->height - 1) * step_other));
}

static NtmvM2dSize GetBitmapSize(const NtmvM2dWindowBorder *border)
{
    const NtmvM2dWindowBitmap *bitmap = *border->bitmap_ref;
    NtmvM2dSize size;

    size.width = (int16_t)bitmap->width;
    size.height = (int16_t)bitmap->height;
    return size;
}

static void BlitBorder(
    void *context,
    const NtmvM2dPoint *destination,
    const NtmvM2dSize *draw_size,
    const NtmvM2dWindowBorder *border,
    const NtmvM2dPoint *source_origin,
    const NtmvM2dTransformDescriptor *transform)
{
    NtmvM2d_BlitTransformed_020b786c(
        context,
        destination,
        draw_size,
        &border->bitmap_ref,
        source_origin,
        transform);
}

/* 0x020b5ec4, vtable slot 5. The stored transform byte is observably ignored. */
void NtmvM2dWindow_DrawSingleBorder(
    NtmvM2dWindow *window,
    void *context,
    const NtmvM2dPoint *position,
    const NtmvM2dWindowBorder *border,
    const NtmvM2dWindowMargins *margins)
{
    const uint8_t transform_codes[4] = {0, 1, 4, 2};
    const NtmvM2dTransformDescriptor *transform;
    NtmvM2dSize bitmap_size = GetBitmapSize(border);
    NtmvM2dPoint destination;
    NtmvM2dSize draw_size;
    NtmvM2dPoint source_origin;

    NtmvM2dWindow_GetTopRect_020b5950(
        &destination, &draw_size, position, &window->pane.size, margins);
    transform = NtmvM2d_GetTransformDescriptor_020b76bc(transform_codes[0]);
    NtmvM2dWindow_ComputeTopSourceOrigin_020b5984(
        &source_origin, &bitmap_size, transform);
    BlitBorder(context, &destination, &draw_size, border, &source_origin, transform);

    NtmvM2dWindow_GetRightRect_020b5998(
        &destination, &draw_size, position, &window->pane.size, margins);
    transform = NtmvM2d_GetTransformDescriptor_020b76bc(transform_codes[1]);
    NtmvM2dWindow_ComputeRightSourceOrigin_020b59dc(
        &source_origin, &bitmap_size, transform);
    BlitBorder(context, &destination, &draw_size, border, &source_origin, transform);

    NtmvM2dWindow_GetBottomRect_020b5ae4(
        &destination, &draw_size, position, &window->pane.size, margins);
    transform = NtmvM2d_GetTransformDescriptor_020b76bc(transform_codes[2]);
    ComputeBottomSourceOrigin(&source_origin, &draw_size, &bitmap_size, transform);
    BlitBorder(context, &destination, &draw_size, border, &source_origin, transform);

    NtmvM2dWindow_GetLeftRect_020b5a34(
        &destination, &draw_size, position, &window->pane.size, margins);
    transform = NtmvM2d_GetTransformDescriptor_020b76bc(transform_codes[3]);
    ComputeLeftSourceOrigin(&source_origin, &draw_size, &bitmap_size, transform);
    BlitBorder(context, &destination, &draw_size, border, &source_origin, transform);
}

/* 0x020b6158, vtable slot 6. Resource order is top, right, left, bottom. */
void NtmvM2dWindow_DrawFourBorders(
    NtmvM2dWindow *window,
    void *context,
    const NtmvM2dPoint *position,
    const NtmvM2dWindowBorder *borders,
    const NtmvM2dWindowMargins *margins)
{
    const uint8_t border_indices[4] = {0, 1, 3, 2};
    const NtmvM2dTransformDescriptor *transform;
    const NtmvM2dWindowBorder *border;
    NtmvM2dSize bitmap_size;
    NtmvM2dPoint destination;
    NtmvM2dSize draw_size;
    NtmvM2dPoint source_origin;

    border = &borders[border_indices[0]];
    bitmap_size = GetBitmapSize(border);
    NtmvM2dWindow_GetTopRect_020b5950(
        &destination, &draw_size, position, &window->pane.size, margins);
    transform = NtmvM2d_GetTransformDescriptor_020b76bc(border->transform_code);
    NtmvM2dWindow_ComputeTopSourceOrigin_020b5984(
        &source_origin, &bitmap_size, transform);
    BlitBorder(context, &destination, &draw_size, border, &source_origin, transform);

    border = &borders[border_indices[1]];
    bitmap_size = GetBitmapSize(border);
    NtmvM2dWindow_GetRightRect_020b5998(
        &destination, &draw_size, position, &window->pane.size, margins);
    transform = NtmvM2d_GetTransformDescriptor_020b76bc(border->transform_code);
    NtmvM2dWindow_ComputeRightSourceOrigin_020b59dc(
        &source_origin, &bitmap_size, transform);
    BlitBorder(context, &destination, &draw_size, border, &source_origin, transform);

    border = &borders[border_indices[2]];
    bitmap_size = GetBitmapSize(border);
    NtmvM2dWindow_GetBottomRect_020b5ae4(
        &destination, &draw_size, position, &window->pane.size, margins);
    transform = NtmvM2d_GetTransformDescriptor_020b76bc(border->transform_code);
    ComputeBottomSourceOrigin(&source_origin, &draw_size, &bitmap_size, transform);
    BlitBorder(context, &destination, &draw_size, border, &source_origin, transform);

    border = &borders[border_indices[3]];
    bitmap_size = GetBitmapSize(border);
    NtmvM2dWindow_GetLeftRect_020b5a34(
        &destination, &draw_size, position, &window->pane.size, margins);
    transform = NtmvM2d_GetTransformDescriptor_020b76bc(border->transform_code);
    ComputeLeftSourceOrigin(&source_origin, &draw_size, &bitmap_size, transform);
    BlitBorder(context, &destination, &draw_size, border, &source_origin, transform);
}
