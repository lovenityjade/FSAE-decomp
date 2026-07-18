#include "game/datalink_lobby_scene_initialize.h"

#include "game/datalink_graphics_resource.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

Game_DatalinkLobbyState gGameDatalinkLobbyState;
Game_DatalinkLobbyStateSlot gGameDatalinkLobbyStateSlot;
static Game_DatalinkLobbyOutput sOutput;
Game_DatalinkLobbyOutput *gGameDatalinkLobbyOutput = &sOutput;
Game_DatalinkLobbySceneControl gGameDatalinkLobbySceneControl;
uint32_t gGameDatalinkLobbyLayoutVariant;
uint8_t gGameDatalinkLobbyScratchBuffer[0x600];
uint8_t gGameDatalinkLobbyAuxiliaryState[0x7c];
uint8_t
    gGameDatalinkLobbyGraphicsArena[
        GAME_DATALINK_LOBBY_GRAPHICS_ARENA_SIZE
    ];
uint8_t gGameDatalinkLobbyRenderer[0x3c];
uint8_t gGameDatalinkLobbyLinkState[0x40];
volatile uint32_t gGameSubDisplayControl_04001000;

const int32_t gGameDatalinkLobbyInitialX = -0x20;
const int32_t gGameDatalinkLobbyVerticalBase = 0x18;
const Game_DatalinkLobbyTweenDescriptor
    gGameDatalinkLobbyTweenDescriptors[GAME_DATALINK_LOBBY_TWEEN_COUNT] = {
        {0, 0}, {1, 0}, {2, 0}, {3, 0},
        {5, 0}, {6, 0}, {4, 0}, {4, 0},
        {4, 0}, {4, 0}, {7, 0}, {7, 0},
        {7, 0}, {7, 0}, {13, 0}, {14, 0},
        {15, 0}, {16, 0}, {0x80, 0x1000}, {0x81, 0x1000},
        {0x82, 0x1000}, {0x83, 0x1000}, {0x84, 0x1000}, {17, 0},
        {18, 0}, {19, 0}, {20, 0}, {21, 0x1000},
        {-1, -2}, {-3, -4}, {0, 0}, {0, 0}
    };

static Game_DatalinkTweenRecord sTweens[GAME_DATALINK_LOBBY_TWEEN_COUNT];
static uint8_t sFirstTileBuffer[1];
static uint8_t sSecondTileBuffer[1];
static uint8_t sLargeBuffer[1];
static uint8_t sAnimations[30];
static uint8_t sSequences[30];
static uint8_t sLocalCellBank;
static uint8_t sLocalAnimationBank;
static uint8_t sSharedCellBank;
static uint8_t sSharedAnimationBank;
static uint8_t sResourceContext;
static uint8_t sWirelessContext;

static struct {
    uint8_t unknown_00[0x10];
    uint32_t field_10;
    uint32_t field_14;
} sGraphicsManager;

static int sRuntimeAvailable;
static int sWirelessState;
static unsigned int sFillCalls;
static unsigned int sFastClearCalls;
static unsigned int sAllocationCalls;
static unsigned int sGlobalAllocationCalls;
static unsigned int sAuxiliaryClearCalls;
static unsigned int sTweenInitCalls;
static unsigned int sSubDisplayCalls;
static unsigned int sManagerCalls;
static unsigned int sAvailabilityCalls;
static unsigned int sBackgroundCalls;
static unsigned int sCharacterLayerCalls;
static unsigned int sScreenCalls;
static unsigned int sObjectResourceCalls;
static unsigned int sCharacterLayer2Calls;
static unsigned int sCellResourceCalls;
static unsigned int sAnimationAllocationCalls;
static unsigned int sSequenceCalls;
static unsigned int sAnimationInitCalls;
static unsigned int sParticipantCalls;
static unsigned int sResourceContextCalls;
static unsigned int sRendererCalls;
static unsigned int sGraphicsModeCalls;
static unsigned int sWirelessContextCalls;
static unsigned int sWirelessStateCalls;
static unsigned int sCommitCalls;
static unsigned int sRefreshCalls;
static uint32_t sScreenResources[2];
static int sCommitTargets[2];

static uint32_t Test_TargetAddress(const void *pointer)
{
    return (uint32_t)(uintptr_t)pointer;
}

static uint32_t Test_ReadWord(const uint8_t *bytes)
{
    return (uint32_t)bytes[0] |
        ((uint32_t)bytes[1] << 8) |
        ((uint32_t)bytes[2] << 16) |
        ((uint32_t)bytes[3] << 24);
}

