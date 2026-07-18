#include "game/format_runtime_low.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern void Game_ConvertDoubleToDecimal_0200acf8(
    void *workspace,
    uint32_t value_low,
    uint32_t value_high,
    GameFormatDecimal *decimal
);

static uint8_t Game_FormatFloatByte(uint32_t word, unsigned int index)
{
    return (uint8_t)(word >> (index * 8U));
}

static char *Game_CopyFormatResultBackward(
    char *buffer_end,
    const char *text,
    uint32_t length)
{
    char *start;
    uint32_t index;

    if (length > GAME_FORMAT_MAX_FIELD) {
        return NULL;
    }
    start = buffer_end - 1 - length;
    for (index = 0; index < length; ++index) {
        start[index] = text[index];
    }
    start[length] = '\0';
    return start;
}

static uint32_t Game_FormatSpecialFloat(
    char *output,
    uint32_t value_low,
    uint32_t value_high,
    uint8_t conversion)
{
    bool negative = (value_high & UINT32_C(0x80000000)) != 0;
    bool uppercase = conversion >= (uint8_t)'A' &&
        conversion <= (uint8_t)'Z';
    uint32_t exponent = (value_high >> 20) & UINT32_C(0x7ff);
    uint32_t mantissa_high = value_high & UINT32_C(0x000fffff);
    const char *word;
    uint32_t length = 0;

    if (exponent != UINT32_C(0x7ff)) {
        return 0;
    }
    if (negative) {
        output[length++] = '-';
    }
    word = mantissa_high == 0U && value_low == 0U ?
        (uppercase ? "INF" : "inf") :
        (uppercase ? "NAN" : "nan");
    output[length++] = word[0];
    output[length++] = word[1];
    output[length++] = word[2];
    return length;
}

static void Game_AppendUnsignedDecimal(
    char *output,
    uint32_t *length,
    uint32_t value,
    uint32_t minimum_digits)
{
    char reversed[12];
    uint32_t count = 0;

    do {
        reversed[count++] = (char)('0' + value % 10U);
        value /= 10U;
    } while (value != 0U);
    while (count < minimum_digits) {
        reversed[count++] = '0';
    }
    while (count != 0U) {
        output[(*length)++] = reversed[--count];
    }
}

/*
 * 0x02006968..0x02006DEB (1156 bytes).
 * Builds the C99 hexadecimal floating representation, including the retail
 * signed INF/NAN spellings, alternate decimal point, exact exponent sign and
 * requested hexadecimal precision.  Its ten-word string/table pool begins at
 * 0x02006DEC and is excluded.
 */
char *Game_FormatHexFloat_02006968(
    uint32_t value_low,
    uint32_t value_high,
    char *buffer_end,
    uint32_t flags_word,
    uint32_t type_word,
    int32_t width,
    int32_t precision)
{
    char text[GAME_FORMAT_WORK_BUFFER_SIZE];
    uint64_t bits = ((uint64_t)value_high << 32) | value_low;
    uint64_t fraction = bits & UINT64_C(0x000fffffffffffff);
    uint32_t raw_exponent = (value_high >> 20) & UINT32_C(0x7ff);
    uint8_t conversion = Game_FormatFloatByte(type_word, 1);
    uint8_t sign_mode = Game_FormatFloatByte(flags_word, 1);
    bool alternate = Game_FormatFloatByte(flags_word, 3) != 0;
    bool negative = (value_high & UINT32_C(0x80000000)) != 0;
    bool uppercase = conversion == (uint8_t)'A';
    int32_t exponent;
    uint32_t length;
    uint32_t digit;
    int32_t index;

    (void)width;
    if (precision < 0 || precision > GAME_FORMAT_MAX_FIELD) {
        return NULL;
    }
    length = Game_FormatSpecialFloat(
        text,
        value_low,
        value_high,
        conversion
    );
    if (length != 0U) {
        return Game_CopyFormatResultBackward(buffer_end, text, length);
    }

    length = 0;
    if (negative) {
        text[length++] = '-';
    } else if (sign_mode == GAME_FORMAT_SIGN_PLUS) {
        text[length++] = '+';
    } else if (sign_mode == GAME_FORMAT_SIGN_SPACE) {
        text[length++] = ' ';
    }
    text[length++] = '0';
    text[length++] = uppercase ? 'X' : 'x';
    if (raw_exponent == 0U) {
        text[length++] = '0';
        exponent = fraction == 0U ? 0 : -1022;
    } else {
        text[length++] = '1';
        exponent = (int32_t)raw_exponent - 1023;
    }
    if (precision != 0 || alternate) {
        text[length++] = '.';
    }
    for (index = 0; index < precision; ++index) {
        if (index < 13) {
            digit = (uint32_t)(fraction >> (48 - index * 4)) & 0x0fU;
        } else {
            digit = 0;
        }
        text[length++] = (char)(digit < 10U ?
            '0' + digit : (uppercase ? 'A' : 'a') + digit - 10U);
        if (length >= GAME_FORMAT_MAX_FIELD) {
            return NULL;
        }
    }
    text[length++] = uppercase ? 'P' : 'p';
    if (exponent < 0) {
        text[length++] = '-';
        exponent = -exponent;
    } else {
        text[length++] = '+';
    }
    Game_AppendUnsignedDecimal(text, &length, (uint32_t)exponent, 1);
    return Game_CopyFormatResultBackward(buffer_end, text, length);
}

