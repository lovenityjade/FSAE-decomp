#include "ntmv/m2d/items_panel.h"

/* 0x020b8e88..0x020b8ef7 */
int32_t NtmvM2dUIElementArray_FindContainingPoint_020b8e88(
    NtmvM2dUIElement *const *elements,
    int32_t element_count,
    const NtmvM2dPoint *point)
{
    int16_t index;

    for (index = 0; index < element_count; ++index) {
        NtmvM2dPoint local_point = *point;

        if (NtmvM2dUIElement_ContainsPoint(
                elements[index], &local_point)) {
            return index;
        }
    }
    return -1;
}

/* 0x020b8ef8..0x020b8f0b */
int32_t NtmvM2dItemsTocRecord_GetInitialChildIndex_020b8ef8(
    const NtmvM2dItemsTocRecord *record)
{
    return record->reserved_00 != 0 ? -1 : 0;
}
