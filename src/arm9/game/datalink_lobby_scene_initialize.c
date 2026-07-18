#include "game/datalink_lobby_scene_initialize.h"

#include "game/datalink_graphics_resource.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_LOBBY_STATE_SIZE = 0xbc,
    DATALINK_LOBBY_TWEEN_SIZE = 0x58,
    DATALINK_LOBBY_TILE_BUFFER_SIZE = 0x600,
    DATALINK_LOBBY_LARGE_BUFFER_SIZE = 0x5000,
    DATALINK_LOBBY_ALIGNMENT = 0x20,
    DATALINK_LOBBY_FX_ONE = 0x1000,

    DATALINK_LOBBY_SHARED_RESOURCES_OFFSET = 0x1c,
    DATALINK_LOBBY_FIRST_SCREEN_RESOURCES_OFFSET = 0x9c,
    DATALINK_LOBBY_SECOND_SCREEN_RESOURCES_OFFSET = 0xbc,
    DATALINK_LOBBY_RENDERER_SOURCE_OFFSET = 0xb0,
    DATALINK_LOBBY_PARTICIPANT_FIELDS_OFFSET = 0x2eb0,
    DATALINK_LOBBY_PARTICIPANT_FIELD_STRIDE = 0x18,
    DATALINK_LOBBY_POSITION_CACHE_OFFSET = 0x30d8,

    DATALINK_LOBBY_PARTICIPANT_COUNT = 4,
    DATALINK_LOBBY_CACHED_POSITION_COUNT = 3,
    DATALINK_LOBBY_FALLBACK_LAYOUT = 6,
    DATALINK_LOBBY_NORMAL_LAYOUT = 4,
    DATALINK_LOBBY_FALLBACK_STATE = 2,
    DATALINK_LOBBY_PARTICIPANT_FIELD_VALUE = 0x2c000,
    DATALINK_LOBBY_DISPLAY_PLANE_MASK = 0x1f00,
    DATALINK_LOBBY_GRAPHICS_MODE = 0xde,
    DATALINK_LOBBY_WIRELESS_READY_STATE = 8
};

typedef struct Game_DatalinkLobbyGraphicsManager {
    uint8_t unknown_00[0x10];
    uint32_t scene_field_10;
    uint32_t scene_field_14;
} Game_DatalinkLobbyGraphicsManager;

extern volatile uint32_t gGameSubDisplayControl_04001000;

extern void MI_CpuFill(void *destination, uint32_t value, uint32_t size);
extern void MIi_CpuClearFast(
    uint32_t value,
    void *destination,
    uint32_t size);
extern void *EL_GetGlobalAdr(uint32_t size, int alignment);
extern void *Game_AllocateDatalinkLobbyMemory_0208e148(uint32_t size);
extern void Game_ClearDatalinkLobbyAuxiliaryState_020a1a1c(void *state);
extern void Game_ConfigureDatalinkLobbySubDisplay_020b1224(void);
extern Game_DatalinkLobbyGraphicsManager *
    Game_GetDatalinkLobbyGraphicsManager_0208da4c(void);
extern int Game_IsDatalinkLobbyRuntimeAvailable_0208de54(void);
extern void Game_LoadDatalinkLobbyBackground_0209c1d0(
    int character_resource,
    int palette_resource,
    uint32_t screen_resource);
extern void Game_LoadDatalinkLobbyCharacterLayer_0209c384(
    uint32_t resource,
    uint32_t destination_offset);
extern void Game_LoadDatalinkLobbyObjectResources_0209c41c(
    int character_resource,
    int palette_resource,
    int cell_bank_resource,
    int animation_bank_resource);
extern void Game_LoadDatalinkLobbyCharacterLayer2_0209c62c(
    uint32_t resource,
    uint32_t destination_offset);
extern void *Game_AllocateDatalinkCellAnimation_0209be84(
    uint32_t *pool_index);
extern const void *NNS_G2dGetAnimSequenceByIdx(
    const void *animation_bank,
    uint16_t sequence_index);
extern void NNS_G2dInitCellAnimation(
    void *animation,
    const void *sequence,
    const void *cell_bank);
extern void Game_SetDatalinkParticipantIndex_0209cf78(
    int participant,
    int8_t value);
