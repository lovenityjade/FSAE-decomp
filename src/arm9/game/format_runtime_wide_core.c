#include "game/format_runtime_wide.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static uint32_t Game_WideBoundedLength(
    const uint16_t *string,
    uint32_t limit
)
{
    uint32_t length = 0;

    while (length < limit && string[length] != 0) {
        ++length;
    }
    return length;
}

static const uint16_t *Game_FindNextWidePercent(const uint16_t *format)
{
    while (*format != 0 && *format != (uint16_t)'%') {
        ++format;
    }
    return format;
}

static uint32_t Game_PackWideFlags(const GameFormatConversion *conversion)
{
    return (uint32_t)conversion->padding_00 |
        ((uint32_t)conversion->sign_01 << 8) |
        ((uint32_t)conversion->precision_present_02 << 16) |
        ((uint32_t)conversion->alternate_03 << 24);
}

static uint32_t Game_PackWideType(const GameFormatConversion *conversion)
{
    return (uint32_t)conversion->length_04 |
        ((uint32_t)conversion->conversion_05 << 16);
}

static bool Game_WriteWideUnits(
    GameWideFormatWriteCallback callback,
    void *context,
    const uint16_t *data,
    uint32_t length
)
{
    return length == 0U || callback(context, data, length) != 0;
}

static bool Game_WriteWidePadding(
    GameWideFormatWriteCallback callback,
    void *context,
    uint16_t value,
    uint32_t count
)
{
    while (count != 0U) {
        if (!Game_WriteWideUnits(callback, context, &value, 1)) {
            return false;
        }
        --count;
    }
    return true;
}

static bool Game_WritePaddedWideField(
    GameWideFormatWriteCallback callback,
    void *context,
    const GameFormatConversion *conversion,
    const uint16_t *text,
    uint32_t length,
    int *total
)
{
    uint32_t padding = conversion->width_08 > length ?
        conversion->width_08 - length : 0;
    uint32_t prefix = 0;
    uint16_t padding_character = conversion->padding_00 ==
        GAME_FORMAT_PADDING_ZERO ? (uint16_t)'0' : (uint16_t)' ';

    if (conversion->padding_00 != GAME_FORMAT_PADDING_LEFT) {
        if (padding_character == (uint16_t)'0' && length != 0U) {
            if (text[0] == (uint16_t)'+' || text[0] == (uint16_t)'-' ||
                text[0] == (uint16_t)' ') {
                prefix = 1;
            }
            if ((conversion->conversion_05 == (uint8_t)'a' ||
                 conversion->conversion_05 == (uint8_t)'A') &&
                length >= prefix + 2U && text[prefix] == (uint16_t)'0' &&
                (text[prefix + 1U] == (uint16_t)'x' ||
                 text[prefix + 1U] == (uint16_t)'X')) {
                prefix += 2U;
            }
            if (!Game_WriteWideUnits(callback, context, text, prefix)) {
                return false;
            }
            text += prefix;
            length -= prefix;
        }
        if (!Game_WriteWidePadding(
                callback, context, padding_character, padding)) {
            return false;
        }
    }
    if (!Game_WriteWideUnits(callback, context, text, length)) {
        return false;
    }
    if (conversion->padding_00 == GAME_FORMAT_PADDING_LEFT &&
        !Game_WriteWidePadding(
            callback, context, (uint16_t)' ', padding)) {
        return false;
    }
    *total += (int)(length + prefix + padding);
    return true;
}

static int64_t Game_ReadWideSignedArgument(
    va_list *arguments,
    uint8_t length
)
{
    switch (length) {
    case GAME_FORMAT_LENGTH_CHAR:
        return (signed char)va_arg(*arguments, int);
    case GAME_FORMAT_LENGTH_SHORT:
        return (short)va_arg(*arguments, int);
    case GAME_FORMAT_LENGTH_LONG:
        return (int32_t)va_arg(*arguments, long);
    case GAME_FORMAT_LENGTH_LONG_LONG:
        return (int64_t)va_arg(*arguments, long long);
    case GAME_FORMAT_LENGTH_INTMAX:
        return (int64_t)va_arg(*arguments, intmax_t);
    case GAME_FORMAT_LENGTH_SIZE:
    case GAME_FORMAT_LENGTH_PTRDIFF:
        return (int64_t)va_arg(*arguments, ptrdiff_t);
    default:
        return (int32_t)va_arg(*arguments, int);
    }
}

