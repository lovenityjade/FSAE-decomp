#include "game/datalink_common_render_tail.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

Game_DatalinkSceneOwner gGameDatalinkSceneOwner;

static Game_DatalinkFlowState *sExpectedState;
static int sCommands[5];
static size_t sCommandCount;
static size_t sRefreshCount;
static bool sSuppressDuringCommandFour;
static bool sMutateSelectionDuringRows;

void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command
)
{
    assert(scene == &gGameDatalinkSceneOwner);
    assert(sCommandCount < sizeof(sCommands) / sizeof(sCommands[0]));
    sCommands[sCommandCount++] = command;

    if (command == 4 && sSuppressDuringCommandFour) {
        sExpectedState->suppress_redraw_42 = 1;
    }
    if (sMutateSelectionDuringRows && command == 5) {
        sExpectedState->selected_participant_40 = 2;
    }
}

void Game_RefreshDatalinkSelectionRender(Game_DatalinkFlowState *state)
{
    assert(state == sExpectedState);
    assert(sCommandCount == 1);
    assert(sCommands[0] == 4);
    assert(state->suppress_redraw_42 == 0U);
    ++sRefreshCount;
    state->selected_participant_40 = 0;
}

static void ResetHarness(Game_DatalinkFlowState *state)
{
    memset(state, 0xA5, sizeof(*state));
    memset(&gGameDatalinkSceneOwner, 0xA5,
           sizeof(gGameDatalinkSceneOwner));
    state->selected_participant_40 = -1;
    state->suppress_redraw_42 = 0;
    sExpectedState = state;
    sCommandCount = 0;
    sRefreshCount = 0;
    sSuppressDuringCommandFour = false;
    sMutateSelectionDuringRows = false;
}

static void TestSuppressionIsReadAfterCommandFour(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state);
    sSuppressDuringCommandFour = true;

    Game_RenderDatalinkCommonTail(&state);

    assert(sCommandCount == 1);
    assert(sCommands[0] == 4);
    assert(sRefreshCount == 0);
}

static void TestRefreshAndEveryRowPrecedeFinalCommand(void)
{
    Game_DatalinkFlowState state;

    ResetHarness(&state);
    sMutateSelectionDuringRows = true;

    Game_RenderDatalinkCommonTail(&state);

    assert(sRefreshCount == 1);
    assert(sCommandCount == 5);
    assert(sCommands[0] == 4);
    assert(sCommands[1] == 5);
    assert(sCommands[2] == 8);
    assert(sCommands[3] == 9);
    assert(sCommands[4] == 0x17);
    assert(state.selected_participant_40 == 2);
}

int main(void)
{
    TestSuppressionIsReadAfterCommandFour();
    TestRefreshAndEveryRowPrecedeFinalCommand();
    return 0;
}
