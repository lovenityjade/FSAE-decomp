#include "game/datalink_scene_initialize.h"

#include "game/datalink_background_resources.h"
#include "game/datalink_controller.h"
#include "game/datalink_graphics_resource.h"
#include "game/datalink_phase_fifteen.h"
#include "game/datalink_scene_command.h"
#include "game/datalink_scene_lifecycle.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_STATIC_INITIALIZED = 1,
    DATALINK_PRIMARY_STATE_VTABLE = 0x02124b40,
    DATALINK_SECONDARY_STATE_VTABLE = 0x02124b88,
    DATALINK_SECONDARY_STATE_DATA = 0x02124bb0,

    DATALINK_SCENE_STATE_LINK_OFFSET = 0x2d,
    DATALINK_SCENE_STATE_LINK_ADDRESS_OFFSET = 0x430,
    DATALINK_SCENE_PRIMARY_STATE_OFFSET = 0x434,
    DATALINK_SCENE_SECONDARY_STATE_OFFSET = 0x438,
    DATALINK_SCENE_TWEEN_RECORDS_OFFSET = 0x43c,
    DATALINK_SCENE_ACTIVE_TWEEN_COUNT_OFFSET = 0x253c,
    DATALINK_SCENE_PENDING_SOUND_OFFSET = 0x253e,
    DATALINK_SCENE_GRAPHICS_CONTEXT_OFFSET = 0x2540,
    DATALINK_SCENE_SMALL_ALLOCATION_OFFSET = 0x2544,
    DATALINK_SCENE_LOCAL_SPRITE_RESOURCES_OFFSET = 0x2548,
    DATALINK_SCENE_FIRST_SCRATCH_OFFSET = 0x25ac,
    DATALINK_SCENE_FIRST_SCREEN_RESOURCES_OFFSET = 0x25cc,
    DATALINK_SCENE_SECOND_SCREEN_RESOURCES_OFFSET = 0x25ec,
    DATALINK_SCENE_SECOND_SCRATCH_OFFSET = 0x262c,
    DATALINK_SCENE_CHARACTER_RESOURCES_OFFSET = 0x264c,
    DATALINK_SCENE_FIRST_SCREEN_ALLOCATION_OFFSET = 0x26ac,
    DATALINK_SCENE_SECOND_SCREEN_ALLOCATION_OFFSET = 0x26b0,
    DATALINK_SCENE_LARGE_ALLOCATION_OFFSET = 0x26b4,

    DATALINK_GRAPHICS_SHARED_SPRITE_RESOURCES_OFFSET = 0x1c,
    DATALINK_GRAPHICS_CLEAR_OFFSET = 0x7c,
    DATALINK_GRAPHICS_FIRST_BACKGROUND_OFFSET = 0x9c,
    DATALINK_GRAPHICS_SECOND_BACKGROUND_OFFSET = 0xbc,
    DATALINK_GRAPHICS_IMAGE_PROXY_OFFSET = 0x28,
    DATALINK_GRAPHICS_PALETTE_PROXY_OFFSET = 0x4c,

    DATALINK_TWEEN_SEQUENCE_OFFSET = 0x48,
    DATALINK_TWEEN_POOL_INDEX_OFFSET = 0x4c,
    DATALINK_TWEEN_ANIMATION_DELTA_OFFSET = 0x50,
    DATALINK_TWEEN_ANIMATION_ADDRESS_OFFSET = 0x54,
    DATALINK_PLAYER_NUMBER_OFFSET = 2,

    DATALINK_LOCAL_SEQUENCE_COUNT = 3,
    DATALINK_SHARED_SEQUENCE_COUNT = 51,
    DATALINK_PLAYER_COUNT = 4,
    DATALINK_PLAYER_RECORD_BASE = 0x36,
    DATALINK_ANIMATION_DELTA_FX = 0x1000,

    DATALINK_SMALL_ALLOCATION_SIZE = 0x200,
    DATALINK_LARGE_ALLOCATION_SIZE = 0x8000,
    DATALINK_SCRATCH_SIZE = 0x80,
    DATALINK_PHASE_OBJECT_VALUE_OFFSET = 0x808,
    DATALINK_GRAPHICS_MODE_TOKEN = 0xde
};

