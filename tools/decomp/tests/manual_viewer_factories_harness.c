#include "game/manual_viewer_factories.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

static uint32_t sAllocator;
static NtmvM2dUIPanel sUiPanel;
static NtmvM2dUIElement sUiElement;
static NtmvM2dItemsPanel sItemsPanel;
static NtmvM2dScrollButton sScrollButton;
static NtmvM2dManualTocPanel sTocPanel;
static NtmvM2dScrollIndicator sIndicator;
static NtmvM2dPageHeaderPanel sHeader;
static NtmvUiRenderer sRenderer;
static void *sExpectedObject;
static uint32_t sExpectedSize;
static char sExpectedConstructor;
static bool sFailAllocation;
static unsigned int sAllocationCalls;
static unsigned int sConstructorCalls;

void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size)
{
    assert(allocator == &sAllocator);
    assert(size == sExpectedSize);
    ++sAllocationCalls;
    return sFailAllocation ? 0 : sExpectedObject;
}

NtmvM2dManualTocPanel *NtmvM2dManualTocPanel_Construct(
    NtmvM2dManualTocPanel *panel)
{
    assert(sExpectedConstructor == 'T');
    assert(panel == &sTocPanel);
    ++sConstructorCalls;
    return panel;
}

NtmvM2dItemsPanel *NtmvM2dItemsPanel_Construct(NtmvM2dItemsPanel *panel)
{
    assert(sExpectedConstructor == 'M');
    assert(panel == &sItemsPanel);
    ++sConstructorCalls;
    return panel;
}

NtmvM2dScrollButton *NtmvM2dScrollButton_Construct(
    NtmvM2dScrollButton *button)
{
    assert(sExpectedConstructor == 'B');
    assert(button == &sScrollButton);
    ++sConstructorCalls;
    return button;
}

void NtmvM2dUIElement_InitWithGeometry(
    NtmvM2dUIElement *element,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size)
{
    assert(sExpectedConstructor == 'E');
    assert(element == &sUiElement);
    ++sConstructorCalls;
    element->local_position = *position;
    element->size = *size;
}

NtmvM2dUIPanel *NtmvM2dUIPanel_ConstructComplete(NtmvM2dUIPanel *panel)
{
    assert(sExpectedConstructor == 'U');
    assert(panel == &sUiPanel);
    ++sConstructorCalls;
    return panel;
}

NtmvM2dScrollIndicator *NtmvM2dScrollIndicator_Construct(
    NtmvM2dScrollIndicator *indicator)
{
    assert(sExpectedConstructor == 'I');
    assert(indicator == &sIndicator);
    ++sConstructorCalls;
    return indicator;
}

NtmvM2dPageHeaderPanel *NtmvM2dPageHeaderPanel_Construct(
    NtmvM2dPageHeaderPanel *panel)
{
    assert(sExpectedConstructor == 'H');
    assert(panel == &sHeader);
    ++sConstructorCalls;
    return panel;
}

NtmvUiRenderer *NtmvUiRenderer_Init(NtmvUiRenderer *renderer)
{
    assert(sExpectedConstructor == 'R');
    assert(renderer == &sRenderer);
    ++sConstructorCalls;
    return renderer;
}

static void Prepare(void *object, uint32_t size, char constructor, bool fail)
{
    sExpectedObject = object;
    sExpectedSize = size;
    sExpectedConstructor = constructor;
    sFailAllocation = fail;
    sAllocationCalls = 0;
    sConstructorCalls = 0;
}

