#include "game/datalink_post_oam.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_POST_OAM_PALETTE_DESTINATION = 0x05000410,
    DATALINK_POST_OAM_PALETTE_SOURCE_STRIDE = 0x10,
    DATALINK_POST_OAM_PALETTE_DESTINATION_STRIDE = 0x20,
    DATALINK_POST_OAM_PALETTE_COPY_SIZE = 0x10,
    DATALINK_POST_OAM_OBJECT_BATCH_CAPACITY = 32,
    DATALINK_POST_OAM_PHASE_STEP = 0x80,
    DATALINK_POST_OAM_PHASE_CYCLE_MASK = 0x0FFF,
    DATALINK_POST_OAM_PHASE_BANK_MASK = 0x0F,
    DATALINK_POST_OAM_PALETTE_BASE_PHASE = 0x20,
    DATALINK_POST_OAM_FAST_PHASE_STEPS = 8,
    DATALINK_POST_OAM_FAST_PHASE_FLAG = 8,
    DATALINK_POST_OAM_BRIGHTNESS_LIMIT = 64,
    DATALINK_POST_OAM_RGB5_MAX = 31,
    DATALINK_POST_OAM_SINE_TABLE_PHASE_STRIDE = 16
};

typedef int (*Game_DatalinkPostOamObjectCallback)(void);

typedef struct Game_DatalinkPostOamObjectVtable {
    uint32_t unknown_00[3];
    uint32_t batch_stop_callback_address_0c;
} Game_DatalinkPostOamObjectVtable;

typedef struct Game_DatalinkPostOamObject {
    uint32_t vtable_address_00;
} Game_DatalinkPostOamObject;

extern const int16_t FX_SinCosTable_[8192];
extern Game_DatalinkPostOamRuntime *
    Game_GetDatalinkPostOamRuntime_0208da4c(void);
extern int Game_IsDatalinkPostOamRuntimeAvailable_0208de54(void);
extern void Game_SetDatalinkPostOamResourceState_02082c54(uint32_t state);
extern void Game_ResetDatalinkPostOamController_020a507c(
    Game_DatalinkPostOamController *controller
);
extern void Game_InstallDatalinkPostOamResource_0208e670(
    void *resource_context,
    void *resource,
    uint32_t resource_size
);
extern void Game_ReleaseDatalinkPostOamHandle_0208e188(uint32_t handle);
extern void MIi_CpuCopyFast(
    const void *source,
    void *destination,
    uint32_t size
);
extern void Game_BindDatalinkPostOamObjectBatch_020a520c(
    Game_DatalinkPostOamController *controller,
    void *palette_record,
    Game_DatalinkPostOamObject *const *objects,
    int count
);
extern void Game_BindDatalinkPostOamDefaultResource_020a5184(
    Game_DatalinkPostOamController *controller,
    void *palette_record,
    const void *default_resource
);

_Static_assert(
    offsetof(Game_DatalinkPostOamState, pending_resource_size_04) == 0x04,
    "post-OAM pending resource size offset"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamState, pending_resource_address_08) == 0x08,
    "post-OAM pending resource address offset"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamState, palette_dirty_0c) == 0x0C,
    "post-OAM palette dirty offset"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamState, object_cursor_10) == 0x10,
    "post-OAM object cursor offset"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamState, active_14) == 0x14,
    "post-OAM active offset"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamState, palette_phase_18) == 0x18,
    "post-OAM palette phase offset"
);
_Static_assert(
    sizeof(Game_DatalinkPostOamState) == 0x1C,
    "post-OAM state size"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamHardwareState, palette_origin_e0) == 0xE0,
    "post-OAM palette origin offset"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamHardwareState, palette_phase_e4) == 0xE4,
    "post-OAM palette phase offset"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamHardwareState, brightness_f8) == 0xF8,
    "post-OAM hardware brightness offset"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamRuntime, resource_context_44) == 0x44,
    "post-OAM resource context offset"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamRuntime, resource_handle_58) == 0x58,
    "post-OAM resource handle offset"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamRuntime, object_list_address_64) == 0x64,
    "post-OAM object list offset"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamRuntime, object_count_68) == 0x68,
    "post-OAM object count offset"
);

