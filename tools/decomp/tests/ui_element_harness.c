#include "ntmv/m2d/ui_element.h"

#include <assert.h>

int main(void)
{
    NtmvM2dUIElement element;
    NtmvM2dPoint position = {10, 20};
    NtmvM2dSize size = {40, 30};
    NtmvM2dPoint parent = {100, 50};
    NtmvM2dPoint point;

    NtmvM2dUIElement_InitWithGeometry(&element, &position, &size);
    assert(element.vtable == &gNtmvM2dUIElementVTable);
    assert(NtmvM2dUIElement_UpdateWorldCenter(&element, &parent));
    assert(element.world_center.x == 110 && element.world_center.y == 70);

    NtmvM2dUIElement_GetTopLeft(&point, &element);
    assert(point.x == 90 && point.y == 55);
    point.x = 130;
    point.y = 85;
    assert(NtmvM2dUIElement_ContainsPoint(&element, &point));
    NtmvM2dUIElement_UpdateHitState(&element, &point);
    assert((element.flags & NTMV_M2D_UI_HOVERED) != 0);

    NtmvM2dUIElement_SetVisible(&element, false);
    assert(!NtmvM2dUIElement_ContainsPoint(&element, &point));
    assert(!NtmvM2dUIElement_UpdateWorldCenter(&element, &parent));
    NtmvM2dUIElement_SetVisible(&element, true);
    assert((element.flags & NTMV_M2D_UI_HIDDEN) == 0);
    return 0;
}