extern void *Game_GetDatalinkLobbyResourceContext_0208dfa4(void);
extern uint32_t Game_InitializeDatalinkLobbyRenderer_020a2b14(
    void *renderer,
    void *resource_context,
    int argument);
extern void Game_SetDatalinkLobbyGraphicsMode_020a3778(uint32_t mode);
extern void *Game_GetDatalinkWirelessContext_020909c8(void);
extern int Game_GetDatalinkWirelessState_02090ea4(void *context);
extern void Game_CommitDatalinkLobbyBackground_0209da54(int target);
extern void Game_RefreshDatalinkLobbyTilemaps_020b4044(void);

_Static_assert(
    sizeof(Game_DatalinkLobbyResourceSet32) == 0x28,
    "lobby resource-set target size");
_Static_assert(
    offsetof(Game_DatalinkLobbyState, participant_states_10) == 0x10,
    "lobby participant-state offset");
_Static_assert(
    offsetof(Game_DatalinkLobbyState, phase_28) == 0x28,
    "lobby phase offset");
_Static_assert(
    offsetof(Game_DatalinkLobbyState, tween_records_address_34) == 0x34,
    "lobby tween-address offset");
_Static_assert(
    offsetof(Game_DatalinkLobbyState, local_resources_3c) == 0x3c,
    "lobby local-resource offset");
_Static_assert(
    offsetof(Game_DatalinkLobbyState, wireless_state_a4) == 0xa4,
    "lobby wireless-state offset");
_Static_assert(
    offsetof(Game_DatalinkLobbyState, fallback_enabled_ad) == 0xad,
    "lobby fallback flag offset");
_Static_assert(
    offsetof(Game_DatalinkLobbyState, first_tile_buffer_address_b0) == 0xb0,
    "lobby first tile-buffer offset");
_Static_assert(
    sizeof(Game_DatalinkLobbyState) == DATALINK_LOBBY_STATE_SIZE,
    "lobby state target size");
_Static_assert(
    offsetof(Game_DatalinkLobbyOutput, first_output_count_51c) == 0x51c,
    "lobby output-count offset");
_Static_assert(
    offsetof(Game_DatalinkLobbySceneControl, field_f4c) == 0xf4c,
    "lobby scene field offset");
_Static_assert(
    offsetof(Game_DatalinkLobbySceneControl, fallback_state_f64) == 0xf64,
    "lobby fallback-state offset");

static uint32_t Game_DatalinkLobbyTargetAddress(const void *pointer)
{
    return (uint32_t)(uintptr_t)pointer;
}

static uint32_t Game_ReadDatalinkLobbyWord(const uint8_t *bytes)
{
    return (uint32_t)bytes[0] |
        ((uint32_t)bytes[1] << 8) |
        ((uint32_t)bytes[2] << 16) |
        ((uint32_t)bytes[3] << 24);
}

static void Game_WriteDatalinkLobbyWord(uint8_t *bytes, uint32_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
    bytes[2] = (uint8_t)(value >> 16);
    bytes[3] = (uint8_t)(value >> 24);
}

static void Game_WriteDatalinkLobbyHalfword(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
}

static const void *Game_DatalinkLobbyTargetPointer(uint32_t address)
{
    return (const void *)(uintptr_t)address;
}

static void Game_InitializeDatalinkLobbyAnimation(
    Game_DatalinkTweenRecord *tween,
    const Game_DatalinkLobbyTweenDescriptor *descriptor,
    const Game_DatalinkLobbyResourceSet32 *local_resources)
{
    const Game_DatalinkLobbyResourceSet32 *resources = local_resources;
    uint32_t sequence = (uint32_t)descriptor->sequence;
    void *animation;
    const void *animation_sequence;

    if (descriptor->sequence < 0) {
        tween->unknown_02 = (uint8_t)(-(int8_t)sequence);
        return;
    }
    if ((sequence & UINT32_C(0x80)) != 0U) {
        resources = (const Game_DatalinkLobbyResourceSet32 *)(const void *)(
            gGameDatalinkLobbyGraphicsArena +
            DATALINK_LOBBY_SHARED_RESOURCES_OFFSET);
        sequence &= UINT32_C(0x7f);
    }

    Game_WriteDatalinkLobbyHalfword(
        (uint8_t *)(void *)tween + 0x48,
        (uint16_t)sequence);
    tween->cell_animation_delta_fx_50 = descriptor->animation_delta_fx;
    animation = Game_AllocateDatalinkCellAnimation_0209be84(
        (uint32_t *)(void *)((uint8_t *)(void *)tween + 0x4c));
    tween->cell_animation_address_54 =
        Game_DatalinkLobbyTargetAddress(animation);
    animation_sequence = NNS_G2dGetAnimSequenceByIdx(
        Game_DatalinkLobbyTargetPointer(resources->animation_bank_24),
        (uint16_t)sequence);
    NNS_G2dInitCellAnimation(
        animation,
        animation_sequence,
        Game_DatalinkLobbyTargetPointer(resources->cell_bank_1c));
}

