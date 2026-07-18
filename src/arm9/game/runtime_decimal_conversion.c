#include "game/runtime_unwind.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static uint64_t Game_CombineDoubleWords(uint32_t low, uint32_t high)
{
    return ((uint64_t)high << 32) | low;
}

/* 0x0200A75C..0x0200A7F7. */
void Game_RuntimeDecimalFromDigitString_0200a75c(
    GameRuntimeDecimal *decimal,
    const uint8_t *digits,
    int16_t exponent
)
{
    uint32_t count = 0;
    const uint8_t *discarded;

    decimal->negative_00 = 0;
    decimal->reserved_01 = 0;
    decimal->exponent_02 = exponent;
    while (count < 32U && *digits != 0U) {
        decimal->digits_05[count++] = (uint8_t)(*digits++ - (uint8_t)'0');
    }
    decimal->digit_count_04 = (uint8_t)count;
    if (*digits == 0U || *digits < 5U) {
        return;
    }
    if (*digits == 5U) {
        discarded = digits + 1;
        while (*discarded != 0U) {
            if (*discarded != (uint8_t)'0') {
                Game_IncrementRuntimeDecimal_0200a488(decimal, (int32_t)count - 1);
                return;
            }
            ++discarded;
        }
        if (count == 0U || (decimal->digits_05[count - 1U] & 1U) == 0U) {
            return;
        }
    }
    Game_IncrementRuntimeDecimal_0200a488(decimal, (int32_t)count - 1);
}

static void Game_SetSmallRuntimeDecimal(
    GameRuntimeDecimal *decimal,
    uint8_t digit,
    int16_t exponent
)
{
    decimal->negative_00 = 0;
    decimal->reserved_01 = 0;
    decimal->exponent_02 = exponent;
    decimal->digit_count_04 = 1;
    decimal->digits_05[0] = digit;
}

/* 0x0200A7F8..0x0200AB77; the 21-entry decimal-power string table follows. */
void Game_RuntimeDecimalPowerOfTwo_0200a7f8(
    GameRuntimeDecimal *decimal,
    int32_t exponent
)
{
    GameRuntimeDecimal half;
    GameRuntimeDecimal power;
    GameRuntimeDecimal factor;

    if (exponent == 0) {
        Game_SetSmallRuntimeDecimal(decimal, 1, 0);
        return;
    }
    if (exponent == 1) {
        Game_SetSmallRuntimeDecimal(decimal, 2, 0);
        return;
    }
    if (exponent == -1) {
        Game_SetSmallRuntimeDecimal(decimal, 5, -1);
        return;
    }

    Game_RuntimeDecimalPowerOfTwo_0200a7f8(&half, exponent / 2);
    Game_MultiplyRuntimeDecimals_0200a5dc(&power, &half, &half);
    if ((exponent & 1) == 0) {
        *decimal = power;
        return;
    }
    if (exponent < 0) {
        Game_SetSmallRuntimeDecimal(&factor, 5, -1);
    } else {
        Game_SetSmallRuntimeDecimal(&factor, 2, 0);
    }
    Game_MultiplyRuntimeDecimals_0200a5dc(decimal, &power, &factor);
}

/* 0x0200AB78..0x0200ACF7. */
void Game_RuntimeDecimalFromDouble_0200ab78(
    GameRuntimeDecimal *decimal,
    uint32_t value_low,
    uint32_t value_high
)
{
    uint64_t bits = Game_CombineDoubleWords(value_low, value_high);
    uint64_t fraction = bits & UINT64_C(0x000fffffffffffff);
    uint32_t encoded_exponent = (uint32_t)(bits >> 52) & 0x7ffU;
    bool negative = (bits >> 63) != 0U;
    GameRuntimeDecimal magnitude;
    GameRuntimeDecimal power;
    int32_t binary_exponent;

    if (encoded_exponent == 0x7ffU) {
        decimal->negative_00 = (uint8_t)negative;
        decimal->reserved_01 = 0;
        decimal->exponent_02 = 0;
        decimal->digit_count_04 = 1;
        decimal->digits_05[0] = fraction != 0U ? (uint8_t)'N' : (uint8_t)'I';
        return;
    }
    if (encoded_exponent == 0U && fraction == 0U) {
        decimal->negative_00 = (uint8_t)negative;
        decimal->reserved_01 = 0;
        decimal->exponent_02 = 0;
        decimal->digit_count_04 = 1;
        decimal->digits_05[0] = 0;
        return;
    }

    if (encoded_exponent == 0U) {
        binary_exponent = -1074;
    } else {
        fraction |= UINT64_C(0x0010000000000000);
        binary_exponent = (int32_t)encoded_exponent - 1075;
    }
    Game_RuntimeDecimalFromUint64_0200a518(
        &magnitude, (uint32_t)fraction, (uint32_t)(fraction >> 32));
    Game_RuntimeDecimalPowerOfTwo_0200a7f8(&power, binary_exponent);
    Game_MultiplyRuntimeDecimals_0200a5dc(decimal, &magnitude, &power);
    decimal->negative_00 = (uint8_t)negative;
}

