#include "game/datalink_post_oam.h"

#include "game/manual_display.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

Game_DatalinkPostOamHardwareState gGameDatalinkPostOamHardwareState;
uint8_t gGameDatalinkPostOamReleaseResources[1];
Game_DatalinkPostOamSession gGameDatalinkPostOamSession;
Game_DatalinkPostOamLocalSelection gGameDatalinkPostOamLocalSelection;
Game_DatalinkPostOamParticipant
    gGameDatalinkPostOamParticipants[
        GAME_DATALINK_POST_OAM_PARTICIPANT_CAPACITY
    ];
static Game_DatalinkPostOamOutput sOutput;
Game_DatalinkPostOamOutput *gGameDatalinkPostOamOutput = &sOutput;
Game_DatalinkPostOamState gGameDatalinkPostOamState;
Game_DatalinkPostOamController gGameDatalinkPostOamController;
uint16_t
    gGameDatalinkPostOamPalette
        [GAME_DATALINK_POST_OAM_PALETTE_ROWS]
        [GAME_DATALINK_POST_OAM_COLORS_PER_ROW];
uint8_t
    gGameDatalinkPostOamPaletteRecords
        [GAME_DATALINK_POST_OAM_PALETTE_ROWS]
        [GAME_DATALINK_POST_OAM_PALETTE_RECORD_SIZE];
uint8_t gGameDatalinkPostOamDefaultResource[1];
volatile uint16_t gGameDatalinkPostOamFlags;
Game_DatalinkOamManager gGameDatalinkOamManager;
volatile uint32_t gGameSubDisplayControl_04001000;
const int16_t FX_SinCosTable_[8192] = {0};

static Game_DatalinkPostOamRuntime sRuntime;
static int sRuntimeAvailable;
static int sRuntimeReady;
static int sConfigureCount;
static int sLoadCount;
static int sClearCount;
static int sModeCount;
static int sEnterTransitionCount;
static int sExitTransitionCount;
static uint32_t sReleasedIndices[4];
static int sReleaseIndexCount;
static int sReleaseSetCount;
static int sMarkerXs[4];
static uint32_t sMarkerFlags[4];
static uint32_t sMarkerCodes[4];
static int sMarkerCount;
static int sOamSubmitCount;
static int sPaletteCopyCount;
static int sResourceStateCount;
static int sControllerResetCount;
static uintptr_t sInstalledResource;
static uint32_t sInstalledSize;
static uint32_t sReleasedHandle;
static int sDefaultBindCount;
static void *sBoundPaletteRecord;

void Game_ConfigureDatalinkPostOamGraphics_0209bf00(int enabled)
{
    assert(enabled == 1);
    ++sConfigureCount;
}

void Game_LoadDatalinkPostOamResources_0209c41c(
    int resource_a,
    int resource_b,
    int parameter_c,
    int parameter_d)
{
    assert(resource_a == 0x2B);
    assert(resource_b == 0x2A);
    assert(parameter_c == -1);
    assert(parameter_d == -1);
    ++sLoadCount;
}

void Game_ClearDatalinkPostOamBackground_0209dd44(int target)
{
    assert(target == 0);
    ++sClearCount;
}

void Game_SetDatalinkPostOamMode_020a3778(int mode)
{
    assert(mode == 4);
    ++sModeCount;
}

Game_DatalinkPostOamRuntime *Game_GetDatalinkPostOamRuntime_0208da4c(void)
{
    return &sRuntime;
}

void Game_StartDatalinkPostOamTransition_0209b90c(int amount)
{
    assert(amount == 0x2000);
    ++sEnterTransitionCount;
}

void Game_ReleaseDatalinkPostOamResource_0209d690(uint32_t index)
{
    assert(sReleaseIndexCount < 4);
    sReleasedIndices[sReleaseIndexCount] = index;
    ++sReleaseIndexCount;
}

void Game_ReleaseDatalinkPostOamResourceSet_0209d714(void *resources)
{
    assert(resources == gGameDatalinkPostOamReleaseResources);
    ++sReleaseSetCount;
}

void Game_StartDatalinkPostOamExitTransition_0209b880(int amount)
{
    assert(amount == 0x1000);
    ++sExitTransitionCount;
}

int Game_IsDatalinkPostOamRuntimeAvailable_0208de54(void)
{
    return sRuntimeAvailable;
}

int Game_IsDatalinkPostOamRuntimeReady_0209ecd4(const void *state)
{
    assert(state == &sRuntime.readiness_state_1f8);
    return sRuntimeReady;
}

uint32_t Game_MakeDatalinkPostOamMarker_0209ce48(
    int x,
    int y,
    uint32_t marker_flags,
    uint32_t marker_code,
    uint32_t entry_flags,
    Game_DatalinkOamEntry *output)
{
    assert(sMarkerCount < 4);
    assert(y == 104);
    assert(entry_flags == 0x80000000U);
    sMarkerXs[sMarkerCount] = x;
    sMarkerFlags[sMarkerCount] = marker_flags;
    sMarkerCodes[sMarkerCount] = marker_code;
    ++sMarkerCount;
    output->attribute_0 = (uint16_t)y;
    output->attribute_1 = (uint16_t)x;
    output->attribute_2 = (uint16_t)marker_flags;
    output->affine_value = 0;
    return 1;
}