static uint64_t Game_ReadWideUnsignedArgument(
    va_list *arguments,
    uint8_t length
)
{
    switch (length) {
    case GAME_FORMAT_LENGTH_CHAR:
        return (unsigned char)va_arg(*arguments, int);
    case GAME_FORMAT_LENGTH_SHORT:
        return (unsigned short)va_arg(*arguments, int);
    case GAME_FORMAT_LENGTH_LONG:
        return (uint32_t)va_arg(*arguments, unsigned long);
    case GAME_FORMAT_LENGTH_LONG_LONG:
        return (uint64_t)va_arg(*arguments, unsigned long long);
    case GAME_FORMAT_LENGTH_INTMAX:
        return (uint64_t)va_arg(*arguments, uintmax_t);
    case GAME_FORMAT_LENGTH_SIZE:
        return (uint64_t)va_arg(*arguments, size_t);
    case GAME_FORMAT_LENGTH_PTRDIFF:
        return (uint64_t)va_arg(*arguments, uintptr_t);
    default:
        return (uint32_t)va_arg(*arguments, unsigned int);
    }
}

static void Game_StoreWideCount(
    va_list *arguments,
    uint8_t length,
    int count
)
{
    switch (length) {
    case GAME_FORMAT_LENGTH_CHAR:
        *va_arg(*arguments, signed char *) = (signed char)count;
        break;
    case GAME_FORMAT_LENGTH_SHORT:
        *va_arg(*arguments, short *) = (short)count;
        break;
    case GAME_FORMAT_LENGTH_LONG_LONG:
    case GAME_FORMAT_LENGTH_INTMAX:
        *va_arg(*arguments, int64_t *) = (int64_t)count;
        break;
    default:
        *va_arg(*arguments, int32_t *) = (int32_t)count;
        break;
    }
}

static uint16_t *Game_FormatWideIntegerArgument(
    va_list *arguments,
    uint16_t *buffer_end,
    const GameFormatConversion *conversion
)
{
    bool signed_conversion = conversion->conversion_05 == (uint8_t)'d' ||
        conversion->conversion_05 == (uint8_t)'i';
    bool use_64_bit = conversion->length_04 ==
        GAME_FORMAT_LENGTH_LONG_LONG ||
        conversion->length_04 == GAME_FORMAT_LENGTH_INTMAX;
    uint64_t bits = signed_conversion ?
        (uint64_t)Game_ReadWideSignedArgument(
            arguments, conversion->length_04) :
        Game_ReadWideUnsignedArgument(arguments, conversion->length_04);

    if (use_64_bit) {
        return Game_FormatWideInteger64_02008bdc(
            (uint32_t)bits,
            (uint32_t)(bits >> 32),
            buffer_end,
            Game_PackWideFlags(conversion),
            Game_PackWideType(conversion),
            (int32_t)conversion->width_08,
            (int32_t)conversion->precision_0c
        );
    }
    return Game_FormatWideInteger32_02008990(
        (int32_t)bits,
        buffer_end,
        Game_PackWideFlags(conversion),
        Game_PackWideType(conversion),
        (int32_t)conversion->width_08,
        (int32_t)conversion->precision_0c
    );
}

static uint16_t *Game_FormatWideFloatingArgument(
    va_list *arguments,
    uint16_t *buffer_end,
    const GameFormatConversion *conversion
)
{
    union {
        double value;
        uint64_t bits;
    } floating;

    if (conversion->length_04 == GAME_FORMAT_LENGTH_LONG_DOUBLE) {
        floating.value = (double)va_arg(*arguments, long double);
    } else {
        floating.value = va_arg(*arguments, double);
    }
    if (conversion->conversion_05 == (uint8_t)'a' ||
        conversion->conversion_05 == (uint8_t)'A') {
        return Game_FormatWideHexFloat_02008eb4(
            (uint32_t)floating.bits,
            (uint32_t)(floating.bits >> 32),
            buffer_end,
            Game_PackWideFlags(conversion),
            Game_PackWideType(conversion),
            (int32_t)conversion->width_08,
            (int32_t)conversion->precision_0c
        );
    }
    return Game_FormatWideDecimalFloat_02009388(
        (uint32_t)floating.bits,
        (uint32_t)(floating.bits >> 32),
        buffer_end,
        Game_PackWideFlags(conversion),
        Game_PackWideType(conversion),
        (int32_t)conversion->width_08,
        (int32_t)conversion->precision_0c
    );
}

