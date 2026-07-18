#include "game/datalink_renderer.h"

#include "game/datalink_renderer_auxiliary.h"
#include "game/datalink_renderer_buffer_prep.h"
#include "game/datalink_renderer_dirty_gate.h"
#include "game/datalink_renderer_preamble.h"
#include "game/datalink_renderer_row_prep.h"
#include "game/datalink_renderer_unavailable_row.h"
#include "game/datalink_renderer_upload.h"
#include "game/datalink_renderer_variant_overlay.h"

#include <stdbool.h>
#include <stdint.h>

enum {
    DATALINK_RENDERED_SLOT_COUNT = 3
};

/*
 * 0x020A8904
 *
 * Complete semantic recovery of the 1104-byte body through 0x020A8D53.
 * The binding global is a host-safe semantic view of the pointer graph rooted
 * at literals 0x0217A23C, 0x02180D9C, 0x02180DA0, 0x0217D348, and
 * 0x02180DA4.  Metadata and ready-row records correspond to 0x02124BB4 and
 * 0x021350B0.  The target function itself receives only the flow-state
 * pointer, which this signature preserves.
 *
 * Descriptors and row values retain their parent stack lifetimes.  The
 * secondary pointer is captured once after buffer preparation and remains
 * stable through all three rows and the final upload, matching 0x020A89C8.
 * The loop order represents the back-edge at 0x020A8CBC..0x020A8CC7.
 */
void Game_UpdateDatalinkRenderer(Game_DatalinkFlowState *state)
{
    Game_DatalinkRenderDescriptor
        descriptors[GAME_DATALINK_RENDER_DESCRIPTOR_COUNT];
    uint32_t values[GAME_DATALINK_RENDER_ROW_VALUE_COUNT];
    uint8_t *captured_secondary_buffer;
    uint16_t *secondary_tilemap;
    int slot;

    Game_CopyDatalinkRenderDescriptors(descriptors);
    Game_IssueReadyDatalinkSlotCommands();

    if (!Game_ShouldRebuildDatalinkRender(state)) {
        return;
    }

    Game_PrepareDatalinkRenderBuffers(
        state,
        &gGameDatalinkRendererBindings
    );
    captured_secondary_buffer =
        gGameDatalinkRendererBindings.secondary_source;
    secondary_tilemap =
        (uint16_t *)(void *)captured_secondary_buffer;

    for (slot = 0; slot < DATALINK_RENDERED_SLOT_COUNT; ++slot) {
        uint8_t slot_index = (uint8_t)slot;
        bool ready = Game_PrepareDatalinkRenderRow(values, slot_index);

        if (ready) {
            Game_RenderReadyDatalinkRow(
                gGameDatalinkRenderRowMetadata,
                &gGameDatalinkReadyRowRecords[slot],
                descriptors,
                &gGameDatalinkRendererBindings,
                values,
                slot_index
            );
        } else {
            Game_RenderUnavailableDatalinkRow(
                secondary_tilemap,
                slot_index
            );
        }

        Game_RenderDatalinkRowVariantOverlay(
            state,
            &gGameDatalinkRendererBindings,
            secondary_tilemap,
            slot_index
        );
        Game_RefreshDatalinkRowAuxiliaryIfNeeded(state, slot_index);
    }

    Game_UploadDatalinkRenderBuffers(
        &gGameDatalinkRendererBindings,
        captured_secondary_buffer
    );
}
