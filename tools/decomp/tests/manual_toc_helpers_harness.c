#include "ntmv/m2d/items_panel.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

static const NtmvM2dPoint *sOriginalPoint;
static NtmvM2dUIElement *sHitElement;
static unsigned int sContainsCalls;

bool NtmvM2dUIElement_ContainsPoint(
    const NtmvM2dUIElement *element,
    const NtmvM2dPoint *point)
{
    assert(point != sOriginalPoint);
    assert(point->x == sOriginalPoint->x);
    assert(point->y == sOriginalPoint->y);
    ++sContainsCalls;
    return element == sHitElement;
}

static void TestFirstHitScan(void)
{
    NtmvM2dUIElement elements[3];
    NtmvM2dUIElement *element_refs[3] = {
        &elements[0], &elements[1], &elements[2]
    };
    NtmvM2dPoint point = {-12, 93};

    memset(elements, 0, sizeof(elements));
    sOriginalPoint = &point;
    sHitElement = &elements[1];
    sContainsCalls = 0;
    assert(NtmvM2dUIElementArray_FindContainingPoint_020b8e88(
        element_refs, 3, &point) == 1);
    assert(sContainsCalls == 2);

    sHitElement = NULL;
    sContainsCalls = 0;
    assert(NtmvM2dUIElementArray_FindContainingPoint_020b8e88(
        element_refs, 3, &point) == -1);
    assert(sContainsCalls == 3);

    sContainsCalls = 0;
    assert(NtmvM2dUIElementArray_FindContainingPoint_020b8e88(
        element_refs, 0, &point) == -1);
    assert(NtmvM2dUIElementArray_FindContainingPoint_020b8e88(
        element_refs, -4, &point) == -1);
    assert(sContainsCalls == 0);
}

static void TestInitialChildIndex(void)
{
    NtmvM2dItemsTocRecord record;

    memset(&record, 0, sizeof(record));
    assert(NtmvM2dItemsTocRecord_GetInitialChildIndex_020b8ef8(
        &record) == 0);
    record.reserved_00 = 1;
    assert(NtmvM2dItemsTocRecord_GetInitialChildIndex_020b8ef8(
        &record) == -1);
    record.reserved_00 = UINT32_C(0x80000000);
    assert(NtmvM2dItemsTocRecord_GetInitialChildIndex_020b8ef8(
        &record) == -1);
}

int main(void)
{
    TestFirstHitScan();
    TestInitialChildIndex();
    return 0;
}
