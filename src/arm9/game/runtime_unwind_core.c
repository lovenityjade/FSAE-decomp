#include "game/runtime_unwind.h"

#include "game/fatal_error.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern int Game_UnwindTypeMatches_0200c594(
    uint32_t exception_type,
    uint32_t candidate_type,
    uint32_t *adjustment
);
extern uint32_t Game_UnwindResolveContext_0200c93c(
    GameUnwindContext *context,
    GameUnwindDescriptor *descriptor
);
extern void Game_UnwindSynchronizeDescriptor_0200c998(
    GameUnwindContext *context,
    GameUnwindDescriptor *descriptor
);
extern void Game_UnwindCommitDescriptor_0200ca80(
    GameUnwindContext *context,
    GameUnwindDescriptor *descriptor,
    uint32_t program
);
extern void Game_UnwindAbort_0200c55c(void);

extern void (*gGameStaticInitializerTable[])(void);

static uint32_t Game_UnwindReadU32(const uint8_t *data)
{
    return (uint32_t)data[0] |
        ((uint32_t)data[1] << 8) |
        ((uint32_t)data[2] << 16) |
        ((uint32_t)data[3] << 24);
}

static uint32_t Game_UnwindLoadWord(
    const GameUnwindContext *context,
    int32_t offset,
    bool from_register
)
{
    if (from_register) {
        return context->registers_1c[offset];
    }
    return *(const uint32_t *)(uintptr_t)(context->stack_base_18 +
        (uint32_t)offset);
}

static uint32_t Game_UnwindLoadCondition(
    const GameUnwindContext *context,
    int32_t offset,
    bool from_register,
    bool halfword
)
{
    if (from_register) {
        return halfword ?
            (uint16_t)context->registers_1c[offset] :
            (uint8_t)context->registers_1c[offset];
    }
    if (halfword) {
        return *(const uint16_t *)(uintptr_t)(context->stack_base_18 +
            (uint32_t)offset);
    }
    return *(const uint8_t *)(uintptr_t)(context->stack_base_18 +
        (uint32_t)offset);
}

static void Game_UnwindCallDestructor(
    uint32_t callback_address,
    uint32_t object_address,
    int reason
)
{
    GameUnwindDestructor callback =
        (GameUnwindDestructor)(uintptr_t)callback_address;

    callback((void *)(uintptr_t)object_address, reason);
}

static const uint8_t *Game_UnwindReadSigned(
    const uint8_t *cursor,
    int32_t *value
)
{
    return Game_DecodeSignedUnwindValue_0200b1c8(cursor, value);
}

static const uint8_t *Game_UnwindReadUnsigned(
    const uint8_t *cursor,
    uint32_t *value
)
{
    return Game_DecodeUnsignedUnwindValue_0200b24c(cursor, value);
}

/* 0x0200B714..0x0200BF47.  Executes CodeWarrior compact cleanup opcodes for
 * one frame, stopping at an optional landing-pad record.  The two words after
 * the body are an epilogue veneer, not a separately catalogued function. */
