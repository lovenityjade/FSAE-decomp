#include "game/format_runtime_low.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern int Game_ConvertWideString_02005d48(
    char *destination,
    const uint16_t *source,
    uint32_t capacity
);
extern void Game_RaiseFormatConstraint_02008110(void);
extern int Game_StreamWriteElements_02005798(
    const void *data,
    uint32_t element_size,
    uint32_t element_count,
    void *stream
);

static uint32_t Game_FormatStringLength(const char *string)
{
    uint32_t length = 0;

    while (string[length] != '\0') {
        ++length;
    }
    return length;
}

static uint32_t Game_FormatBoundedStringLength(
    const char *string,
    uint32_t limit)
{
    uint32_t length = 0;

    while (length < limit && string[length] != '\0') {
        ++length;
    }
    return length;
}

static const char *Game_FindNextFormatPercent(const char *format)
{
    while (*format != '\0' && *format != '%') {
        ++format;
    }
    return format;
}

static uint32_t Game_PackFormatFlags(
    const GameFormatConversion *conversion)
{
    return (uint32_t)conversion->padding_00 |
        ((uint32_t)conversion->sign_01 << 8) |
        ((uint32_t)conversion->precision_present_02 << 16) |
        ((uint32_t)conversion->alternate_03 << 24);
}

static uint32_t Game_PackFormatType(
    const GameFormatConversion *conversion)
{
    return (uint32_t)conversion->length_04 |
        ((uint32_t)conversion->conversion_05 << 8);
}

static bool Game_WriteFormatBytes(
    GameFormatWriteCallback callback,
    void *context,
    const void *data,
    uint32_t length)
{
    return length == 0U || callback(context, data, length) != 0;
}

static bool Game_WriteFormatPadding(
    GameFormatWriteCallback callback,
    void *context,
    char padding,
    uint32_t count)
{
    while (count != 0U) {
        if (!Game_WriteFormatBytes(callback, context, &padding, 1)) {
            return false;
        }
        --count;
    }
    return true;
}

static bool Game_WritePaddedFormatField(
    GameFormatWriteCallback callback,
    void *context,
    const GameFormatConversion *conversion,
    const char *text,
    uint32_t length,
    int *total)
{
    uint32_t padding = conversion->width_08 > length ?
        conversion->width_08 - length : 0;
    uint32_t prefix = 0;
    char padding_character = conversion->padding_00 ==
        GAME_FORMAT_PADDING_ZERO ? '0' : ' ';

    if (conversion->padding_00 != GAME_FORMAT_PADDING_LEFT) {
        if (padding_character == '0' && length != 0U) {
            if (text[0] == '+' || text[0] == '-' || text[0] == ' ') {
                prefix = 1;
            }
            if ((conversion->conversion_05 == (uint8_t)'a' ||
                 conversion->conversion_05 == (uint8_t)'A') &&
                length >= prefix + 2U && text[prefix] == '0' &&
                (text[prefix + 1U] == 'x' || text[prefix + 1U] == 'X')) {
                prefix += 2U;
            }
            if (!Game_WriteFormatBytes(
                    callback, context, text, prefix)) {
                return false;
            }
            text += prefix;
            length -= prefix;
        }
        if (!Game_WriteFormatPadding(
                callback, context, padding_character, padding)) {
            return false;
        }
    }
    if (!Game_WriteFormatBytes(callback, context, text, length)) {
        return false;
    }
    if (conversion->padding_00 == GAME_FORMAT_PADDING_LEFT &&
        !Game_WriteFormatPadding(callback, context, ' ', padding)) {
        return false;
    }
    *total += (int)(length + prefix + padding);
    return true;
}

static int64_t Game_ReadSignedFormatArgument(
    va_list *arguments,
    uint8_t length)
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
        return (int64_t)va_arg(*arguments, ptrdiff_t);
    case GAME_FORMAT_LENGTH_PTRDIFF:
        return (int64_t)va_arg(*arguments, ptrdiff_t);
    default:
        return (int32_t)va_arg(*arguments, int);
    }
}

static uint64_t Game_ReadUnsignedFormatArgument(
    va_list *arguments,
    uint8_t length)
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

static void Game_StoreFormatCount(
    va_list *arguments,
    uint8_t length,
    int count)
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

static char *Game_FormatIntegerArgument(
    va_list *arguments,
    char *buffer_end,
    const GameFormatConversion *conversion)
{
    uint32_t flags_word = Game_PackFormatFlags(conversion);
    uint32_t type_word = Game_PackFormatType(conversion);
    bool signed_conversion = conversion->conversion_05 == (uint8_t)'d' ||
        conversion->conversion_05 == (uint8_t)'i';
    bool wide = conversion->length_04 == GAME_FORMAT_LENGTH_LONG_LONG ||
        conversion->length_04 == GAME_FORMAT_LENGTH_INTMAX;
    uint64_t bits;

    if (signed_conversion) {
        bits = (uint64_t)Game_ReadSignedFormatArgument(
            arguments,
            conversion->length_04
        );
    } else {
        bits = Game_ReadUnsignedFormatArgument(
            arguments,
            conversion->length_04
        );
    }
    if (wide) {
        return Game_FormatInteger64_02006694(
            (uint32_t)bits,
            (uint32_t)(bits >> 32),
            buffer_end,
            flags_word,
            type_word,
            (int32_t)conversion->width_08,
            (int32_t)conversion->precision_0c
        );
    }
    return Game_FormatInteger32_02006444(
        (int32_t)bits,
        buffer_end,
        flags_word,
        type_word,
        (int32_t)conversion->width_08,
        (int32_t)conversion->precision_0c
    );
}