/*
 * 0x02006E14..0x02006F27 (276 bytes).
 * Rounds the decimal digit record to the requested length using round-half-
 * to-even, carries through runs of nines and canonicalizes a discarded zero
 * result.  This body reaches the next prologue directly and has no pool.
 */
void Game_RoundDecimalDigits_02006e14(
    GameFormatDecimal *decimal,
    int32_t retained_digits)
{
    bool round_up = false;
    int32_t index;

    if (retained_digits >= 0 &&
        retained_digits < (int32_t)decimal->digit_count_04) {
        uint8_t first_discarded = decimal->digits_05[retained_digits];

        if (first_discarded == (uint8_t)'5') {
            index = (int32_t)decimal->digit_count_04;
            while (index > retained_digits + 1 &&
                   decimal->digits_05[index - 1] == (uint8_t)'0') {
                --index;
            }
            if (index == retained_digits + 1) {
                round_up = retained_digits != 0 &&
                    (decimal->digits_05[retained_digits - 1] & 1U) != 0U;
            } else {
                round_up = true;
            }
        } else {
            round_up = first_discarded > (uint8_t)'5';
        }

        index = retained_digits;
        while (index != 0) {
            uint8_t digit;

            --index;
            digit = (uint8_t)(decimal->digits_05[index] - '0' +
                              (round_up ? 1 : 0));
            round_up = digit > 9U;
            decimal->digits_05[index] = (uint8_t)('0' + digit % 10U);
            if (!round_up && digit != 0U) {
                break;
            }
        }
        if (round_up) {
            decimal->digit_count_04 = 1;
            decimal->digits_05[0] = (uint8_t)'1';
            ++decimal->exponent_02;
            return;
        }
        if (retained_digits != 0) {
            decimal->digit_count_04 = (uint8_t)retained_digits;
            return;
        }
    }

    decimal->digit_count_04 = 1;
    decimal->digits_05[0] = (uint8_t)'0';
    decimal->exponent_02 = 0;
}

static uint32_t Game_BuildExponentSuffix(
    char *text,
    uint32_t length,
    int32_t exponent,
    uint8_t conversion)
{
    text[length++] = (char)conversion;
    if (exponent < 0) {
        text[length++] = '-';
        exponent = -exponent;
    } else {
        text[length++] = '+';
    }
    Game_AppendUnsignedDecimal(text, &length, (uint32_t)exponent, 2);
    return length;
}

static uint32_t Game_BuildScientificDecimal(
    char *text,
    const GameFormatDecimal *decimal,
    uint32_t length,
    int32_t precision,
    uint8_t conversion,
    bool alternate)
{
    int32_t index;

    text[length++] = (char)decimal->digits_05[0];
    if (precision != 0 || alternate) {
        text[length++] = '.';
    }
    for (index = 0; index < precision; ++index) {
        text[length++] = index + 1 < decimal->digit_count_04 ?
            (char)decimal->digits_05[index + 1] : '0';
    }
    return Game_BuildExponentSuffix(
        text,
        length,
        decimal->exponent_02,
        conversion
    );
}