void Game_ExecuteUnwindCleanup_0200b714(
    GameUnwindContext *context,
    GameUnwindDescriptor *descriptor,
    const uint8_t *stop
)
{
    for (;;) {
        const uint8_t *instruction =
            (const uint8_t *)(uintptr_t)descriptor->cursor_08;
        const uint8_t *cursor;
        uint8_t encoded;
        uint8_t opcode;
        int32_t first;
        int32_t second;
        int32_t third;
        uint32_t count;
        uint32_t extent;
        uint32_t callback;
        uint32_t object;
        uint32_t index;

        if (instruction == NULL) {
            uint32_t address = Game_UnwindResolveContext_0200c93c(
                context, descriptor);

            Game_LoadUnwindDescriptor_0200b334(
                address, descriptor);
            if (descriptor->program_04 == 0U) {
                Game_UnwindAbort_0200c55c();
                return;
            }
            Game_UnwindSynchronizeDescriptor_0200c998(context, descriptor);
            instruction = (const uint8_t *)(uintptr_t)descriptor->cursor_08;
            if (instruction == NULL) {
                continue;
            }
        }

        encoded = *instruction;
        opcode = encoded & 0x1fU;
        cursor = instruction + 1;
        switch (opcode) {
        case 1:
            cursor = Game_UnwindReadSigned(cursor, &first);
            cursor = instruction + first;
            break;
        case 2:
            cursor = Game_UnwindReadSigned(cursor, &first);
            callback = Game_UnwindReadU32(cursor);
            cursor += 4;
            Game_UnwindCallDestructor(
                callback, context->stack_base_18 + (uint32_t)first, -1);
            break;
        case 3:
            cursor = Game_UnwindReadSigned(cursor, &first);
            cursor = Game_UnwindReadSigned(cursor, &second);
            callback = Game_UnwindReadU32(cursor);
            cursor += 4;
            if (Game_UnwindLoadCondition(
                    context, first, (encoded & 0x40U) != 0U, false) != 0U) {
                Game_UnwindCallDestructor(
                    callback,
                    context->stack_base_18 + (uint32_t)second,
                    -1
                );
            }
            break;
        case 4:
            cursor = Game_UnwindReadSigned(cursor, &first);
            callback = Game_UnwindReadU32(cursor);
            cursor += 4;
            object = Game_UnwindLoadWord(
                context, first, (encoded & 0x20U) != 0U);
            Game_UnwindCallDestructor(callback, object, -1);
            break;
        case 5:
            cursor = Game_UnwindReadSigned(cursor, &first);
            cursor = Game_UnwindReadUnsigned(cursor, &count);
            cursor = Game_UnwindReadUnsigned(cursor, &extent);
            callback = Game_UnwindReadU32(cursor);
            cursor += 4;
            object = context->stack_base_18 + (uint32_t)first + count * extent;
            for (index = 0; index < count; ++index) {
                object -= extent;
                Game_UnwindCallDestructor(callback, object, -1);
            }
            break;
        case 6: case 7:
            cursor = Game_UnwindReadSigned(cursor, &first);
            cursor = Game_UnwindReadSigned(cursor, &second);
            callback = Game_UnwindReadU32(cursor);
            cursor += 4;
            object = Game_UnwindLoadWord(
                context, first, (encoded & 0x20U) != 0U);
            Game_UnwindCallDestructor(
                callback,
                object + (uint32_t)second,
                opcode == 6U ? 0 : -1
            );
            break;
        case 8:
            cursor = Game_UnwindReadSigned(cursor, &first);
            cursor = Game_UnwindReadSigned(cursor, &second);
            cursor = Game_UnwindReadSigned(cursor, &third);
            callback = Game_UnwindReadU32(cursor);
            cursor += 4;
            if (Game_UnwindLoadCondition(
                    context, first, (encoded & 0x40U) != 0U, true) != 0U) {
                object = Game_UnwindLoadWord(
                    context, second, (encoded & 0x20U) != 0U);
                Game_UnwindCallDestructor(
                    callback, object + (uint32_t)third, -1);
            }
            break;
        case 9:
            cursor = Game_UnwindReadSigned(cursor, &first);
            cursor = Game_UnwindReadSigned(cursor, &second);
            cursor = Game_UnwindReadUnsigned(cursor, &count);
            cursor = Game_UnwindReadUnsigned(cursor, &extent);
            callback = Game_UnwindReadU32(cursor);
            cursor += 4;
            object = Game_UnwindLoadWord(
                context, first, (encoded & 0x20U) != 0U) +
                (uint32_t)second + count * extent;
            for (index = 0; index < count; ++index) {
                object -= extent;
                Game_UnwindCallDestructor(callback, object, -1);
            }
            break;
        case 10:
            cursor = Game_UnwindReadSigned(cursor, &first);
            callback = Game_UnwindReadU32(cursor);
            cursor += 4;
            object = Game_UnwindLoadWord(
                context, first, (encoded & 0x20U) != 0U);
            Game_UnwindCallDestructor(callback, object, -1);
            break;
        case 11:
            cursor = Game_UnwindReadSigned(cursor, &first);
            cursor = Game_UnwindReadSigned(cursor, &second);
            callback = Game_UnwindReadU32(cursor);
            cursor += 4;
            if (Game_UnwindLoadCondition(
                    context, first, (encoded & 0x40U) != 0U, false) != 0U) {
                object = Game_UnwindLoadWord(
                    context, second, (encoded & 0x20U) != 0U);
                Game_UnwindCallDestructor(callback, object, -1);
            }
            break;
        case 12:
            if (instruction == stop) {
                return;
            }
            cursor = Game_UnwindReadUnsigned(instruction + 5, &count);
            cursor = Game_UnwindReadSigned(cursor, &first) + 4;
            break;
        case 13: {
            uint32_t *frame;
            GameUnwindDestructor frame_callback;

            cursor = Game_UnwindReadSigned(cursor, &first);
            frame = (uint32_t *)(uintptr_t)(
                context->stack_base_18 + (uint32_t)first);
            frame_callback = (GameUnwindDestructor)(uintptr_t)frame[2];
            if (frame_callback != NULL) {
                if (context->exception_object_04 == frame[0]) {
                    context->state_08 = frame[2];
                } else {
                    frame_callback((void *)(uintptr_t)frame[0], -1);
                }
            }
            break;
        }
        case 14:
            Game_UnwindAbort_0200c55c();
            return;
        case 15:
            if (instruction == stop) {
                return;
            }
            cursor = Game_UnwindReadUnsigned(cursor, &count);
            cursor = Game_UnwindReadUnsigned(cursor, &extent);
            cursor = Game_UnwindReadSigned(cursor, &first) + count * 4U + 4U;
            break;
        case 16:
        {
            int32_t object_offset;
            int32_t object_adjustment;
            int32_t reason_adjustment;
            int32_t reason_base;

            cursor = Game_UnwindReadSigned(cursor, &object_offset);
            cursor = Game_UnwindReadSigned(cursor, &object_adjustment);
            reason_base = (int32_t)Game_UnwindReadU32(cursor);
            cursor += 4;
            cursor = Game_UnwindReadSigned(cursor, &reason_adjustment);
            callback = Game_UnwindReadU32(cursor);
            cursor += 4;
            object = Game_UnwindLoadWord(
                context, object_offset, (encoded & 0x20U) != 0U);
            Game_UnwindCallDestructor(
                callback,
                object + (uint32_t)object_adjustment,
                reason_base + reason_adjustment
            );
            break;
        }
        case 17:
        {
            uint8_t second_flags;
            int32_t left_offset;
            int32_t left_adjustment;
            int32_t right_offset;
            int32_t right_adjustment;
            uint32_t right_object;

            cursor = Game_UnwindReadSigned(cursor, &left_offset);
            cursor = Game_UnwindReadSigned(cursor, &left_adjustment);
            second_flags = *cursor++;
            cursor = Game_UnwindReadSigned(cursor, &right_offset);
            cursor = Game_UnwindReadSigned(cursor, &right_adjustment);
            callback = Game_UnwindReadU32(cursor);
            cursor += 4;
            object = Game_UnwindLoadWord(
                context, left_offset, (encoded & 0x20U) != 0U);
            right_object = Game_UnwindLoadWord(
                context, right_offset, (second_flags & 0x20U) != 0U);
            Game_UnwindCallDestructor(
                callback,
                object + (uint32_t)left_adjustment,
                (int)(right_object + (uint32_t)right_adjustment)
            );
            break;
        }
        case 18:
            cursor = Game_UnwindReadSigned(cursor, &first);
            encoded = *cursor++;
            cursor = Game_UnwindReadSigned(cursor, &second);
            cursor = Game_UnwindReadUnsigned(cursor, &extent);
            callback = Game_UnwindReadU32(cursor);
            cursor += 4;
            object = Game_UnwindLoadWord(
                context, first, (encoded & 0x20U) != 0U);
            index = Game_UnwindLoadWord(
                context, second, (encoded & 0x20U) != 0U) / extent;
            count = Game_UnwindLoadWord(
                context, second, (encoded & 0x20U) != 0U);
            object += count;
            for (count = 0; count < index; ++count) {
                object -= extent;
                Game_UnwindCallDestructor(callback, object, -1);
            }
            break;
        case 19:
            cursor = Game_UnwindReadSigned(cursor, &first) + 4;
            break;
        default:
            Game_UnwindAbort_0200c55c();
            return;
        }

        descriptor->cursor_08 = (uint32_t)(uintptr_t)cursor;
        if ((*instruction & 0x80U) != 0U) {
            descriptor->cursor_08 = 0;
        }
    }
}