static const uint8_t sDatalinkSharedAnimationSequences[
    DATALINK_SHARED_SEQUENCE_COUNT
] = {
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 36, 32, 33, 32, 33,
    32, 33, 32, 33, 32, 33, 32, 33, 32, 33,
    37
};

extern void *EL_GetGlobalAdr(uint32_t size, int alignment);
extern void MI_CpuFill8(void *destination, uint8_t value, uint32_t size);
extern volatile uint32_t gGameSubDisplayControl_04001000;

extern void Game_ResetDatalinkCellAnimationPool_0209bee4(void);
extern void *Game_AllocateDatalinkCellAnimation_0209be84(
    uint32_t *pool_index
);
extern const void *NNS_G2dGetAnimSequenceByIdx(
    const void *animation_bank,
    uint16_t sequence_index
);
extern void NNS_G2dInitCellAnimation(
    void *animation,
    const void *sequence,
    const void *cell_bank
);
extern void NNS_G2dInitImageProxy(void *proxy);
extern void NNS_G2dInitImagePaletteProxy(void *proxy);
extern void Game_SetDatalinkGraphicsModeToken_020a3778(uint32_t token);
extern void *Game_GetDatalinkGraphicsManager_0208da4c(void);

/* Calls virtual slot zero of a state object. */
extern void Game_InitializeDatalinkStateInterface(
    Game_DatalinkState *state
);

_Static_assert(
    offsetof(Game_DatalinkSceneStaticStateArena, primary_guard_014) == 0x14,
    "datalink scene primary-state guard offset"
);
_Static_assert(
    offsetof(
        Game_DatalinkSceneStaticStateArena,
        primary_state_vtable_018
    ) == 0x18,
    "datalink scene primary-state offset"
);
_Static_assert(
    offsetof(
        Game_DatalinkSceneStaticStateArena,
        secondary_state_vtable_080
    ) == 0x80,
    "datalink scene secondary-state offset"
);
_Static_assert(
    offsetof(
        Game_DatalinkSceneStaticStateArena,
        secondary_state_data_0b0
    ) == 0xb0,
    "datalink scene secondary-state data offset"
);

static void Game_WriteDatalinkSceneWord(
    uint8_t *destination,
    uint32_t value)
{
    destination[0] = (uint8_t)value;
    destination[1] = (uint8_t)(value >> 8);
    destination[2] = (uint8_t)(value >> 16);
    destination[3] = (uint8_t)(value >> 24);
}

static uint32_t Game_DatalinkTargetAddress(const void *pointer)
{
    return (uint32_t)(uintptr_t)pointer;
}

static Game_DatalinkTweenRecord *Game_GetDatalinkSceneTween(
    uint8_t *scene_bytes,
    uint8_t index)
{
    return (Game_DatalinkTweenRecord *)(void *)(
        scene_bytes + DATALINK_SCENE_TWEEN_RECORDS_OFFSET +
        (unsigned int)index * sizeof(Game_DatalinkTweenRecord));
}

static void Game_InitializeDatalinkSceneAnimation(
    Game_DatalinkTweenRecord *tween,
    uint16_t sequence_index,
    Game_DatalinkGraphicsResourceSet *resources)
{
    uint8_t *tween_bytes = (uint8_t *)(void *)tween;
    uint32_t *pool_index = (uint32_t *)(void *)(
        tween_bytes + DATALINK_TWEEN_POOL_INDEX_OFFSET);
    void *animation;
    const void *sequence;

    tween_bytes[DATALINK_TWEEN_SEQUENCE_OFFSET] =
        (uint8_t)sequence_index;
    tween_bytes[DATALINK_TWEEN_SEQUENCE_OFFSET + 1] =
        (uint8_t)(sequence_index >> 8);
    Game_WriteDatalinkSceneWord(
        tween_bytes + DATALINK_TWEEN_ANIMATION_DELTA_OFFSET,
        DATALINK_ANIMATION_DELTA_FX);

    animation = Game_AllocateDatalinkCellAnimation_0209be84(pool_index);
    Game_WriteDatalinkSceneWord(
        tween_bytes + DATALINK_TWEEN_ANIMATION_ADDRESS_OFFSET,
        Game_DatalinkTargetAddress(animation));
    sequence = NNS_G2dGetAnimSequenceByIdx(
        resources->animation_bank_24,
        sequence_index);
    NNS_G2dInitCellAnimation(
        animation,
        sequence,
        resources->cell_bank_1c);
}