void NNS_G2dEntryOamManagerOam(
    Game_DatalinkOamManager *manager,
    const Game_DatalinkOamEntry *oam_entries,
    uint32_t count)
{
    assert(manager == &gGameDatalinkOamManager);
    assert(oam_entries == &sOutput.scratch_oams_3e0[0]);
    assert(count == 1U);
    ++sOamSubmitCount;
}

void Game_SetDatalinkPostOamResourceState_02082c54(uint32_t state)
{
    assert(state == 0U);
    ++sResourceStateCount;
}

void Game_ResetDatalinkPostOamController_020a507c(
    Game_DatalinkPostOamController *controller)
{
    assert(controller == &gGameDatalinkPostOamController);
    ++sControllerResetCount;
}

void Game_InstallDatalinkPostOamResource_0208e670(
    void *resource_context,
    void *resource,
    uint32_t resource_size)
{
    assert(resource_context == &sRuntime.resource_context_44[0]);
    sInstalledResource = (uintptr_t)resource;
    sInstalledSize = resource_size;
}

void Game_ReleaseDatalinkPostOamHandle_0208e188(uint32_t handle)
{
    sReleasedHandle = handle;
}

void MIi_CpuCopyFast(const void *source, void *destination, uint32_t size)
{
    uintptr_t expected_destination =
        0x05000410U + (uintptr_t)sPaletteCopyCount * 0x20U;
    const void *expected_source =
        &gGameDatalinkPostOamPalette[sPaletteCopyCount][0];

    assert(sPaletteCopyCount < GAME_DATALINK_POST_OAM_PALETTE_ROWS);
    assert(source == expected_source);
    assert((uintptr_t)destination == expected_destination);
    assert(size == 0x10U);
    ++sPaletteCopyCount;
}

void Game_BindDatalinkPostOamObjectBatch_020a520c(
    Game_DatalinkPostOamController *controller,
    void *palette_record,
    void *const *objects,
    int count)
{
    (void)controller;
    (void)palette_record;
    (void)objects;
    (void)count;
    assert(0 && "the zero-object harness should select the default resource");
}

void Game_BindDatalinkPostOamDefaultResource_020a5184(
    Game_DatalinkPostOamController *controller,
    void *palette_record,
    const void *default_resource)
{
    assert(controller == &gGameDatalinkPostOamController);
    assert(default_resource == gGameDatalinkPostOamDefaultResource);
    sBoundPaletteRecord = palette_record;
    ++sDefaultBindCount;
}

static void ResetHarness(void)
{
    memset(&gGameDatalinkPostOamHardwareState, 0, sizeof(gGameDatalinkPostOamHardwareState));
    memset(&gGameDatalinkPostOamSession, 0, sizeof(gGameDatalinkPostOamSession));
    memset(&gGameDatalinkPostOamLocalSelection, 0, sizeof(gGameDatalinkPostOamLocalSelection));
    memset(gGameDatalinkPostOamParticipants, 0, sizeof(gGameDatalinkPostOamParticipants));
    memset(&sOutput, 0, sizeof(sOutput));
    memset(&gGameDatalinkPostOamState, 0, sizeof(gGameDatalinkPostOamState));
    memset(gGameDatalinkPostOamPalette, 0, sizeof(gGameDatalinkPostOamPalette));
    memset(&sRuntime, 0, sizeof(sRuntime));
    gGameDatalinkPostOamFlags = 0;
    sRuntimeAvailable = 1;
    sRuntimeReady = 1;
    sConfigureCount = 0;
    sLoadCount = 0;
    sClearCount = 0;
    sModeCount = 0;
    sEnterTransitionCount = 0;
    sExitTransitionCount = 0;
    sReleaseIndexCount = 0;
    sReleaseSetCount = 0;
    sMarkerCount = 0;
    sOamSubmitCount = 0;
    sPaletteCopyCount = 0;
    sResourceStateCount = 0;
    sControllerResetCount = 0;
    sInstalledResource = 0;
    sInstalledSize = 0;
    sReleasedHandle = 0;
    sDefaultBindCount = 0;
    sBoundPaletteRecord = NULL;
}