static void Game_ConfigureDatalinkLobbyTweenPositions(
    Game_DatalinkTweenRecord *tweens)
{
    int32_t upper_y_fx = gGameDatalinkLobbyInitialX * DATALINK_LOBBY_FX_ONE;
    int32_t lower_y_fx =
        (gGameDatalinkLobbyVerticalBase + 0xc0) * DATALINK_LOBBY_FX_ONE;

    tweens[0].duration_fx_28 = 0;
    tweens[0].current_x_fx_30 = 0x100000;
    tweens[0].current_y_fx_34 = upper_y_fx;
    tweens[1].duration_fx_28 = 0;
    tweens[1].current_x_fx_30 = 0x100000;
    tweens[1].current_y_fx_34 = upper_y_fx;

    tweens[4].duration_fx_28 = 0;
    tweens[4].current_x_fx_30 = 0x18000;
    tweens[4].current_y_fx_34 =
        (gGameDatalinkLobbyVerticalBase + 0xb4) *
        DATALINK_LOBBY_FX_ONE;

    tweens[3].duration_fx_28 = 0;
    tweens[3].current_x_fx_30 = 0x100000;
    tweens[3].current_y_fx_34 = lower_y_fx;
    tweens[2].duration_fx_28 = 0;
    tweens[2].current_x_fx_30 = 0x100000;
    tweens[2].current_y_fx_34 = lower_y_fx;

    tweens[5].duration_fx_28 = 0;
    tweens[5].current_x_fx_30 = 0x80000;
    tweens[5].current_y_fx_34 = 0x60000;
    tweens[5].unknown_04 = 1;
    tweens[5].frame_counter_05 = 0;
}