static void Test_WriteWord(uint8_t *bytes, uint32_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
    bytes[2] = (uint8_t)(value >> 16);
    bytes[3] = (uint8_t)(value >> 24);
}

static void Test_Reset(int runtime_available, int wireless_state)
{
    memset(&gGameDatalinkLobbyState, 0xa5, sizeof(gGameDatalinkLobbyState));
    memset(&gGameDatalinkLobbyStateSlot, 0, sizeof(gGameDatalinkLobbyStateSlot));
    memset(&sOutput, 0xa5, sizeof(sOutput));
    memset(&gGameDatalinkLobbySceneControl, 0xa5,
        sizeof(gGameDatalinkLobbySceneControl));
    memset(gGameDatalinkLobbyScratchBuffer, 0xa5,
        sizeof(gGameDatalinkLobbyScratchBuffer));
    memset(gGameDatalinkLobbyAuxiliaryState, 0xa5,
        sizeof(gGameDatalinkLobbyAuxiliaryState));
    memset(gGameDatalinkLobbyGraphicsArena, 0,
        sizeof(gGameDatalinkLobbyGraphicsArena));
    memset(gGameDatalinkLobbyRenderer, 0,
        sizeof(gGameDatalinkLobbyRenderer));
    memset(gGameDatalinkLobbyLinkState, 0xa5,
        sizeof(gGameDatalinkLobbyLinkState));
    memset(sTweens, 0xa5, sizeof(sTweens));
    memset(&sGraphicsManager, 0xa5, sizeof(sGraphicsManager));
    Test_WriteWord(gGameDatalinkLobbyGraphicsArena + 0xb0, 0x20000000);

    gGameDatalinkLobbyLayoutVariant = 0;
    gGameSubDisplayControl_04001000 = UINT32_C(0xabcd1234);
    sRuntimeAvailable = runtime_available;
    sWirelessState = wireless_state;
    sFillCalls = 0;
    sFastClearCalls = 0;
    sAllocationCalls = 0;
    sGlobalAllocationCalls = 0;
    sAuxiliaryClearCalls = 0;
    sTweenInitCalls = 0;
    sSubDisplayCalls = 0;
    sManagerCalls = 0;
    sAvailabilityCalls = 0;
    sBackgroundCalls = 0;
    sCharacterLayerCalls = 0;
    sScreenCalls = 0;
    sObjectResourceCalls = 0;
    sCharacterLayer2Calls = 0;
    sCellResourceCalls = 0;
    sAnimationAllocationCalls = 0;
    sSequenceCalls = 0;
    sAnimationInitCalls = 0;
    sParticipantCalls = 0;
    sResourceContextCalls = 0;
    sRendererCalls = 0;
    sGraphicsModeCalls = 0;
    sWirelessContextCalls = 0;
    sWirelessStateCalls = 0;
    sCommitCalls = 0;
    sRefreshCalls = 0;
}

void MI_CpuFill(void *destination, uint32_t value, uint32_t size)
{
    assert(destination == &gGameDatalinkLobbyState);
    assert(value == 0);
    assert(size == 0xbc);
    memset(destination, (int)value, size);
    ++sFillCalls;
}

void MIi_CpuClearFast(uint32_t value, void *destination, uint32_t size)
{
    assert(value == 0);
    assert(destination == gGameDatalinkLobbyScratchBuffer);
    assert(size == sizeof(gGameDatalinkLobbyScratchBuffer));
    memset(destination, 0, size);
    ++sFastClearCalls;
}

void *Game_AllocateDatalinkLobbyMemory_0208e148(uint32_t size)
{
    assert(size == sizeof(sTweens));
    ++sAllocationCalls;
    return sTweens;
}

void *EL_GetGlobalAdr(uint32_t size, int alignment)
{
    void *result;

    assert(alignment == 0x20);
    if (sGlobalAllocationCalls == 0) {
        assert(size == 0x600);
        result = sFirstTileBuffer;
    } else if (sGlobalAllocationCalls == 1) {
        assert(size == 0x600);
        result = sSecondTileBuffer;
    } else {
        assert(sGlobalAllocationCalls == 2);
        assert(size == 0x5000);
        result = sLargeBuffer;
    }
    ++sGlobalAllocationCalls;
    return result;
}

void Game_ClearDatalinkLobbyAuxiliaryState_020a1a1c(void *state)
{
    assert(state == gGameDatalinkLobbyAuxiliaryState);
    memset(state, 0, sizeof(gGameDatalinkLobbyAuxiliaryState));
    ++sAuxiliaryClearCalls;
}

