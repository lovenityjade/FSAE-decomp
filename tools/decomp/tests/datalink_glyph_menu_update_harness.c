#include "game/datalink_glyph_menu_update.h"

#include "game/datalink_phase_ten_finalizer.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

typedef struct TweenCall {
    Game_DatalinkTweenRecord *tween;
    int32_t x_fx;
    int32_t y_fx;
    uint32_t duration;
    int mode;
} TweenCall;

volatile uint16_t gGameDatalinkControllerInput;
Game_DatalinkSceneOwner gGameDatalinkSceneOwner;
Game_DatalinkGlyphRuntimeOwner gGameDatalinkGlyphGraphicsRuntime;
Game_DatalinkGlyphPanelDisplay gGameDatalinkGlyphPanelDisplay;
Game_DatalinkTweenRecord gGameDatalinkGlyphCursorTweenD;
Game_DatalinkTweenRecord gGameDatalinkGlyphMotionTweenA;
Game_DatalinkTweenRecord gGameDatalinkGlyphMotionTweenB;
int32_t gGameDatalinkGlyphFooterPoint[2];
int32_t gGameDatalinkGlyphFooterYOffset;
uint32_t gGameDatalinkGlyphFooterTweenDuration;

static Game_DatalinkGlyphPoint sCursorPoint;
static Game_DatalinkGlyphPoint sHeaderPoint;
static int32_t sFooterProgress;
static int32_t sMotionProgress;
static int32_t sSqrtResult;
static int sResolvedMode;
static int8_t sClampedRow;
static uint32_t sSounds[16];
static int sSceneCommands[16];
static TweenCall sTweenCalls[8];
static unsigned int sSoundCount;
static unsigned int sSceneCommandCount;
static unsigned int sTweenCallCount;
static unsigned int sInputRepeatCalls;
static unsigned int sResolveModeCalls;
static unsigned int sFocusFooterCalls;
static unsigned int sDeleteCalls;
static unsigned int sSynchronizeCalls;
static unsigned int sConfirmCalls;
static unsigned int sClampCalls;
static unsigned int sHighlightCalls;
static unsigned int sPageRefreshCalls;
static unsigned int sUploadCalls;
static unsigned int sCursorTweenSetupCalls;
static unsigned int sCursorPointCalls;
static unsigned int sHeaderPointCalls;
static unsigned int sFooterProgressCalls;
static unsigned int sMotionProgressCalls;

void Game_UpdateDirectionalInputRepeat_0202a2a0(void)
{
    ++sInputRepeatCalls;
}

void Game_PlayDatalinkSoundEffect(uint32_t sound_id)
{
    assert(sSoundCount < sizeof(sSounds) / sizeof(sSounds[0]));
    sSounds[sSoundCount++] = sound_id;
}

int32_t FX_Sqrt(int32_t value)
{
    assert(value >= 0);
    return sSqrtResult;
}

int Game_GetDatalinkGlyphKeyboardMode_020af050(
    const Game_DatalinkGlyphMenuState *state,
    int column,
    int row)
{
    (void)state;
    (void)column;
    (void)row;
    ++sResolveModeCalls;
    return sResolvedMode;
}

void Game_EnterDatalinkGlyphConfirmColumn_020af0b0(
    Game_DatalinkGlyphMenuState *state)
{
    (void)state;
    ++sFocusFooterCalls;
}

void Game_DeleteDatalinkGlyph_020af104(
    Game_DatalinkGlyphMenuState *state)
{
    (void)state;
    ++sDeleteCalls;
}

void Game_SynchronizeDatalinkGlyphPage_020af17c(
    Game_DatalinkGlyphMenuState *state)
{
    (void)state;
    ++sSynchronizeCalls;
}

void Game_CommitDatalinkGlyphCell_020af1f4(
    Game_DatalinkGlyphMenuState *state)
{
    (void)state;
    ++sConfirmCalls;
}

void Game_ClampDatalinkGlyphRowForMode_020af380(
    const Game_DatalinkGlyphMenuState *state,
    int8_t *row)
{
    (void)state;
    ++sClampCalls;
    *row = sClampedRow;
}

void Game_DrawDatalinkGlyphModeIndicators_020af7ec(
    Game_DatalinkGlyphMenuState *state,
    int upload)
{
    (void)state;
    assert(upload == 1);
    ++sHighlightCalls;
}

