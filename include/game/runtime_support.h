#ifndef FSAE_GAME_RUNTIME_SUPPORT_H
#define FSAE_GAME_RUNTIME_SUPPORT_H

#include "game/runtime_unwind.h"

#include <stdbool.h>
#include <stdint.h>

typedef void (*GameRuntimeObjectCallback)(void *object);

void Game_UnwindAbort_0200c55c(void);
int Game_UnwindTypeMatches_0200c594(
    uint32_t active_type,
    uint32_t candidate_type,
    uint32_t *adjustment
);
void Game_OperatorDelete(void *object);
void Game_DestroyObjectRangeReverse_0200c75c(
    uint8_t *begin,
    uint8_t *end,
    uint32_t element_size,
    GameRuntimeObjectCallback destructor
);
void Game_ConstructObjectArray_0200c7b8(
    uint8_t *begin,
    uint32_t count,
    uint32_t element_size,
    GameRuntimeObjectCallback constructor,
    GameRuntimeObjectCallback cleanup
);
void Game_DestroyObjectArray_0200c868(
    uint8_t *begin,
    uint32_t count,
    uint32_t element_size,
    GameRuntimeObjectCallback destructor
);
void Game_RuntimeBaseDestructor_0200c8c4(void *object);
void *Game_RuntimeBaseDestructorThunk_0200c8c8(void *object);
void *Game_RuntimeDeletingDestructor_0200c8dc(void *object);
void *Game_RuntimeDerivedDestructor_0200c8f8(void *object);
void *Game_RuntimeDerivedDestructorThunk_0200c90c(void *object);
void *Game_RuntimeDerivedDeletingDestructor_0200c920(void *object);

uint64_t Game_AddDoubleBits_0200cafc(
    uint32_t left_low,
    uint32_t left_high,
    uint32_t right_low,
    uint32_t right_high
);
uint64_t Game_SubtractDoubleBits_0200ce2c(
    uint32_t left_low,
    uint32_t left_high,
    uint32_t right_low,
    uint32_t right_high
);
uint64_t Game_MultiplyDoubleBits_0200d1e0(
    uint32_t left_low,
    uint32_t left_high,
    uint32_t right_low,
    uint32_t right_high
);
bool Game_DoubleBitsLessThan_0200d544(
    uint32_t left_low,
    uint32_t left_high,
    uint32_t right_low,
    uint32_t right_high
);
bool Game_DoubleBitsEqual_0200d5e0(
    uint32_t left_low,
    uint32_t left_high,
    uint32_t right_low,
    uint32_t right_high
);
bool Game_DoubleBitsNotEqual_0200d66c(
    uint32_t left_low,
    uint32_t left_high,
    uint32_t right_low,
    uint32_t right_high
);
uint32_t Game_Int32ToFloatBits_0200d6f8(int32_t value);
uint64_t Game_DivideUint64_0200d900(
    uint32_t dividend_low,
    uint32_t dividend_high,
    uint32_t divisor_low,
    uint32_t divisor_high
);
uint64_t Game_RemainderUint64_0200d90c(
    uint32_t dividend_low,
    uint32_t dividend_high,
    uint32_t divisor_low,
    uint32_t divisor_high
);
uint64_t Game_DivideInt32WithRemainder_0200d948(
    int32_t dividend,
    int32_t divisor
);
uint32_t Game_DivideUint32Checked_0200db54(
    uint32_t dividend,
    uint32_t divisor
);
uint32_t Game_DivideUint32_0200db5c(uint32_t dividend, uint32_t divisor);
uint32_t Game_FloatBitsToUint32_0200dd38(uint32_t value_bits);
uint64_t Game_DoubleBitsToUint64_0200dd78(
    uint32_t value_low,
    uint32_t value_high
);
void Game_RuntimeNoOp_0200de04(void);

#endif /* FSAE_GAME_RUNTIME_SUPPORT_H */
