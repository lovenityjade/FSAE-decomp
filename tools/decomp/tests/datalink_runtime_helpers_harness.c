#include "game/datalink_controller.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

typedef enum DatalinkStateEvent {
    DATALINK_STATE_EVENT_ENTER_OLD,
    DATALINK_STATE_EVENT_EXIT_OLD,
    DATALINK_STATE_EVENT_ENTER_NEW,
    DATALINK_STATE_EVENT_EXIT_NEW
} DatalinkStateEvent;

static DatalinkStateEvent sStateEvents[8];
static size_t sStateEventCount;
static Game_DatalinkState sOldState;
static Game_DatalinkState sNewState;

void MI_CpuCopy(const void *source, void *destination, uint32_t size)
{
    memcpy(destination, source, size);
}

static void RecordStateEvent(DatalinkStateEvent event)
{
    assert(sStateEventCount < sizeof(sStateEvents) / sizeof(sStateEvents[0]));
    sStateEvents[sStateEventCount++] = event;
}

static void OldStateEnter(Game_DatalinkState *state)
{
    assert(state == &sOldState);
    RecordStateEvent(DATALINK_STATE_EVENT_ENTER_OLD);
}

static void OldStateExit(Game_DatalinkState *state)
{
    assert(state == &sOldState);
    RecordStateEvent(DATALINK_STATE_EVENT_EXIT_OLD);
}

static void NewStateEnter(Game_DatalinkState *state)
{
    assert(state == &sNewState);
    RecordStateEvent(DATALINK_STATE_EVENT_ENTER_NEW);
}

static void NewStateExit(Game_DatalinkState *state)
{
    assert(state == &sNewState);
    RecordStateEvent(DATALINK_STATE_EVENT_EXIT_NEW);
}

static const Game_DatalinkStateVTable sOldStateVTable = {
    NULL,
    NULL,
    NULL,
    OldStateEnter,
    OldStateExit
};
static const Game_DatalinkStateVTable sNewStateVTable = {
    NULL,
    NULL,
    NULL,
    NewStateEnter,
    NewStateExit
};

static void TestTweenProgress(void)
{
    Game_DatalinkTweenRecord tween;
    Game_DatalinkTweenRecord before;

    memset(&tween, 0xa5, sizeof(tween));
    tween.duration_fx_28 = 0;
    tween.elapsed_fx_2c = INT32_MIN;
    before = tween;
    assert(Game_GetDatalinkTweenProgress(&tween) == 0x1000);
    assert(memcmp(&before, &tween, sizeof(tween)) == 0);

    tween.duration_fx_28 = 5 * 0x1000;
    tween.elapsed_fx_2c = 0;
    assert(Game_GetDatalinkTweenProgress(&tween) == 0);
    tween.elapsed_fx_2c = 0x1000;
    assert(Game_GetDatalinkTweenProgress(&tween) == 0x333);
    tween.elapsed_fx_2c = 5 * 0x1000;
    assert(Game_GetDatalinkTweenProgress(&tween) == 0x1000);

    tween.duration_fx_28 = 4 * 0x1000;
    tween.elapsed_fx_2c = -0x1001;
    assert(Game_GetDatalinkTweenProgress(&tween) == -0x400);
}

static void TestStateStackInit(void)
{
    Game_DatalinkStateStack stack;
    uintptr_t state_addresses[8];

    memset(&stack, 0xa5, sizeof(stack));
    memcpy(state_addresses, stack.state_addresses_0c,
           sizeof(state_addresses));

    Game_DatalinkStateStack_Init_020acd98(&stack);
    assert(stack.state_code_04 == 7);
    assert(stack.result_08 == 0);
    assert(stack.depth_2c == 0);
    assert(memcmp(stack.state_addresses_0c, state_addresses,
                  sizeof(state_addresses)) == 0);
    assert(stack.unknown_00[0] == 0xa5 && stack.unknown_00[3] == 0xa5);
    assert(stack.unknown_2d[0] == 0xa5 && stack.unknown_2d[2] == 0xa5);
}

static void ResetStateHarness(Game_DatalinkStateStack *stack)
{
    memset(stack, 0xa5, sizeof(*stack));
    memset(&sOldState, 0, sizeof(sOldState));
    memset(&sNewState, 0, sizeof(sNewState));
    memset(sStateEvents, 0, sizeof(sStateEvents));
    sStateEventCount = 0;
    sOldState.vtable_00 = &sOldStateVTable;
    sNewState.vtable_00 = &sNewStateVTable;
}

static void TestEnterRootState(void)
{
    Game_DatalinkStateStack stack;

    ResetStateHarness(&stack);
    Game_DatalinkStateStack_EnterRootState(&stack, &sOldState);

    assert(stack.state_addresses_0c[0] == (uintptr_t)&sOldState);
    assert(stack.depth_2c == 0);
    assert(sOldState.owner_stack_08 == &stack);
    assert(sStateEventCount == 1);
    assert(sStateEvents[0] == DATALINK_STATE_EVENT_ENTER_OLD);
}

