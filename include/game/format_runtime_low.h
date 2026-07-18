#ifndef FSAE_GAME_FORMAT_RUNTIME_LOW_H
#define FSAE_GAME_FORMAT_RUNTIME_LOW_H

#include "game/log_format.h"

#include <stdarg.h>
#include <stdint.h>

enum {
    GAME_FORMAT_PADDING_LEFT = 0,
    GAME_FORMAT_PADDING_SPACE = 1,
    GAME_FORMAT_PADDING_ZERO = 2,
    GAME_FORMAT_SIGN_NONE = 0,
    GAME_FORMAT_SIGN_PLUS = 1,
    GAME_FORMAT_SIGN_SPACE = 2,
    GAME_FORMAT_LENGTH_DEFAULT = 0,
    GAME_FORMAT_LENGTH_CHAR = 1,
    GAME_FORMAT_LENGTH_SHORT = 2,
    GAME_FORMAT_LENGTH_LONG = 3,
    GAME_FORMAT_LENGTH_LONG_LONG = 4,
    GAME_FORMAT_LENGTH_WIDE = 5,
    GAME_FORMAT_LENGTH_INTMAX = 6,
    GAME_FORMAT_LENGTH_SIZE = 7,
    GAME_FORMAT_LENGTH_PTRDIFF = 8,
    GAME_FORMAT_LENGTH_LONG_DOUBLE = 9,
    GAME_FORMAT_MAX_FIELD = 509,
    GAME_FORMAT_WORK_BUFFER_SIZE = 512,
    GAME_FORMAT_DECIMAL_DIGITS = 35
};

/* Exact 16-byte descriptor written by 0x02005F38. */
typedef struct GameFormatConversion {
    uint8_t padding_00;
    uint8_t sign_01;
    uint8_t precision_present_02;
    uint8_t alternate_03;
    uint8_t length_04;
    uint8_t conversion_05;
    uint8_t reserved_06[2];
    uint32_t width_08;
    uint32_t precision_0c;
} GameFormatConversion;

/* Decimal record consumed and rounded by 0x02006E14. */
typedef struct GameFormatDecimal {
    uint8_t negative_00;
    uint8_t reserved_01;
    int16_t exponent_02;
    uint8_t digit_count_04;
    uint8_t digits_05[GAME_FORMAT_DECIMAL_DIGITS];
} GameFormatDecimal;

_Static_assert(sizeof(GameFormatConversion) == 0x10,
               "format conversion ABI size");
_Static_assert(sizeof(GameFormatDecimal) == 0x28,
               "format decimal ABI size");

/* 0x02005F38..0x0200643B; two-word pool follows. */
const char *Game_ParseFormatConversion_02005f38(
    const char *percent,
    va_list *arguments,
    GameFormatConversion *conversion
);

/* 0x02006444..0x0200668F; one-word pool follows. */
char *Game_FormatInteger32_02006444(
    int32_t value,
    char *buffer_end,
    uint32_t flags_word,
    uint32_t type_word,
    int32_t width,
    int32_t precision
);

/* 0x02006694..0x02006963; one-word pool follows. */
char *Game_FormatInteger64_02006694(
    uint32_t value_low,
    uint32_t value_high,
    char *buffer_end,
    uint32_t flags_word,
    uint32_t type_word,
    int32_t width,
    int32_t precision
);

/* 0x02006968..0x02006DEB; ten-word pool follows. */
char *Game_FormatHexFloat_02006968(
    uint32_t value_low,
    uint32_t value_high,
    char *buffer_end,
    uint32_t flags_word,
    uint32_t type_word,
    int32_t width,
    int32_t precision
);

/* 0x02006E14..0x02006F27; no pool. */
void Game_RoundDecimalDigits_02006e14(
    GameFormatDecimal *decimal,
    int32_t retained_digits
);

/* 0x02006F28..0x0200760B; eleven-word pool follows. */
char *Game_FormatDecimalFloat_02006f28(
    uint32_t value_low,
    uint32_t value_high,
    char *buffer_end,
    uint32_t flags_word,
    uint32_t type_word,
    int32_t width,
    int32_t precision
);

/* 0x02007638..0x02007EDF; two-word pool follows. */
int Game_RunFormatter(
    GameFormatWriteCallback callback,
    void *context,
    const char *format,
    va_list arguments,
    int mode
);

/* 0x02007EE8..0x02007F13; no pool. */
int Game_LogStreamWrite(
    void *context,
    const void *data,
    uint32_t byte_count
);

#endif /* FSAE_GAME_FORMAT_RUNTIME_LOW_H */
