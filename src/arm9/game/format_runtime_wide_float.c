#include "game/format_runtime_wide.h"

#include <stddef.h>
#include <stdint.h>

static uint32_t Game_WideFloatTypeToNarrow(uint32_t type_word)
{
    return (type_word & UINT32_C(0x000000ff)) |
        ((type_word >> 8) & UINT32_C(0x0000ff00));
}

static uint16_t *Game_WidenFloatResult(
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

/* 0x02008EB4..0x02009273; the 14-word IEEE-754 constant table follows. */
uint16_t *Game_FormatWideHexFloat_02008eb4(
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
    char *result = Game_FormatHexFloat_02006968(
        value_low,
        value_high,
        workspace + sizeof(workspace),
        flags_word,
        Game_WideFloatTypeToNarrow(type_word),
        width,
        precision
    );

    if (result == NULL) {
        return NULL;
    }
    return Game_WidenFloatResult(result, buffer_end);
}

/* 0x02009274..0x02009387.  This body is instruction-identical to the byte
 * formatter's decimal rounding helper at 0x02006E14. */
void Game_RoundWideDecimalDigits_02009274(
    GameFormatDecimal *decimal,
    int32_t retained_digits
)
{
    Game_RoundDecimalDigits_02006e14(decimal, retained_digits);
}

/* 0x02009388..0x0200997F; eleven floating-point constants follow. */
uint16_t *Game_FormatWideDecimalFloat_02009388(
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
    char *result = Game_FormatDecimalFloat_02006f28(
        value_low,
        value_high,
        workspace + sizeof(workspace),
        flags_word,
        Game_WideFloatTypeToNarrow(type_word),
        width,
        precision
    );

    if (result == NULL) {
        return NULL;
    }
    return Game_WidenFloatResult(result, buffer_end);
}