void Game_UpdateDatalinkGlyphPage_020af8c8(
    Game_DatalinkGlyphMenuState *state)
{
    (void)state;
    ++sPageRefreshCalls;
}

void Game_UploadDatalinkGlyphCells_020ae72c(
    Game_DatalinkGlyphMenuState *state)
{
    (void)state;
    ++sUploadCalls;
}

void Game_ConfigureDatalinkGlyphCursorTweens_020ae968(void)
{
    ++sCursorTweenSetupCalls;
}

void Game_GetDatalinkGlyphCursorPoint_020ae830(
    Game_DatalinkGlyphPoint *point,
    const Game_DatalinkGlyphMenuState *state)
{
    (void)state;
    ++sCursorPointCalls;
    *point = sCursorPoint;
}

void Game_GetDatalinkGlyphHeaderPoint_020ae8c0(
    Game_DatalinkGlyphPoint *point,
    const Game_DatalinkGlyphMenuState *state)
{
    (void)state;
    ++sHeaderPointCalls;
    *point = sHeaderPoint;
}

int32_t Game_GetDatalinkTweenProgress(
    const Game_DatalinkTweenRecord *tween)
{
    if (tween == &gGameDatalinkGlyphCursorTweenD) {
        ++sFooterProgressCalls;
        return sFooterProgress;
    }
    assert(tween == &gGameDatalinkGlyphMotionTweenA);
    ++sMotionProgressCalls;
    return sMotionProgress;
}

void Game_ConfigureDatalinkTween(
    Game_DatalinkTweenRecord *tween,
    int32_t target_x_fx,
    int32_t target_y_fx,
    uint32_t duration_frames,
    int mode)
{
    TweenCall *call;

    assert(sTweenCallCount < sizeof(sTweenCalls) / sizeof(sTweenCalls[0]));
    call = &sTweenCalls[sTweenCallCount++];
    call->tween = tween;
    call->x_fx = target_x_fx;
    call->y_fx = target_y_fx;
    call->duration = duration_frames;
    call->mode = mode;
}

void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command)
{
    assert(scene == &gGameDatalinkSceneOwner);
    assert(sSceneCommandCount <
        sizeof(sSceneCommands) / sizeof(sSceneCommands[0]));
    sSceneCommands[sSceneCommandCount++] = command;
}

static void ResetHarness(Game_DatalinkGlyphMenuState *state)
{
    memset(state, 0, sizeof(*state));
    memset(&gGameDatalinkSceneOwner, 0, sizeof(gGameDatalinkSceneOwner));
    memset(
        &gGameDatalinkGlyphGraphicsRuntime,
        0,
        sizeof(gGameDatalinkGlyphGraphicsRuntime));
    memset(
        &gGameDatalinkGlyphPanelDisplay,
        0,
        sizeof(gGameDatalinkGlyphPanelDisplay));
    memset(&gGameDatalinkGlyphCursorTweenD, 0, sizeof(Game_DatalinkTweenRecord));
    memset(&gGameDatalinkGlyphMotionTweenA, 0, sizeof(Game_DatalinkTweenRecord));
    memset(&gGameDatalinkGlyphMotionTweenB, 0, sizeof(Game_DatalinkTweenRecord));
    memset(sSounds, 0, sizeof(sSounds));
    memset(sSceneCommands, 0, sizeof(sSceneCommands));
    memset(sTweenCalls, 0, sizeof(sTweenCalls));
    gGameDatalinkControllerInput = 0;
    gGameDatalinkGlyphGraphicsRuntime.active_player_slot_53d = 3;
    gGameDatalinkGlyphFooterPoint[0] = 30;
    gGameDatalinkGlyphFooterPoint[1] = 40;
    gGameDatalinkGlyphFooterYOffset = -5;
    gGameDatalinkGlyphFooterTweenDuration = 7;
    sCursorPoint.x = 10;
    sCursorPoint.y = 20;
    sHeaderPoint.x = 50;
    sHeaderPoint.y = 60;
    sFooterProgress = 0;
    sMotionProgress = 0;
    sSqrtResult = 768;
    sResolvedMode = 0;
    sClampedRow = 0;
    sSoundCount = 0;
    sSceneCommandCount = 0;
    sTweenCallCount = 0;
    sInputRepeatCalls = 0;
    sResolveModeCalls = 0;
    sFocusFooterCalls = 0;
    sDeleteCalls = 0;
    sSynchronizeCalls = 0;
    sConfirmCalls = 0;
    sClampCalls = 0;
    sHighlightCalls = 0;
    sPageRefreshCalls = 0;
    sUploadCalls = 0;
    sCursorTweenSetupCalls = 0;
    sCursorPointCalls = 0;
    sHeaderPointCalls = 0;
    sFooterProgressCalls = 0;
    sMotionProgressCalls = 0;
}