/* 0x0200BF48..0x0200C09B. */
uint32_t Game_FindUnwindCatchTarget_0200bf48(
    GameUnwindContext *context,
    const GameUnwindDescriptor *source_descriptor
)
{
    GameUnwindDescriptor descriptor = *source_descriptor;

    for (;;) {
        uint8_t opcode = Game_GetUnwindOpcode_0200b43c(&descriptor);

        if (opcode == 13U) {
            const uint8_t *instruction =
                (const uint8_t *)(uintptr_t)descriptor.cursor_08;
            int32_t offset;
            uint32_t *frame;

            Game_DecodeSignedUnwindValue_0200b1c8(
                instruction + 1, &offset);
            frame = (uint32_t *)(uintptr_t)(
                context->stack_base_18 + (uint32_t)offset);
            context->exception_type_00 = frame[1];
            context->exception_object_04 = frame[0];
            context->state_08 = 0;
            context->landing_pad_0c = (uint32_t)(uintptr_t)frame;
            return (uint32_t)(uintptr_t)frame;
        }
        if (opcode == 14U || opcode == UINT8_MAX) {
            return 0;
        }
        Game_AdvanceUnwindOpcode_0200b458(&descriptor);
    }
}

/* 0x0200C09C..0x0200C11B. */
int Game_UnwindMatchesTypeList_0200c09c(
    uint32_t exception_type,
    const uint32_t *encoded_list
)
{
    uint32_t count = encoded_list[0];
    const uint32_t *types =
        (const uint32_t *)(uintptr_t)encoded_list[3];
    uint32_t index;

    for (index = 0; index < count; ++index) {
        uint32_t adjustment;

        if (Game_UnwindTypeMatches_0200c594(
                exception_type, types[index], &adjustment) != 0) {
            return 1;
        }
    }
    return 0;
}

