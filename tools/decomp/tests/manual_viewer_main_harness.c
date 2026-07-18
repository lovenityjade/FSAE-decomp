#include "game/manual_viewer_main.h"

#include "game/input_state.h"
#include "game/manual_display.h"
#include "game/manual_toc_input.h"
#include "ntmv/m2d/scroll_controls.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum {
    TEST_VIEWER_SIZE = 0x40628,
    TEST_EXIT_OFFSET = 0x40621,
    TEST_MODE_OFFSET = 0x4061e,
    TEST_FADE_FRAMES = 17
};

enum TestHandler {
    TEST_SCROLL = 0,
    TEST_TOC,
    TEST_PAGE,
    TEST_DPAD,
    TEST_BUTTONS,
    TEST_HANDLER_COUNT
};

static uint8_t sViewer[TEST_VIEWER_SIZE];
static bool sHandlerResults[TEST_HANDLER_COUNT];
static char sHandlers[16];
static unsigned int sHandlerCount;
static int32_t sBrightness[40];
static unsigned int sBrightnessCount;
static unsigned int sRenderCount;
static unsigned int sWaitCount;
static unsigned int sTransitionCount;
static unsigned int sDisplayOnCount;
static unsigned int sShutdownCount;
static unsigned int sButtonUpdateCount;
static unsigned int sTouchUpdateCount;
static unsigned int sSleepCount;
static unsigned int sOperationStep;
static unsigned int sSleepStep;
static unsigned int sTouchStep;
static bool sExitOnInputRender;

volatile uint32_t gGameSubDisplayControl_04001000;
volatile uint16_t gGameXyButtonState_02ffffa8;

static void RecordHandler(char handler)
{
    assert(sHandlerCount + 1 < sizeof(sHandlers));
    sHandlers[sHandlerCount++] = handler;
    sHandlers[sHandlerCount] = '\0';
}

void OS_WaitVBlankIntr(void)
{
    ++sWaitCount;
}

void GX_DispOn(void)
{
    ++sDisplayOnCount;
}

void PM_GoSleepMode(
    uint32_t trigger, uint32_t key_logic, uint32_t lcd_power)
{
    assert(trigger == 4);
    assert(key_logic == 0);
    assert(lcd_power == 0);
    sSleepStep = ++sOperationStep;
    ++sSleepCount;
}

void GameManualViewer_RenderFrame_020bd00c(
    GameManualViewerPageFlow *viewer)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    ++sRenderCount;
    if (sExitOnInputRender && sRenderCount == TEST_FADE_FRAMES + 1) {
        sViewer[TEST_EXIT_OFFSET] = 1;
    }
}

void Game_SetMasterBrightnessBoth(int32_t brightness)
{
    assert(sBrightnessCount < sizeof(sBrightness) / sizeof(sBrightness[0]));
    sBrightness[sBrightnessCount++] = brightness;
}

void GameManualViewer_SetPageTransitionOffset_020bd128(
    GameManualViewerPageFlow *viewer,
    int32_t horizontal_offset)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    assert(horizontal_offset == 0);
    ++sTransitionCount;
}

void GameButtonState_Update(GameButtonState *state)
{
    assert(state == (GameButtonState *)(sViewer + 0x403a0));
    ++sButtonUpdateCount;
}

bool GameTouchSampler_Update(
    GameTouchSampler *sampler,
    GameTouchState *state)
{
    assert(sampler == (GameTouchSampler *)(sViewer + 0x403a6));
    assert(state->x == 0 && state->y == 0);
    state->x = 12;
    state->y = 34;
    state->held = 1;
    state->pressed = 1;
    state->released = 0;
    state->padding_07 = 0;
    sTouchStep = ++sOperationStep;
    ++sTouchUpdateCount;
    return true;
}

static void AssertPointer(const void *pointer)
{
    const GameTouchState *touch = pointer;

    assert(touch->x == 12 && touch->y == 34);
    assert(touch->held == 1 && touch->pressed == 1);
    assert(touch->released == 0);
}

bool GameManualViewer_HandleScrollPointer(
    GameManualViewerPageFlow *viewer,
    const NtmvM2dScrollPointerState *pointer)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    AssertPointer(pointer);
    RecordHandler('S');
    return sHandlerResults[TEST_SCROLL];
}

bool GameManualViewer_HandleTocPointer(
    GameManualViewerPageFlow *viewer,
    const NtmvM2dItemsPointerState *pointer)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    AssertPointer(pointer);
    RecordHandler('T');
    return sHandlerResults[TEST_TOC];
}

bool GameManualViewer_HandlePagePointer_020bd474(
    GameManualViewerPageFlow *viewer,
    const GameTouchState *pointer)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    AssertPointer(pointer);
    RecordHandler('P');
    return sHandlerResults[TEST_PAGE];
}