static void Game_InitializeDatalinkSceneStaticStates(void)
{
    volatile Game_DatalinkSceneStaticStateArena *arena =
        &gGameDatalinkSceneStaticStateArena;
    uint32_t guard = arena->primary_guard_014;

    if ((guard & DATALINK_STATIC_INITIALIZED) == 0U) {
        arena->primary_state_vtable_018 =
            DATALINK_PRIMARY_STATE_VTABLE;
        arena->primary_guard_014 = guard | DATALINK_STATIC_INITIALIZED;
    }

    guard = arena->secondary_guard_000;
    if ((guard & DATALINK_STATIC_INITIALIZED) == 0U) {
        arena->secondary_state_vtable_080 =
            DATALINK_SECONDARY_STATE_VTABLE;
        arena->secondary_state_data_0b0 =
            DATALINK_SECONDARY_STATE_DATA;
        arena->secondary_guard_000 = guard | DATALINK_STATIC_INITIALIZED;
    }
}

/*
 * 0x020AD070..0x020AD4EB: complete 1148-byte scene initializer.
 * The 15-word literal pool at 0x020ad4ec..0x020ad527 is excluded.
 */
void Game_InitializeDatalinkScene_020ad070(
    Game_DatalinkSceneOwner *scene)
{
    uint8_t *scene_bytes = (uint8_t *)(void *)scene;
    uint8_t *graphics_arena = gGameDatalinkSceneGraphicsArena;
    Game_DatalinkBackgroundResources *first_background =
        (Game_DatalinkBackgroundResources *)(void *)(
            graphics_arena + DATALINK_GRAPHICS_FIRST_BACKGROUND_OFFSET);
    Game_DatalinkBackgroundResources *second_background =
        (Game_DatalinkBackgroundResources *)(void *)(
            graphics_arena + DATALINK_GRAPHICS_SECOND_BACKGROUND_OFFSET);
    Game_DatalinkGraphicsResourceSet *shared_sprite_resources =
        (Game_DatalinkGraphicsResourceSet *)(void *)(
            graphics_arena +
            DATALINK_GRAPHICS_SHARED_SPRITE_RESOURCES_OFFSET);
    Game_DatalinkGraphicsResourceSet *local_sprite_resources =
        (Game_DatalinkGraphicsResourceSet *)(void *)(
            scene_bytes + DATALINK_SCENE_LOCAL_SPRITE_RESOURCES_OFFSET);
    Game_DatalinkSceneStaticStateArena *state_arena =
        (Game_DatalinkSceneStaticStateArena *)(void *)
            &gGameDatalinkSceneStaticStateArena;
    Game_DatalinkState *primary_state =
        (Game_DatalinkState *)(void *)&state_arena->primary_state_vtable_018;
    Game_DatalinkState *secondary_state =
        (Game_DatalinkState *)(void *)&state_arena->secondary_state_vtable_080;
    void *graphics_manager;
    unsigned int index;
    uint8_t tween_index;

    Game_InitializeDatalinkSceneStaticStates();
    Game_DatalinkStateStack_Init_020acd98(
        (Game_DatalinkStateStack *)(void *)scene_bytes);
    scene_bytes[DATALINK_SCENE_PENDING_SOUND_OFFSET] = 0;
    Game_WriteDatalinkSceneWord(
        scene_bytes + DATALINK_SCENE_SMALL_ALLOCATION_OFFSET,
        Game_DatalinkTargetAddress(
            EL_GetGlobalAdr(DATALINK_SMALL_ALLOCATION_SIZE, 4)));
    Game_WriteDatalinkSceneWord(
        scene_bytes + DATALINK_SCENE_LARGE_ALLOCATION_OFFSET,
        Game_DatalinkTargetAddress(
            EL_GetGlobalAdr(DATALINK_LARGE_ALLOCATION_SIZE, 0x20)));

    Game_InitializeDatalinkSceneTweenRecords_020ad020(scene);
    MI_CpuFill8(
        graphics_arena + DATALINK_GRAPHICS_CLEAR_OFFSET,
        0,
        DATALINK_SCRATCH_SIZE);
    MI_CpuFill8(
        scene_bytes + DATALINK_SCENE_FIRST_SCRATCH_OFFSET,
        0,
        DATALINK_SCRATCH_SIZE);
    MI_CpuFill8(
        scene_bytes + DATALINK_SCENE_SECOND_SCRATCH_OFFSET,
        0,
        DATALINK_SCRATCH_SIZE);
    Game_ConfigureDatalinkSubDisplay_020ad528();

    Game_WriteDatalinkSceneWord(
        scene_bytes + DATALINK_SCENE_GRAPHICS_CONTEXT_OFFSET,
        Game_DatalinkTargetAddress(graphics_arena));
    Game_LoadDatalinkBackgroundResources(
        first_background,
        1,
        0x12,
        0x13,
        UINT32_MAX,
        0);
    Game_LoadDatalinkCharacterResource(
        (Game_DatalinkGraphicsResourceSet *)(void *)(
            scene_bytes + DATALINK_SCENE_CHARACTER_RESOURCES_OFFSET),
        1,
        UINT32_C(0x80000000),
        0x2000,
        0);
    Game_LoadDatalinkBackgroundResources(
        second_background,
        2,
        -1,
        UINT32_MAX,
        UINT32_MAX,
        0);
    Game_LoadDatalinkScreenResourceIntoSet(
        (Game_DatalinkGraphicsResourceSet *)(void *)first_background,
        1,
        0x21);
    Game_LoadDatalinkScreenResourceIntoSet(
        (Game_DatalinkGraphicsResourceSet *)(void *)second_background,
        2,
        0x22);
    Game_WriteDatalinkSceneWord(
        scene_bytes + DATALINK_SCENE_FIRST_SCREEN_ALLOCATION_OFFSET,
        Game_DatalinkTargetAddress(EL_GetGlobalAdr(
            first_background->screen_data_14->data_size_08,
            4)));
    Game_WriteDatalinkSceneWord(
        scene_bytes + DATALINK_SCENE_SECOND_SCREEN_ALLOCATION_OFFSET,
        Game_DatalinkTargetAddress(EL_GetGlobalAdr(
            second_background->screen_data_14->data_size_08,
            4)));

    Game_LoadDatalinkScreenResourceIntoSet(
        (Game_DatalinkGraphicsResourceSet *)(void *)(
            scene_bytes + DATALINK_SCENE_FIRST_SCREEN_RESOURCES_OFFSET),
        1,
        UINT32_C(0x80000005));
    Game_LoadDatalinkScreenResourceIntoSet(
        (Game_DatalinkGraphicsResourceSet *)(void *)(
            scene_bytes + DATALINK_SCENE_SECOND_SCREEN_RESOURCES_OFFSET),
        2,
        0x29);

    gGameSubDisplayControl_04001000 &= UINT32_C(0xffcfffef);
    Game_LoadDatalinkObjectGraphicsResources(
        shared_sprite_resources,
        UINT32_C(0x80000002),
        -1,
        0x1f,
        500);
    Game_LoadDatalinkCellAnimationResources(
        shared_sprite_resources,
        0x1d,
        0x1e);
    Game_LoadDatalinkObjectGraphicsResources(
        local_sprite_resources,
        0x17,
        0x2000,
        UINT32_MAX,
        0);
    Game_LoadDatalinkCellAnimationResources(
        local_sprite_resources,
        0x1b,
        0x1c);
    gGameSubDisplayControl_04001000 =
        (gGameSubDisplayControl_04001000 & UINT32_C(0xffffe0ff)) |
        UINT32_C(0x1900);

    NNS_G2dInitImageProxy(
        graphics_arena + DATALINK_GRAPHICS_SHARED_SPRITE_RESOURCES_OFFSET +
        DATALINK_GRAPHICS_IMAGE_PROXY_OFFSET);
    NNS_G2dInitImagePaletteProxy(
        graphics_arena + DATALINK_GRAPHICS_SHARED_SPRITE_RESOURCES_OFFSET +
        DATALINK_GRAPHICS_PALETTE_PROXY_OFFSET);
    Game_ResetDatalinkCellAnimationPool_0209bee4();

    tween_index = 0;
    for (index = 0; index < DATALINK_LOCAL_SEQUENCE_COUNT; ++index) {
        Game_InitializeDatalinkSceneAnimation(
            Game_GetDatalinkSceneTween(scene_bytes, tween_index),
            0,
            local_sprite_resources);
        ++tween_index;
        ++scene_bytes[DATALINK_SCENE_ACTIVE_TWEEN_COUNT_OFFSET];
    }
    for (index = 0; index < DATALINK_SHARED_SEQUENCE_COUNT; ++index) {
        Game_InitializeDatalinkSceneAnimation(
            Game_GetDatalinkSceneTween(scene_bytes, tween_index),
            sDatalinkSharedAnimationSequences[index],
            shared_sprite_resources);
        ++tween_index;
        ++scene_bytes[DATALINK_SCENE_ACTIVE_TWEEN_COUNT_OFFSET];
    }
    for (index = 0; index < DATALINK_PLAYER_COUNT; ++index) {
        Game_DatalinkTweenRecord *tween = Game_GetDatalinkSceneTween(
            scene_bytes,
            (uint8_t)(DATALINK_PLAYER_RECORD_BASE + index));
        uint8_t *tween_bytes = (uint8_t *)(void *)tween;

        tween_bytes[DATALINK_PLAYER_NUMBER_OFFSET] = (uint8_t)(index + 1U);
        ++scene_bytes[DATALINK_SCENE_ACTIVE_TWEEN_COUNT_OFFSET];
    }

    Game_WriteDatalinkSceneWord(
        gGameDatalinkScenePhaseObjectStorage +
            DATALINK_PHASE_OBJECT_VALUE_OFFSET,
        UINT32_C(0x40000));
    Game_SetDatalinkGraphicsModeToken_020a3778(
        DATALINK_GRAPHICS_MODE_TOKEN);
    graphics_manager = Game_GetDatalinkGraphicsManager_0208da4c();
    Game_WriteDatalinkSceneWord(
        (uint8_t *)graphics_manager + 0x10,
        0);
    Game_WriteDatalinkSceneWord(
        (uint8_t *)graphics_manager + 0x14,
        0);

    Game_WriteDatalinkSceneWord(
        scene_bytes + DATALINK_SCENE_PRIMARY_STATE_OFFSET,
        Game_DatalinkTargetAddress(primary_state));
    Game_WriteDatalinkSceneWord(
        scene_bytes + DATALINK_SCENE_SECONDARY_STATE_OFFSET,
        Game_DatalinkTargetAddress(secondary_state));
    if (gGameDatalinkCompletionOwner.completion_flag_d0 == 0U ||
        (gGameDatalinkCompletionOwner.completion_flag_d1 != 2U &&
         gGameDatalinkCompletionOwner.completion_flag_d1 != 3U &&
         gGameDatalinkCompletionOwner.completion_flag_d1 != 4U)) {
        Game_InitializeDatalinkStateInterface(primary_state);
        Game_InitializeDatalinkStateInterface(secondary_state);
    }
    Game_DatalinkStateStack_EnterRootState(
        (Game_DatalinkStateStack *)(void *)scene_bytes,
        secondary_state);
    Game_WriteDatalinkSceneWord(
        scene_bytes + DATALINK_SCENE_STATE_LINK_ADDRESS_OFFSET,
        Game_DatalinkTargetAddress(
            scene_bytes + DATALINK_SCENE_STATE_LINK_OFFSET));
    gGameDatalinkSceneSharedWord = 0;
}