/* 0x0200C11C..0x0200C17B. */
void Game_InstallUnwindCatchContext_0200c11c(
    GameUnwindContext *context,
    GameUnwindDescriptor *descriptor,
    const uint8_t *record,
    const uint8_t *landing_pad
)
{
    uint32_t frame_offset = Game_UnwindReadU32(record + 8);
    uint32_t *frame;

    Game_ExecuteUnwindCleanup_0200b714(context, descriptor, landing_pad);
    frame = (uint32_t *)(uintptr_t)(context->stack_base_18 + frame_offset);
    frame[0] = context->exception_object_04;
    frame[1] = context->exception_type_00;
    frame[2] = context->state_08;
    frame[5] = (uint32_t)(uintptr_t)landing_pad;
    Game_UnwindCommitDescriptor_0200ca80(
        context,
        descriptor,
        descriptor->function_start_00 + Game_UnwindReadU32(record + 4)
    );
}

/* 0x0200C17C..0x0200C357. */
const uint8_t *Game_FindUnwindLandingPad_0200c17c(
    GameUnwindContext *context,
    const GameUnwindDescriptor *source_descriptor,
    uint32_t *frame_adjustment
)
{
    GameUnwindDescriptor descriptor = *source_descriptor;

    for (;;) {
        const uint8_t *instruction =
            (const uint8_t *)(uintptr_t)descriptor.cursor_08;
        uint8_t opcode = Game_GetUnwindOpcode_0200b43c(&descriptor);

        if (opcode == 12U && instruction != NULL) {
            uint32_t candidate_type = Game_UnwindReadU32(instruction + 1);
            uint32_t adjustment;

            if (Game_UnwindTypeMatches_0200c594(
                    context->exception_type_00,
                    candidate_type,
                    &adjustment) != 0) {
                *frame_adjustment = adjustment;
                return instruction;
            }
        } else if (opcode == 15U && instruction != NULL) {
            const uint8_t *cursor = instruction + 1;
            uint32_t count;
            uint32_t ignored;

            cursor = Game_UnwindReadUnsigned(cursor, &count);
            cursor = Game_UnwindReadUnsigned(cursor, &ignored);
            if (!Game_UnwindMatchesTypeList_0200c09c(
                    context->exception_type_00,
                    (const uint32_t *)(const void *)instruction)) {
                Game_InstallUnwindCatchContext_0200c11c(
                    context, &descriptor, cursor, instruction);
            }
        } else if (opcode == 14U || opcode == UINT8_MAX) {
            Game_UnwindAbort_0200c55c();
            return instruction;
        }
        Game_AdvanceUnwindOpcode_0200b458(&descriptor);
    }
}

