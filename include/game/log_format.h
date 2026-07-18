#ifndef FSAE_GAME_LOG_FORMAT_H
#define FSAE_GAME_LOG_FORMAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Common callback shape consumed by the formatter at 0x02007638. */
typedef int (*GameFormatWriteCallback)(
    void *context,
    const void *data,
    uint32_t byte_count
);

/*
 * 0x02007F58
 *
 * Variadic formatter used by the assertion path at 0x020056AC.  It writes to
 * the global stream at 0x020DF628 and returns -1 if that stream cannot enter
 * the required orientation.
 */
int Game_LogFormat(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* FSAE_GAME_LOG_FORMAT_H */