void Game_InitializeDatalinkTweenRecord_020acbf4(
    Game_DatalinkTweenRecord *tween,
    uint8_t slot_index)
{
    assert(tween == &sTweens[sTweenInitCalls]);
    assert(slot_index == sTweenInitCalls);
    memset(tween, 0, sizeof(*tween));
    tween->slot_index_00 = slot_index;
    tween->initialized_01 = 1;
    tween->active_03 = 1;
    ++sTweenInitCalls;
}

void Game_ConfigureDatalinkLobbySubDisplay_020b1224(void)
{
    ++sSubDisplayCalls;
}

void *Game_GetDatalinkLobbyGraphicsManager_0208da4c(void)
{
    ++sManagerCalls;
    return &sGraphicsManager;
}

int Game_IsDatalinkLobbyRuntimeAvailable_0208de54(void)
{
    ++sAvailabilityCalls;
    return sRuntimeAvailable;
}

void Game_LoadDatalinkLobbyBackground_0209c1d0(
    int character_resource,
    int palette_resource,
    uint32_t screen_resource)
{
    assert(character_resource == 0x12);
    assert(palette_resource == 0x13);
    assert(screen_resource == UINT32_MAX);
    ++sBackgroundCalls;
}

void Game_LoadDatalinkLobbyCharacterLayer_0209c384(
    uint32_t resource,
    uint32_t destination_offset)
{
    assert(resource == 0);
    assert(destination_offset == 0x2000);
    ++sCharacterLayerCalls;
}

void Game_LoadDatalinkScreenResourceIntoSet(
    Game_DatalinkGraphicsResourceSet *resources,
    int target,
    uint32_t resource_id)
{
    uint8_t *expected = gGameDatalinkLobbyGraphicsArena +
        (sScreenCalls == 0 ? 0xbc : 0x9c);

    assert((uint8_t *)(void *)resources == expected);
    assert(target == (sScreenCalls == 0 ? 2 : 1));
    assert(sScreenCalls < 2);
    sScreenResources[sScreenCalls] = resource_id;
    ++sScreenCalls;
}

void Game_LoadDatalinkLobbyObjectResources_0209c41c(
    int character_resource,
    int palette_resource,
    int cell_bank_resource,
    int animation_bank_resource)
{
    assert(character_resource == 0x17);
    assert(palette_resource == 0x1a);
    assert(cell_bank_resource == 0x1b);
    assert(animation_bank_resource == 0x1c);
    Test_WriteWord(
        gGameDatalinkLobbyGraphicsArena + 0x1c + 0x1c,
        Test_TargetAddress(&sSharedCellBank));
    Test_WriteWord(
        gGameDatalinkLobbyGraphicsArena + 0x1c + 0x24,
        Test_TargetAddress(&sSharedAnimationBank));
    ++sObjectResourceCalls;
}

void Game_LoadDatalinkLobbyCharacterLayer2_0209c62c(
    uint32_t resource,
    uint32_t destination_offset)
{
    assert(resource == 1);
    assert(destination_offset == 0x2000);
    ++sCharacterLayer2Calls;
}

void Game_LoadDatalinkCellAnimationResources(
    Game_DatalinkGraphicsResourceSet *resources,
    uint32_t cell_bank_resource_id,
    uint32_t animation_bank_resource_id)
{
    uint8_t *bytes = (uint8_t *)(void *)resources;

    assert(bytes == (uint8_t *)(void *)
        &gGameDatalinkLobbyState.local_resources_3c);
    assert(cell_bank_resource_id == 0x18);
    assert(animation_bank_resource_id == 0x19);
    Test_WriteWord(bytes + 0x1c, Test_TargetAddress(&sLocalCellBank));
    Test_WriteWord(bytes + 0x24, Test_TargetAddress(&sLocalAnimationBank));
    ++sCellResourceCalls;
}

void *Game_AllocateDatalinkCellAnimation_0209be84(uint32_t *pool_index)
{
    assert(sAnimationAllocationCalls < 30);
    *pool_index = 0x100 + sAnimationAllocationCalls;
    return &sAnimations[sAnimationAllocationCalls++];
}

const void *NNS_G2dGetAnimSequenceByIdx(
    const void *animation_bank,
    uint16_t sequence_index)
{
    static const uint8_t expected_sequences[30] = {
        0, 1, 2, 3, 5, 6, 4, 4, 4, 4, 7, 7, 7, 7,
        13, 14, 15, 16, 0, 1, 2, 3, 4, 17, 18, 19, 20, 21, 0, 0
    };
    uint32_t expected_bank = sSequenceCalls >= 18 && sSequenceCalls < 23
        ? Test_TargetAddress(&sSharedAnimationBank)
        : Test_TargetAddress(&sLocalAnimationBank);

    assert((uint32_t)(uintptr_t)animation_bank == expected_bank);
    assert(sequence_index == expected_sequences[sSequenceCalls]);
    return &sSequences[sSequenceCalls++];
}

