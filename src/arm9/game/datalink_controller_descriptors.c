#include "game/datalink_controller.h"

#include "game/datalink_descriptor_build.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_CONTROLLER_DESCRIPTOR_COUNT = 2,
    DATALINK_CONTROLLER_RESOURCE_ID = 0x084F,
    DATALINK_CONTROLLER_RENDER_CONTEXT_OFFSET = 0x01BC,
    DATALINK_CONTROLLER_RUNTIME_SUBOBJECT_OFFSET = 0x30,
    DATALINK_CONTROLLER_CENTER_WIDTH = 256,
    DATALINK_CONTROLLER_TILE_OFFSET_UNIT = 8,
    DATALINK_CONTROLLER_BUFFER_ROW_SHIFT = 5,
    DATALINK_CONTROLLER_VARIANT_STRIDE = 6,
    DATALINK_CONTROLLER_VARIANT_BASE = 5,
    DATALINK_CONTROLLER_DESCRIPTOR_TYPE = 2,
    DATALINK_CONTROLLER_DESCRIPTOR_MODE = 14,
    DATALINK_CONTROLLER_TRANSFER_CHANNEL = 1,
    DATALINK_CONTROLLER_TRANSFER_OFFSET = 0x6400,
    DATALINK_CONTROLLER_TRANSFER_SIZE = 0x0A80
};

/* External helper 0x020A2D30 prepares the shared rendering state. */
extern void Game_PrepareDatalinkDescriptorBuild(void);

/* External helper 0x0208DA4C returns the current rendering context. */
extern uint8_t *Game_GetDatalinkRenderContext(void);

/* External helper 0x0204C2E0 measures resource 0x084F in the context at +0x1BC. */
extern int Game_MeasureDatalinkResource(
    uint32_t resource_id,
    int variant,
    const void *render_context
);

/* External helper 0x0208DFA4 supplies the value consumed by 0x020A2B14. */
extern uint32_t Game_GetDatalinkRenderValue(void);

/* Host-safe representation of 0x020A2B14's target-address argument. */
extern void Game_RenderDatalinkControllerDescriptors(
    Game_DatalinkDescriptorRenderer *renderer,
    uint32_t render_value,
    uint32_t runtime_subobject_address
);

/* Opaque 0x020184B8 cache-range flush. */
extern void Game_FlushDatalinkControllerDescriptorRange(
    uint32_t source_address,
    uint32_t byte_size
);

/* Opaque 0x0209DAA0 range transfer. */
extern void Game_TransferDatalinkControllerDescriptorRange(
    int channel,
    uint32_t source_address,
    uint32_t destination_offset,
    uint32_t byte_size
);

_Static_assert(
    sizeof(Game_DatalinkSlotDescriptor) == 0x34,
    "datalink controller descriptor stride"
);
_Static_assert(
    offsetof(Game_DatalinkSlotDescriptor, buffer_address_08) == 0x08,
    "datalink controller descriptor buffer offset"
);
_Static_assert(
    offsetof(Game_DatalinkSlotDescriptor, buffer_row_0c) == 0x0C,
    "datalink controller descriptor row offset"
);
_Static_assert(
    offsetof(Game_DatalinkSlotDescriptor, tile_offset_quotient_10) == 0x10,
    "datalink controller descriptor quotient offset"
);
_Static_assert(
    offsetof(Game_DatalinkSlotDescriptor, tile_offset_remainder_24) == 0x24,
    "datalink controller descriptor remainder offset"
);
_Static_assert(
    offsetof(Game_DatalinkDescriptorRenderer, layout_value_38) == 0x38,
    "datalink controller renderer layout offset"
);