/* 0x020b1378..0x020b18db */
void Game_InitializeDatalinkLobbyScene_020b1378(void)
{
    Game_DatalinkLobbyTweenDescriptor
        descriptors[GAME_DATALINK_LOBBY_TWEEN_COUNT];
    Game_DatalinkLobbyGraphicsManager *graphics_manager;
    Game_DatalinkTweenRecord *tweens;
    void *wireless_context;
    void *resource_context;
    uint32_t screen_resource;
    unsigned int index;

    gGameDatalinkLobbyOutput->first_output_count_51c = 0;
    gGameDatalinkLobbyOutput->second_output_count_520 = 0;
    gGameDatalinkLobbyStateSlot.state_address_04 =
        Game_DatalinkLobbyTargetAddress(&gGameDatalinkLobbyState);
    MI_CpuFill(
        &gGameDatalinkLobbyState,
        0,
        DATALINK_LOBBY_STATE_SIZE);

    gGameDatalinkLobbyState.phase_28 = 0;
    gGameDatalinkLobbyState.tween_count_38 =
        GAME_DATALINK_LOBBY_TWEEN_COUNT;
    tweens = Game_AllocateDatalinkLobbyMemory_0208e148(
        GAME_DATALINK_LOBBY_TWEEN_COUNT * DATALINK_LOBBY_TWEEN_SIZE);
    gGameDatalinkLobbyState.tween_records_address_34 =
        Game_DatalinkLobbyTargetAddress(tweens);
    gGameDatalinkLobbyState.first_tile_buffer_address_b0 =
        Game_DatalinkLobbyTargetAddress(
            EL_GetGlobalAdr(
                DATALINK_LOBBY_TILE_BUFFER_SIZE,
                DATALINK_LOBBY_ALIGNMENT));
    gGameDatalinkLobbyState.second_tile_buffer_address_b4 =
        Game_DatalinkLobbyTargetAddress(
            EL_GetGlobalAdr(
                DATALINK_LOBBY_TILE_BUFFER_SIZE,
                DATALINK_LOBBY_ALIGNMENT));
    gGameDatalinkLobbyState.large_buffer_address_b8 =
        Game_DatalinkLobbyTargetAddress(
            EL_GetGlobalAdr(
                DATALINK_LOBBY_LARGE_BUFFER_SIZE,
                DATALINK_LOBBY_ALIGNMENT));

    MIi_CpuClearFast(
        0,
        gGameDatalinkLobbyScratchBuffer,
        DATALINK_LOBBY_TILE_BUFFER_SIZE);
    Game_ClearDatalinkLobbyAuxiliaryState_020a1a1c(
        gGameDatalinkLobbyAuxiliaryState);
    for (index = 0; index < DATALINK_LOBBY_PARTICIPANT_COUNT; ++index) {
        gGameDatalinkLobbyState.participant_states_10[index] = 3;
    }
    for (index = 0; index < GAME_DATALINK_LOBBY_TWEEN_COUNT; ++index) {
        Game_InitializeDatalinkTweenRecord_020acbf4(
            &tweens[index],
            (uint8_t)index);
    }

    Game_ConfigureDatalinkLobbySubDisplay_020b1224();
    (void)Game_GetDatalinkLobbyGraphicsManager_0208da4c();
    if (Game_IsDatalinkLobbyRuntimeAvailable_0208de54() == 0) {
        gGameDatalinkLobbySceneControl.fallback_state_f64 =
            DATALINK_LOBBY_FALLBACK_STATE;
        gGameDatalinkLobbyState.phase_28 = 1;
        gGameDatalinkLobbyState.fallback_enabled_ad = 1;
        gGameDatalinkLobbyState.fallback_mode_ae = 2;
        gGameDatalinkLobbyState.fallback_variant_af = 2;
        gGameDatalinkLobbyLayoutVariant = DATALINK_LOBBY_FALLBACK_LAYOUT;
    } else {
        gGameDatalinkLobbySceneControl.fallback_state_f64 = 0;
        gGameDatalinkLobbyLayoutVariant = DATALINK_LOBBY_NORMAL_LAYOUT;
    }
    gGameDatalinkLobbySceneControl.transition_state_f6c = 0;
    gGameDatalinkLobbySceneControl.transition_timer_f70 = 0;
    gGameDatalinkLobbySceneControl.field_f4c = 0x40;

    Game_LoadDatalinkLobbyBackground_0209c1d0(0x12, 0x13, UINT32_MAX);
    (void)Game_GetDatalinkLobbyGraphicsManager_0208da4c();
    screen_resource =
        Game_IsDatalinkLobbyRuntimeAvailable_0208de54() == 0
        ? UINT32_C(0x2e)
        : UINT32_C(0x14);
    Game_LoadDatalinkLobbyCharacterLayer_0209c384(0, 0x2000);
    Game_LoadDatalinkScreenResourceIntoSet(
        (Game_DatalinkGraphicsResourceSet *)(void *)(
            gGameDatalinkLobbyGraphicsArena +
            DATALINK_LOBBY_SECOND_SCREEN_RESOURCES_OFFSET),
        2,
        screen_resource);
    Game_LoadDatalinkScreenResourceIntoSet(
        (Game_DatalinkGraphicsResourceSet *)(void *)(
            gGameDatalinkLobbyGraphicsArena +
            DATALINK_LOBBY_FIRST_SCREEN_RESOURCES_OFFSET),
        1,
        0x15);
    Game_LoadDatalinkLobbyObjectResources_0209c41c(
        0x17, 0x1a, 0x1b, 0x1c);
    Game_LoadDatalinkLobbyCharacterLayer2_0209c62c(1, 0x2000);

    for (index = 0; index < GAME_DATALINK_LOBBY_TWEEN_COUNT; ++index) {
        descriptors[index] = gGameDatalinkLobbyTweenDescriptors[index];
    }
    Game_LoadDatalinkCellAnimationResources(
        (Game_DatalinkGraphicsResourceSet *)(void *)
            &gGameDatalinkLobbyState.local_resources_3c,
        0x18,
        0x19);
    for (index = 0; index < GAME_DATALINK_LOBBY_TWEEN_COUNT; ++index) {
        Game_InitializeDatalinkLobbyAnimation(
            &tweens[index],
            &descriptors[index],
            &gGameDatalinkLobbyState.local_resources_3c);
    }

    for (index = 0; index < DATALINK_LOBBY_PARTICIPANT_COUNT; ++index) {
        Game_SetDatalinkParticipantIndex_0209cf78(
            (int)index,
            (int8_t)index);
        Game_WriteDatalinkLobbyWord(
            gGameDatalinkLobbyGraphicsArena +
                DATALINK_LOBBY_PARTICIPANT_FIELDS_OFFSET +
                index * DATALINK_LOBBY_PARTICIPANT_FIELD_STRIDE,
            DATALINK_LOBBY_PARTICIPANT_FIELD_VALUE);
    }

    tweens[29].duration_fx_28 = 0;
    tweens[29].current_x_fx_30 = -0x100000;
    tweens[29].current_y_fx_34 = 0;
    tweens[30].duration_fx_28 = 0;
    tweens[30].current_x_fx_30 = -0x100000;
    tweens[30].current_y_fx_34 = 0;
    for (index = 0; index < DATALINK_LOBBY_CACHED_POSITION_COUNT; ++index) {
        Game_WriteDatalinkLobbyWord(
            gGameDatalinkLobbyGraphicsArena +
                DATALINK_LOBBY_POSITION_CACHE_OFFSET + index * 8,
            (uint32_t)(tweens[28 + index].current_x_fx_30 >> 12));
        Game_WriteDatalinkLobbyWord(
            gGameDatalinkLobbyGraphicsArena +
                DATALINK_LOBBY_POSITION_CACHE_OFFSET + index * 8 + 4,
            (uint32_t)(tweens[28 + index].current_y_fx_34 >> 12));
    }

    (void)Game_GetDatalinkLobbyGraphicsManager_0208da4c();
    resource_context = Game_GetDatalinkLobbyResourceContext_0208dfa4();
    Game_WriteDatalinkLobbyWord(
        gGameDatalinkLobbyRenderer + 0x38,
        Game_ReadDatalinkLobbyWord(
            gGameDatalinkLobbyGraphicsArena +
            DATALINK_LOBBY_RENDERER_SOURCE_OFFSET) + 0x0c);
    (void)Game_InitializeDatalinkLobbyRenderer_020a2b14(
        gGameDatalinkLobbyRenderer,
        resource_context,
        0);
    Game_SetDatalinkLobbyGraphicsMode_020a3778(
        DATALINK_LOBBY_GRAPHICS_MODE);
    graphics_manager = Game_GetDatalinkLobbyGraphicsManager_0208da4c();
    graphics_manager->scene_field_10 = 0;
    graphics_manager->scene_field_14 = 0;
    gGameSubDisplayControl_04001000 =
        (gGameSubDisplayControl_04001000 &
         ~((uint32_t)DATALINK_LOBBY_DISPLAY_PLANE_MASK)) |
        DATALINK_LOBBY_DISPLAY_PLANE_MASK;
    Game_WriteDatalinkLobbyWord(gGameDatalinkLobbyLinkState + 0x3c, 0);

    Game_ConfigureDatalinkLobbyTweenPositions(tweens);
    (void)Game_GetDatalinkLobbyGraphicsManager_0208da4c();
    if (Game_IsDatalinkLobbyRuntimeAvailable_0208de54() != 0) {
        wireless_context = Game_GetDatalinkWirelessContext_020909c8();
        if (Game_GetDatalinkWirelessState_02090ea4(wireless_context) ==
            DATALINK_LOBBY_WIRELESS_READY_STATE) {
            gGameDatalinkLobbyState.wireless_state_a4 = 1;
        }
    }
    Game_CommitDatalinkLobbyBackground_0209da54(1);
    Game_CommitDatalinkLobbyBackground_0209da54(2);
    Game_RefreshDatalinkLobbyTilemaps_020b4044();
}
