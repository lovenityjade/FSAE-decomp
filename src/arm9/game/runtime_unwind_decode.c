#include "game/runtime_unwind.h"

#include <stddef.h>
#include <stdint.h>

extern int Game_UnwindLocateTables_0200ca38(
    GameUnwindDescriptor *descriptor,
    uint32_t address
);
extern const uint8_t *Game_UnwindGetProgram_0200ca58(uint32_t program);
extern void Game_RuntimeNoOp_0200b128(void);

/* 0x0200B10C..0x0200B117; literal target 0x0200B128 follows. */
void Game_RuntimeNoOpThunk_0200b10c(void)
{
    Game_RuntimeNoOp_0200b128();
}

/* 0x0200B118..0x0200B11B. */
void Game_RuntimeInputBegin_0200b118(void *buffer)
{
    (void)buffer;
}

/* 0x0200B11C..0x0200B11F. */
void Game_RuntimeInputEnd_0200b11c(void *buffer)
{
    (void)buffer;
}

/* 0x0200B120..0x0200B127. */
int Game_RuntimeReadByte_0200b120(void)
{
    return 0;
}

/* 0x0200B128..0x0200B12B. */
void Game_RuntimeNoOp_0200b128(void)
{
}

/* 0x0200B12C..0x0200B17B. */
int Game_ReadRuntimeLine_0200b12c(
    void *context,
    char *buffer,
    uint32_t *count
)
{
    uint32_t requested = *count;
    uint32_t index;

    (void)context;
    for (index = 0; index < requested; ++index) {
        int value = Game_RuntimeReadByte_0200b120();

        buffer[index] = (char)value;
        if ((uint8_t)value == (uint8_t)'\r' ||
            (uint8_t)value == (uint8_t)'\n') {
            *count = index + 1U;
            break;
        }
    }
    return 0;
}

/* 0x0200B17C..0x0200B1C7; the trailing two-instruction return-zero veneer is
 * included in this extent but is not a separately catalogued function. */
int Game_FinalizeRuntimeLine_0200b17c(
    void *context,
    char *buffer,
    uint32_t *count,
    int final_character
)
{
    char temporary = (char)final_character;

    (void)context;
    if (*count > 1U) {
        temporary = buffer[*count - 1U];
        buffer[*count - 1U] = '\0';
        Game_RuntimeInputEnd_0200b11c(buffer);
        buffer = &temporary;
    }
    Game_RuntimeInputBegin_0200b118(buffer);
    return 0;
}

/* 0x0200B1C8..0x0200B24B: signed compact-value decoder. */
const uint8_t *Game_DecodeSignedUnwindValue_0200b1c8(
    const uint8_t *cursor,
    int32_t *value
)
{
    int32_t first = (int8_t)cursor[0];

    if ((first & 1) == 0) {
        *value = first >> 1;
        return cursor + 1;
    }
    if ((first & 2) == 0) {
        *value = ((first >> 2) << 8) | cursor[1];
        return cursor + 2;
    }
    if ((first & 4) == 0) {
        *value = ((first >> 3) << 16) |
            ((int32_t)cursor[1] << 8) | cursor[2];
        return cursor + 3;
    }
    *value = ((first >> 3) << 24) |
        ((int32_t)cursor[1] << 16) |
        ((int32_t)cursor[2] << 8) | cursor[3];
    return cursor + 4;
}

/* 0x0200B24C..0x0200B2CF: unsigned compact-value decoder. */
const uint8_t *Game_DecodeUnsignedUnwindValue_0200b24c(
    const uint8_t *cursor,
    uint32_t *value
)
{
    uint32_t first = cursor[0];

    if ((first & 1U) == 0U) {
        *value = first >> 1;
        return cursor + 1;
    }
    if ((first & 2U) == 0U) {
        *value = (first >> 2 << 8) | cursor[1];
        return cursor + 2;
    }
    if ((first & 4U) == 0U) {
        *value = (first >> 3 << 16) |
            ((uint32_t)cursor[1] << 8) | cursor[2];
        return cursor + 3;
    }
    *value = (first >> 3 << 24) |
        ((uint32_t)cursor[1] << 16) |
        ((uint32_t)cursor[2] << 8) | cursor[3];
    return cursor + 4;
}