/* 0x0200C358..0x0200C3B3. */
void Game_WriteUnwindFrameRecord_0200c358(
    GameUnwindContext *context,
    uint32_t destination_offset,
    uint32_t object_adjustment
)
{
    uint32_t *record = (uint32_t *)(uintptr_t)(
        context->stack_base_18 + destination_offset);
    const uint8_t *type_name =
        (const uint8_t *)(uintptr_t)context->exception_type_00;

    record[0] = context->exception_object_04;
    record[1] = context->exception_type_00;
    record[2] = context->state_08;
    if (*type_name == (uint8_t)'*') {
        record[3] = (uint32_t)(uintptr_t)&record[4];
        record[4] = *(const uint32_t *)(uintptr_t)context->exception_object_04 +
            object_adjustment;
    } else {
        record[3] = context->exception_type_00 + object_adjustment;
    }
}

/* 0x0200C3B4..0x0200C4A7. */
void Game_UnwindOneFrame_0200c3b4(GameUnwindContext *context)
{
    GameUnwindDescriptor descriptor;
    const uint8_t *landing_pad;
    uint32_t adjustment = 0;
    uint32_t destination_offset;
    uint32_t program_extent;

    Game_LoadUnwindDescriptor_0200b334(
        context->program_counter_10, &descriptor);
    if (descriptor.program_04 == 0U) {
        Game_UnwindAbort_0200c55c();
        return;
    }
    Game_UnwindSynchronizeDescriptor_0200c998(context, &descriptor);
    if (context->exception_type_00 == 0U) {
        context->landing_pad_0c =
            Game_FindUnwindCatchTarget_0200bf48(context, &descriptor);
        if (context->landing_pad_0c == 0U) {
            Game_UnwindAbort_0200c55c();
            return;
        }
    } else {
        context->landing_pad_0c = 0;
    }

    landing_pad = Game_FindUnwindLandingPad_0200c17c(
        context, &descriptor, &adjustment);
    destination_offset = Game_UnwindReadU32(landing_pad + 1);
    Game_DecodeUnsignedUnwindValue_0200b24c(
        landing_pad + 5, &program_extent);
    Game_ExecuteUnwindCleanup_0200b714(context, &descriptor, landing_pad);
    Game_WriteUnwindFrameRecord_0200c358(
        context, destination_offset, adjustment);
    Game_UnwindCommitDescriptor_0200ca80(
        context, &descriptor, descriptor.function_start_00 + program_extent);
}

/* 0x0200C4A8..0x0200C4CF. */
void Game_InvokeUnwindDestructor_0200c4a8(GameUnwindContext *context)
{
    if (context->exception_type_00 != 0U && context->state_08 != 0U) {
        Game_UnwindCallDestructor(
            context->state_08, context->exception_type_00, -1);
    }
}

/* 0x0200C4D0..0x0200C4FF; initializer-table literal follows. */
void Game_RunStaticInitializerTable_0200c4d0(void)
{
    void (**initializer)(void) = gGameStaticInitializerTable;

    if (initializer == NULL) {
        return;
    }
    while (*initializer != NULL) {
        (*initializer)();
        ++initializer;
    }
}

/* 0x0200C544..0x0200C55B.  The eight-byte thunk at 0x0200C550 and its
 * literal are part of the inter-function extent before 0x0200C55C. */
void Game_HandleFatalAssertionThunk_0200c544(void)
{
    Game_HandleFatalAssertion();
}
