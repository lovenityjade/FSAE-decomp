#include "game/hud_tilemap_refresh.h"

#include "game/hud_four_step_meter.h"
#include "game/hud_two_step_meter.h"
#include "game/hud_two_step_meter_stack.h"
#include "game/time_counter.h"

#include <stddef.h>

/* 0x0209F570 updates the HUD values before each redraw. */
extern void Game_UpdateHudRefreshValues(Game_HudRefreshValues *values);

/* 0x020144B8 is the SDK data-cache clean/invalidate range primitive. */
extern void Game_FlushDataCacheRange(void *address, uint32_t byte_size);

/* 0x0200FBB0 transfers a screen-data range into background VRAM. */
extern void Game_UploadBackgroundTilemap(
    const void *source,
    uint32_t destination_offset,
    uint32_t byte_size
);

/* 0x02011854 waits for the selected DMA channel to become idle. */
extern void Game_WaitForDma(int dma_channel);

_Static_assert(
    offsetof(Game_HudRefreshValues, total_seconds) == 0x08,
    "HUD time offset"
);
_Static_assert(
    offsetof(Game_HudRefreshValues, top_meter_step) == 0x0C,
    "top HUD meter offset"
);
_Static_assert(
    offsetof(Game_HudRefreshValues, middle_meter_step) == 0x10,
    "middle HUD meter offset"
);
_Static_assert(
    offsetof(Game_HudRefreshValues, bottom_meter_step) == 0x14,
    "bottom HUD meter offset"
);
_Static_assert(
    offsetof(Game_HudRefreshValues, right_meter_step) == 0x18,
    "right HUD meter offset"
);
_Static_assert(
    offsetof(Game_HudTilemapResource, byte_size) == 0x08,
    "HUD screen-data size offset"
);
_Static_assert(
    offsetof(Game_HudTilemapResource, tilemap) == 0x0C,
    "HUD tilemap offset"
);

/*
 * Semantic block 0x020A6FF0..0x020A70B0.
 *
 * The original function obtains values at owner+0x4B4, stores resource+0x0C
 * into owner+0x934, and performs this sequence before any sprite gating:
 *
 * 1. update the five fields through 0x0209F570;
 * 2. draw time and the four adjacent meters into the shared tilemap;
 * 3. flush exactly resource->byte_size bytes from that tilemap;
 * 4. upload the same range at background destination offset zero;
 * 5. wait for the configured DMA channel.
 *
 * The context makes the recovered global owners explicit while preserving the
 * observed values and call order.
 */
void Game_RefreshHudTilemap(const Game_HudTilemapRefreshContext *context)
{
    Game_HudRefreshValues *values = context->values;
    Game_HudTilemapResource *resource = context->resource;
    uint16_t *tilemap;

    Game_UpdateHudRefreshValues(values);

    tilemap = resource->tilemap;
    *context->active_tilemap = tilemap;

    Game_DrawTimeCounter(values->total_seconds, tilemap);
    Game_DrawTopHudTwoStepMeter(values->top_meter_step, tilemap);
    Game_DrawMiddleHudTwoStepMeter(values->middle_meter_step, tilemap);
    Game_DrawBottomHudTwoStepMeter(values->bottom_meter_step, tilemap);
    Game_DrawRightHudFourStepMeter(values->right_meter_step, tilemap);

    Game_FlushDataCacheRange(tilemap, resource->byte_size);
    Game_UploadBackgroundTilemap(tilemap, 0, resource->byte_size);
    Game_WaitForDma(context->dma_channel);
}
