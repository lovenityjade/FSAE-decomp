#include "game/runtime_support.h"

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

typedef union GameDoubleBits {
    double value;
    uint64_t bits;
} GameDoubleBits;

typedef union GameFloatBits {
    float value;
    uint32_t bits;
} GameFloatBits;

static uint64_t Game_CombineRuntimeWords(uint32_t low, uint32_t high)
{
    return ((uint64_t)high << 32) | low;
}

static GameDoubleBits Game_RuntimeDouble(uint32_t low, uint32_t high)
{
    GameDoubleBits result;

    result.bits = Game_CombineRuntimeWords(low, high);
    return result;
}

/* 0x0200CAFC..0x0200CE2B.  The catalogued arithmetic body has an 11-word
 * non-contiguous sign/NaN tail before the subtraction entry. */
uint64_t Game_AddDoubleBits_0200cafc(
    uint32_t left_low,
    uint32_t left_high,
    uint32_t right_low,
    uint32_t right_high
)
{
    GameDoubleBits left = Game_RuntimeDouble(left_low, left_high);
    GameDoubleBits right = Game_RuntimeDouble(right_low, right_high);
    GameDoubleBits result;

    result.value = left.value + right.value;
    return result.bits;
}

/* 0x0200CE2C..0x0200D1DF; infinity literal follows. */
uint64_t Game_SubtractDoubleBits_0200ce2c(
    uint32_t left_low,
    uint32_t left_high,
    uint32_t right_low,
    uint32_t right_high
)
{
    GameDoubleBits left = Game_RuntimeDouble(left_low, left_high);
    GameDoubleBits right = Game_RuntimeDouble(right_low, right_high);
    GameDoubleBits result;

    result.value = left.value - right.value;
    return result.bits;
}

/* 0x0200D1E0..0x0200D543; infinity literal follows. */
uint64_t Game_MultiplyDoubleBits_0200d1e0(
    uint32_t left_low,
    uint32_t left_high,
    uint32_t right_low,
    uint32_t right_high
)
{
    GameDoubleBits left = Game_RuntimeDouble(left_low, left_high);
    GameDoubleBits right = Game_RuntimeDouble(right_low, right_high);
    GameDoubleBits result;

    result.value = left.value * right.value;
    return result.bits;
}

/* 0x0200D544..0x0200D5DF. */
bool Game_DoubleBitsLessThan_0200d544(
    uint32_t left_low,
    uint32_t left_high,
    uint32_t right_low,
    uint32_t right_high
)
{
    GameDoubleBits left = Game_RuntimeDouble(left_low, left_high);
    GameDoubleBits right = Game_RuntimeDouble(right_low, right_high);

    return left.value < right.value;
}

/* 0x0200D5E0..0x0200D66B. */
bool Game_DoubleBitsEqual_0200d5e0(
    uint32_t left_low,
    uint32_t left_high,
    uint32_t right_low,
    uint32_t right_high
)
{
    GameDoubleBits left = Game_RuntimeDouble(left_low, left_high);
    GameDoubleBits right = Game_RuntimeDouble(right_low, right_high);

    return left.value == right.value;
}

/* 0x0200D66C..0x0200D6F7. */
bool Game_DoubleBitsNotEqual_0200d66c(
    uint32_t left_low,
    uint32_t left_high,
    uint32_t right_low,
    uint32_t right_high
)
{
    return !Game_DoubleBitsEqual_0200d5e0(
        left_low, left_high, right_low, right_high);
}

/* 0x0200D6F8..0x0200D8FF.  Only 72 bytes are attached to the catalogued
 * entry; the remaining extent contains shared integer-conversion/division
 * tails and alternate runtime entries not represented in the catalog. */
uint32_t Game_Int32ToFloatBits_0200d6f8(int32_t value)
{
    GameFloatBits result;

    result.value = (float)value;
    return result.bits;
}

/* 0x0200D900 is the full unsigned 64-bit quotient helper.  Its catalogued
 * body intentionally overlaps the alternate remainder entry at 0x0200D90C. */
uint64_t Game_DivideUint64_0200d900(
    uint32_t dividend_low,
    uint32_t dividend_high,
    uint32_t divisor_low,
    uint32_t divisor_high
)
{
    uint64_t dividend = Game_CombineRuntimeWords(
        dividend_low, dividend_high);
    uint64_t divisor = Game_CombineRuntimeWords(divisor_low, divisor_high);

    return divisor == 0U ? dividend : dividend / divisor;
}

/* 0x0200D90C..0x0200D947: alternate entry into the shared 64-bit division
 * engine, returning the remainder path. */
uint64_t Game_RemainderUint64_0200d90c(
    uint32_t dividend_low,
    uint32_t dividend_high,
    uint32_t divisor_low,
    uint32_t divisor_high
)
{
    uint64_t dividend = Game_CombineRuntimeWords(
        dividend_low, dividend_high);
    uint64_t divisor = Game_CombineRuntimeWords(divisor_low, divisor_high);

    return divisor == 0U ? dividend : dividend % divisor;
}

/* 0x0200D948..0x0200DB53.  The compact entry is followed by its unrolled
 * quotient/remainder instruction table through the next standalone entry. */
uint64_t Game_DivideInt32WithRemainder_0200d948(
    int32_t dividend,
    int32_t divisor
)
{
    int32_t quotient;
    int32_t remainder;

    if (divisor == 0) {
        quotient = dividend;
        remainder = 0;
    } else {
        int64_t wide_dividend = dividend;
        int64_t wide_divisor = divisor;

        quotient = (int32_t)(wide_dividend / wide_divisor);
        remainder = (int32_t)(wide_dividend % wide_divisor);
    }
    return ((uint64_t)(uint32_t)remainder << 32) | (uint32_t)quotient;
}

/* 0x0200DB54..0x0200DB5B: checked entry into the shared 32-bit divider. */
uint32_t Game_DivideUint32Checked_0200db54(
    uint32_t dividend,
    uint32_t divisor
)
{
    return divisor == 0U ? dividend : Game_DivideUint32_0200db5c(
        dividend, divisor);
}

/* 0x0200DB5C..0x0200DD37.  The 80-byte dispatch entry is followed by its
 * unrolled non-restoring division instruction table. */
uint32_t Game_DivideUint32_0200db5c(uint32_t dividend, uint32_t divisor)
{
    return divisor == 0U ? dividend : dividend / divisor;
}

/* 0x0200DD38..0x0200DD77. */
uint32_t Game_FloatBitsToUint32_0200dd38(uint32_t value_bits)
{
    GameFloatBits input;

    input.bits = value_bits;
    if (!(input.value > 0.0f)) {
        return 0;
    }
    if (input.value >= (float)UINT32_MAX) {
        return UINT32_MAX;
    }
    return (uint32_t)input.value;
}

/* 0x0200DD78..0x0200DE03; maximum accepted exponent literal follows. */
uint64_t Game_DoubleBitsToUint64_0200dd78(
    uint32_t value_low,
    uint32_t value_high
)
{
    GameDoubleBits input = Game_RuntimeDouble(value_low, value_high);

    if (!(input.value > 0.0)) {
        return 0;
    }
    if (input.value >= 18446744073709551615.0) {
        return UINT64_MAX;
    }
    return (uint64_t)input.value;
}