static void TestSceneSetupAndRelease(void)
{
    int index;

    ResetHarness();
    sRuntime.scene_field_10 = 7;
    sRuntime.scene_field_14 = 9;
    gGameSubDisplayControl_04001000 = 0xFFFFFFFFU;
    Game_InitializeDatalinkPostOamSceneGraphics();
    assert(sRuntime.scene_field_10 == 0U);
    assert(sRuntime.scene_field_14 == 0U);
    assert(gGameSubDisplayControl_04001000 == 0xFFFFF0FFU);
    assert(sConfigureCount == 1);
    assert(sLoadCount == 1);
    assert(sClearCount == 1);
    assert(sModeCount == 1);
    assert(sEnterTransitionCount == 1);

    Game_ReleaseDatalinkPostOamSceneGraphics();
    assert(sReleaseIndexCount == 4);
    for (index = 0; index < 4; ++index) {
        assert(sReleasedIndices[index] == (uint32_t)index);
    }
    assert(sReleaseSetCount == 1);
    assert(sExitTransitionCount == 1);

    gGameDatalinkPostOamHardwareState.display_state_d0 = 1;
    gGameDatalinkPostOamHardwareState.display_variant_d1 = 2;
    sReleaseIndexCount = 0;
    Game_ReleaseDatalinkPostOamSceneGraphics();
    assert(sExitTransitionCount == 1);
}

static void TestRemoteParticipantMarkerLayout(void)
{
    ResetHarness();
    gGameDatalinkPostOamSession.participant_count_00 = 4;
    gGameDatalinkPostOamSession.layout_mode_8e = 1;
    gGameDatalinkPostOamLocalSelection.local_participant_index_12 = 1;
    gGameDatalinkPostOamParticipants[0].marker_variant_24 = 0;
    gGameDatalinkPostOamParticipants[1].marker_variant_24 = 1;
    gGameDatalinkPostOamParticipants[2].marker_variant_24 = 2;
    gGameDatalinkPostOamParticipants[3].marker_variant_24 = 3;

    Game_RenderDatalinkRemoteParticipantOams();
    assert(sMarkerCount == 3);
    assert(sOamSubmitCount == 3);
    assert(sMarkerXs[0] == 62);
    assert(sMarkerXs[1] == 112);
    assert(sMarkerXs[2] == 162);
    assert(sMarkerFlags[0] == 8U);
    assert(sMarkerCodes[0] == 3U);
    assert(sMarkerCodes[1] == 5U);
    assert(sMarkerCodes[2] == 6U);
    assert(sOutput.submitted_oam_count_460 == 3U);
}

static void TestPendingResourceCommit(void)
{
    ResetHarness();
    sRuntime.resource_handle_58 = 0xAABBCCDDU;
    gGameDatalinkPostOamState.pending_resource_address_08 = 0x12345678U;
    gGameDatalinkPostOamState.pending_resource_size_04 = 0x400U;

    Game_CommitDatalinkPostOamPendingResource();
    assert(sResourceStateCount == 1);
    assert(sControllerResetCount == 1);
    assert(sInstalledResource == 0x12345678U);
    assert(sInstalledSize == 0x400U);
    assert(sReleasedHandle == 0xAABBCCDDU);
    assert(gGameDatalinkPostOamState.pending_resource_address_08 == 0U);
    assert(gGameDatalinkPostOamState.pending_resource_size_04 == 0U);
}

static void TestPaletteWaveAndUpload(void)
{
    ResetHarness();
    gGameDatalinkPostOamState.brightness_00 = 60;
    gGameDatalinkPostOamState.brightness_step_01 = 4;
    gGameDatalinkPostOamState.active_14 = 1;
    gGameDatalinkPostOamState.palette_phase_18 = 0x0F80U;
    Game_UpdateDatalinkPostOamPaletteWave();
    assert(gGameDatalinkPostOamState.brightness_00 == 64);
    assert(gGameDatalinkPostOamState.brightness_step_01 == 0);
    assert(gGameDatalinkPostOamHardwareState.brightness_f8 == 16);
    assert(gGameDatalinkPostOamState.palette_phase_18 == 0x1000U);
    assert(gGameDatalinkPostOamState.palette_dirty_0c == 1U);

    sRuntime.object_count_68 = 0;
    gGameDatalinkPostOamState.object_cursor_10 = 0;
    Game_UploadDatalinkPostOamPaletteAndObjects();
    assert(sPaletteCopyCount == GAME_DATALINK_POST_OAM_PALETTE_ROWS);
    assert(gGameDatalinkPostOamHardwareState.palette_origin_e0 == 0U);
    assert(gGameDatalinkPostOamHardwareState.palette_phase_e4 == 0x30U);
    assert(sDefaultBindCount == 1);
    assert(sBoundPaletteRecord == &gGameDatalinkPostOamPaletteRecords[1][0]);
    assert(gGameDatalinkPostOamState.active_14 == 0U);
    assert(gGameDatalinkPostOamState.palette_dirty_0c == 0U);

    ResetHarness();
    sRuntimeAvailable = 0;
    gGameDatalinkPostOamFlags = 8;
    gGameDatalinkPostOamState.active_14 = 1;
    Game_UpdateDatalinkPostOamPaletteWave();
    assert(gGameDatalinkPostOamState.palette_phase_18 == 0x400U);
}

int main(void)
{
    TestSceneSetupAndRelease();
    TestRemoteParticipantMarkerLayout();
    TestPendingResourceCommit();
    TestPaletteWaveAndUpload();
    return 0;
}
