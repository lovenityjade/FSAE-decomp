#include "game/datalink_post_oam.h"

#include "game/manual_display.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_POST_OAM_DISPLAY_PLANE_MASK = 0x1F00,
    DATALINK_POST_OAM_DISPLAY_PLANE = 0x1000,
    DATALINK_POST_OAM_ENTER_TRANSITION = 0x2000,
    DATALINK_POST_OAM_EXIT_TRANSITION = 0x1000,
    DATALINK_POST_OAM_MARKER_Y = 104,
    DATALINK_POST_OAM_MARKER_SPACING = 50,
    DATALINK_POST_OAM_MARKER_CENTER_STEP = 25,
    DATALINK_POST_OAM_MARKER_CENTER_X = 112,
    DATALINK_POST_OAM_MARKER_FLAG_LAYOUT_1_OR_4 = 8,
    DATALINK_POST_OAM_MARKER_FLAG_LAYOUT_2 = 16,
    DATALINK_POST_OAM_MARKER_CODE_LAYOUT_1_OR_4 = 3,
    DATALINK_POST_OAM_MARKER_CODE_DEFAULT = 7,
    DATALINK_POST_OAM_MARKER_CODE_LAYOUT_2 = 11
};

extern void Game_ConfigureDatalinkPostOamGraphics_0209bf00(int enabled);
extern void Game_LoadDatalinkPostOamResources_0209c41c(
    int resource_a,
    int resource_b,
    int parameter_c,
    int parameter_d
);
extern void Game_ClearDatalinkPostOamBackground_0209dd44(int target);
extern void Game_SetDatalinkPostOamMode_020a3778(int mode);
extern Game_DatalinkPostOamRuntime *
    Game_GetDatalinkPostOamRuntime_0208da4c(void);
extern void Game_StartDatalinkPostOamTransition_0209b90c(int amount);
extern void Game_ReleaseDatalinkPostOamResource_0209d690(uint32_t index);
extern void Game_ReleaseDatalinkPostOamResourceSet_0209d714(void *resources);
extern void Game_StartDatalinkPostOamExitTransition_0209b880(int amount);
extern int Game_IsDatalinkPostOamRuntimeAvailable_0208de54(void);
extern int Game_IsDatalinkPostOamRuntimeReady_0209ecd4(const void *state);
extern uint32_t Game_MakeDatalinkPostOamMarker_0209ce48(
    int x,
    int y,
    uint32_t marker_flags,
    uint32_t marker_code,
    uint32_t entry_flags,
    Game_DatalinkOamEntry *output
);
extern void NNS_G2dEntryOamManagerOam(
    Game_DatalinkOamManager *manager,
    const Game_DatalinkOamEntry *oam_entries,
    uint32_t count
);

_Static_assert(
    offsetof(Game_DatalinkPostOamHardwareState, display_state_d0) == 0xD0,
    "post-OAM display state offset"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamHardwareState, display_variant_d1) == 0xD1,
    "post-OAM display variant offset"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamSession, layout_mode_8e) == 0x8E,
    "post-OAM session layout-mode offset"
);
_Static_assert(
    offsetof(
        Game_DatalinkPostOamLocalSelection,
        local_participant_index_12
    ) == 0x12,
    "post-OAM local participant offset"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamParticipant, marker_variant_24) == 0x24,
    "post-OAM participant marker offset"
);
_Static_assert(
    sizeof(Game_DatalinkPostOamParticipant) == 0x34,
    "post-OAM participant stride"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamOutput, scratch_oams_3e0) == 0x3E0,
    "post-OAM scratch OAM offset"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamOutput, submitted_oam_count_460) == 0x460,
    "post-OAM submitted OAM count offset"
);
_Static_assert(
    offsetof(Game_DatalinkPostOamRuntime, readiness_state_1f8) == 0x1F8,
    "post-OAM readiness-state offset"
);

/*
 * 0x020ADB04..0x020ADB63: complete 96-byte body. The display-register
 * literal at 0x020ADB64 is excluded.
 */
void Game_InitializeDatalinkPostOamSceneGraphics(void)
{
    Game_DatalinkPostOamRuntime *runtime;

    Game_ConfigureDatalinkPostOamGraphics_0209bf00(1);
    Game_LoadDatalinkPostOamResources_0209c41c(0x2B, 0x2A, -1, -1);
    Game_ClearDatalinkPostOamBackground_0209dd44(0);
    Game_SetDatalinkPostOamMode_020a3778(4);

    runtime = Game_GetDatalinkPostOamRuntime_0208da4c();
    runtime->scene_field_10 = 0;
    runtime->scene_field_14 = 0;
    gGameSubDisplayControl_04001000 =
        (gGameSubDisplayControl_04001000 &
         ~((uint32_t)DATALINK_POST_OAM_DISPLAY_PLANE_MASK)) |
        DATALINK_POST_OAM_DISPLAY_PLANE;
    Game_StartDatalinkPostOamTransition_0209b90c(
        DATALINK_POST_OAM_ENTER_TRANSITION
    );
}

