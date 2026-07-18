#include "game/datalink_flow_persistence.h"

#include <stddef.h>

enum {
    DATALINK_ROW_RESOURCE_ID_BASE = 0x2328,
    DATALINK_ROW_RECORD_ADDRESS = 0x021350B0,
    DATALINK_ROW_RECORD_STRIDE = 0x50,
    DATALINK_ROW_RESOURCE_DATA_OFFSET = 8,
    DATALINK_ROW_RESOURCE_SIZE = 0x10,
    DATALINK_SERIALIZED_BYTE_COUNT = 4,
    DATALINK_RETURN_STACK_BYTE_COUNT = 0x20
};

/* Opaque 0x0208FC6C resource-owner query. */
extern uint32_t Game_GetDatalinkRowResourceOwner(void);

/* Host-safe dispatch of member virtual method +0x14. */
extern void Game_SerializeDatalinkFlowMember(
    uint32_t member_address,
    uint8_t **cursor
);

/* Host-safe dispatch of member virtual method +0x18. */
extern void Game_DeserializeDatalinkFlowMember(
    uint32_t member_address,
    const uint8_t **cursor
);

/* Opaque 0x02015E3C source-to-destination byte copy. */
extern void Game_CopyDatalinkPersistenceBytes(
    const void *source,
    void *destination,
    uint32_t size
);

_Static_assert(
    sizeof(Game_DatalinkRowResource) == 0x0C,
    "datalink row-resource result size"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, primary_transition_address_34) == 0x34,
    "datalink primary transition offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, ready_transition_address_38) == 0x38,
    "datalink ready transition offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, unavailable_transition_address_3c) ==
        0x3C,
    "datalink unavailable transition offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, selected_participant_40) == 0x40,
    "datalink persisted byte range offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, return_phase_depth_48) == 0x48,
    "datalink persisted depth offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, return_phase_stack_4c) == 0x4C,
    "datalink persisted stack offset"
);

/*
 * 0x020A8D78
 *
 * Complete 60-byte virtual resource resolver.  IDs begin at 0x2328, matching
 * the values published in the renderer's 0x34-byte row metadata.  Each ID
 * selects the 16-byte view at slot-record offset +8.  The first virtual-method
 * argument is unused by the target.
 */
int Game_ResolveDatalinkRowResource(
    uint32_t unused_owner_address,
    Game_DatalinkRowResource *resource,
    uint32_t resource_id
)
{
    uint32_t record_index =
        resource_id - DATALINK_ROW_RESOURCE_ID_BASE;

    (void)unused_owner_address;
    resource->owner_address_00 = Game_GetDatalinkRowResourceOwner();
    resource->data_address_04 =
        DATALINK_ROW_RECORD_ADDRESS +
        record_index * DATALINK_ROW_RECORD_STRIDE +
        DATALINK_ROW_RESOURCE_DATA_OFFSET;
    resource->size_08 = DATALINK_ROW_RESOURCE_SIZE;
    return 1;
}

/*
 * 0x020A8DBC
 *
 * Complete 224-byte virtual serializer.  Three member objects serialize in
 * +0x34/+0x38/+0x3C order.  The parent then appends bytes +0x40..+0x43,
 * skips the transient +0x44..+0x47 range, and appends +0x48 followed by the
 * 32-byte return stack.  Cursor increments preserve the target call order.
 */
void Game_SerializeDatalinkFlowState(
    Game_DatalinkFlowState *state,
    uint8_t **cursor
)
{
    const uint8_t *serialized_bytes =
        (const uint8_t *)&state->selected_participant_40;
    uint32_t index;

    Game_SerializeDatalinkFlowMember(
        state->primary_transition_address_34,
        cursor
    );
    Game_SerializeDatalinkFlowMember(
        state->ready_transition_address_38,
        cursor
    );
    Game_SerializeDatalinkFlowMember(
        state->unavailable_transition_address_3c,
        cursor
    );

    for (index = 0; index < DATALINK_SERIALIZED_BYTE_COUNT; ++index) {
        Game_CopyDatalinkPersistenceBytes(
            &serialized_bytes[index],
            *cursor,
            1
        );
        *cursor += 1;
    }
    Game_CopyDatalinkPersistenceBytes(
        &state->return_phase_depth_48,
        *cursor,
        sizeof(state->return_phase_depth_48)
    );
    *cursor += sizeof(state->return_phase_depth_48);
    Game_CopyDatalinkPersistenceBytes(
        state->return_phase_stack_4c,
        *cursor,
        DATALINK_RETURN_STACK_BYTE_COUNT
    );
    *cursor += DATALINK_RETURN_STACK_BYTE_COUNT;
}

/*
 * 0x020A8E9C
 *
 * Complete 224-byte inverse of Game_SerializeDatalinkFlowState.  Member
 * virtual method +0x18 runs for +0x34/+0x38/+0x3C, then the same 40-byte
 * parent payload is consumed in the same order.
 */
void Game_DeserializeDatalinkFlowState(
    Game_DatalinkFlowState *state,
    const uint8_t **cursor
)
{
    uint8_t *serialized_bytes =
        (uint8_t *)&state->selected_participant_40;
    uint32_t index;

    Game_DeserializeDatalinkFlowMember(
        state->primary_transition_address_34,
        cursor
    );
    Game_DeserializeDatalinkFlowMember(
        state->ready_transition_address_38,
        cursor
    );
    Game_DeserializeDatalinkFlowMember(
        state->unavailable_transition_address_3c,
        cursor
    );

    for (index = 0; index < DATALINK_SERIALIZED_BYTE_COUNT; ++index) {
        Game_CopyDatalinkPersistenceBytes(
            *cursor,
            &serialized_bytes[index],
            1
        );
        *cursor += 1;
    }
    Game_CopyDatalinkPersistenceBytes(
        *cursor,
        &state->return_phase_depth_48,
        sizeof(state->return_phase_depth_48)
    );
    *cursor += sizeof(state->return_phase_depth_48);
    Game_CopyDatalinkPersistenceBytes(
        *cursor,
        state->return_phase_stack_4c,
        DATALINK_RETURN_STACK_BYTE_COUNT
    );
    *cursor += DATALINK_RETURN_STACK_BYTE_COUNT;
}