static char *Game_FormatFloatingArgument(
    va_list *arguments,
    char *buffer_end,
    const GameFormatConversion *conversion)
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
        return Game_FormatHexFloat_02006968(
            (uint32_t)floating.bits,
            (uint32_t)(floating.bits >> 32),
            buffer_end,
            Game_PackFormatFlags(conversion),
            Game_PackFormatType(conversion),
            (int32_t)conversion->width_08,
            (int32_t)conversion->precision_0c
        );
    }
    return Game_FormatDecimalFloat_02006f28(
        (uint32_t)floating.bits,
        (uint32_t)(floating.bits >> 32),
        buffer_end,
        Game_PackFormatFlags(conversion),
        Game_PackFormatType(conversion),
        (int32_t)conversion->width_08,
        (int32_t)conversion->precision_0c
    );
}

/*
 * 0x02007638..0x02007EDF (2216 bytes).
 *
 * Core callback formatter.  It streams literal spans, parses one exact
 * 16-byte conversion descriptor, consumes the saved ARM varargs according to
 * the recovered length code, dispatches integer and floating helpers, applies
 * sign-aware zero/space padding, handles strings/chars/%n and returns -1 on a
 * sink or constraint failure.  The null-string literals at 0x02007EE0 are not
 * part of the body.
 */
int Game_RunFormatter(
    GameFormatWriteCallback callback,
    void *context,
    const char *format,
    va_list arguments,
    int mode)
{
    static const char sNullString[] = "";
    GameFormatConversion conversion;
    char buffer[GAME_FORMAT_WORK_BUFFER_SIZE];
    va_list cursor;
    int total = 0;

    va_copy(cursor, arguments);
    while (*format != '\0') {
        const char *percent = Game_FindNextFormatPercent(format);
        uint32_t literal_length = (uint32_t)(percent - format);
        const char *next;
        const char *text = NULL;
        uint32_t length = 0;

        if (literal_length != 0U) {
            if (!Game_WriteFormatBytes(
                    callback, context, format, literal_length)) {
                total = -1;
                break;
            }
            total += (int)literal_length;
        }
        if (*percent == '\0') {
            break;
        }

        next = Game_ParseFormatConversion_02005f38(
            percent,
            &cursor,
            &conversion
        );
        format = next;
        switch (conversion.conversion_05) {
        case 'd':
        case 'i':
        case 'o':
        case 'u':
        case 'x':
        case 'X':
            text = Game_FormatIntegerArgument(
                &cursor,
                buffer + sizeof(buffer),
                &conversion
            );
            if (text != NULL) {
                length = Game_FormatStringLength(text);
            }
            break;
        case 'a':
        case 'A':
        case 'e':
        case 'E':
        case 'f':
        case 'F':
        case 'g':
        case 'G':
            text = Game_FormatFloatingArgument(
                &cursor,
                buffer + sizeof(buffer),
                &conversion
            );
            if (text != NULL) {
                length = Game_FormatStringLength(text);
            }
            break;
        case 'c':
            buffer[0] = (char)va_arg(cursor, int);
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
                argument = sNullString;
            }
            if (conversion.length_04 == GAME_FORMAT_LENGTH_WIDE) {
                int converted = Game_ConvertWideString_02005d48(
                    buffer,
                    (const uint16_t *)argument,
                    sizeof(buffer)
                );

                if (converted < 0) {
                    total = -1;
                    goto finished;
                }
                text = buffer;
                length = (uint32_t)converted;
            } else {
                text = (const char *)argument;
                length = conversion.precision_present_02 != 0U ?
                    Game_FormatBoundedStringLength(
                        text,
                        conversion.precision_0c
                    ) : Game_FormatStringLength(text);
            }
            break;
        }
        case 'n':
            if (mode != 0) {
                Game_RaiseFormatConstraint_02008110();
                total = -1;
                goto finished;
            }
            Game_StoreFormatCount(&cursor, conversion.length_04, total);
            continue;
        case '%':
            buffer[0] = '%';
            text = buffer;
            length = 1;
            break;
        default:
            text = percent;
            length = Game_FormatStringLength(percent);
            conversion.padding_00 = GAME_FORMAT_PADDING_LEFT;
            conversion.width_08 = 0;
            format = percent + length;
            break;
        }

        if (text == NULL ||
            !Game_WritePaddedFormatField(
                callback,
                context,
                &conversion,
                text,
                length,
                &total)) {
            total = -1;
            break;
        }
    }

finished:
    va_end(cursor);
    return total;
}

/*
 * 0x02007EE8..0x02007F13 (44 bytes).
 * fwrite-shaped callback used by Game_LogFormat: exactly one-byte elements
 * are requested and the original non-null stream context is returned only
 * when every requested byte is accepted.
 */
int Game_LogStreamWrite(
    void *context,
    const void *data,
    uint32_t byte_count)
{
    int written = Game_StreamWriteElements_02005798(
        data,
        1,
        byte_count,
        context
    );

    if (written != (int)byte_count) {
        return 0;
    }
    return (int)(uintptr_t)context;
}
