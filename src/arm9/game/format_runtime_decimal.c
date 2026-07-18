#include "game/format_runtime_wide.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Public destination after checkpoint release:
 * src/arm9/game/format_runtime_decimal.c */

/* 0x0200A3AC..0x0200A3C7. */
uint32_t Game_WideStringLength_0200a3ac(const uint16_t *string)
{
    const uint16_t *cursor = string;

    while (*cursor != 0) {
        ++cursor;
    }
    return (uint32_t)(cursor - string);
}

/* 0x0200A3C8..0x0200A3E7. */
void Game_CopyWideString_0200a3c8(
    uint16_t *destination,
    const uint16_t *source
)
{
    while ((*destination++ = *source++) != 0) {
    }
}

/* 0x0200A3E8..0x0200A41B. */
uint16_t *Game_FindWideCharacter_0200a3e8(
    const uint16_t *string,
    uint16_t character
)
{
    for (;;) {
        if (*string == character) {
            return (uint16_t *)string;
        }
        if (*string++ == 0) {
            return NULL;
        }
    }
}

/* 0x0200A41C..0x0200A487.  Returns the direction used by the SDK's
 * round-to-nearest, ties-to-even policy for digits discarded at position. */
int Game_CompareDecimalRoundingHalf_0200a41c(
    const GameRuntimeDecimal *decimal,
    int32_t position
)
{
    uint8_t digit;
    int32_t index;

    if (position < 0 || position >= decimal->digit_count_04) {
        return -1;
    }
    digit = decimal->digits_05[position];
    if (digit > 5U) {
        return 1;
    }
    if (digit < 5U) {
        return -1;
    }

    for (index = position + 1; index < decimal->digit_count_04; ++index) {
        if (decimal->digits_05[index] != 0U) {
            return 1;
        }
    }
    if (position != 0 && (decimal->digits_05[position - 1] & 1U) != 0U) {
        return 1;
    }
    return -1;
}

/* 0x0200A488..0x0200A4D7. */
void Game_IncrementRuntimeDecimal_0200a488(
    GameRuntimeDecimal *decimal,
    int32_t position
)
{
    while (position >= 0) {
        if (++decimal->digits_05[position] < 10U) {
            return;
        }
        decimal->digits_05[position] = 0;
        --position;
    }

    decimal->digits_05[0] = 1;
    ++decimal->exponent_02;
}

/* 0x0200A4D8..0x0200A517. */
void Game_RoundRuntimeDecimal_0200a4d8(
    GameRuntimeDecimal *decimal,
    int32_t retained_digits
)
{
    int direction;

    if (retained_digits < 0) {
        retained_digits = 0;
    }
    if (retained_digits >= decimal->digit_count_04) {
        return;
    }

    direction = Game_CompareDecimalRoundingHalf_0200a41c(
        decimal, retained_digits);
    decimal->digit_count_04 = (uint8_t)retained_digits;
    if (direction > 0) {
        if (retained_digits == 0) {
            decimal->digits_05[0] = 1;
            decimal->digit_count_04 = 1;
            ++decimal->exponent_02;
        } else {
            Game_IncrementRuntimeDecimal_0200a488(
                decimal, retained_digits - 1);
        }
    }
}

/* 0x0200A518..0x0200A5DB. */
void Game_RuntimeDecimalFromUint64_0200a518(
    GameRuntimeDecimal *decimal,
    uint32_t value_low,
    uint32_t value_high
)
{
    uint64_t value = ((uint64_t)value_high << 32) | value_low;
    uint32_t count = 0;
    uint32_t index;

    decimal->negative_00 = 0;
    decimal->reserved_01 = 0;
    while (value != 0U) {
        decimal->digits_05[count++] = (uint8_t)(value % 10U);
        value /= 10U;
    }
    decimal->digit_count_04 = (uint8_t)count;
    decimal->exponent_02 = (int16_t)((int32_t)count - 1);

    for (index = 0; index < count / 2U; ++index) {
        uint8_t temporary = decimal->digits_05[index];

        decimal->digits_05[index] = decimal->digits_05[count - 1U - index];
        decimal->digits_05[count - 1U - index] = temporary;
    }
}

/* 0x0200A5DC..0x0200A75B; reciprocal constant 0xCCCCCCCD follows. */
void Game_MultiplyRuntimeDecimals_0200a5dc(
    GameRuntimeDecimal *product,
    const GameRuntimeDecimal *left,
    const GameRuntimeDecimal *right
)
{
    uint32_t accumulator[64] = {0};
    uint8_t digits[64];
    uint32_t left_count = left->digit_count_04;
    uint32_t right_count = right->digit_count_04;
    uint32_t extent = left_count + right_count;
    uint32_t first = 0;
    uint32_t significant;
    uint32_t retained;
    uint32_t index;
    uint32_t other;
    bool round_up = false;

    product->negative_00 = 0;
    product->reserved_01 = 0;
    product->exponent_02 = (int16_t)(
        left->exponent_02 + right->exponent_02);

    if (left_count == 0U || right_count == 0U) {
        product->digit_count_04 = 0;
        return;
    }
    for (index = 0; index < left_count; ++index) {
        for (other = 0; other < right_count; ++other) {
            accumulator[index + other + 1U] +=
                (uint32_t)left->digits_05[index] *
                right->digits_05[other];
        }
    }
    for (index = extent; index > 0U; --index) {
        accumulator[index - 1U] += accumulator[index] / 10U;
        digits[index] = (uint8_t)(accumulator[index] % 10U);
    }
    digits[0] = (uint8_t)accumulator[0];

    if (digits[0] == 0U) {
        first = 1;
    } else {
        ++product->exponent_02;
    }
    significant = extent + 1U - first;
    retained = significant < 32U ? significant : 32U;
    product->digit_count_04 = (uint8_t)retained;
    for (index = 0; index < retained; ++index) {
        product->digits_05[index] = digits[first + index];
    }

    if (significant > retained) {
        uint8_t first_discarded = digits[first + retained];

        if (first_discarded > 5U) {
            round_up = true;
        } else if (first_discarded == 5U) {
            for (index = first + retained + 1U;
                 index <= extent;
                 ++index) {
                if (digits[index] != 0U) {
                    round_up = true;
                    break;
                }
            }
            if (!round_up &&
                (product->digits_05[retained - 1U] & 1U) != 0U) {
                round_up = true;
            }
        }
    }
    if (round_up) {
        Game_IncrementRuntimeDecimal_0200a488(
            product, (int32_t)retained - 1);
    }
}
