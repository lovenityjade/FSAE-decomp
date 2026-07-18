#include "game/format_runtime_low.h"

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

static bool Game_IsFormatDigit(unsigned char value)
{
    return value >= (unsigned char)'0' && value <= (unsigned char)'9';
}

static uint32_t Game_ParseFormatUnsigned(const char **cursor)
{
    uint32_t value = 0;

    while (Game_IsFormatDigit((unsigned char)**cursor)) {
        uint32_t digit = (uint32_t)(unsigned char)**cursor - '0';

        if (value > (UINT32_MAX - digit) / 10U) {
            value = UINT32_MAX;
        } else {
            value = value * 10U + digit;
        }
        ++*cursor;
    }
    return value;
}

static bool Game_IsFormatIntegerLength(uint8_t length)
{
    return length != GAME_FORMAT_LENGTH_WIDE &&
        length != GAME_FORMAT_LENGTH_LONG_DOUBLE;
}

static void Game_InvalidateFormatConversion(GameFormatConversion *conversion)
{
    conversion->conversion_05 = UINT8_MAX;
}

/*
 * 0x02005F38..0x0200643B (1284 bytes).
 *
 * Parses one conversion beginning at '%'.  The retail routine writes the
 * same four-word descriptor and advances the saved ARM va_list when '*' is
 * used for width or precision.  Its ctype-table and maximum-field literals
 * at 0x0200643C are deliberately outside this body.
 */
