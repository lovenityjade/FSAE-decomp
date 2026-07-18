#include "game/runtime_support.h"

#include <stddef.h>
#include <stdint.h>

extern void (*gGameUnwindAbortHandler)(void);
extern void Game_FreeMemory_02005628(void *memory);

/* 0x0200C55C..0x0200C573; abort-handler literal follows. */
void Game_UnwindAbort_0200c55c(void)
{
    gGameUnwindAbortHandler();
}

static bool Game_RuntimeTypeIsQualifiedPointer(const char *type)
{
    const char *cursor;

    if (*type != 'P') {
        return false;
    }
    cursor = type + 1;
    if (*cursor == 'V') {
        ++cursor;
    }
    if (*cursor == 'K') {
        ++cursor;
    }
    return *cursor == 'v';
}

/* 0x0200C594..0x0200C747.  Matches the CodeWarrior compact RTTI spelling,
 * including pointer cv-qualification and the !...! adjustment encoding. */
int Game_UnwindTypeMatches_0200c594(
    uint32_t active_type_address,
    uint32_t candidate_type_address,
    uint32_t *adjustment
)
{
    const char *active = (const char *)(uintptr_t)active_type_address;
    const char *candidate =
        (const char *)(uintptr_t)candidate_type_address;

    *adjustment = 0;
    if (candidate == NULL) {
        return 1;
    }
    if (Game_RuntimeTypeIsQualifiedPointer(candidate) &&
        (*active == 'P' || *active == '*')) {
        return 1;
    }

    if (*active == '!' || *active == '*') {
        const unsigned char *left = (const unsigned char *)active + 1;
        const unsigned char *right = (const unsigned char *)candidate + 1;

        if (*active != *candidate) {
            return 0;
        }
        for (;;) {
            while (*left == *right) {
                unsigned char value = *left++;

                ++right;
                if (value == (unsigned char)'!') {
                    uint32_t parsed = 0;

                    while (*left != (unsigned char)'!') {
                        parsed = parsed * 10U +
                            (uint32_t)(*left++ - (unsigned char)'0');
                    }
                    *adjustment = parsed;
                    return 1;
                }
            }
            while (*left++ != (unsigned char)'!') {
            }
            while (*left++ != (unsigned char)'!') {
            }
            if (*left == 0U) {
                return 0;
            }
            right = (const unsigned char *)candidate + 1;
        }
    }

    for (;;) {
        char active_code = *active;

        if ((active_code != 'P' && active_code != 'R') ||
            active_code != *candidate) {
            while (active_code == *candidate) {
                if (active_code == '\0') {
                    return 1;
                }
                active_code = *++active;
                ++candidate;
            }
            return 0;
        }

        ++active;
        ++candidate;
        if (*candidate == 'K') {
            ++candidate;
            if (*active == 'K') {
                ++active;
            }
        }
        if (*active == 'K') {
            return 0;
        }
        if (*candidate == 'V') {
            if (*active == 'V') {
                ++active;
            }
            ++candidate;
        }
        if (*active == 'V') {
            return 0;
        }
    }
}

/* 0x0200C748..0x0200C75B. */
void Game_OperatorDelete(void *object)
{
    if (object != NULL) {
        Game_FreeMemory_02005628(object);
    }
}

/* 0x0200C75C..0x0200C7B7.  The catalogued body has a non-contiguous epilogue
 * tail at 0x0200C7A4..0x0200C7B7. */
void Game_DestroyObjectRangeReverse_0200c75c(
    uint8_t *begin,
    uint8_t *end,
    uint32_t element_size,
    GameRuntimeObjectCallback destructor
)
{
    while (begin < end) {
        end -= element_size;
        destructor(end);
    }
}

/* 0x0200C7B8..0x0200C867.  Construction and its exception-cleanup tail share
 * one function extent in the retail runtime. */
void Game_ConstructObjectArray_0200c7b8(
    uint8_t *begin,
    uint32_t count,
    uint32_t element_size,
    GameRuntimeObjectCallback constructor,
    GameRuntimeObjectCallback cleanup
)
{
    uint32_t index;

    (void)cleanup;
    if (constructor == NULL) {
        return;
    }
    for (index = 0; index < count; ++index) {
        constructor(begin + index * element_size);
    }
}

/* 0x0200C868..0x0200C8C3; its epilogue occupies the non-contiguous tail. */
void Game_DestroyObjectArray_0200c868(
    uint8_t *begin,
    uint32_t count,
    uint32_t element_size,
    GameRuntimeObjectCallback destructor
)
{
    uint8_t *end;

    if (destructor == NULL) {
        return;
    }
    end = begin + count * element_size;
    while (count-- != 0U) {
        end -= element_size;
        destructor(end);
    }
}

/* 0x0200C8C4..0x0200C8C7. */
void Game_RuntimeBaseDestructor_0200c8c4(void *object)
{
    (void)object;
}

/* 0x0200C8C8..0x0200C8DB. */
void *Game_RuntimeBaseDestructorThunk_0200c8c8(void *object)
{
    Game_RuntimeBaseDestructor_0200c8c4(object);
    return object;
}

