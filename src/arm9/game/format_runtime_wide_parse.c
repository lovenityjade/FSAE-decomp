#include "game/format_runtime_wide.h"

#include <stddef.h>
#include <stdint.h>

/* The wide SDK routines use the same flag byte layout as their byte-string
 * twins, but place the conversion code in bits 16..31 of the type word. */
static uint32_t Game_WideTypeToNarrow(uint32_t type_word)
{
    return (type_word & UINT32_C(0x000000ff)) |
        ((type_word >> 8) & UINT32_C(0x0000ff00));
}

static uint16_t *Game_WidenFormattedResult(
    const char *source,
    uint16_t *buffer_end
)
{
    size_t length = Game_StringLength_02008264(source);
    uint16_t *destination = buffer_end - (length + 1U);
    size_t index;

    for (index = 0; index <= length; ++index) {
        destination[index] = (uint8_t)source[index];
    }
    return destination;
}

/* 0x02008478..0x0200898F; ctype and field-limit literals follow.  Format
 * directives are the ASCII subset of UTF-16, so the recovered byte parser is
 * also the exact policy engine for flags, lengths, '*' and validation. */
const uint16_t *Game_ParseWideFormatConversion_02008478(
    const uint16_t *percent,
    va_list *arguments,
    GameFormatConversion *conversion
)
{
    char narrow[GAME_FORMAT_WORK_BUFFER_SIZE];
    const char *narrow_end;
    uint32_t count = 0;

    while (count + 1U < GAME_FORMAT_WORK_BUFFER_SIZE && percent[count] != 0) {
        narrow[count] = (char)(uint8_t)percent[count];
        ++count;
    }
    narrow[count] = '\0';

    narrow_end = Game_ParseFormatConversion_02005f38(
        narrow,
        arguments,
        conversion
    );
    return percent + (narrow_end - narrow);
}

/* 0x02008990..0x02008BDB; one-word field-limit pool follows. */
uint16_t *Game_FormatWideInteger32_02008990(
    int32_t value,
    uint16_t *buffer_end,
    uint32_t flags_word,
    uint32_t type_word,
    int32_t width,
    int32_t precision
)
{
    char workspace[GAME_FORMAT_WORK_BUFFER_SIZE];
    char *result = Game_FormatInteger32_02006444(
        value,
        workspace + sizeof(workspace),
        flags_word,
        Game_WideTypeToNarrow(type_word),
        width,
        precision
    );

    if (result == NULL) {
        return NULL;
    }
    return Game_WidenFormattedResult(result, buffer_end);
}

/* 0x02008BDC..0x02008EB3; one-word field-limit pool follows. */
uint16_t *Game_FormatWideInteger64_02008bdc(
    uint32_t value_low,
    uint32_t value_high,
    uint16_t *buffer_end,
    uint32_t flags_word,
    uint32_t type_word,
    int32_t width,
    int32_t precision
)
{
    char workspace[GAME_FORMAT_WORK_BUFFER_SIZE];
    char *result = Game_FormatInteger64_02006694(
        value_low,
        value_high,
        workspace + sizeof(workspace),
        flags_word,
        Game_WideTypeToNarrow(type_word),
        width,
        precision
    );

    if (result == NULL) {
        return NULL;
    }
    return Game_WidenFormattedResult(result, buffer_end);
}
