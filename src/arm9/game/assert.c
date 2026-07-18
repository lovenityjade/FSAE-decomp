#include "game/assert.h"

/*
 * Recovered dependency at 0x02007F58.  Its variadic, printf-style behavior is
 * proved by this call site; its final project name is still an hypothesis.
 */
extern int Game_LogFormat(const char *format, ...);

/*
 * Recovered dependency at 0x02005488.  It enters the fatal-error/termination
 * path.  Whether it is formally noreturn remains an explicit open question.
 */
extern void Game_HandleFatalAssertion(void);

static const char sAssertionFormat[] =
    "Assertion (%s) failed in \"%s\", function \"%s\", line %d\n";

/* 0x020056AC */
void Game_AssertFail(
    const char *expression,
    const char *file,
    const char *function,
    int line
)
{
    (void)Game_LogFormat(sAssertionFormat, expression, file, function, line);
    Game_HandleFatalAssertion();
}