/*
 * 0x020ADF84..0x020ADFDB: complete 88-byte body. The two-word global pool at
 * 0x020ADFDC is excluded, as are the adapter and its literal through 0x020ADFEF.
 */
void Game_CommitDatalinkPostOamPendingResource(void)
{
    Game_DatalinkPostOamState *state = &gGameDatalinkPostOamState;
    Game_DatalinkPostOamRuntime *runtime;
    uint32_t previous_handle;

    if (state->pending_resource_address_08 == 0U) {
        return;
    }

    Game_SetDatalinkPostOamResourceState_02082c54(0);
    Game_ResetDatalinkPostOamController_020a507c(
        &gGameDatalinkPostOamController
    );
    runtime = Game_GetDatalinkPostOamRuntime_0208da4c();
    previous_handle = runtime->resource_handle_58;
    runtime = Game_GetDatalinkPostOamRuntime_0208da4c();
    Game_InstallDatalinkPostOamResource_0208e670(
        &runtime->resource_context_44[0],
        (void *)(uintptr_t)state->pending_resource_address_08,
        state->pending_resource_size_04
    );
    state->pending_resource_address_08 = 0;
    state->pending_resource_size_04 = 0;
    Game_ReleaseDatalinkPostOamHandle_0208e188(previous_handle);
}

static Game_DatalinkPostOamObject *Game_GetDatalinkPostOamObject(
    uint32_t object_address)
{
    return (Game_DatalinkPostOamObject *)(uintptr_t)object_address;
}

static int Game_DatalinkPostOamObjectStopsBatch(
    const Game_DatalinkPostOamObject *object)
{
    const Game_DatalinkPostOamObjectVtable *vtable =
        (const Game_DatalinkPostOamObjectVtable *)(uintptr_t)
            object->vtable_address_00;
    Game_DatalinkPostOamObjectCallback callback =
        (Game_DatalinkPostOamObjectCallback)(uintptr_t)
            vtable->batch_stop_callback_address_0c;

    return callback();
}

/*
 * 0x020ADFF0..0x020AE117: complete 296-byte body. Its six-word global pool at
 * 0x020AE118 is excluded.
 */
void Game_UploadDatalinkPostOamPaletteAndObjects(void)
{
    Game_DatalinkPostOamState *state = &gGameDatalinkPostOamState;
    Game_DatalinkPostOamObject *objects[
        DATALINK_POST_OAM_OBJECT_BATCH_CAPACITY
    ];
    uint32_t palette_bank;
    int batch_count;
    int row;

    for (row = 0; row < GAME_DATALINK_POST_OAM_PALETTE_ROWS; ++row) {
        MIi_CpuCopyFast(
            &gGameDatalinkPostOamPalette[row][0],
            (void *)(uintptr_t)(
                DATALINK_POST_OAM_PALETTE_DESTINATION +
                row * DATALINK_POST_OAM_PALETTE_DESTINATION_STRIDE
            ),
            DATALINK_POST_OAM_PALETTE_COPY_SIZE
        );
    }

    if (state->active_14 == 0U) {
        return;
    }

    gGameDatalinkPostOamHardwareState.palette_origin_e0 = 0;
    gGameDatalinkPostOamHardwareState.palette_phase_e4 =
        (state->palette_phase_18 >> 8) +
        DATALINK_POST_OAM_PALETTE_BASE_PHASE;
    if (state->palette_dirty_0c == 0U) {
        return;
    }

    palette_bank =
        (state->palette_phase_18 >> 12) &
        DATALINK_POST_OAM_PHASE_BANK_MASK;
    batch_count = 0;
    {
        Game_DatalinkPostOamRuntime *runtime =
            Game_GetDatalinkPostOamRuntime_0208da4c();

        if (state->object_cursor_10 < runtime->object_count_68) {
            do {
                const uint32_t *object_addresses =
                    (const uint32_t *)(uintptr_t)
                        runtime->object_list_address_64;
                uint32_t cursor = state->object_cursor_10;

                objects[batch_count] = Game_GetDatalinkPostOamObject(
                    object_addresses[cursor]
                );
                state->object_cursor_10 = cursor + 1U;
                if (Game_DatalinkPostOamObjectStopsBatch(
                        objects[batch_count]
                    )) {
                    break;
                }
                ++batch_count;
            } while (
                batch_count < DATALINK_POST_OAM_OBJECT_BATCH_CAPACITY
            );

            Game_BindDatalinkPostOamObjectBatch_020a520c(
                &gGameDatalinkPostOamController,
                &gGameDatalinkPostOamPaletteRecords[palette_bank][0],
                objects,
                batch_count
            );
        } else {
            Game_BindDatalinkPostOamDefaultResource_020a5184(
                &gGameDatalinkPostOamController,
                &gGameDatalinkPostOamPaletteRecords[palette_bank][0],
                gGameDatalinkPostOamDefaultResource
            );
            state->active_14 = 0;
        }
    }

    state->palette_dirty_0c = 0;
}

