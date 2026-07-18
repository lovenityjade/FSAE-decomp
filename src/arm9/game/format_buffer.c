#include "game/format_buffer.h"

#include <string.h>

/*
 * 0x02007F14
 *
 * Proof anchors:
 * - 0x020080E0 stores 0x02007F14 as the callback passed by 0x02008080;
 * - the target reads capacity at +4 and length at +8;
 * - the copy target is buffer + old length;
 * - a request crossing capacity is reduced to capacity - old length;
 * - the target always returns 1.
 *
 * As in the original code, output->length <= output->capacity is a caller
 * invariant.  The uint32_t addition preserves the ARM9 unsigned comparison.
 */
int GameFormatBuffer_Write(
    GameFormatBuffer *output,
    const void *source,
    uint32_t byte_count
)
{
    uint32_t write_count = byte_count;
    uint32_t old_length = output->length;

    if (output->capacity < old_length + write_count) {
        write_count = output->capacity - old_length;
    }

    if (write_count != 0) {
        memcpy(output->buffer + old_length, source, write_count);
    }
    output->length = old_length + write_count;
    return 1;
}
