#ifndef FSAE_GAME_RUNTIME_UNWIND_H
#define FSAE_GAME_RUNTIME_UNWIND_H

#include "game/format_runtime_wide.h"

#include <stdint.h>

typedef void (*GameUnwindDestructor)(void *object, int reason);

typedef struct GameUnwindCodeRange {
    uint32_t start_00;
    uint32_t extent_flags_04;
    uint32_t program_08;
} GameUnwindCodeRange;

typedef struct GameUnwindDescriptor {
    uint32_t function_start_00;
    uint32_t program_04;
    uint32_t cursor_08;
    uint32_t range_begin_0c;
    uint32_t range_end_10;
    uint32_t reserved_14;
} GameUnwindDescriptor;

/* Exact 28-word execution-context image copied by 0x0200BF48/0x0200C17C. */
typedef struct GameUnwindContext {
    uint32_t exception_type_00;
    uint32_t exception_object_04;
    uint32_t state_08;
    uint32_t landing_pad_0c;
    uint32_t program_counter_10;
    uint32_t reserved_14;
    uint32_t stack_base_18;
    uint32_t registers_1c[21];
} GameUnwindContext;

#if UINTPTR_MAX == UINT32_MAX
_Static_assert(sizeof(GameUnwindCodeRange) == 0x0c, "unwind range ABI");
_Static_assert(sizeof(GameUnwindDescriptor) == 0x18, "unwind descriptor ABI");
_Static_assert(sizeof(GameUnwindContext) == 0x70, "unwind context ABI");
#endif

void Game_RuntimeDecimalFromDigitString_0200a75c(
    GameRuntimeDecimal *decimal,
    const uint8_t *digits,
    int16_t exponent
);
void Game_RuntimeDecimalPowerOfTwo_0200a7f8(
    GameRuntimeDecimal *decimal,
    int32_t exponent
);
void Game_RuntimeDecimalFromDouble_0200ab78(
    GameRuntimeDecimal *decimal,
    uint32_t value_low,
    uint32_t value_high
);
void Game_ConvertDoubleToDecimal_0200acf8(
    const void *conversion_spec,
    uint32_t value_low,
    uint32_t value_high,
    GameFormatDecimal *decimal
);
uint64_t Game_CopyDoubleSign_0200ada4(
    uint32_t magnitude_low,
    uint32_t magnitude_high,
    uint32_t sign_low,
    uint32_t sign_high
);
uint64_t Game_AbsoluteDouble_0200adcc(uint32_t value_low, uint32_t value_high);
uint64_t Game_NormalizeDoubleFraction_0200adec(
    uint32_t value_low,
    uint32_t value_high,
    int32_t *exponent
);
uint64_t Game_ScaleDoubleByPowerOfTwo_0200aeac(
    uint32_t value_low,
    uint32_t value_high,
    int32_t exponent
);

void Game_RuntimeNoOpThunk_0200b10c(void);
void Game_RuntimeInputBegin_0200b118(void *buffer);
void Game_RuntimeInputEnd_0200b11c(void *buffer);
int Game_RuntimeReadByte_0200b120(void);
void Game_RuntimeNoOp_0200b128(void);
int Game_ReadRuntimeLine_0200b12c(void *context, char *buffer, uint32_t *count);
int Game_FinalizeRuntimeLine_0200b17c(
    void *context,
    char *buffer,
    uint32_t *count,
    int final_character
);

const uint8_t *Game_DecodeSignedUnwindValue_0200b1c8(
    const uint8_t *cursor,
    int32_t *value
);
const uint8_t *Game_DecodeUnsignedUnwindValue_0200b24c(
    const uint8_t *cursor,
    uint32_t *value
);
const GameUnwindCodeRange *Game_FindUnwindCodeRange_0200b2d0(
    const GameUnwindCodeRange *ranges,
    int32_t count,
    uint32_t address
);
void Game_LoadUnwindDescriptor_0200b334(
    uint32_t address,
    GameUnwindDescriptor *descriptor
);
uint8_t Game_GetUnwindOpcode_0200b43c(const GameUnwindDescriptor *descriptor);
uint8_t Game_AdvanceUnwindOpcode_0200b458(GameUnwindDescriptor *descriptor);
void Game_ExecuteUnwindCleanup_0200b714(
    GameUnwindContext *context,
    GameUnwindDescriptor *descriptor,
    const uint8_t *stop
);
uint32_t Game_FindUnwindCatchTarget_0200bf48(
    GameUnwindContext *context,
    const GameUnwindDescriptor *descriptor
);
int Game_UnwindMatchesTypeList_0200c09c(
    uint32_t exception_type,
    const uint32_t *encoded_list
);
void Game_InstallUnwindCatchContext_0200c11c(
    GameUnwindContext *context,
    GameUnwindDescriptor *descriptor,
    const uint8_t *record,
    const uint8_t *landing_pad
);
const uint8_t *Game_FindUnwindLandingPad_0200c17c(
    GameUnwindContext *context,
    const GameUnwindDescriptor *descriptor,
    uint32_t *frame_adjustment
);
void Game_WriteUnwindFrameRecord_0200c358(
    GameUnwindContext *context,
    uint32_t destination_offset,
    uint32_t object_adjustment
);
void Game_UnwindOneFrame_0200c3b4(GameUnwindContext *context);
void Game_InvokeUnwindDestructor_0200c4a8(GameUnwindContext *context);
void Game_RunStaticInitializerTable_0200c4d0(void);
void Game_HandleFatalAssertionThunk_0200c544(void);

#endif /* FSAE_GAME_RUNTIME_UNWIND_H */