void NNS_G2dInitCellAnimation(
    void *animation,
    const void *sequence,
    const void *cell_bank)
{
    uint32_t expected_bank = sAnimationInitCalls >= 18 &&
            sAnimationInitCalls < 23
        ? Test_TargetAddress(&sSharedCellBank)
        : Test_TargetAddress(&sLocalCellBank);

    assert(animation == &sAnimations[sAnimationInitCalls]);
    assert(sequence == &sSequences[sAnimationInitCalls]);
    assert((uint32_t)(uintptr_t)cell_bank == expected_bank);
    ++sAnimationInitCalls;
}

void Game_SetDatalinkParticipantIndex_0209cf78(
    int participant,
    int8_t value)
{
    assert(participant == (int)sParticipantCalls);
    assert(value == (int8_t)sParticipantCalls);
    ++sParticipantCalls;
}

void *Game_GetDatalinkLobbyResourceContext_0208dfa4(void)
{
    ++sResourceContextCalls;
    return &sResourceContext;
}

uint32_t Game_InitializeDatalinkLobbyRenderer_020a2b14(
    void *renderer,
    void *resource_context,
    int argument)
{
    assert(renderer == gGameDatalinkLobbyRenderer);
    assert(resource_context == &sResourceContext);
    assert(argument == 0);
    ++sRendererCalls;
    return 1;
}

void Game_SetDatalinkLobbyGraphicsMode_020a3778(uint32_t mode)
{
    assert(mode == 0xde);
    ++sGraphicsModeCalls;
}

void *Game_GetDatalinkWirelessContext_020909c8(void)
{
    ++sWirelessContextCalls;
    return &sWirelessContext;
}

int Game_GetDatalinkWirelessState_02090ea4(void *context)
{
    assert(context == &sWirelessContext);
    ++sWirelessStateCalls;
    return sWirelessState;
}

void Game_CommitDatalinkLobbyBackground_0209da54(int target)
{
    assert(sCommitCalls < 2);
    sCommitTargets[sCommitCalls++] = target;
}

void Game_RefreshDatalinkLobbyTilemaps_020b4044(void)
{
    ++sRefreshCalls;
}