/*
 * 0x020ADB68..0x020ADBAF: complete 72-byte body. Its two-word pool at
 * 0x020ADBB0 is excluded.
 */
void Game_ReleaseDatalinkPostOamSceneGraphics(void)
{
    uint32_t index;

    for (index = 0; index < 4U; ++index) {
        Game_ReleaseDatalinkPostOamResource_0209d690(index);
    }
    Game_ReleaseDatalinkPostOamResourceSet_0209d714(
        gGameDatalinkPostOamReleaseResources
    );

    if (gGameDatalinkPostOamHardwareState.display_state_d0 != 1U ||
        gGameDatalinkPostOamHardwareState.display_variant_d1 != 2) {
        Game_StartDatalinkPostOamExitTransition_0209b880(
            DATALINK_POST_OAM_EXIT_TRANSITION
        );
    }
}

static void Game_GetDatalinkPostOamMarkerStyle(
    uint32_t variant,
    uint32_t *marker_flags,
    uint32_t *marker_code)
{
    uint32_t layout_mode = gGameDatalinkPostOamSession.layout_mode_8e;

    *marker_flags = 0;
    *marker_code = variant + DATALINK_POST_OAM_MARKER_CODE_DEFAULT;
    if (layout_mode == 1U || layout_mode == 4U) {
        *marker_flags = DATALINK_POST_OAM_MARKER_FLAG_LAYOUT_1_OR_4;
        *marker_code =
            variant + DATALINK_POST_OAM_MARKER_CODE_LAYOUT_1_OR_4;
    } else if (layout_mode == 2U) {
        *marker_flags = DATALINK_POST_OAM_MARKER_FLAG_LAYOUT_2;
        *marker_code = variant + DATALINK_POST_OAM_MARKER_CODE_LAYOUT_2;
    }
}

/*
 * 0x020ADBB8..0x020ADCE7: complete 304-byte body. The five-word global pool
 * at 0x020ADCE8 and the independent prologue at 0x020ADCFC are excluded.
 */
void Game_RenderDatalinkRemoteParticipantOams(void)
{
    Game_DatalinkPostOamRuntime *runtime =
        Game_GetDatalinkPostOamRuntime_0208da4c();
    uint32_t participant_count;
    uint32_t local_index;
    uint32_t participant_index;
    int remote_index;

    (void)Game_GetDatalinkPostOamRuntime_0208da4c();
    if (Game_IsDatalinkPostOamRuntimeAvailable_0208de54() == 0) {
        return;
    }
    if (Game_IsDatalinkPostOamRuntimeReady_0209ecd4(
            &runtime->readiness_state_1f8
        ) == 0) {
        return;
    }

    participant_count = gGameDatalinkPostOamSession.participant_count_00;
    local_index =
        gGameDatalinkPostOamLocalSelection.local_participant_index_12 & 3U;
    remote_index = 0;
    for (participant_index = 0;
         participant_index < participant_count;
         ++participant_index) {
        uint32_t marker_flags;
        uint32_t marker_code;
        uint32_t made_count;
        uint32_t variant;
        int x;
        Game_DatalinkPostOamOutput *output;

        if (participant_index == local_index) {
            continue;
        }

        variant =
            gGameDatalinkPostOamParticipants[participant_index]
                .marker_variant_24 & 3U;
        Game_GetDatalinkPostOamMarkerStyle(
            variant,
            &marker_flags,
            &marker_code
        );
        x = remote_index * DATALINK_POST_OAM_MARKER_SPACING -
            ((int)participant_count - 2) *
                DATALINK_POST_OAM_MARKER_CENTER_STEP +
            DATALINK_POST_OAM_MARKER_CENTER_X;

        output = gGameDatalinkPostOamOutput;
        made_count = Game_MakeDatalinkPostOamMarker_0209ce48(
            x,
            DATALINK_POST_OAM_MARKER_Y,
            marker_flags,
            marker_code,
            0x80000000U,
            &output->scratch_oams_3e0[0]
        );
        output = gGameDatalinkPostOamOutput;
        NNS_G2dEntryOamManagerOam(
            &gGameDatalinkOamManager,
            &output->scratch_oams_3e0[0],
            made_count & 0xFFFFU
        );
        ++remote_index;
        output = gGameDatalinkPostOamOutput;
        output->submitted_oam_count_460 += made_count;
    }
}
