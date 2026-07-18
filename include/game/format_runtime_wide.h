#ifndef FSAE_GAME_FORMAT_RUNTIME_WIDE_H
#define FSAE_GAME_FORMAT_RUNTIME_WIDE_H

#include "game/format_runtime_low.h"

#include <stdarg.h>
#include <stdint.h>

typedef int (*GameWideFormatWriteCallback)(
    void *context,
    const uint16_t *data,
    uint32_t unit_count
);

typedef struct GameWideFormatBuffer {
    uint16_t *buffer;
    uint32_t capacity;
    uint32_t length;
} GameWideFormatBuffer;

/* Numeric, base-ten record used by 0x0200A41C..0x0200A75B. */
typedef struct GameRuntimeDecimal {
    uint8_t negative_00;
    uint8_t reserved_01;
    int16_t exponent_02;
    uint8_t digit_count_04;
    uint8_t digits_05[32];
} GameRuntimeDecimal;

#if UINTPTR_MAX == UINT32_MAX
_Static_assert(sizeof(GameWideFormatBuffer) == 0x0c,
               "wide format buffer ABI size");
#endif

int Game_VSNPrintf_02008080(
    char *output,
    uint32_t capacity,
    const char *format,
    va_list arguments
);
int Game_SPrintf_020080e4(char *output, const char *format, ...);
void Game_RaiseFormatConstraint_02008110(void);
void Game_DefaultFormatConstraintHandler_02008138(void);
int Game_RaiseRuntimeSignal(int signal_number);

uint32_t Game_StringLength_02008264(const char *string);
void Game_CopyString_02008280(char *destination, const char *source);
void Game_CopyStringBounded_02008348(
    char *destination,
    const char *source,
    uint32_t count
);
char *Game_FindCharacter_02008398(const char *string, int character);
int Game_LogPrepareStream(void *stream, int orientation);

void Game_CopyWideUnits_02008440(
    uint16_t *destination,
    const uint16_t *source,
    uint32_t count
);
uint16_t *Game_FindWideUnit_02008450(
    const uint16_t *string,
    uint16_t value,
    uint32_t count
);
const uint16_t *Game_ParseWideFormatConversion_02008478(
    const uint16_t *percent,
    va_list *arguments,
    GameFormatConversion *conversion
);
uint16_t *Game_FormatWideInteger32_02008990(
    int32_t value,
    uint16_t *buffer_end,
    uint32_t flags_word,
    uint32_t type_word,
    int32_t width,
    int32_t precision
);
uint16_t *Game_FormatWideInteger64_02008bdc(
    uint32_t value_low,
    uint32_t value_high,
    uint16_t *buffer_end,
    uint32_t flags_word,
    uint32_t type_word,
    int32_t width,
    int32_t precision
);
uint16_t *Game_FormatWideHexFloat_02008eb4(
    uint32_t value_low,
    uint32_t value_high,
    uint16_t *buffer_end,
    uint32_t flags_word,
    uint32_t type_word,
    int32_t width,
    int32_t precision
);
void Game_RoundWideDecimalDigits_02009274(
    GameFormatDecimal *decimal,
    int32_t retained_digits
);
uint16_t *Game_FormatWideDecimalFloat_02009388(
    uint32_t value_low,
    uint32_t value_high,
    uint16_t *buffer_end,
    uint32_t flags_word,
    uint32_t type_word,
    int32_t width,
    int32_t precision
);
int Game_RunWideFormatter_02009980(
    GameWideFormatWriteCallback callback,
    void *context,
    const uint16_t *format,
    va_list arguments,
    int mode
);
int GameWideFormatBuffer_Write_0200a2e4(
    GameWideFormatBuffer *output,
    const uint16_t *source,
    uint32_t unit_count
);
int NtmvM2d_FormatWideText_0200a324(
    uint16_t *output,
    uint32_t capacity,
    const uint16_t *format,
    ...
);
int Game_VSNPrintfWide_0200a34c(
    uint16_t *output,
    uint32_t capacity,
    const uint16_t *format,
    va_list arguments
);

uint32_t Game_WideStringLength_0200a3ac(const uint16_t *string);
void Game_CopyWideString_0200a3c8(
    uint16_t *destination,
    const uint16_t *source
);
uint16_t *Game_FindWideCharacter_0200a3e8(
    const uint16_t *string,
    uint16_t character
);
int Game_CompareDecimalRoundingHalf_0200a41c(
    const GameRuntimeDecimal *decimal,
    int32_t position
);
void Game_IncrementRuntimeDecimal_0200a488(
    GameRuntimeDecimal *decimal,
    int32_t position
);
void Game_RoundRuntimeDecimal_0200a4d8(
    GameRuntimeDecimal *decimal,
    int32_t retained_digits
);
void Game_RuntimeDecimalFromUint64_0200a518(
    GameRuntimeDecimal *decimal,
    uint32_t value_low,
    uint32_t value_high
);
void Game_MultiplyRuntimeDecimals_0200a5dc(
    GameRuntimeDecimal *product,
    const GameRuntimeDecimal *left,
    const GameRuntimeDecimal *right
);

#endif /* FSAE_GAME_FORMAT_RUNTIME_WIDE_H */
