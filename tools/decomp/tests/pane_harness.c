#include "ntmv/m2d/pane.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static void *sFreed[8];
static unsigned int sFreedCount;
static NtmvM2dPane *sAllocationStorage[4];
static uint32_t sExpectedAllocationSize;
static unsigned int sAllocationCount;
static bool sFailAllocation;
static NtmvM2dPane *sUpdateOrder[4];
static NtmvM2dPoint sInheritedPositions[4];
static unsigned int sUpdateCount;
static uint32_t sUpdateContext;
static NtmvM2dPane *sDrawOrder[4];
static unsigned int sDrawCount;
static uint32_t sDrawContext;

void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size)
{
    assert(allocator == (void *)(uintptr_t)0x1234);
    assert(size == sExpectedAllocationSize);
    ++sAllocationCount;
    return sFailAllocation ? NULL : sAllocationStorage;
}

void NNS_FndFreeToAllocator(void *allocator, void *memory)
{
    assert(allocator == (void *)(uintptr_t)0x1234);
    sFreed[sFreedCount++] = memory;
}

static void TestTreeUpdate(
    NtmvM2dPane *pane,
    void *context,
    const NtmvM2dPoint *parent_position)
{
    assert(context == &sUpdateContext);
    assert(sUpdateCount < 4);
    sUpdateOrder[sUpdateCount] = pane;
    sInheritedPositions[sUpdateCount] = *parent_position;
    ++sUpdateCount;
    NtmvM2dPane_UpdatePosition(pane, context, parent_position);
}

static void TestTreeDraw(NtmvM2dPane *pane, void *context)
{
    assert(context == &sDrawContext);
    assert(sDrawCount < 4);
    sDrawOrder[sDrawCount++] = pane;
}

static const NtmvM2dPaneVTable sTreeVTable = {
    .update_position = TestTreeUpdate,
    .draw = TestTreeDraw,
};

static void ResetAllocationFixture(uint32_t size, bool fail)
{
    unsigned int index;

    for (index = 0; index < 4; ++index) {
        sAllocationStorage[index] = (NtmvM2dPane *)(uintptr_t)0xfeed0000u;
    }
    sExpectedAllocationSize = size;
    sAllocationCount = 0;
    sFailAllocation = fail;
}

static void TestChildArrayOwnership(void)
{
    NtmvM2dAllocatorContext allocator = {
        (void *)(uintptr_t)0x1234
    };
    NtmvM2dPane pane;
    NtmvM2dPane **children;

    ResetAllocationFixture(2 * sizeof(NtmvM2dPane *), false);
    children = NtmvM2dPane_AllocateChildArray_020b56dc(&allocator, 2);
    assert(children == sAllocationStorage);
    assert(children[0] == NULL && children[1] == NULL);
    assert(sAllocationCount == 1);

    ResetAllocationFixture(sizeof(NtmvM2dPane *), false);
    children = NtmvM2dPane_AllocateChildArray_020b56dc(&allocator, 0);
    assert(children == sAllocationStorage);
    assert(children[0] == (NtmvM2dPane *)(uintptr_t)0xfeed0000u);

    ResetAllocationFixture(3 * sizeof(NtmvM2dPane *), true);
    assert(NtmvM2dPane_AllocateChildArray_020b56dc(&allocator, 3) == NULL);
    assert(sAllocationCount == 1);

    memset(&pane, 0, sizeof(pane));
    pane.child_count = 7;
    ResetAllocationFixture(2 * sizeof(NtmvM2dPane *), false);
    NtmvM2dPane_AllocateChildren_020b56b8(&pane, &allocator, 2);
    assert(pane.children == sAllocationStorage);
    assert(pane.child_capacity == 2);
    assert(pane.child_count == 7); /* This helper does not reset the count. */

    ResetAllocationFixture(sizeof(NtmvM2dPane *), true);
    NtmvM2dPane_AllocateChildren_020b56b8(&pane, &allocator, 1);
    assert(pane.children == NULL);
    assert(pane.child_capacity == 1);
    assert(pane.child_count == 7);

    sFreedCount = 0;
    NtmvM2dPane_FreeChildArray_020b56a0(&allocator, NULL);
    assert(sFreedCount == 0);
    NtmvM2dPane_FreeChildArray_020b56a0(
        &allocator, sAllocationStorage);
    assert(sFreedCount == 1);
    assert(sFreed[0] == sAllocationStorage);
}

static void TestAddChild(void)
{
    NtmvM2dPane parent;
    NtmvM2dPane child;
    NtmvM2dPane *children[2] = {NULL, NULL};

    memset(&parent, 0, sizeof(parent));
    memset(&child, 0, sizeof(child));
    parent.children = children;
    parent.child_capacity = 1;
    NtmvM2dPane_AddChild_020b5728(&parent, &child);
    assert(parent.child_count == 1);
    assert(parent.children[0] == &child);
    assert(child.parent == &parent);
}