static void Test_AssertCommon(int runtime_available, int wireless_state)
{
    unsigned int index;

    assert(sFillCalls == 1);
    assert(sFastClearCalls == 1);
    assert(sAllocationCalls == 1);
    assert(sGlobalAllocationCalls == 3);
    assert(sAuxiliaryClearCalls == 1);
    assert(sTweenInitCalls == GAME_DATALINK_LOBBY_TWEEN_COUNT);
    assert(sSubDisplayCalls == 1);
    assert(sManagerCalls == 5);
    assert(sAvailabilityCalls == 3);
    assert(sBackgroundCalls == 1);
    assert(sCharacterLayerCalls == 1);
    assert(sScreenCalls == 2);
    assert(sObjectResourceCalls == 1);
    assert(sCharacterLayer2Calls == 1);
    assert(sCellResourceCalls == 1);
    assert(sAnimationAllocationCalls == 30);
    assert(sSequenceCalls == 30);
    assert(sAnimationInitCalls == 30);
    assert(sParticipantCalls == 4);
    assert(sResourceContextCalls == 1);
    assert(sRendererCalls == 1);
    assert(sGraphicsModeCalls == 1);
    assert(sCommitCalls == 2);
    assert(sCommitTargets[0] == 1);
    assert(sCommitTargets[1] == 2);
    assert(sRefreshCalls == 1);

    assert(sOutput.first_output_count_51c == 0);
    assert(sOutput.second_output_count_520 == 0);
    assert(gGameDatalinkLobbyStateSlot.state_address_04 ==
        Test_TargetAddress(&gGameDatalinkLobbyState));
    assert(gGameDatalinkLobbyState.tween_records_address_34 ==
        Test_TargetAddress(sTweens));
    assert(gGameDatalinkLobbyState.tween_count_38 == 32);
    assert(gGameDatalinkLobbyState.first_tile_buffer_address_b0 ==
        Test_TargetAddress(sFirstTileBuffer));
    assert(gGameDatalinkLobbyState.second_tile_buffer_address_b4 ==
        Test_TargetAddress(sSecondTileBuffer));
    assert(gGameDatalinkLobbyState.large_buffer_address_b8 ==
        Test_TargetAddress(sLargeBuffer));
    for (index = 0; index < 4; ++index) {
        assert(gGameDatalinkLobbyState.participant_states_10[index] == 3);
        assert(Test_ReadWord(
            gGameDatalinkLobbyGraphicsArena + 0x2eb0 + index * 0x18) ==
            0x2c000);
    }

    assert(sScreenResources[0] ==
        (runtime_available ? UINT32_C(0x14) : UINT32_C(0x2e)));
    assert(sScreenResources[1] == 0x15);
    assert(sTweens[28].unknown_02 == 1);
    assert(sTweens[29].unknown_02 == 3);
    assert(sTweens[30].unknown_02 == 0);
    assert(sTweens[31].unknown_02 == 0);
    assert(sTweens[29].current_x_fx_30 == -0x100000);
    assert(sTweens[30].current_x_fx_30 == -0x100000);
    assert(Test_ReadWord(gGameDatalinkLobbyGraphicsArena + 0x30d8) == 0);
    assert(Test_ReadWord(gGameDatalinkLobbyGraphicsArena + 0x30e0) ==
        UINT32_C(0xffffff00));
    assert(Test_ReadWord(gGameDatalinkLobbyGraphicsArena + 0x30e8) ==
        UINT32_C(0xffffff00));

    assert(Test_ReadWord(gGameDatalinkLobbyRenderer + 0x38) ==
        UINT32_C(0x2000000c));
    assert(sGraphicsManager.field_10 == 0);
    assert(sGraphicsManager.field_14 == 0);
    assert(gGameSubDisplayControl_04001000 ==
        ((UINT32_C(0xabcd1234) & ~UINT32_C(0x1f00)) |
         UINT32_C(0x1f00)));
    assert(Test_ReadWord(gGameDatalinkLobbyLinkState + 0x3c) == 0);

    assert(sTweens[0].current_x_fx_30 == 0x100000);
    assert(sTweens[0].current_y_fx_34 == -0x20000);
    assert(sTweens[1].current_y_fx_34 == -0x20000);
    assert(sTweens[2].current_y_fx_34 == 0xd8000);
    assert(sTweens[3].current_y_fx_34 == 0xd8000);
    assert(sTweens[4].current_x_fx_30 == 0x18000);
    assert(sTweens[4].current_y_fx_34 == 0xcc000);
    assert(sTweens[5].current_x_fx_30 == 0x80000);
    assert(sTweens[5].current_y_fx_34 == 0x60000);
    assert(sTweens[5].unknown_04 == 1);
    assert(sTweens[5].frame_counter_05 == 0);

    if (runtime_available) {
        assert(gGameDatalinkLobbySceneControl.fallback_state_f64 == 0);
        assert(gGameDatalinkLobbyState.phase_28 == 0);
        assert(gGameDatalinkLobbyLayoutVariant == 4);
        assert(sWirelessContextCalls == 1);
        assert(sWirelessStateCalls == 1);
        assert(gGameDatalinkLobbyState.wireless_state_a4 ==
            (wireless_state == 8 ? 1 : 0));
    } else {
        assert(gGameDatalinkLobbySceneControl.fallback_state_f64 == 2);
        assert(gGameDatalinkLobbyState.phase_28 == 1);
        assert(gGameDatalinkLobbyState.fallback_enabled_ad == 1);
        assert(gGameDatalinkLobbyState.fallback_mode_ae == 2);
        assert(gGameDatalinkLobbyState.fallback_variant_af == 2);
        assert(gGameDatalinkLobbyLayoutVariant == 6);
        assert(sWirelessContextCalls == 0);
        assert(sWirelessStateCalls == 0);
    }
    assert(gGameDatalinkLobbySceneControl.transition_state_f6c == 0);
    assert(gGameDatalinkLobbySceneControl.transition_timer_f70 == 0);
    assert(gGameDatalinkLobbySceneControl.field_f4c == 0x40);
}

int main(void)
{
    Test_Reset(0, 0);
    Game_InitializeDatalinkLobbyScene_020b1378();
    Test_AssertCommon(0, 0);

    Test_Reset(1, 8);
    Game_InitializeDatalinkLobbyScene_020b1378();
    Test_AssertCommon(1, 8);

    Test_Reset(1, 5);
    Game_InitializeDatalinkLobbyScene_020b1378();
    Test_AssertCommon(1, 5);
    return 0;
}