static uint16_t Game_MakeDatalinkPostOamGray(uint32_t level)
{
    return (uint16_t)(level | (level << 5) | (level << 10));
}

static void Game_GenerateDatalinkPostOamPalette(uint32_t phase)
{
    int row;

    for (row = 0; row < GAME_DATALINK_POST_OAM_PALETTE_ROWS; ++row) {
        uint32_t row_phase =
            ((phase >> 8) - (uint32_t)(row * 16)) & 0xFFU;
        int sine = FX_SinCosTable_[
            row_phase * DATALINK_POST_OAM_SINE_TABLE_PHASE_STRIDE
        ];
        uint32_t level = ((uint32_t)sine << 9) >> 16;
        uint16_t color;
        int color_index;

        if (level > DATALINK_POST_OAM_RGB5_MAX) {
            level = DATALINK_POST_OAM_RGB5_MAX;
        }
        color = Game_MakeDatalinkPostOamGray(level);
        for (color_index = 0;
             color_index < GAME_DATALINK_POST_OAM_COLORS_PER_ROW;
             ++color_index) {
            gGameDatalinkPostOamPalette[row][color_index] = color;
        }
    }
}

/*
 * 0x020AE130..0x020AE267: complete 312-byte body. Its six-word global and
 * sine-table pool at 0x020AE268 is excluded.
 */
void Game_UpdateDatalinkPostOamPaletteWave(void)
{
    Game_DatalinkPostOamState *state = &gGameDatalinkPostOamState;
    int phase_steps = 1;
    int step;
    uint32_t phase;

    if (state->brightness_step_01 != 0) {
        int brightness;

        state->brightness_00 = (int8_t)(
            (uint8_t)state->brightness_00 +
            (uint8_t)state->brightness_step_01
        );
        brightness = state->brightness_00;
        if (brightness >= DATALINK_POST_OAM_BRIGHTNESS_LIMIT ||
            brightness == 0 ||
            brightness <= -DATALINK_POST_OAM_BRIGHTNESS_LIMIT) {
            state->brightness_step_01 = 0;
        }
        gGameDatalinkPostOamHardwareState.brightness_f8 = brightness >> 2;
    }

    if (state->active_14 == 0U) {
        return;
    }

    (void)Game_GetDatalinkPostOamRuntime_0208da4c();
    if (Game_IsDatalinkPostOamRuntimeAvailable_0208de54() == 0 &&
        (gGameDatalinkPostOamFlags & DATALINK_POST_OAM_FAST_PHASE_FLAG) != 0U) {
        phase_steps = DATALINK_POST_OAM_FAST_PHASE_STEPS;
    }

    phase = state->palette_phase_18;
    for (step = 0; step < phase_steps; ++step) {
        phase += DATALINK_POST_OAM_PHASE_STEP;
        if ((phase & DATALINK_POST_OAM_PHASE_CYCLE_MASK) == 0U) {
            state->palette_dirty_0c = 1;
        }
    }
    state->palette_phase_18 = phase;
    Game_GenerateDatalinkPostOamPalette(phase);
}