/* 0x0200B2D0..0x0200B333. */
const GameUnwindCodeRange *Game_FindUnwindCodeRange_0200b2d0(
    const GameUnwindCodeRange *ranges,
    int32_t count,
    uint32_t address
)
{
    int32_t lower = 0;
    int32_t upper = count - 1;

    while (lower <= upper) {
        int32_t middle = (lower + upper) / 2;
        const GameUnwindCodeRange *range = &ranges[middle];
        uint32_t end = range->start_00 + (range->extent_flags_04 & ~1U);

        if (address < range->start_00) {
            upper = middle - 1;
        } else if (address > end) {
            lower = middle + 1;
        } else {
            return range;
        }
    }
    return NULL;
}

/* 0x0200B334..0x0200B43B; reciprocal-of-twelve literal follows. */
void Game_LoadUnwindDescriptor_0200b334(
    uint32_t address,
    GameUnwindDescriptor *descriptor
)
{
    const GameUnwindCodeRange *range;
    const uint8_t *program;
    uint32_t offset = 0;
    int32_t range_count;

    descriptor->program_04 = 0;
    descriptor->cursor_08 = 0;
    if (Game_UnwindLocateTables_0200ca38(descriptor, address) == 0) {
        return;
    }
    range_count = (int32_t)((descriptor->range_end_10 -
        descriptor->range_begin_0c) / sizeof(GameUnwindCodeRange));
    range = Game_FindUnwindCodeRange_0200b2d0(
        (const GameUnwindCodeRange *)(uintptr_t)descriptor->range_begin_0c,
        range_count,
        address
    );
    if (range == NULL) {
        return;
    }

    descriptor->function_start_00 = range->start_00;
    if ((range->extent_flags_04 & 1U) != 0U) {
        program = (const uint8_t *)&range->program_08;
    } else {
        program = (const uint8_t *)(uintptr_t)range->program_08;
    }
    descriptor->program_04 = (uint32_t)(uintptr_t)program;
    program = Game_UnwindGetProgram_0200ca58(descriptor->program_04);

    for (;;) {
        uint32_t code_extent;
        uint32_t gap_extent;
        uint32_t program_offset;

        program = Game_DecodeUnsignedUnwindValue_0200b24c(
            program, &code_extent);
        if (code_extent == 0U) {
            return;
        }
        program = Game_DecodeUnsignedUnwindValue_0200b24c(
            program, &gap_extent);
        program = Game_DecodeUnsignedUnwindValue_0200b24c(
            program, &program_offset);
        if (address - range->start_00 < offset + code_extent) {
            return;
        }
        offset += code_extent + gap_extent;
        if (address - range->start_00 <= offset) {
            descriptor->cursor_08 = descriptor->program_04 + program_offset;
            return;
        }
    }
}

/* 0x0200B43C..0x0200B457. */
uint8_t Game_GetUnwindOpcode_0200b43c(const GameUnwindDescriptor *descriptor)
{
    if (descriptor->cursor_08 == 0U) {
        return 0;
    }
    return *(const uint8_t *)(uintptr_t)descriptor->cursor_08 & 0x1fU;
}

static const uint8_t *Game_SkipSigned(const uint8_t *cursor)
{
    int32_t ignored;

    return Game_DecodeSignedUnwindValue_0200b1c8(cursor, &ignored);
}

static const uint8_t *Game_SkipUnsigned(const uint8_t *cursor)
{
    uint32_t ignored;

    return Game_DecodeUnsignedUnwindValue_0200b24c(cursor, &ignored);
}

/* 0x0200B458..0x0200B713.  Advances one compact cleanup instruction without
 * executing it, then follows opcode-one continuation records. */