/* 0x02009980..0x0200A2E3; the two null-string literals follow. */
int Game_RunWideFormatter_02009980(
    GameWideFormatWriteCallback callback,
    void *context,
    const uint16_t *format,
    va_list arguments,
    int mode
)
{
    static const uint16_t sEmpty[] = {0};
    GameFormatConversion conversion;
    uint16_t buffer[GAME_FORMAT_WORK_BUFFER_SIZE];
    va_list cursor;
    int total = 0;

    va_copy(cursor, arguments);
    while (*format != 0) {
        const uint16_t *percent = Game_FindNextWidePercent(format);
        uint32_t literal_length = (uint32_t)(percent - format);
        const uint16_t *text = NULL;
        uint32_t length = 0;

        if (!Game_WriteWideUnits(
                callback, context, format, literal_length)) {
            total = -1;
            break;
        }
        total += (int)literal_length;
        if (*percent == 0) {
            break;
        }

        format = Game_ParseWideFormatConversion_02008478(
            percent,
            &cursor,
            &conversion
        );
        switch (conversion.conversion_05) {
        case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
            text = Game_FormatWideIntegerArgument(
                &cursor, buffer + GAME_FORMAT_WORK_BUFFER_SIZE, &conversion);
            if (text != NULL) {
                length = Game_WideStringLength_0200a3ac(text);
            }
            break;
        case 'a': case 'A': case 'e': case 'E':
        case 'f': case 'F': case 'g': case 'G':
            text = Game_FormatWideFloatingArgument(
                &cursor, buffer + GAME_FORMAT_WORK_BUFFER_SIZE, &conversion);
            if (text != NULL) {
                length = Game_WideStringLength_0200a3ac(text);
            }
            break;
        case 'c':
            buffer[0] = (uint16_t)va_arg(cursor, int);
            text = buffer;
            length = 1;
            break;
        case 's': {
            const void *argument = va_arg(cursor, const void *);

            if (mode != 0 && argument == NULL) {
                Game_RaiseFormatConstraint_02008110();
                total = -1;
                goto finished;
            }
            if (argument == NULL) {
                argument = sEmpty;
                conversion.length_04 = GAME_FORMAT_LENGTH_WIDE;
            }
            if (conversion.length_04 == GAME_FORMAT_LENGTH_WIDE) {
                text = (const uint16_t *)argument;
                length = conversion.precision_present_02 != 0U ?
                    Game_WideBoundedLength(text, conversion.precision_0c) :
                    Game_WideStringLength_0200a3ac(text);
            } else {
                const unsigned char *source =
                    (const unsigned char *)argument;

                while (length + 1U < GAME_FORMAT_WORK_BUFFER_SIZE &&
                       source[length] != 0 &&
                       (conversion.precision_present_02 == 0U ||
                        length < conversion.precision_0c)) {
                    buffer[length] = source[length];
                    ++length;
                }
                buffer[length] = 0;
                text = buffer;
            }
            break;
        }
        case 'n':
            if (mode != 0) {
                Game_RaiseFormatConstraint_02008110();
                total = -1;
                goto finished;
            }
            Game_StoreWideCount(&cursor, conversion.length_04, total);
            continue;
        case '%':
            buffer[0] = (uint16_t)'%';
            text = buffer;
            length = 1;
            break;
        default:
            text = percent;
            length = Game_WideStringLength_0200a3ac(percent);
            conversion.padding_00 = GAME_FORMAT_PADDING_LEFT;
            conversion.width_08 = 0;
            format = percent + length;
            break;
        }

        if (text == NULL || !Game_WritePaddedWideField(
                callback, context, &conversion, text, length, &total)) {
            total = -1;
            break;
        }
    }

finished:
    va_end(cursor);
    return total;
}

/* 0x0200A2E4..0x0200A323. */
int GameWideFormatBuffer_Write_0200a2e4(
    GameWideFormatBuffer *output,
    const uint16_t *source,
    uint32_t unit_count
)
{
    uint32_t available = output->length < output->capacity ?
        output->capacity - output->length : 0;
    uint32_t copied = unit_count < available ? unit_count : available;

    if (output->buffer != NULL && copied != 0U) {
        Game_CopyWideUnits_02008440(
            output->buffer + output->length,
            source,
            copied
        );
    }
    output->length += unit_count;
    return 1;
}

/* 0x0200A324..0x0200A34B. */
int NtmvM2d_FormatWideText_0200a324(
    uint16_t *output,
    uint32_t capacity,
    const uint16_t *format,
    ...
)
{
    int result;
    va_list arguments;

    va_start(arguments, format);
    result = Game_VSNPrintfWide_0200a34c(
        output, capacity, format, arguments);
    va_end(arguments);
    return result;
}

/* 0x0200A34C..0x0200A3AB; callback literal 0x0200A2E4 follows. */
int Game_VSNPrintfWide_0200a34c(
    uint16_t *output,
    uint32_t capacity,
    const uint16_t *format,
    va_list arguments
)
{
    GameWideFormatBuffer state;
    int result;

    state.buffer = output;
    state.capacity = capacity;
    state.length = 0;
    result = Game_RunWideFormatter_02009980(
        (GameWideFormatWriteCallback)GameWideFormatBuffer_Write_0200a2e4,
        &state,
        format,
        arguments,
        0
    );
    if (output != NULL && capacity != 0U) {
        if (result >= 0 && (uint32_t)result < capacity) {
            output[result] = 0;
        } else {
            output[capacity - 1U] = 0;
            result = -1;
        }
    }
    return result;
}