static int32_t ReadSceneS32(unsigned int offset)
{
    int32_t value;

    memcpy(
        &value,
        (const uint8_t *)&gGameDatalinkSceneOwner + offset,
        sizeof(value));
    return value;
}

static void WriteSceneS32(unsigned int offset, int32_t value)
{
    memcpy(
        (uint8_t *)&gGameDatalinkSceneOwner + offset,
        &value,
        sizeof(value));
}

static void AssertCommonRender(void)
{
    assert(sInputRepeatCalls == 1);
    assert(sSynchronizeCalls == 1);
    assert(sCursorPointCalls == 1);
    assert(sHeaderPointCalls == 1);
    assert(sSceneCommandCount == 2);
    assert(sSceneCommands[0] == 0x19);
    assert(sSceneCommands[1] == 0x25);
}

static void TestIdlePublishesCursorAndHeader(void)
{
    Game_DatalinkGlyphMenuState state;

    ResetHarness(&state);
    state.selected_cell_4a = 7;
    Game_UpdateDatalinkGlyphMenu_020aea40(&state);

    assert(state.selected_column_4c == 2);
    assert(state.selected_row_4d == 1);
    assert(state.selected_cell_4a == 7);
    assert(sResolveModeCalls == 1);
    assert(ReadSceneS32(0xcfc) == 0);
    assert(ReadSceneS32(0xd04) == 10 * 0x1000);
    assert(ReadSceneS32(0xd08) == 20 * 0x1000);
    assert(gGameDatalinkGlyphPanelDisplay.cursor_x_fx_17c == 10 * 0x1000);
    assert(gGameDatalinkGlyphPanelDisplay.cursor_y_fx_180 == 20 * 0x1000);
    assert(gGameDatalinkGlyphPanelDisplay.header_x_fx_124 == 50 * 0x1000);
    assert(gGameDatalinkGlyphPanelDisplay.header_y_fx_128 == 60 * 0x1000);
    assert(state.frame_counter_45 == 1);
    assert(sTweenCallCount == 0);
    AssertCommonRender();
}

static void TestRightEntersKeyboardModeAndMovesFooter(void)
{
    Game_DatalinkGlyphMenuState state;

    ResetHarness(&state);
    state.selected_cell_4a = 14;
    gGameDatalinkControllerInput = 0x80;
    sResolvedMode = 1;
    sFooterProgress = 0x1000;
    sMotionProgress = 0x1000;
    Game_UpdateDatalinkGlyphMenu_020aea40(&state);

    assert(state.keyboard_mode_46 == 1);
    assert(state.selected_column_4c == 4);
    assert(state.selected_row_4d == 2);
    assert(state.selected_cell_4a == 14);
    assert(sSoundCount == 1 && sSounds[0] == 0x3d);
    assert(sTweenCallCount == 1);
    assert(sTweenCalls[0].tween == &gGameDatalinkGlyphCursorTweenD);
    assert(sTweenCalls[0].x_fx == 30 * 0x1000);
    assert(sTweenCalls[0].y_fx == 40 * 0x1000);
    assert(sTweenCalls[0].duration == 7);
    assert(sTweenCalls[0].mode == 2);
    assert(sHighlightCalls == 1);
    assert(sSceneCommands[0] == 0x26);
}

static void TestNextPageCyclesMode(void)
{
    Game_DatalinkGlyphMenuState state;

    ResetHarness(&state);
    state.selected_cell_4a = 4;
    state.keyboard_mode_46 = 1;
    state.font_page_47 = 0;
    gGameDatalinkControllerInput = 0x100;
    Game_UpdateDatalinkGlyphMenu_020aea40(&state);

    assert(state.font_page_47 == 1);
    assert(state.keyboard_mode_46 == 2);
    assert(sPageRefreshCalls == 1);
    assert(sUploadCalls == 1);
    assert(sSoundCount == 1 && sSounds[0] == 0x3e);
    assert(sResolveModeCalls == 0);
}