int main(void)
{
    void *allocator_context = &sAllocator;
    const NtmvM2dPoint position = {-30, 36};
    const NtmvM2dSize size = {0xba, 0xc0};

    Prepare(&sScrollButton, 0x24, 'B', false);
    assert(GameManualScrollButton_Allocate_020ba71c(&allocator_context) ==
           &sScrollButton);
    assert(sAllocationCalls == 1 && sConstructorCalls == 1);
    Prepare(&sScrollButton, 0x24, 'B', true);
    assert(GameManualScrollButton_Allocate_020ba71c(&allocator_context) == 0);
    assert(sAllocationCalls == 1 && sConstructorCalls == 0);

    Prepare(&sUiPanel, 0x1c, 'U', false);
    assert(GameManualUIPanel_Allocate_020ba748(&allocator_context) ==
        &sUiPanel);
    assert(sAllocationCalls == 1 && sConstructorCalls == 1);
    Prepare(&sUiPanel, 0x1c, 'U', true);
    assert(GameManualUIPanel_Allocate_020ba748(&allocator_context) == 0);
    assert(sAllocationCalls == 1 && sConstructorCalls == 0);

    Prepare(&sUiElement, 0x14, 'E', false);
    assert(GameManualUIElement_AllocateWithGeometry_020ba774(
               &allocator_context, &position, &size) == &sUiElement);
    assert(sAllocationCalls == 1 && sConstructorCalls == 1);
    assert(sUiElement.local_position.x == -30);
    assert(sUiElement.local_position.y == 36);
    assert(sUiElement.size.width == 0xba);
    assert(sUiElement.size.height == 0xc0);
    Prepare(&sUiElement, 0x14, 'E', true);
    assert(GameManualUIElement_AllocateWithGeometry_020ba774(
               &allocator_context, &position, &size) == 0);
    assert(sAllocationCalls == 1 && sConstructorCalls == 0);

    Prepare(&sItemsPanel, 0x40, 'M', false);
    assert(GameManualItemsPanel_Allocate_020ba7e0(&allocator_context) ==
           &sItemsPanel);
    assert(sAllocationCalls == 1 && sConstructorCalls == 1);
    Prepare(&sItemsPanel, 0x40, 'M', true);
    assert(GameManualItemsPanel_Allocate_020ba7e0(&allocator_context) == 0);
    assert(sAllocationCalls == 1 && sConstructorCalls == 0);

    Prepare(&sTocPanel, 0xb0, 'T', false);
    assert(GameManualTocPanel_Allocate_020bcd34(&allocator_context) ==
        &sTocPanel);
    assert(sAllocationCalls == 1 && sConstructorCalls == 1);
    Prepare(&sTocPanel, 0xb0, 'T', true);
    assert(GameManualTocPanel_Allocate_020bcd34(&allocator_context) == 0);
    assert(sAllocationCalls == 1 && sConstructorCalls == 0);

    Prepare(&sIndicator, 0x24, 'I', false);
    assert(GameManualScrollIndicator_Allocate_020bcd8c(&allocator_context) ==
        &sIndicator);
    assert(sAllocationCalls == 1 && sConstructorCalls == 1);
    Prepare(&sIndicator, 0x24, 'I', true);
    assert(GameManualScrollIndicator_Allocate_020bcd8c(&allocator_context) ==
        0);
    assert(sAllocationCalls == 1 && sConstructorCalls == 0);

    Prepare(&sHeader, 0x24, 'H', false);
    assert(GameManualPageHeaderPanel_Allocate_020bcdb8(&allocator_context) ==
        &sHeader);
    assert(sAllocationCalls == 1 && sConstructorCalls == 1);
    Prepare(&sHeader, 0x24, 'H', true);
    assert(GameManualPageHeaderPanel_Allocate_020bcdb8(&allocator_context) ==
        0);
    assert(sAllocationCalls == 1 && sConstructorCalls == 0);

    Prepare(&sRenderer, 0x3034, 'R', false);
    assert(GameManualRenderer_Allocate_020bcde4(&allocator_context) ==
        &sRenderer);
    assert(sAllocationCalls == 1 && sConstructorCalls == 1);
    Prepare(&sRenderer, 0x3034, 'R', true);
    assert(GameManualRenderer_Allocate_020bcde4(&allocator_context) == 0);
    assert(sAllocationCalls == 1 && sConstructorCalls == 0);
    return 0;
}
