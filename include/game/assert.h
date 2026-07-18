#ifndef FSAE_GAME_ASSERT_H
#define FSAE_GAME_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 0x020056AC
 *
 * The argument order is proven by the format string used by the function:
 *   Assertion (%s) failed in "%s", function "%s", line %d
 *
 * The function invokes the game's fatal-error path after logging.  It is not
 * declared noreturn yet because that property has not been proved for every
 * runtime configuration.
 */
void Game_AssertFail(
    const char *expression,
    const char *file,
    const char *function,
    int line
);

#ifdef __cplusplus
}
#endif

#endif /* FSAE_GAME_ASSERT_H */
