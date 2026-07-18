#ifndef FSAE_GAME_FORMAT_BUFFER_H
#define FSAE_GAME_FORMAT_BUFFER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Output state used by the formatter callback at 0x02007F14.  On ARM9 the
 * fields are located at +0x00, +0x04 and +0x08 respectively.
 */
typedef struct GameFormatBuffer {
    uint8_t *buffer;
    uint32_t capacity;
    uint32_t length;
} GameFormatBuffer;

#if UINTPTR_MAX == UINT32_MAX
typedef char GameFormatBufferTargetSizeCheck[
    sizeof(GameFormatBuffer) == 0x0C ? 1 : -1];
typedef char GameFormatBufferCapacityOffsetCheck[
    offsetof(GameFormatBuffer, capacity) == 0x04 ? 1 : -1];
typedef char GameFormatBufferLengthOffsetCheck[
    offsetof(GameFormatBuffer, length) == 0x08 ? 1 : -1];
#endif

/*
 * 0x02007F14
 *
 * Bounded byte sink used indirectly by the formatter at 0x02008080.  It does
 * not append a terminator; that caller performs termination after formatting.
 */
int GameFormatBuffer_Write(
    GameFormatBuffer *output,
    const void *source,
    uint32_t byte_count
);

#ifdef __cplusplus
}
#endif

#endif /* FSAE_GAME_FORMAT_BUFFER_H */
