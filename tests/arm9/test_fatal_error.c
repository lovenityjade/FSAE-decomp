#include "game/fatal_error.h"

#include <assert.h>
#include <string.h>

enum {
    EVENT_SIGNAL = 1,
    EVENT_TERMINATION = 2
};

GameFatalRuntimeState gGameFatalRuntimeState;

static int sEvents[2];
static unsigned int sEventCount;

int Game_RaiseRuntimeSignal(int signal_number)
{
    assert(signal_number == 1);
    assert(sEventCount == 0);
    sEvents[sEventCount++] = EVENT_SIGNAL;
    return 0;
}

void Game_RunTermination(int status)
{
    assert(status == 1);
    assert(sEventCount == 1);
    assert(gGameFatalRuntimeState.termination_requested == 1);
    sEvents[sEventCount++] = EVENT_TERMINATION;
}

int main(void)
{
    memset(&gGameFatalRuntimeState, 0, sizeof(gGameFatalRuntimeState));
    memset(sEvents, 0, sizeof(sEvents));
    sEventCount = 0;

    Game_HandleFatalAssertion();

    assert(sEventCount == 2);
    assert(sEvents[0] == EVENT_SIGNAL);
    assert(sEvents[1] == EVENT_TERMINATION);
    assert(gGameFatalRuntimeState.termination_requested == 1);
    return 0;
}