static uint32_t Game_BuildFixedDecimal(
    char *text,
    const GameFormatDecimal *decimal,
    uint32_t length,
    int32_t precision,
    bool alternate)
{
    int32_t integer_digits = (int32_t)decimal->exponent_02 + 1;
    int32_t position;

    if (integer_digits <= 0) {
        text[length++] = '0';
    } else {
        for (position = 0; position < integer_digits; ++position) {
            text[length++] = position < decimal->digit_count_04 ?
                (char)decimal->digits_05[position] : '0';
        }
    }
    if (precision != 0 || alternate) {
        text[length++] = '.';
    }
    for (position = 0; position < precision; ++position) {
        int32_t digit_index = integer_digits + position;

        text[length++] = digit_index >= 0 &&
            digit_index < decimal->digit_count_04 ?
                (char)decimal->digits_05[digit_index] : '0';
    }
    return length;
}

/*
 * 0x02006F28..0x0200760B (1764 bytes).
 * Converts the runtime decimal record to %e/%E, %f/%F or %g/%G, applies the
 * dedicated half-even rounder, strips redundant source zeroes, handles all
 * signed INF/NAN spellings and emits the recovered sign/alternate modes.  Its
 * eleven-word ctype/string/divisor pool at 0x0200760C is excluded.
 */
char *Game_FormatDecimalFloat_02006f28(
    uint32_t value_low,
    uint32_t value_high,
    char *buffer_end,
    uint32_t flags_word,
    uint32_t type_word,
    int32_t width,
    int32_t precision)
{
    GameFormatDecimal decimal;
    uint32_t workspace = 0;
    char text[GAME_FORMAT_WORK_BUFFER_SIZE];
    uint8_t conversion = Game_FormatFloatByte(type_word, 1);
    uint8_t sign_mode = Game_FormatFloatByte(flags_word, 1);
    bool alternate = Game_FormatFloatByte(flags_word, 3) != 0;
    uint32_t length;
    int32_t retained_digits;
    int32_t output_precision = precision;

    (void)width;
    if (precision < 0 || precision > GAME_FORMAT_MAX_FIELD) {
        return NULL;
    }
    length = Game_FormatSpecialFloat(
        text,
        value_low,
        value_high,
        conversion
    );
    if (length != 0U) {
        return Game_CopyFormatResultBackward(buffer_end, text, length);
    }

    Game_ConvertDoubleToDecimal_0200acf8(
        &workspace,
        value_low,
        value_high,
        &decimal
    );
    while (decimal.digit_count_04 > 1U &&
           decimal.digits_05[decimal.digit_count_04 - 1U] == (uint8_t)'0') {
        --decimal.digit_count_04;
        ++decimal.exponent_02;
    }
    if (decimal.digits_05[0] == (uint8_t)'0') {
        decimal.exponent_02 = 0;
    } else {
        decimal.exponent_02 = (int16_t)(
            decimal.exponent_02 + decimal.digit_count_04 - 1
        );
    }

    if (conversion == (uint8_t)'g' || conversion == (uint8_t)'G') {
        if (decimal.exponent_02 < -4 || decimal.exponent_02 >= precision) {
            conversion = conversion == (uint8_t)'g' ? (uint8_t)'e' :
                (uint8_t)'E';
            output_precision = precision - 1;
            retained_digits = precision;
        } else {
            conversion = conversion == (uint8_t)'g' ? (uint8_t)'f' :
                (uint8_t)'F';
            output_precision = precision - (decimal.exponent_02 + 1);
            if (output_precision < 0) {
                output_precision = 0;
            }
            retained_digits = decimal.exponent_02 + 1 + output_precision;
        }
    } else if (conversion == (uint8_t)'e' ||
               conversion == (uint8_t)'E') {
        retained_digits = precision + 1;
    } else {
        retained_digits = decimal.exponent_02 + 1 + precision;
    }
    if (retained_digits < (int32_t)decimal.digit_count_04) {
        Game_RoundDecimalDigits_02006e14(&decimal, retained_digits);
    }

    length = 0;
    if (decimal.negative_00 != 0U) {
        text[length++] = '-';
    } else if (sign_mode == GAME_FORMAT_SIGN_PLUS) {
        text[length++] = '+';
    } else if (sign_mode == GAME_FORMAT_SIGN_SPACE) {
        text[length++] = ' ';
    }
    if (conversion == (uint8_t)'e' || conversion == (uint8_t)'E') {
        length = Game_BuildScientificDecimal(
            text,
            &decimal,
            length,
            output_precision,
            conversion,
            alternate
        );
    } else {
        length = Game_BuildFixedDecimal(
            text,
            &decimal,
            length,
            output_precision,
            alternate
        );
    }
    return Game_CopyFormatResultBackward(buffer_end, text, length);
}
