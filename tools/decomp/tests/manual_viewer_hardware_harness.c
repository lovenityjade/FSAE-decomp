#include "game/manual_viewer_lifecycle.h"

#include "game/input_state.h"
#include "game/manual_display.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

volatile uint32_t gGameSubDisplayControl_04001000;

static uint8_t sViewer[0x40628];
static char sEvents[24];
static unsigned int sEventCount;
static bool sExternalState;
static bool sPreviousVBlankState;
static bool sDmaConfigured;
static unsigned int sVBlankCallCount;

static void Event(char event)
{
    assert(sEventCount + 1 < sizeof(sEvents));
    sEvents[sEventCount++] = event;
    sEvents[sEventCount] = '\0';
}

void OSi_InitCommon(void)
{
    Event('I');
}

void Game_GraphicsPreInitHook_02012c3c(void)
{
    Event('P');
}

bool Game_IsExternalDisplayStateActive_0218a870(void)
{
    Event('E');
    return sExternalState;
}

void Game_SetExternalDisplayState_0218a860(uint32_t state)
{
    assert(state == 1);
    Event('S');
}

bool GX_VBlankIntr(bool enabled)
{
    assert(enabled);
    Event('V');
    ++sVBlankCallCount;
    return sVBlankCallCount == 1 ? sPreviousVBlankState : true;
}

void GX_Init(void)
{
    Event('G');
}

bool Game_IsGraphicsDefaultDmaConfigured_020bc528(void)
{
    Event('Q');
    return sDmaConfigured;
}

void GX_SetDefaultDMA(uint32_t dma_channel)
{
    assert(dma_channel == 3);
    Event('D');
}

void GX_DispOff(void)
{
    Event('O');
}

void GameButtonState_Update(GameButtonState *state)
{
    assert(state == (GameButtonState *)(sViewer + 0x403a0));
    Event('B');
}

void GameTouchSampler_Start(GameTouchSampler *sampler)
{
    assert(sampler == (GameTouchSampler *)(sViewer + 0x403a6));
    Event('T');
}

static void RunScenario(
    bool external_state,
    bool previous_vblank_state,
    bool dma_configured,
    const char *expected_events)
{
    static const uint32_t initial_context[4] = {
        0x11223344,
        0x55667788,
        0x99aabbcc,
        0xddeeff00,
    };

    memset(sViewer, 0, sizeof(sViewer));
    sEventCount = 0;
    sEvents[0] = '\0';
    sExternalState = external_state;
    sPreviousVBlankState = previous_vblank_state;
    sDmaConfigured = dma_configured;
    sVBlankCallCount = 0;
    gGameSubDisplayControl_04001000 = 0xffffffffu;

    GameManualViewer_InitializeHardware_020bc8bc(
        (GameManualViewerPageFlow *)sViewer,
        initial_context);

    assert(memcmp(sViewer, initial_context, sizeof(initial_context)) == 0);
    assert(strcmp(sEvents, expected_events) == 0);
    assert(sVBlankCallCount == 2);
    assert(sViewer[0x40624] == (external_state ? 1 : 0));
    assert(sViewer[0x40623] == (previous_vblank_state ? 1 : 0));
    assert(sViewer[0x40622] == (dma_configured ? 0 : 1));
    assert((gGameSubDisplayControl_04001000 & 0x00010000u) == 0);
    assert((gGameSubDisplayControl_04001000 & 0xfffeffffu) ==
        0xfffeffffu);
}

int main(void)
{
    RunScenario(false, false, false, "IPESVGQDOBTV");
    RunScenario(true, true, true, "IPEVGQOBTV");
    return 0;
}