/* 0x0200C8DC..0x0200C8F7. */
void *Game_RuntimeDeletingDestructor_0200c8dc(void *object)
{
    Game_RuntimeBaseDestructor_0200c8c4(object);
    Game_OperatorDelete(object);
    return object;
}

/* 0x0200C8F8..0x0200C90B. */
void *Game_RuntimeDerivedDestructor_0200c8f8(void *object)
{
    Game_RuntimeBaseDestructor_0200c8c4(object);
    return object;
}

/* 0x0200C90C..0x0200C91F. */
void *Game_RuntimeDerivedDestructorThunk_0200c90c(void *object)
{
    Game_RuntimeDerivedDestructor_0200c8f8(object);
    return object;
}

/* 0x0200C920..0x0200C93B. */
void *Game_RuntimeDerivedDeletingDestructor_0200c920(void *object)
{
    Game_RuntimeDerivedDestructor_0200c8f8(object);
    Game_OperatorDelete(object);
    return object;
}

/* 0x0200C93C..0x0200C997. */
uint32_t Game_UnwindResolveContext_0200c93c(
    GameUnwindContext *context,
    GameUnwindDescriptor *descriptor
)
{
    uint8_t *raw = (uint8_t *)(void *)context;
    uint32_t stack_adjustment = *(uint32_t *)(void *)(raw + 0x60);
    uint16_t register_mask = *(uint16_t *)(void *)(raw + 0x68);
    uint32_t *restore = (uint32_t *)(uintptr_t)(
        context->stack_base_18 + stack_adjustment -
        (raw[0x6a] != 0U ? 16U : 0U));
    int register_index;

    (void)descriptor;
    for (register_index = 15; register_index >= 0; --register_index) {
        if ((register_mask & (uint16_t)(1U << register_index)) != 0U) {
            context->registers_1c[register_index] = *--restore;
        }
    }
    context->reserved_14 = context->stack_base_18 + stack_adjustment;
    return context->registers_1c[14];
}

/* 0x0200C998..0x0200CA37. */
void Game_UnwindSynchronizeDescriptor_0200c998(
    GameUnwindContext *context,
    GameUnwindDescriptor *descriptor
)
{
    uint8_t *raw = (uint8_t *)(void *)context;
    const uint8_t *program =
        (const uint8_t *)(uintptr_t)descriptor->program_04;
    uint8_t flags = program[0];
    uint32_t decoded;
    const uint8_t *cursor;

    raw[0x6b] = (flags & 0x40U) != 0U;
    raw[0x6c] = (flags & 0x80U) != 0U;
    raw[0x6a] = (flags & 0x20U) != 0U;
    *(uint16_t *)(void *)(raw + 0x68) =
        (uint16_t)((uint16_t)program[1] << 4) | UINT16_C(0x4000);
    cursor = Game_DecodeUnsignedUnwindValue_0200b24c(program + 2, &decoded);
    *(uint32_t *)(void *)(raw + 0x60) = decoded;
    if ((flags & 0x40U) != 0U) {
        cursor = Game_DecodeUnsignedUnwindValue_0200b24c(cursor, &decoded);
        *(uint32_t *)(void *)(raw + 0x64) = decoded;
    }
    (void)cursor;

    if ((flags & 0x40U) == 0U) {
        context->stack_base_18 = context->reserved_14;
    } else if ((flags & 0x80U) == 0U) {
        context->stack_base_18 = context->registers_1c[11];
    } else {
        context->stack_base_18 = context->registers_1c[7];
    }
}

/* 0x0200CA38..0x0200CA57; two identical range-bound literals follow. */
int Game_UnwindLocateTables_0200ca38(
    GameUnwindDescriptor *descriptor,
    uint32_t address
)
{
    (void)address;
    descriptor->range_begin_0c = UINT32_C(0x020c8cfc);
    descriptor->range_end_10 = UINT32_C(0x020c8cfc);
    return 1;
}

/* 0x0200CA58..0x0200CA7F. */
const uint8_t *Game_UnwindGetProgram_0200ca58(uint32_t program_address)
{
    const uint8_t *program = (const uint8_t *)(uintptr_t)program_address;
    uint8_t flags = *program++;
    uint32_t ignored;

    ++program;
    program = Game_DecodeUnsignedUnwindValue_0200b24c(program, &ignored);
    if ((flags & 0x40U) != 0U) {
        program = Game_DecodeUnsignedUnwindValue_0200b24c(program, &ignored);
    }
    return program;
}

/* 0x0200CA80..0x0200CAFB.  The catalogued 48-byte entry is followed by its
 * non-contiguous register-restore tail before the next standalone function. */
void Game_UnwindCommitDescriptor_0200ca80(
    GameUnwindContext *context,
    GameUnwindDescriptor *descriptor,
    uint32_t program
)
{
    void (*landing_pad)(void) = (void (*)(void))(uintptr_t)program;

    (void)descriptor;
    context->stack_base_18 -=
        *(uint32_t *)(void *)((uint8_t *)(void *)context + 0x64);
    landing_pad();
}