uint8_t Game_AdvanceUnwindOpcode_0200b458(GameUnwindDescriptor *descriptor)
{
    const uint8_t *instruction;
    uint8_t opcode;

    for (;;) {
        instruction = (const uint8_t *)(uintptr_t)descriptor->cursor_08;
        if (instruction == NULL || (*instruction & 0x80U) != 0U) {
            Game_LoadUnwindDescriptor_0200b334(
                descriptor->function_start_00, descriptor);
            instruction = (const uint8_t *)(uintptr_t)descriptor->cursor_08;
            if (instruction == NULL) {
                return UINT8_MAX;
            }
        }

        opcode = *instruction & 0x1fU;
        switch (opcode) {
        case 2:
            instruction = Game_SkipSigned(instruction + 1) + 4;
            break;
        case 3:
            instruction = Game_SkipSigned(instruction + 1);
            instruction = Game_SkipSigned(instruction) + 4;
            break;
        case 4:
            instruction = Game_SkipSigned(instruction + 1) + 4;
            break;
        case 5:
            instruction = Game_SkipSigned(instruction + 1);
            instruction = Game_SkipUnsigned(instruction);
            instruction = Game_SkipUnsigned(instruction) + 4;
            break;
        case 6: case 7:
            instruction = Game_SkipSigned(instruction + 1);
            instruction = Game_SkipSigned(instruction) + 4;
            break;
        case 8:
            instruction = Game_SkipSigned(instruction + 1);
            instruction = Game_SkipSigned(instruction);
            instruction = Game_SkipSigned(instruction) + 4;
            break;
        case 9:
            instruction = Game_SkipSigned(instruction + 1);
            instruction = Game_SkipSigned(instruction);
            instruction = Game_SkipUnsigned(instruction);
            instruction = Game_SkipUnsigned(instruction) + 4;
            break;
        case 10:
            instruction = Game_SkipSigned(instruction + 1) + 4;
            break;
        case 11:
            instruction = Game_SkipSigned(instruction + 1);
            instruction = Game_SkipSigned(instruction) + 4;
            break;
        case 12:
            instruction = Game_SkipUnsigned(instruction + 5);
            instruction = Game_SkipSigned(instruction) + 4;
            break;
        case 13:
            instruction = Game_SkipSigned(instruction + 1) + 4;
            break;
        case 15: {
            uint32_t table_count;

            instruction = Game_DecodeUnsignedUnwindValue_0200b24c(
                instruction + 1, &table_count);
            instruction = Game_SkipUnsigned(instruction);
            instruction = Game_SkipSigned(instruction) + table_count * 4U + 4U;
            break;
        }
        case 16:
            instruction = Game_SkipSigned(instruction + 1);
            instruction = Game_SkipSigned(instruction) + 4;
            instruction = Game_SkipSigned(instruction) + 4;
            break;
        case 17:
            instruction = Game_SkipSigned(instruction + 1);
            instruction = Game_SkipSigned(instruction) + 1;
            instruction = Game_SkipSigned(instruction);
            instruction = Game_SkipSigned(instruction) + 4;
            break;
        case 18:
            instruction = Game_SkipSigned(instruction + 1) + 1;
            instruction = Game_SkipSigned(instruction);
            instruction = Game_SkipUnsigned(instruction) + 4;
            break;
        case 19:
            instruction = Game_SkipSigned(instruction + 1) + 4;
            break;
        default:
            return UINT8_MAX;
        }
        descriptor->cursor_08 = (uint32_t)(uintptr_t)instruction;
        if ((*instruction & 0x80U) != 0U) {
            descriptor->cursor_08 = 0;
        }
        if (opcode != 1U) {
            break;
        }
    }

    while (descriptor->cursor_08 != 0U &&
           Game_GetUnwindOpcode_0200b43c(descriptor) == 1U) {
        int32_t continuation;

        instruction = (const uint8_t *)(uintptr_t)descriptor->cursor_08;
        instruction = Game_DecodeSignedUnwindValue_0200b1c8(
            instruction + 1, &continuation);
        descriptor->cursor_08 += (uint32_t)continuation;
    }
    return Game_GetUnwindOpcode_0200b43c(descriptor);
}