bool GameManualViewer_HandleDpadScroll(GameManualViewerPageFlow *viewer)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    RecordHandler('D');
    return sHandlerResults[TEST_DPAD];
}

bool GameManualViewer_HandleButtons(GameManualViewerPageFlow *viewer)
{
    assert(viewer == (GameManualViewerPageFlow *)sViewer);
    RecordHandler('B');
    return sHandlerResults[TEST_BUTTONS];
}

void GameManualViewer_Shutdown(GameManualViewerDisplay *viewer)
{
    assert(viewer == (GameManualViewerDisplay *)sViewer);
    ++sShutdownCount;
}

static void ResetRun(int16_t interaction_mode, bool initially_exiting)
{
    memset(sViewer, 0, sizeof(sViewer));
    memset(sHandlerResults, 0, sizeof(sHandlerResults));
    memset(sBrightness, 0, sizeof(sBrightness));
    memset(sHandlers, 0, sizeof(sHandlers));
    *(int16_t *)(sViewer + TEST_MODE_OFFSET) = interaction_mode;
    sViewer[TEST_EXIT_OFFSET] = initially_exiting ? 1 : 0;
    sHandlerCount = 0;
    sBrightnessCount = 0;
    sRenderCount = 0;
    sWaitCount = 0;
    sTransitionCount = 0;
    sDisplayOnCount = 0;
    sShutdownCount = 0;
    sButtonUpdateCount = 0;
    sTouchUpdateCount = 0;
    sSleepCount = 0;
    sOperationStep = 0;
    sSleepStep = 0;
    sTouchStep = 0;
    sExitOnInputRender = !initially_exiting;
    gGameSubDisplayControl_04001000 = 0x12345678u;
    gGameXyButtonState_02ffffa8 = 0;
}

static void AssertFadeSequence(bool includes_input_frame)
{
    unsigned int index;

    assert(sBrightnessCount == TEST_FADE_FRAMES * 2);
    for (index = 0; index < TEST_FADE_FRAMES; ++index) {
        assert(sBrightness[index] ==
               TEST_FADE_FRAMES - 1 - (int32_t)index);
        assert(sBrightness[TEST_FADE_FRAMES + index] == (int32_t)index);
    }
    assert(sRenderCount ==
           TEST_FADE_FRAMES * 2 + (includes_input_frame ? 1u : 0u));
    assert(sWaitCount == sRenderCount + 1);
    assert(sTransitionCount == sRenderCount);
    assert(sDisplayOnCount == 1);
    assert(sShutdownCount == 1);
    assert(gGameSubDisplayControl_04001000 == 0x12355678u);
}

static void RunSingleInput(
    int16_t interaction_mode,
    const char *expected_handlers)
{
    ResetRun(interaction_mode, false);
    GameManualViewer_RunMainLoop_020bce14(
        (GameManualViewerPageFlow *)sViewer);
    AssertFadeSequence(true);
    assert(sButtonUpdateCount == 1);
    assert(sTouchUpdateCount == 1);
    assert(strcmp(sHandlers, expected_handlers) == 0);
}

int main(void)
{
    unsigned int stop_handler;

    /* An already-requested exit still performs both complete fades. */
    ResetRun(0, true);
    GameManualViewer_RunMainLoop_020bce14(
        (GameManualViewerPageFlow *)sViewer);
    AssertFadeSequence(false);
    assert(sButtonUpdateCount == 0 && sTouchUpdateCount == 0);
    assert(sHandlerCount == 0);

    /* Mode zero follows the five-step priority chain. */
    RunSingleInput(0, "STPDB");
    for (stop_handler = 0;
         stop_handler < TEST_HANDLER_COUNT;
         ++stop_handler) {
        static const char *const expected[] = {
            "S", "ST", "STP", "STPD", "STPDB",
        };

        ResetRun(0, false);
        sHandlerResults[stop_handler] = true;
        GameManualViewer_RunMainLoop_020bce14(
            (GameManualViewerPageFlow *)sViewer);
        AssertFadeSequence(true);
        assert(strcmp(sHandlers, expected[stop_handler]) == 0);
    }

    /* Captured modes dispatch exactly one specialized handler. */
    RunSingleInput(1, "S");
    RunSingleInput(2, "T");
    RunSingleInput(3, "P");
    RunSingleInput(4, "D");
    RunSingleInput(-1, "");
    RunSingleInput(5, "");

    /* Lid-close state enters sleep before touch sampling and dispatch. */
    ResetRun(4, false);
    gGameXyButtonState_02ffffa8 = 0x8000;
    GameManualViewer_RunMainLoop_020bce14(
        (GameManualViewerPageFlow *)sViewer);
    AssertFadeSequence(true);
    assert(sSleepCount == 1);
    assert(sSleepStep != 0 && sSleepStep < sTouchStep);
    assert(strcmp(sHandlers, "D") == 0);
    return 0;
}