static void TestPushStateExitsCurrentAndEntersNew(void)
{
    Game_DatalinkStateStack stack;

    ResetStateHarness(&stack);
    stack.depth_2c = 0;
    stack.state_addresses_0c[0] = (uintptr_t)&sOldState;
    sOldState.owner_stack_08 = &stack;

    Game_DatalinkStateStack_PushState(&stack, &sNewState);

    assert(sStateEventCount == 2);
    assert(sStateEvents[0] == DATALINK_STATE_EVENT_EXIT_OLD);
    assert(sStateEvents[1] == DATALINK_STATE_EVENT_ENTER_NEW);
    assert(sOldState.owner_stack_08 == NULL);
    assert(stack.depth_2c == 1);
    assert(stack.state_addresses_0c[1] == (uintptr_t)&sNewState);
    assert(sNewState.owner_stack_08 == &stack);
}

static void TestPopStateExitsCurrentAndRestoresPrevious(void)
{
    Game_DatalinkStateStack stack;

    ResetStateHarness(&stack);
    Game_DatalinkStateStack_EnterRootState(&stack, &sOldState);
    Game_DatalinkStateStack_PushState(&stack, &sNewState);
    sStateEventCount = 0;
    memset(sStateEvents, 0, sizeof(sStateEvents));

    Game_DatalinkStateStack_PopState(&stack);

    assert(sStateEventCount == 2);
    assert(sStateEvents[0] == DATALINK_STATE_EVENT_EXIT_NEW);
    assert(sStateEvents[1] == DATALINK_STATE_EVENT_ENTER_OLD);
    assert(stack.depth_2c == 0);
    assert(stack.state_addresses_0c[0] == (uintptr_t)&sOldState);
    assert(stack.state_addresses_0c[1] == (uintptr_t)&sNewState);
    assert(sNewState.owner_stack_08 == NULL);
    assert(sOldState.owner_stack_08 == NULL);
}

static void TestStateStackPersistence(void)
{
    Game_DatalinkStateStack source;
    Game_DatalinkStateStack destination;
    uint8_t buffer[64];
    uint8_t *write_cursor = buffer;
    const uint8_t *read_cursor = buffer;
    uintptr_t preserved_current;
    unsigned int index;

    memset(&source, 0xa5, sizeof(source));
    memset(buffer, 0xcc, sizeof(buffer));
    source.state_code_04 = UINT32_C(0x11223344);
    source.result_08 = UINT32_C(0x55667788);
    source.depth_2c = 3;
    source.state_addresses_0c[0] = UINT32_C(0x01234567);
    source.state_addresses_0c[1] = UINT32_C(0x89abcdef);
    source.state_addresses_0c[2] = UINT32_C(0x76543210);
    source.state_addresses_0c[3] = UINT32_C(0xdeadbeef);

    Game_DatalinkStateStack_Serialize(&source, &write_cursor);
    assert(write_cursor == buffer + 21);
    assert(memcmp(buffer, &source.state_code_04, 4) == 0);
    assert(memcmp(buffer + 4, &source.result_08, 4) == 0);
    assert(buffer[8] == 3);
    for (index = 0; index < 3; ++index) {
        uint32_t address = (uint32_t)source.state_addresses_0c[index];

        assert(memcmp(buffer + 9 + index * 4, &address, 4) == 0);
    }
    assert(buffer[21] == 0xcc);

    memset(&destination, 0xa5, sizeof(destination));
    preserved_current = destination.state_addresses_0c[3];
    Game_DatalinkStateStack_Deserialize(&destination, &read_cursor);
    assert(read_cursor == buffer + 21);
    assert(destination.state_code_04 == source.state_code_04);
    assert(destination.result_08 == source.result_08);
    assert(destination.depth_2c == 3);
    for (index = 0; index < 3; ++index) {
        assert(destination.state_addresses_0c[index] ==
               source.state_addresses_0c[index]);
    }
    assert(destination.state_addresses_0c[3] == preserved_current);

    source.depth_2c = 0;
    write_cursor = buffer;
    memset(buffer, 0xcc, sizeof(buffer));
    Game_DatalinkStateStack_Serialize(&source, &write_cursor);
    assert(write_cursor == buffer + 9);
    assert(buffer[9] == 0xcc);

    memset(&destination, 0xa5, sizeof(destination));
    preserved_current = destination.state_addresses_0c[0];
    read_cursor = buffer;
    Game_DatalinkStateStack_Deserialize(&destination, &read_cursor);
    assert(read_cursor == buffer + 9);
    assert(destination.depth_2c == 0);
    assert(destination.state_addresses_0c[0] == preserved_current);
}

int main(void)
{
    TestTweenProgress();
    TestStateStackInit();
    TestEnterRootState();
    TestPushStateExitsCurrentAndEntersNew();
    TestPopStateExitsCurrentAndRestoresPrevious();
    TestStateStackPersistence();
    return 0;
}
