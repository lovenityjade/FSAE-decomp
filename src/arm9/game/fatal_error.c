#include "game/fatal_error.h"

/*
 * 0x0200813C accepts values 1..7 and dispatches a registered runtime signal.
 * Its final source name is unknown; this descriptive dependency is provisional.
 */
extern int Game_RaiseRuntimeSignal(int signal_number);

/*
 * 0x020054B0 performs callback cleanup when permitted and then enters the
 * wider termination dispatcher.  The descriptive name is provisional.
 */
extern void Game_RunTermination(int status);

/*
 * 0x02005488
 *
 * Instruction proof:
 *   0x02005494 -> call 0x0200813C with r0 = 1
 *   0x020054A0 -> store 1 at [0x02127280 + 0x0C]
 *   0x020054A4 -> call 0x020054B0 with r0 = 1
 */
void Game_HandleFatalAssertion(void)
{
    (void)Game_RaiseRuntimeSignal(1);
    gGameFatalRuntimeState.termination_requested = 1;
    Game_RunTermination(1);
}