static void TestCancelWithoutNameExitsImmediately(void)
{
    Game_DatalinkGlyphMenuState state;

    ResetHarness(&state);
    state.header_selection_42 = 0;
    gGameDatalinkControllerInput = 2;
    Game_UpdateDatalinkGlyphMenu_020aea40(&state);

    assert(state.phase_49 == 3);
    assert(gGameDatalinkGlyphGraphicsRuntime.active_player_slot_53d == 0);
    assert(sCursorTweenSetupCalls == 1);
    assert(sSoundCount == 1 && sSounds[0] == 0x44);
    assert(sInputRepeatCalls == 1);
    assert(sSynchronizeCalls == 0);
    assert(sSceneCommandCount == 0);
}

static void TestCancelDeletesAndStartsFlash(void)
{
    Game_DatalinkGlyphMenuState state;

    ResetHarness(&state);
    state.header_selection_42 = 2;
    gGameDatalinkControllerInput = 2;
    Game_UpdateDatalinkGlyphMenu_020aea40(&state);

    assert(sDeleteCalls == 1);
    assert(state.transition_counter_48 == 7);
    assert(sHighlightCalls == 1);
}

static void TestTransitionStartsMotionTweens(void)
{
    Game_DatalinkGlyphMenuState state;

    ResetHarness(&state);
    state.selected_cell_4a = 0;
    state.transition_state_44 = 1;
    sCursorPoint.x = 3;
    sCursorPoint.y = 4;
    WriteSceneS32(0xd04, 0);
    WriteSceneS32(0xd08, 0);
    Game_UpdateDatalinkGlyphMenu_020aea40(&state);

    assert(state.transition_state_44 == 2);
    assert(sFooterProgressCalls == 0);
    assert(sTweenCallCount == 2);
    assert(sTweenCalls[0].tween == &gGameDatalinkGlyphMotionTweenA);
    assert(sTweenCalls[1].tween == &gGameDatalinkGlyphMotionTweenB);
    assert(sTweenCalls[0].x_fx == 3 * 0x1000);
    assert(sTweenCalls[0].y_fx == 4 * 0x1000);
    assert(sTweenCalls[0].duration == 1);
    assert(sTweenCalls[0].mode == 2);
    assert(sSceneCommands[0] == 0x19);
}

static void TestCompletedMotionPublishesTarget(void)
{
    Game_DatalinkGlyphMenuState state;

    ResetHarness(&state);
    state.transition_state_44 = 2;
    sMotionProgress = 0x1000;
    Game_UpdateDatalinkGlyphMenu_020aea40(&state);

    assert(state.transition_state_44 == 0);
    assert(sHighlightCalls == 1);
    assert(ReadSceneS32(0xd04) == 10 * 0x1000);
    assert(ReadSceneS32(0xd08) == 20 * 0x1000);
    assert(sTweenCallCount == 0);
}

static void TestExitHeaderSkipsHeaderRendering(void)
{
    Game_DatalinkGlyphMenuState state;

    ResetHarness(&state);
    state.header_selection_42 = 6;
    state.frame_counter_45 = 12;
    Game_UpdateDatalinkGlyphMenu_020aea40(&state);

    assert(sSceneCommandCount == 1);
    assert(sSceneCommands[0] == 0x19);
    assert(sHeaderPointCalls == 0);
    assert(state.frame_counter_45 == 12);
}

static void TestHeaderBlinkWrapsAfterSixty(void)
{
    Game_DatalinkGlyphMenuState state;

    ResetHarness(&state);
    state.frame_counter_45 = 60;
    Game_UpdateDatalinkGlyphMenu_020aea40(&state);

    assert(sSceneCommandCount == 1);
    assert(sSceneCommands[0] == 0x19);
    assert(state.frame_counter_45 == 0);
}

int main(void)
{
    assert(sizeof(Game_DatalinkGlyphMenuState) == 0x50);
    TestIdlePublishesCursorAndHeader();
    TestRightEntersKeyboardModeAndMovesFooter();
    TestNextPageCyclesMode();
    TestCancelWithoutNameExitsImmediately();
    TestCancelDeletesAndStartsFlash();
    TestTransitionStartsMotionTweens();
    TestCompletedMotionPublishesTarget();
    TestExitHeaderSkipsHeaderRendering();
    TestHeaderBlinkWrapsAfterSixty();
    return 0;
}
