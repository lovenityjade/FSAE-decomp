#include "game/datalink_flow_persistence.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
    MEMBER_COUNT = 3,
    COPY_COUNT = 6,
    MEMBER_PREFIX_SIZE = 3,
    PARENT_PAYLOAD_SIZE = 40
};

static const uint32_t sExpectedMembers[MEMBER_COUNT] = {
    UINT32_C(0x11111111),
    UINT32_C(0x22222222),
    UINT32_C(0x33333333)
};

static size_t sSerializeMemberCount;
static size_t sDeserializeMemberCount;
static size_t sCopyCount;
static size_t sOwnerQueryCount;

uint32_t Game_GetDatalinkRowResourceOwner(void)
{
    ++sOwnerQueryCount;
    return UINT32_C(0x11223344);
}

void Game_SerializeDatalinkFlowMember(
    uint32_t member_address,
    uint8_t **cursor
)
{
    assert(sSerializeMemberCount < MEMBER_COUNT);
    assert(member_address == sExpectedMembers[sSerializeMemberCount]);
    **cursor = (uint8_t)(0xE0U + sSerializeMemberCount);
    *cursor += 1;
    ++sSerializeMemberCount;
}

void Game_DeserializeDatalinkFlowMember(
    uint32_t member_address,
    const uint8_t **cursor
)
{
    assert(sDeserializeMemberCount < MEMBER_COUNT);
    assert(member_address == sExpectedMembers[sDeserializeMemberCount]);
    assert(**cursor == (uint8_t)(0xE0U + sDeserializeMemberCount));
    *cursor += 1;
    ++sDeserializeMemberCount;
}

void Game_CopyDatalinkPersistenceBytes(
    const void *source,
    void *destination,
    uint32_t size
)
{
    assert(sCopyCount < COPY_COUNT);
    assert(size == 1U || size == 4U || size == 0x20U);
    memcpy(destination, source, size);
    ++sCopyCount;
}

static void SetMemberAddresses(Game_DatalinkFlowState *state)
{
    state->primary_transition_address_34 = sExpectedMembers[0];
    state->ready_transition_address_38 = sExpectedMembers[1];
    state->unavailable_transition_address_3c = sExpectedMembers[2];
}

static void TestResolvesRendererResourceIdToSlotPayload(void)
{
    Game_DatalinkRowResource resource;

    memset(&resource, 0xA5, sizeof(resource));
    sOwnerQueryCount = 0U;

    assert(Game_ResolveDatalinkRowResource(
               UINT32_C(0xDEADBEEF),
               &resource,
               0x232AU
           ) == 1);

    assert(sOwnerQueryCount == 1U);
    assert(resource.owner_address_00 == UINT32_C(0x11223344));
    assert(resource.data_address_04 == UINT32_C(0x02135158));
    assert(resource.size_08 == 0x10U);
}

static void TestSerializesMembersThenFortyByteParentPayload(void)
{
    Game_DatalinkFlowState state;
    uint8_t buffer[MEMBER_PREFIX_SIZE + PARENT_PAYLOAD_SIZE];
    uint8_t *cursor = buffer;
    uint32_t index;

    memset(&state, 0xA5, sizeof(state));
    memset(buffer, 0, sizeof(buffer));
    SetMemberAddresses(&state);
    state.selected_participant_40 = -3;
    state.phase_41 = 0x12U;
    state.suppress_redraw_42 = 0x34U;
    state.selection_dirty_43 = 0x56U;
    state.return_phase_depth_48 = UINT32_C(0x89ABCDEF);
    for (index = 0; index < 8U; ++index) {
        state.return_phase_stack_4c[index] =
            UINT32_C(0x10203040) + index;
    }
    sSerializeMemberCount = 0U;
    sCopyCount = 0U;

    Game_SerializeDatalinkFlowState(&state, &cursor);

    assert(sSerializeMemberCount == MEMBER_COUNT);
    assert(sCopyCount == COPY_COUNT);
    assert(cursor == buffer + sizeof(buffer));
    assert(buffer[0] == 0xE0U);
    assert(buffer[1] == 0xE1U);
    assert(buffer[2] == 0xE2U);
    assert(buffer[3] == (uint8_t)-3);
    assert(buffer[4] == 0x12U);
    assert(buffer[5] == 0x34U);
    assert(buffer[6] == 0x56U);
    assert(memcmp(&buffer[7], &state.return_phase_depth_48, 4U) == 0);
    assert(memcmp(&buffer[11], state.return_phase_stack_4c, 0x20U) == 0);
}

static void TestDeserializesSamePayloadAndPreservesTransientGap(void)
{
    Game_DatalinkFlowState state;
    uint8_t buffer[MEMBER_PREFIX_SIZE + PARENT_PAYLOAD_SIZE];
    const uint8_t *cursor = buffer;
    uint32_t expected_stack[8];
    uint32_t expected_depth = UINT32_C(0x76543210);
    uint32_t index;

    memset(&state, 0xA5, sizeof(state));
    memset(buffer, 0, sizeof(buffer));
    SetMemberAddresses(&state);
    buffer[0] = 0xE0U;
    buffer[1] = 0xE1U;
    buffer[2] = 0xE2U;
    buffer[3] = 0xFEU;
    buffer[4] = 0x21U;
    buffer[5] = 0x43U;
    buffer[6] = 0x65U;
    memcpy(&buffer[7], &expected_depth, sizeof(expected_depth));
    for (index = 0; index < 8U; ++index) {
        expected_stack[index] = UINT32_C(0xA0B0C000) + index;
    }
    memcpy(&buffer[11], expected_stack, sizeof(expected_stack));
    sDeserializeMemberCount = 0U;
    sCopyCount = 0U;

    Game_DeserializeDatalinkFlowState(&state, &cursor);

    assert(sDeserializeMemberCount == MEMBER_COUNT);
    assert(sCopyCount == COPY_COUNT);
    assert(cursor == buffer + sizeof(buffer));
    assert((uint8_t)state.selected_participant_40 == 0xFEU);
    assert(state.phase_41 == 0x21U);
    assert(state.suppress_redraw_42 == 0x43U);
    assert(state.selection_dirty_43 == 0x65U);
    assert(state.unknown_44 == 0xA5U);
    assert(state.phase_timer_45 == 0xA5U);
    assert(state.return_phase_depth_48 == expected_depth);
    assert(memcmp(state.return_phase_stack_4c,
                  expected_stack, sizeof(expected_stack)) == 0);
}

int main(void)
{
    TestResolvesRendererResourceIdToSlotPayload();
    TestSerializesMembersThenFortyByteParentPayload();
    TestDeserializesSamePayloadAndPreservesTransientGap();
    return 0;
}
