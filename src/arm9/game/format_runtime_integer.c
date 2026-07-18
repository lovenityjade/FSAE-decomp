#include "game/format_runtime_low.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static uint8_t Game_FormatFlagByte(uint32_t word, unsigned int index)
{
    return (uint8_t)(word >> (index * 8U));
}

static char Game_FormatIntegerDigit(uint32_t digit, uint8_t conversion)
{
    if (digit < 10U) {
        return (char)('0' + digit);
    }
    return (char)((conversion == (uint8_t)'x' ? 'a' : 'A') + digit - 10U);
}

static char *Game_FormatUnsignedIntegerBackward(
    uint64_t magnitude,
    bool negative,
    char *buffer_end,
    uint32_t flags_word,
    uint32_t type_word,
    int32_t width,
    int32_t precision)
{
    uint8_t padding = Game_FormatFlagByte(flags_word, 0);
    uint8_t sign = Game_FormatFlagByte(flags_word, 1);
    bool alternate = Game_FormatFlagByte(flags_word, 3) != 0;
    uint8_t conversion = Game_FormatFlagByte(type_word, 1);
    uint32_t base;
    uint32_t digit_count = 0;
    uint32_t prefix_count = 0;
    int32_t zero_target = precision;
    char *cursor = buffer_end - 1;

    *cursor = '\0';
    switch (conversion) {
    case 'o':
        base = 8;
        sign = GAME_FORMAT_SIGN_NONE;
        break;
    case 'x':
    case 'X':
        base = 16;
        sign = GAME_FORMAT_SIGN_NONE;
        break;
    case 'u':
        base = 10;
        sign = GAME_FORMAT_SIGN_NONE;
        break;
    default:
        base = 10;
        break;
    }

    if (magnitude != 0U || precision != 0 ||
        (conversion == (uint8_t)'o' && alternate)) {
        do {
            uint32_t digit = (uint32_t)(magnitude % base);

            magnitude /= base;
            --cursor;
            *cursor = Game_FormatIntegerDigit(digit, conversion);
            ++digit_count;
        } while (magnitude != 0U);
    }

    if (conversion == (uint8_t)'o' && alternate &&
        (digit_count == 0U || *cursor != '0')) {
        --cursor;
        *cursor = '0';
        ++digit_count;
    }
    if ((conversion == (uint8_t)'x' || conversion == (uint8_t)'X') &&
        alternate && digit_count != 0U) {
        prefix_count = 2;
    }

    if (padding == GAME_FORMAT_PADDING_ZERO) {
        zero_target = width;
        if (negative || sign != GAME_FORMAT_SIGN_NONE) {
            --zero_target;
        }
        zero_target -= (int32_t)prefix_count;
    }
    if (zero_target < 0 ||
        zero_target + (int32_t)(buffer_end - cursor) >
            GAME_FORMAT_MAX_FIELD) {
        return NULL;
    }
    while ((int32_t)digit_count < zero_target) {
        --cursor;
        *cursor = '0';
        ++digit_count;
    }

    if (prefix_count != 0U) {
        --cursor;
        *cursor = (char)conversion;
        --cursor;
        *cursor = '0';
    }
    if (negative) {
        --cursor;
        *cursor = '-';
    } else if (sign == GAME_FORMAT_SIGN_PLUS) {
        --cursor;
        *cursor = '+';
    } else if (sign == GAME_FORMAT_SIGN_SPACE) {
        --cursor;
        *cursor = ' ';
    }
    return cursor;
}

/*
 * 0x02006444..0x0200668F (588 bytes).
 * Converts the 32-bit integer to an octal, decimal or hexadecimal string in
 * reverse workspace order and incorporates precision zeroes, alternate-form
 * prefixes and the recovered sign mode.  The 509-byte limit literal at
 * 0x02006690 is excluded.
 */
char *Game_FormatInteger32_02006444(
    int32_t value,
    char *buffer_end,
    uint32_t flags_word,
    uint32_t type_word,
    int32_t width,
    int32_t precision)
{
    uint8_t conversion = Game_FormatFlagByte(type_word, 1);
    bool signed_conversion = conversion == (uint8_t)'d' ||
        conversion == (uint8_t)'i';
    bool negative = signed_conversion && value < 0;
    uint32_t magnitude = negative ?
        UINT32_C(0) - (uint32_t)value : (uint32_t)value;

    return Game_FormatUnsignedIntegerBackward(
        magnitude,
        negative,
        buffer_end,
        flags_word,
        type_word,
        width,
        precision
    );
}

/*
 * 0x02006694..0x02006963 (720 bytes).
 * 64-bit counterpart of 0x02006444.  The retail body performs the division
 * through the ARM runtime helpers at 0x0200D900/0x0200D90C and excludes its
 * 509-byte limit literal at 0x02006964.
 */
char *Game_FormatInteger64_02006694(
    uint32_t value_low,
    uint32_t value_high,
    char *buffer_end,
    uint32_t flags_word,
    uint32_t type_word,
    int32_t width,
    int32_t precision)
{
    uint8_t conversion = Game_FormatFlagByte(type_word, 1);
    bool signed_conversion = conversion == (uint8_t)'d' ||
        conversion == (uint8_t)'i';
    uint64_t bits = ((uint64_t)value_high << 32) | value_low;
    bool negative = signed_conversion && (value_high & UINT32_C(0x80000000));
    uint64_t magnitude = negative ? UINT64_C(0) - bits : bits;

    return Game_FormatUnsignedIntegerBackward(
        magnitude,
        negative,
        buffer_end,
        flags_word,
        type_word,
        width,
        precision
    );
}
