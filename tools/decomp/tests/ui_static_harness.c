#include "ntmv/m2d/ui_static.h"
#include "ntmv/ui_renderer.h"

#include <assert.h>
#include <stdint.h>

static void *sExpectedRenderer = (void *)(uintptr_t)0x1234;
static int32_t sResource;
static int32_t sX;
static int32_t sY;

bool NtmvUiRenderer_SubmitCell(
    NtmvUiRenderer *renderer,
    int32_t resource_id,
    int32_t x_fx12,
    int32_t y_fx12)
{
    assert(renderer == (NtmvUiRenderer *)sExpectedRenderer);
    sResource = resource_id;
    sX = x_fx12;
    sY = y_fx12;
    return true;
}

bool NtmvUiRenderer_SetAnimationFrame(
    NtmvUiRenderer *renderer, int32_t resource_id, uint32_t frame)
{
    assert(renderer == (NtmvUiRenderer *)sExpectedRenderer);
    assert(frame == 3);
    sResource = resource_id;
    return true;
}

bool NtmvUiRenderer_TickAnimation(
    NtmvUiRenderer *renderer, int32_t resource_id, int32_t scale_fx12)
{
    assert(renderer == (NtmvUiRenderer *)sExpectedRenderer);
    assert(scale_fx12 == 0x1000);
    sResource = resource_id;
    return true;
}

int main(void)
{
    NtmvM2dUIStatic element;
    NtmvM2dPoint position = {12, -5};
    NtmvM2dSize size = {32, 16};
    NtmvM2dPoint parent = {100, 80};
    void *context = sExpectedRenderer;

    assert(NtmvM2dUIStatic_ConstructComplete(&element) == &element);
    assert(element.resource_id == -1);
    NtmvM2dUIStatic_Configure(&element, &position, &size, 7);
    NtmvM2dUIStatic_Render(&element, &context, &parent);
    assert(sResource == 7);
    assert(sX == 112 * 0x1000);
    assert(sY == 75 * 0x1000);
    NtmvM2dUIStatic_SetAnimationFrame(&element, sExpectedRenderer, 3);
    NtmvM2dUIStatic_TickAnimation(&element, sExpectedRenderer);
    return 0;
}