static void Game_BuildDatalinkControllerDescriptor(
    int slot,
    int centered_offset
)
{
    Game_DatalinkSlotDescriptor *descriptor =
        &gGameDatalinkSlotDescriptors[slot];

    descriptor->resource_id_00 = DATALINK_CONTROLLER_RESOURCE_ID;
    descriptor->tile_offset_quotient_10 =
        centered_offset / DATALINK_CONTROLLER_TILE_OFFSET_UNIT;
    descriptor->tile_offset_remainder_24 = (int16_t)(
        centered_offset % DATALINK_CONTROLLER_TILE_OFFSET_UNIT
    );
    descriptor->variant_index_14 =
        slot * DATALINK_CONTROLLER_VARIANT_STRIDE +
        DATALINK_CONTROLLER_VARIANT_BASE;
    descriptor->type_26 = DATALINK_CONTROLLER_DESCRIPTOR_TYPE;
    descriptor->mode_18 = DATALINK_CONTROLLER_DESCRIPTOR_MODE;
    descriptor->buffer_address_08 =
        gGameDatalinkDescriptorBufferBase +
        ((uint32_t)descriptor->buffer_row_0c <<
         DATALINK_CONTROLLER_BUFFER_ROW_SHIFT);
}

/*
 * 0x020A9CE0
 *
 * Complete 280-byte two-descriptor build through 0x020A9DF7. Resource 0x084F
 * is measured in the current context, centered in a 256-unit span, and its
 * signed quotient/remainder by eight are written to two 0x34-byte
 * descriptors. Their preserved row fields select buffer addresses; variant
 * indices are five and eleven, with type two and mode 14. The shared layout
 * value is published, owner +0x30 is supplied only for a nonzero bound owner,
 * and the final `0xA80` bytes at buffer offset `0x6400` are flushed and
 * transferred. Pool `0x020A9DF8..0x020A9E0B` and function `0x020A9E0C` are
 * excluded.
 */
void Game_BuildDatalinkControllerDescriptors(
    const Game_DatalinkController *controller
)
{
    uint8_t *render_context;
    uint32_t runtime_subobject_address;
    uint32_t transfer_address;
    int measured_width;
    int centered_offset;
    int slot;

    Game_PrepareDatalinkDescriptorBuild();
    render_context = Game_GetDatalinkRenderContext();
    measured_width = Game_MeasureDatalinkResource(
        DATALINK_CONTROLLER_RESOURCE_ID,
        0,
        render_context + DATALINK_CONTROLLER_RENDER_CONTEXT_OFFSET
    );
    centered_offset =
        (DATALINK_CONTROLLER_CENTER_WIDTH - measured_width) / 2;

    for (slot = 0;
         slot < DATALINK_CONTROLLER_DESCRIPTOR_COUNT;
         ++slot) {
        Game_BuildDatalinkControllerDescriptor(slot, centered_offset);
    }

    gGameDatalinkDescriptorRenderer.layout_value_38 =
        gGameDatalinkLayoutValue6ac;

    runtime_subobject_address = controller->owner_address_08;
    if (runtime_subobject_address != 0U) {
        runtime_subobject_address +=
            DATALINK_CONTROLLER_RUNTIME_SUBOBJECT_OFFSET;
    }

    (void)Game_GetDatalinkRenderContext();
    Game_RenderDatalinkControllerDescriptors(
        &gGameDatalinkDescriptorRenderer,
        Game_GetDatalinkRenderValue(),
        runtime_subobject_address
    );

    transfer_address =
        gGameDatalinkDescriptorBufferBase +
        DATALINK_CONTROLLER_TRANSFER_OFFSET;
    Game_FlushDatalinkControllerDescriptorRange(
        transfer_address,
        DATALINK_CONTROLLER_TRANSFER_SIZE
    );
    transfer_address =
        gGameDatalinkDescriptorBufferBase +
        DATALINK_CONTROLLER_TRANSFER_OFFSET;
    Game_TransferDatalinkControllerDescriptorRange(
        DATALINK_CONTROLLER_TRANSFER_CHANNEL,
        transfer_address,
        DATALINK_CONTROLLER_TRANSFER_OFFSET,
        DATALINK_CONTROLLER_TRANSFER_SIZE
    );
}