static void TestRecursiveUpdate(void)
{
    NtmvM2dPane root;
    NtmvM2dPane child_a;
    NtmvM2dPane child_b;
    NtmvM2dPane grandchild;
    NtmvM2dPane *root_children[2] = {&child_a, &child_b};
    NtmvM2dPane *child_a_children[1] = {&grandchild};
    NtmvM2dPoint origin = {100, 200};

    memset(&root, 0, sizeof(root));
    memset(&child_a, 0, sizeof(child_a));
    memset(&child_b, 0, sizeof(child_b));
    memset(&grandchild, 0, sizeof(grandchild));
    root.vtable = &sTreeVTable;
    child_a.vtable = &sTreeVTable;
    child_b.vtable = &sTreeVTable;
    grandchild.vtable = &sTreeVTable;
    root.local_position = (NtmvM2dPoint){10, 20};
    child_a.local_position = (NtmvM2dPoint){1, 2};
    child_b.local_position = (NtmvM2dPoint){-3, 4};
    grandchild.local_position = (NtmvM2dPoint){5, -1};
    root.children = root_children;
    root.child_count = 2;
    child_a.children = child_a_children;
    child_a.child_count = 1;
    sUpdateCount = 0;

    NtmvM2dPane_UpdateTree_020b5744(
        &root, &sUpdateContext, &origin);
    assert(sUpdateCount == 4);
    assert(sUpdateOrder[0] == &root);
    assert(sUpdateOrder[1] == &child_a);
    assert(sUpdateOrder[2] == &grandchild);
    assert(sUpdateOrder[3] == &child_b);
    assert(sInheritedPositions[0].x == 100 &&
        sInheritedPositions[0].y == 200);
    assert(sInheritedPositions[1].x == 110 &&
        sInheritedPositions[1].y == 220);
    assert(sInheritedPositions[2].x == 111 &&
        sInheritedPositions[2].y == 222);
    assert(sInheritedPositions[3].x == 110 &&
        sInheritedPositions[3].y == 220);
    assert(root.world_position.x == 110 && root.world_position.y == 220);
    assert(child_a.world_position.x == 111 &&
        child_a.world_position.y == 222);
    assert(grandchild.world_position.x == 116 &&
        grandchild.world_position.y == 221);
    assert(child_b.world_position.x == 107 &&
        child_b.world_position.y == 224);
}

static void TestRecursiveDraw(void)
{
    NtmvM2dPane root;
    NtmvM2dPane child_a;
    NtmvM2dPane child_b;
    NtmvM2dPane grandchild;
    NtmvM2dPane *root_children[2] = {&child_a, &child_b};
    NtmvM2dPane *child_a_children[1] = {&grandchild};

    memset(&root, 0, sizeof(root));
    memset(&child_a, 0, sizeof(child_a));
    memset(&child_b, 0, sizeof(child_b));
    memset(&grandchild, 0, sizeof(grandchild));
    root.vtable = &sTreeVTable;
    child_a.vtable = &sTreeVTable;
    child_b.vtable = &sTreeVTable;
    grandchild.vtable = &sTreeVTable;
    root.children = root_children;
    root.child_count = 2;
    child_a.children = child_a_children;
    child_a.child_count = 1;
    sDrawCount = 0;

    NtmvM2dPane_DrawTree_020b57f4(&root, &sDrawContext);
    assert(sDrawCount == 4);
    assert(sDrawOrder[0] == &root);
    assert(sDrawOrder[1] == &child_a);
    assert(sDrawOrder[2] == &grandchild);
    assert(sDrawOrder[3] == &child_b);
}

static void TestConstructionAndDisposal(void)
{
    NtmvM2dPaneResource resource;
    NtmvM2dPane parent;
    NtmvM2dPane child;
    NtmvM2dPane *children[1];
    NtmvM2dAllocatorContext allocator;
    NtmvM2dPoint parent_position;

    memset(&resource, 0, sizeof(resource));
    resource.position.x = -7;
    resource.position.y = 12;
    resource.size.width = 80;
    resource.size.height = 24;

    assert(NtmvM2dPane_ConstructComplete(&parent, &resource) == &parent);
    assert(parent.vtable == &gNtmvM2dPaneVTable);
    assert(parent.local_position.x == -7 && parent.local_position.y == 12);
    assert(parent.world_position.x == 0 && parent.world_position.y == 0);
    assert(parent.size.width == 80 && parent.size.height == 24);
    assert(parent.parent == NULL && parent.children == NULL);
    assert(parent.child_count == 0 && parent.child_capacity == 0);
    assert(strcmp((const char *)parent.vtable->get_runtime_type(&parent),
                  "N4ntmv3m2d4PaneE") == 0);

    parent_position.x = 100;
    parent_position.y = -2;
    parent.vtable->update_position(&parent, NULL, &parent_position);
    assert(parent.world_position.x == 93 && parent.world_position.y == 10);
    parent.vtable->draw(&parent, NULL);

    assert(NtmvM2dPane_ConstructBase(&child, &resource) == &child);
    child.parent = &parent;
    children[0] = &child;
    parent.children = children;
    parent.child_count = 1;
    parent.child_capacity = 1;
    allocator.nns_allocator = (void *)(uintptr_t)0x1234;
    sFreedCount = 0;
    parent.vtable->dispose(&parent, &allocator);
    assert(sFreedCount == 2);
    assert(sFreed[0] == &child);
    assert(sFreed[1] == children);
}

int main(void)
{
    TestConstructionAndDisposal();
    TestChildArrayOwnership();
    TestAddChild();
    TestRecursiveUpdate();
    TestRecursiveDraw();
    return 0;
}