/* 0x0200ACF8..0x0200ADA3. */
void Game_ConvertDoubleToDecimal_0200acf8(
    const void *conversion_spec,
    uint32_t value_low,
    uint32_t value_high,
    GameFormatDecimal *output
)
{
    const uint8_t *specification = (const uint8_t *)conversion_spec;
    int32_t retained_digits = *(const int16_t *)(specification + 2);
    GameRuntimeDecimal decimal;
    uint32_t index;

    Game_RuntimeDecimalFromDouble_0200ab78(
        &decimal, value_low, value_high);
    if (decimal.digits_05[0] > 9U) {
        output->negative_00 = decimal.negative_00;
        output->reserved_01 = decimal.reserved_01;
        output->exponent_02 = decimal.exponent_02;
        output->digit_count_04 = decimal.digit_count_04;
        output->digits_05[0] = decimal.digits_05[0];
        return;
    }
    if (retained_digits > 32) {
        retained_digits = 32;
    }
    Game_RoundRuntimeDecimal_0200a4d8(&decimal, retained_digits);
    while (decimal.digit_count_04 < retained_digits) {
        decimal.digits_05[decimal.digit_count_04++] = 0;
    }
    decimal.exponent_02 = (int16_t)(decimal.exponent_02 -
        ((int32_t)decimal.digit_count_04 - 1));
    for (index = 0; index < decimal.digit_count_04; ++index) {
        decimal.digits_05[index] += (uint8_t)'0';
    }
    output->negative_00 = decimal.negative_00;
    output->reserved_01 = decimal.reserved_01;
    output->exponent_02 = decimal.exponent_02;
    output->digit_count_04 = decimal.digit_count_04;
    for (index = 0; index < decimal.digit_count_04; ++index) {
        output->digits_05[index] = decimal.digits_05[index];
    }
}

/* 0x0200ADA4..0x0200ADCB: copysign-shaped bit operation. */
uint64_t Game_CopyDoubleSign_0200ada4(
    uint32_t magnitude_low,
    uint32_t magnitude_high,
    uint32_t sign_low,
    uint32_t sign_high
)
{
    (void)sign_low;
    magnitude_high = (magnitude_high & UINT32_C(0x7fffffff)) |
        (sign_high & UINT32_C(0x80000000));
    return Game_CombineDoubleWords(magnitude_low, magnitude_high);
}

/* 0x0200ADCC..0x0200ADEB: fabs-shaped bit operation. */
uint64_t Game_AbsoluteDouble_0200adcc(uint32_t value_low, uint32_t value_high)
{
    return Game_CombineDoubleWords(
        value_low, value_high & UINT32_C(0x7fffffff));
}

/* 0x0200ADEC..0x0200AEAB; four IEEE-754 constants follow. */
uint64_t Game_NormalizeDoubleFraction_0200adec(
    uint32_t value_low,
    uint32_t value_high,
    int32_t *exponent
)
{
    uint64_t bits = Game_CombineDoubleWords(value_low, value_high);
    uint64_t sign = bits & UINT64_C(0x8000000000000000);
    uint64_t magnitude = bits & UINT64_C(0x7fffffffffffffff);
    uint32_t encoded = (uint32_t)(magnitude >> 52);

    *exponent = 0;
    if (encoded >= 0x7ffU || magnitude == 0U) {
        return bits;
    }
    if (encoded == 0U) {
        do {
            magnitude <<= 1;
            --*exponent;
        } while ((magnitude & UINT64_C(0x0010000000000000)) == 0U);
        encoded = 1;
    }
    *exponent += (int32_t)encoded - 1022;
    return sign | (magnitude & UINT64_C(0x000fffffffffffff)) |
        UINT64_C(0x3fe0000000000000);
}

/* 0x0200AEAC..0x0200B10B; eleven scaling constants follow. */
uint64_t Game_ScaleDoubleByPowerOfTwo_0200aeac(
    uint32_t value_low,
    uint32_t value_high,
    int32_t exponent
)
{
    uint64_t bits = Game_CombineDoubleWords(value_low, value_high);
    uint64_t sign = bits & UINT64_C(0x8000000000000000);
    uint64_t fraction = bits & UINT64_C(0x000fffffffffffff);
    int32_t encoded = (int32_t)((bits >> 52) & UINT64_C(0x7ff));

    if (encoded == 0x7ff || (encoded == 0 && fraction == 0U)) {
        return bits;
    }
    if (encoded == 0) {
        do {
            fraction <<= 1;
            --exponent;
        } while ((fraction & UINT64_C(0x0010000000000000)) == 0U);
        fraction &= UINT64_C(0x000fffffffffffff);
        encoded = 1;
    }
    encoded += exponent;
    if (encoded >= 0x7ff) {
        return sign | UINT64_C(0x7ff0000000000000);
    }
    if (encoded > 0) {
        return sign | ((uint64_t)encoded << 52) | fraction;
    }
    if (encoded <= -52) {
        return sign;
    }
    fraction |= UINT64_C(0x0010000000000000);
    return sign | (fraction >> (1 - encoded));
}