const char *Game_ParseFormatConversion_02005f38(
    const char *percent,
    va_list *arguments,
    GameFormatConversion *conversion)
{
    const char *cursor = percent + 1;
    uint32_t width = 0;
    uint32_t precision = 0;
    unsigned char code;

    conversion->padding_00 = GAME_FORMAT_PADDING_SPACE;
    conversion->sign_01 = GAME_FORMAT_SIGN_NONE;
    conversion->precision_present_02 = 0;
    conversion->alternate_03 = 0;
    conversion->length_04 = GAME_FORMAT_LENGTH_DEFAULT;
    conversion->conversion_05 = 0;
    conversion->reserved_06[0] = 0;
    conversion->reserved_06[1] = 0;
    conversion->width_08 = 0;
    conversion->precision_0c = 0;

    if (*cursor == '%') {
        conversion->conversion_05 = (uint8_t)'%';
        return cursor + 1;
    }

    for (;;) {
        switch (*cursor) {
        case '-':
            conversion->padding_00 = GAME_FORMAT_PADDING_LEFT;
            ++cursor;
            continue;
        case '+':
            conversion->sign_01 = GAME_FORMAT_SIGN_PLUS;
            ++cursor;
            continue;
        case ' ':
            if (conversion->sign_01 != GAME_FORMAT_SIGN_PLUS) {
                conversion->sign_01 = GAME_FORMAT_SIGN_SPACE;
            }
            ++cursor;
            continue;
        case '#':
            conversion->alternate_03 = 1;
            ++cursor;
            continue;
        case '0':
            if (conversion->padding_00 != GAME_FORMAT_PADDING_LEFT) {
                conversion->padding_00 = GAME_FORMAT_PADDING_ZERO;
            }
            ++cursor;
            continue;
        default:
            break;
        }
        break;
    }

    if (*cursor == '*') {
        int supplied_width = va_arg(*arguments, int);

        ++cursor;
        if (supplied_width < 0) {
            conversion->padding_00 = GAME_FORMAT_PADDING_LEFT;
            width = supplied_width == INT_MIN ?
                (uint32_t)INT_MAX + 1U : (uint32_t)-supplied_width;
        } else {
            width = (uint32_t)supplied_width;
        }
    } else {
        width = Game_ParseFormatUnsigned(&cursor);
    }
    conversion->width_08 = width;
    if (width > GAME_FORMAT_MAX_FIELD) {
        Game_InvalidateFormatConversion(conversion);
        return *cursor == '\0' ? cursor : cursor + 1;
    }

    if (*cursor == '.') {
        int supplied_precision;

        ++cursor;
        conversion->precision_present_02 = 1;
        if (*cursor == '*') {
            supplied_precision = va_arg(*arguments, int);
            ++cursor;
            if (supplied_precision < 0) {
                conversion->precision_present_02 = 0;
            } else {
                precision = (uint32_t)supplied_precision;
            }
        } else {
            precision = Game_ParseFormatUnsigned(&cursor);
        }
    }

    if (*cursor == 'h') {
        conversion->length_04 = GAME_FORMAT_LENGTH_SHORT;
        ++cursor;
        if (*cursor == 'h') {
            conversion->length_04 = GAME_FORMAT_LENGTH_CHAR;
            ++cursor;
        }
    } else if (*cursor == 'l') {
        conversion->length_04 = GAME_FORMAT_LENGTH_LONG;
        ++cursor;
        if (*cursor == 'l') {
            conversion->length_04 = GAME_FORMAT_LENGTH_LONG_LONG;
            ++cursor;
        }
    } else if (*cursor == 'j') {
        conversion->length_04 = GAME_FORMAT_LENGTH_INTMAX;
        ++cursor;
    } else if (*cursor == 'z') {
        conversion->length_04 = GAME_FORMAT_LENGTH_SIZE;
        ++cursor;
    } else if (*cursor == 't') {
        conversion->length_04 = GAME_FORMAT_LENGTH_PTRDIFF;
        ++cursor;
    } else if (*cursor == 'L') {
        conversion->length_04 = GAME_FORMAT_LENGTH_LONG_DOUBLE;
        ++cursor;
    }

    code = (unsigned char)*cursor;
    conversion->conversion_05 = code;
    if (*cursor != '\0') {
        ++cursor;
    }

    switch (code) {
    case 'd':
    case 'i':
    case 'o':
    case 'u':
    case 'x':
    case 'X':
        if (!Game_IsFormatIntegerLength(conversion->length_04)) {
            Game_InvalidateFormatConversion(conversion);
        }
        if (conversion->precision_present_02 == 0) {
            precision = 1;
        } else if (conversion->padding_00 == GAME_FORMAT_PADDING_ZERO) {
            conversion->padding_00 = GAME_FORMAT_PADDING_SPACE;
        }
        break;
    case 'p':
        conversion->conversion_05 = (uint8_t)'x';
        conversion->alternate_03 = 1;
        conversion->length_04 = GAME_FORMAT_LENGTH_LONG;
        precision = 8;
        break;
    case 'c':
        if (conversion->length_04 == GAME_FORMAT_LENGTH_LONG) {
            conversion->length_04 = GAME_FORMAT_LENGTH_CHAR;
        } else if (conversion->length_04 != GAME_FORMAT_LENGTH_DEFAULT) {
            Game_InvalidateFormatConversion(conversion);
        }
        break;
    case 's':
        if (conversion->length_04 == GAME_FORMAT_LENGTH_LONG) {
            conversion->length_04 = GAME_FORMAT_LENGTH_WIDE;
        } else if (conversion->length_04 != GAME_FORMAT_LENGTH_DEFAULT) {
            Game_InvalidateFormatConversion(conversion);
        }
        break;
    case 'n':
        if (conversion->length_04 == GAME_FORMAT_LENGTH_LONG_DOUBLE) {
            Game_InvalidateFormatConversion(conversion);
        }
        break;
    case 'a':
    case 'A':
        if (conversion->precision_present_02 == 0) {
            precision = 13;
        }
        if (conversion->length_04 != GAME_FORMAT_LENGTH_DEFAULT &&
            conversion->length_04 != GAME_FORMAT_LENGTH_LONG_DOUBLE) {
            Game_InvalidateFormatConversion(conversion);
        }
        break;
    case 'f':
    case 'F':
    case 'e':
    case 'E':
        if (conversion->precision_present_02 == 0) {
            precision = 6;
        }
        if (conversion->length_04 != GAME_FORMAT_LENGTH_DEFAULT &&
            conversion->length_04 != GAME_FORMAT_LENGTH_LONG_DOUBLE) {
            Game_InvalidateFormatConversion(conversion);
        }
        break;
    case 'g':
    case 'G':
        if (conversion->precision_present_02 == 0) {
            precision = 6;
        } else if (precision == 0) {
            precision = 1;
        }
        if (conversion->length_04 != GAME_FORMAT_LENGTH_DEFAULT &&
            conversion->length_04 != GAME_FORMAT_LENGTH_LONG_DOUBLE) {
            Game_InvalidateFormatConversion(conversion);
        }
        break;
    case '%':
        break;
    default:
        Game_InvalidateFormatConversion(conversion);
        break;
    }

    conversion->precision_0c = precision;
    return cursor;
}
