#include "game/datalink_controller.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_FX_ONE = 0x1000,
    DATALINK_FRACTION_BITS = 12,
    DATALINK_STATE_STACK_INITIAL_CODE = 7,
    DATALINK_SERIALIZED_WORD_SIZE = 4,
    DATALINK_SERIALIZED_DEPTH_SIZE = 1
};

extern void MI_CpuCopy(
    const void *source,
    void *destination,
    uint32_t size);

typedef char GameDatalinkTweenDurationOffsetCheck[
    offsetof(Game_DatalinkTweenRecord, duration_fx_28) == 0x28 ? 1 : -1];
typedef char GameDatalinkTweenElapsedOffsetCheck[
    offsetof(Game_DatalinkTweenRecord, elapsed_fx_2c) == 0x2c ? 1 : -1];
typedef char GameDatalinkStateStackCodeOffsetCheck[
    offsetof(Game_DatalinkStateStack, state_code_04) == 0x04 ? 1 : -1];
typedef char GameDatalinkStateStackResultOffsetCheck[
    offsetof(Game_DatalinkStateStack, result_08) == 0x08 ? 1 : -1];
#if UINTPTR_MAX == UINT32_MAX
typedef char GameDatalinkStateStackDepthOffsetCheck[
    offsetof(Game_DatalinkStateStack, depth_2c) == 0x2c ? 1 : -1];
typedef char GameDatalinkStateOwnerOffsetCheck[
    offsetof(Game_DatalinkState, owner_stack_08) == 0x08 ? 1 : -1];
#endif

/* 0x020acd74..0x020acd97 */
int32_t Game_GetDatalinkTweenProgress(
    const Game_DatalinkTweenRecord *tween)
{
    if (tween->duration_fx_28 == 0) {
        return DATALINK_FX_ONE;
    }
    return tween->elapsed_fx_2c /
        (tween->duration_fx_28 >> DATALINK_FRACTION_BITS);
}

/* 0x020acd98..0x020acdaf */
void Game_DatalinkStateStack_Init_020acd98(
    Game_DatalinkStateStack *stack)
{
    stack->state_code_04 = DATALINK_STATE_STACK_INITIAL_CODE;
    stack->result_08 = 0;
    stack->depth_2c = 0;
}

static Game_DatalinkState *Game_DatalinkStateStack_GetCurrentState(
    const Game_DatalinkStateStack *stack)
{
    return (Game_DatalinkState *)(uintptr_t)
        stack->state_addresses_0c[stack->depth_2c];
}

static void Game_DatalinkState_Enter(Game_DatalinkState *state)
{
    state->vtable_00->enter_0c(state);
}

static void Game_DatalinkState_Exit(Game_DatalinkState *state)
{
    state->vtable_00->exit_10(state);
}

/* 0x020acdb0..0x020acdd7 */
void Game_DatalinkStateStack_EnterRootState(
    Game_DatalinkStateStack *stack,
    Game_DatalinkState *state)
{
    stack->state_addresses_0c[0] = (uintptr_t)state;
    stack->depth_2c = 0;
    state->owner_stack_08 = stack;
    Game_DatalinkState_Enter(state);
}

/* 0x020acdd8..0x020ace3f */
void Game_DatalinkStateStack_PushState(
    Game_DatalinkStateStack *stack,
    Game_DatalinkState *state)
{
    Game_DatalinkState *current =
        Game_DatalinkStateStack_GetCurrentState(stack);

    Game_DatalinkState_Exit(current);
    current->owner_stack_08 = NULL;
    ++stack->depth_2c;
    stack->state_addresses_0c[stack->depth_2c] = (uintptr_t)state;
    state->owner_stack_08 = stack;
    Game_DatalinkState_Enter(state);
}

/* 0x020ace40..0x020ace9b */
void Game_DatalinkStateStack_PopState(
    Game_DatalinkStateStack *stack)
{
    Game_DatalinkState *current =
        Game_DatalinkStateStack_GetCurrentState(stack);

    Game_DatalinkState_Exit(current);
    current->owner_stack_08 = NULL;
    --stack->depth_2c;
    Game_DatalinkState_Enter(
        Game_DatalinkStateStack_GetCurrentState(stack));
}

static void Game_DatalinkStateStack_WriteBytes(
    uint8_t **cursor,
    const void *source,
    uint32_t size)
{
    MI_CpuCopy(source, *cursor, size);
    *cursor += size;
}

static void Game_DatalinkStateStack_ReadBytes(
    const uint8_t **cursor,
    void *destination,
    uint32_t size)
{
    MI_CpuCopy(*cursor, destination, size);
    *cursor += size;
}

/* 0x020ace9c..0x020acf5b */
void Game_DatalinkStateStack_Serialize(
    const Game_DatalinkStateStack *stack,
    uint8_t **cursor)
{
    uint32_t index;

    Game_DatalinkStateStack_WriteBytes(
        cursor, &stack->state_code_04, DATALINK_SERIALIZED_WORD_SIZE);
    Game_DatalinkStateStack_WriteBytes(
        cursor, &stack->result_08, DATALINK_SERIALIZED_WORD_SIZE);
    Game_DatalinkStateStack_WriteBytes(
        cursor, &stack->depth_2c, DATALINK_SERIALIZED_DEPTH_SIZE);

    for (index = 0; index < (uint32_t)stack->depth_2c; ++index) {
        uint32_t state_address =
            (uint32_t)stack->state_addresses_0c[index];

        Game_DatalinkStateStack_WriteBytes(
            cursor, &state_address, DATALINK_SERIALIZED_WORD_SIZE);
    }
}

/* 0x020acf5c..0x020ad01f */
void Game_DatalinkStateStack_Deserialize(
    Game_DatalinkStateStack *stack,
    const uint8_t **cursor)
{
    uint32_t index;

    Game_DatalinkStateStack_ReadBytes(
        cursor, &stack->state_code_04, DATALINK_SERIALIZED_WORD_SIZE);
    Game_DatalinkStateStack_ReadBytes(
        cursor, &stack->result_08, DATALINK_SERIALIZED_WORD_SIZE);
    Game_DatalinkStateStack_ReadBytes(
        cursor, &stack->depth_2c, DATALINK_SERIALIZED_DEPTH_SIZE);

    for (index = 0; index < (uint32_t)stack->depth_2c; ++index) {
        uint32_t state_address;

        Game_DatalinkStateStack_ReadBytes(
            cursor, &state_address, DATALINK_SERIALIZED_WORD_SIZE);
        stack->state_addresses_0c[index] = (uintptr_t)state_address;
    }
}
