#include "game/datalink_descriptor_build.h"

#include "game/datalink_slot_readiness.h"

#include <stddef.h>

enum {
    DATALINK_DEFAULT_RESOURCE_ID = 0x084F,
    DATALINK_READY_RESOURCE_ID_BASE = 9000,
    DATALINK_RENDER_CONTEXT_OFFSET = 0x01BC,
    DATALINK_RUNTIME_SUBOBJECT_OFFSET = 0x30,
    DATALINK_TILE_OFFSET_UNIT = 8,
    DATALINK_BUFFER_ROW_SHIFT = 5,
    DATALINK_TRANSFER_OFFSET = 0x6400,
    DATALINK_TRANSFER_SIZE = 0x0A80
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

/* External helper 0x020A2B14 publishes the completed descriptor set. */
extern void Game_RenderDatalinkSlotDescriptors(
    Game_DatalinkDescriptorRenderer *renderer,
    uint32_t render_value,
    const void *runtime_subobject
);

/* 0x020144B8 flushes a target-addressed cache range. */
extern void Game_FlushDatalinkDescriptorRange(
    uint32_t source_address,
    uint32_t byte_size
);

/* External helper 0x0209DAA0 transfers the flushed range to target offset 0x6400. */
extern void Game_TransferDatalinkDescriptorRange(
    int channel,
    uint32_t source_address,
    uint32_t destination_offset,
    uint32_t byte_size
);

_Static_assert(
    sizeof(Game_DatalinkSlotDescriptor) == 0x34,
    "datalink descriptor stride"
);
_Static_assert(
    offsetof(Game_DatalinkSlotDescriptor, buffer_address_08) == 0x08,
    "datalink buffer address offset"
);
_Static_assert(
    offsetof(Game_DatalinkSlotDescriptor, buffer_row_0c) == 0x0C,
    "datalink buffer row offset"
);
_Static_assert(
    offsetof(Game_DatalinkSlotDescriptor, tile_offset_quotient_10) == 0x10,
    "datalink tile quotient offset"
);
_Static_assert(
    offsetof(Game_DatalinkSlotDescriptor, tile_offset_remainder_24) == 0x24,
    "datalink tile remainder offset"
);
_Static_assert(
    offsetof(Game_DatalinkSlotDescriptor, type_26) == 0x26,
    "datalink descriptor type offset"
);
_Static_assert(
    offsetof(Game_DatalinkDescriptorRenderer, layout_value_38) == 0x38,
    "datalink renderer layout offset"
);

static void Game_BuildDatalinkSlotDescriptor(
    int slot,
    int centered_offset
)
{
    Game_DatalinkSlotDescriptor *descriptor =
        &gGameDatalinkSlotDescriptors[slot];

    if (Game_IsDatalinkPlayerSlotReady(slot)) {
        descriptor->resource_id_00 =
            DATALINK_READY_RESOURCE_ID_BASE + (uint32_t)slot;
        descriptor->tile_offset_quotient_10 = 8;
        descriptor->tile_offset_remainder_24 = 0;
        descriptor->variant_index_14 = slot * 6 + 4;
        descriptor->type_26 = 3;
        descriptor->mode_18 = 11;
    } else {
        descriptor->resource_id_00 = DATALINK_DEFAULT_RESOURCE_ID;
        descriptor->tile_offset_quotient_10 =
            centered_offset / DATALINK_TILE_OFFSET_UNIT;
        descriptor->tile_offset_remainder_24 =
            (int16_t)(centered_offset % DATALINK_TILE_OFFSET_UNIT);
        descriptor->variant_index_14 = slot * 6 + 5;
        descriptor->type_26 = 2;
        descriptor->mode_18 = 14;
    }

    descriptor->buffer_address_08 =
        gGameDatalinkDescriptorBufferBase +
        ((uint32_t)descriptor->buffer_row_0c << DATALINK_BUFFER_ROW_SHIFT);
}

/*
 * 0x020A777C..0x020A78DF (catalog/Ghidra body size: 356 bytes).
 *
 * The literal pool occupies 0x020A78E0..0x020A78F3; the next catalogued
 * function starts at 0x020A7918.  This function measures the default label,
 * centers it in a 256-unit span, and decomposes that signed offset into a
 * quotient and remainder by 8 for each unavailable player slot.  Ready
 * slots instead select resources 9000..9002 and fixed placement values.
 *
 * Fields not explicitly written here, including each descriptor's row at
 * +0x0C, remain owned by the earlier setup path exactly as on the target.
 */
void Game_BuildDatalinkSlotDescriptors(void *runtime)
{
    uint8_t *render_context;
    const void *runtime_subobject = runtime;
    uint32_t transfer_address;
    int measured_width;
    int centered_offset;
    int slot;

    Game_PrepareDatalinkDescriptorBuild();
    render_context = Game_GetDatalinkRenderContext();
    measured_width = Game_MeasureDatalinkResource(
        DATALINK_DEFAULT_RESOURCE_ID,
        0,
        render_context + DATALINK_RENDER_CONTEXT_OFFSET
    );
    centered_offset = (256 - measured_width) / 2;

    for (slot = 0; slot < GAME_DATALINK_DESCRIPTOR_COUNT; ++slot) {
        Game_BuildDatalinkSlotDescriptor(slot, centered_offset);
    }

    gGameDatalinkDescriptorRenderer.layout_value_38 =
        gGameDatalinkLayoutValue6ac;

    if (runtime != NULL) {
        runtime_subobject =
            (const uint8_t *)runtime + DATALINK_RUNTIME_SUBOBJECT_OFFSET;
    }

    (void)Game_GetDatalinkRenderContext();
    Game_RenderDatalinkSlotDescriptors(
        &gGameDatalinkDescriptorRenderer,
        Game_GetDatalinkRenderValue(),
        runtime_subobject
    );

    transfer_address =
        gGameDatalinkDescriptorBufferBase + DATALINK_TRANSFER_OFFSET;
    Game_FlushDatalinkDescriptorRange(
        transfer_address,
        DATALINK_TRANSFER_SIZE
    );
    Game_TransferDatalinkDescriptorRange(
        1,
        transfer_address,
        DATALINK_TRANSFER_OFFSET,
        DATALINK_TRANSFER_SIZE
    );
}
