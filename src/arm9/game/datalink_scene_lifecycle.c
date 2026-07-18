#include "game/datalink_scene_lifecycle.h"

#include "game/datalink_controller.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_SUB_BG_REGISTER_MASK = 0x0043,
    DATALINK_SUB_BG_PRIORITY_MASK = 0x0003,
    DATALINK_SUB_BG0_CONFIGURATION = 0x150c,
    DATALINK_SUB_BG1_CONFIGURATION = 0x5300,
    DATALINK_SUB_BG2_CONFIGURATION = 0x5100,
    DATALINK_SUB_BG3_CONFIGURATION = 0x1090,
    DATALINK_SUB_BG_BANK = 4,
    DATALINK_SUB_BG_EXT_PALETTE_BANK = 0x80,
    DATALINK_SCREEN_CLEAR_VALUE = 0,
    DATALINK_SCREEN_TAIL_OFFSET = 0x600,
    DATALINK_SCREEN_TAIL_SIZE = 0x200,
    DATALINK_SCREEN_BODY_OFFSET = 0x800,
    DATALINK_SCREEN_BODY_SIZE = 0x800,

    DATALINK_SCENE_SERIALIZED_COUNT_OFFSET = 0x253c,
    DATALINK_SCENE_SERIALIZED_FLAG_OFFSET = 0x253d,
    DATALINK_SCENE_GRAPHICS_OWNER_ADDRESS_OFFSET = 0x2540,
    DATALINK_SCENE_OAM_BUFFER_ADDRESS_OFFSET = 0x2544,
    DATALINK_SCENE_INLINE_RESOURCE_SET_OFFSET = 0x2548,
    DATALINK_SCENE_RESOURCE_GROUP_A_OFFSET = 0x25ac,
    DATALINK_SCENE_RESOURCE_GROUP_B_OFFSET = 0x262c,
    DATALINK_SCENE_AUXILIARY_A_ADDRESS_OFFSET = 0x26ac,
    DATALINK_SCENE_AUXILIARY_B_ADDRESS_OFFSET = 0x26b0,
    DATALINK_SCENE_AUXILIARY_C_ADDRESS_OFFSET = 0x26b4,
    DATALINK_SCENE_RESOURCE_GROUP_COUNT = 4,
    DATALINK_SCENE_RESOURCE_GROUP_STRIDE = 0x20,
    DATALINK_SCENE_RESOURCE_ADDRESS_STRIDE = 8,
    DATALINK_SCENE_RESOURCE_ADDRESS_COUNT = 4,
    DATALINK_SCENE_GRAPHICS_OWNER_RESOURCE_OFFSET = 0x1c
};

extern void GXS_SetGraphicsMode(uint32_t background_mode);
extern void GX_SetBankForSubBG(uint32_t bank);
extern void GX_SetBankForSubBGExtPltt(uint32_t bank);
extern void *G2S_GetBG1ScrPtr(void);
extern void *G2S_GetBG2ScrPtr(void);
extern void MIi_CpuClearFast(
    uint32_t value,
    void *destination,
    uint32_t size);
extern void MI_CpuCopy(
    const void *source,
    void *destination,
    uint32_t size);

/* Heap-free wrapper at 0x0208e188. */
extern void Game_FreeAllocation_0208e188(void *allocation);
/* Clears the shared cell-animation allocation map at 0x0209bee4. */
extern void Game_ResetCellAnimationPool_0209bee4(void);
/* Releases one shared graphics-resource slot at 0x0209d690. */
extern void Game_ReleaseGraphicsResourceSlot_0209d690(
    unsigned int slot);
/* Releases the address fields in one graphics-resource set at 0x0209d714. */
extern void Game_ReleaseGraphicsResourceSet_0209d714(
    void *resource_set);

_Static_assert(
    offsetof(Game_DatalinkSubDisplayConfigOwner, screen_base_blocks_fc) ==
        0xfc,
    "sub-display screen-base table offset");
_Static_assert(
    sizeof(Game_DatalinkSceneResourceGroup) == 0x20,
    "scene resource-group stride");

static uint8_t *Game_DatalinkSceneBytes(Game_DatalinkSceneOwner *scene)
{
    return (uint8_t *)scene;
}

static const uint8_t *Game_DatalinkConstSceneBytes(
    const Game_DatalinkSceneOwner *scene)
{
    return (const uint8_t *)scene;
}

static uint32_t Game_ReadDatalinkSceneWord(
    const uint8_t *scene_bytes,
    unsigned int offset)
{
    return (uint32_t)scene_bytes[offset] |
        ((uint32_t)scene_bytes[offset + 1] << 8) |
        ((uint32_t)scene_bytes[offset + 2] << 16) |
        ((uint32_t)scene_bytes[offset + 3] << 24);
}

static void Game_ClearDatalinkSceneWord(
    uint8_t *scene_bytes,
    unsigned int offset)
{
    scene_bytes[offset] = 0;
    scene_bytes[offset + 1] = 0;
    scene_bytes[offset + 2] = 0;
    scene_bytes[offset + 3] = 0;
}

static void Game_ReleaseDatalinkSceneAllocation(
    uint8_t *scene_bytes,
    unsigned int offset)
{
    uint32_t allocation_address =
        Game_ReadDatalinkSceneWord(scene_bytes, offset);

    if (allocation_address != 0) {
        Game_FreeAllocation_0208e188(
            (void *)(uintptr_t)allocation_address);
        Game_ClearDatalinkSceneWord(scene_bytes, offset);
    }
}

static void Game_ReleaseDatalinkSceneResourceGroup(
    uint8_t *scene_bytes,
    unsigned int offset)
{
    unsigned int resource_index;

    for (resource_index = 0;
         resource_index < DATALINK_SCENE_RESOURCE_ADDRESS_COUNT;
         ++resource_index) {
        Game_ReleaseDatalinkSceneAllocation(
            scene_bytes,
            offset +
                resource_index * DATALINK_SCENE_RESOURCE_ADDRESS_STRIDE);
    }
}

/* 0x020ad528..0x020ad65f */
void Game_ConfigureDatalinkSubDisplay_020ad528(void)
{
    uint8_t *screen;

    GXS_SetGraphicsMode(0);
    GX_SetBankForSubBG(DATALINK_SUB_BG_BANK);
    GX_SetBankForSubBGExtPltt(DATALINK_SUB_BG_EXT_PALETTE_BANK);

    gGameDatalinkSubBgControl_04001008[3] = (uint16_t)(
        (gGameDatalinkSubBgControl_04001008[3] &
            DATALINK_SUB_BG_REGISTER_MASK) |
        DATALINK_SUB_BG3_CONFIGURATION);
    gGameDatalinkSubBgControl_04001008[2] = (uint16_t)(
        (gGameDatalinkSubBgControl_04001008[2] &
            DATALINK_SUB_BG_REGISTER_MASK) |
        DATALINK_SUB_BG2_CONFIGURATION);
    gGameDatalinkSubBgControl_04001008[1] = (uint16_t)(
        (gGameDatalinkSubBgControl_04001008[1] &
            DATALINK_SUB_BG_REGISTER_MASK) |
        DATALINK_SUB_BG1_CONFIGURATION);
    gGameDatalinkSubBgControl_04001008[0] = (uint16_t)(
        (gGameDatalinkSubBgControl_04001008[0] &
            DATALINK_SUB_BG_REGISTER_MASK) |
        DATALINK_SUB_BG0_CONFIGURATION);

    screen = (uint8_t *)G2S_GetBG1ScrPtr();
    MIi_CpuClearFast(
        DATALINK_SCREEN_CLEAR_VALUE,
        screen + DATALINK_SCREEN_TAIL_OFFSET,
        DATALINK_SCREEN_TAIL_SIZE);
    screen = (uint8_t *)G2S_GetBG1ScrPtr();
    MIi_CpuClearFast(
        DATALINK_SCREEN_CLEAR_VALUE,
        screen + DATALINK_SCREEN_BODY_OFFSET,
        DATALINK_SCREEN_BODY_SIZE);
    screen = (uint8_t *)G2S_GetBG2ScrPtr();
    MIi_CpuClearFast(
        DATALINK_SCREEN_CLEAR_VALUE,
        screen + DATALINK_SCREEN_TAIL_OFFSET,
        DATALINK_SCREEN_TAIL_SIZE);
    screen = (uint8_t *)G2S_GetBG2ScrPtr();
    MIi_CpuClearFast(
        DATALINK_SCREEN_CLEAR_VALUE,
        screen + DATALINK_SCREEN_BODY_OFFSET,
        DATALINK_SCREEN_BODY_SIZE);

    gGameDatalinkSubBgControl_04001008[0] = (uint16_t)(
        gGameDatalinkSubBgControl_04001008[0] &
        ~DATALINK_SUB_BG_PRIORITY_MASK);
    gGameDatalinkSubBgControl_04001008[1] = (uint16_t)(
        (gGameDatalinkSubBgControl_04001008[1] &
            ~DATALINK_SUB_BG_PRIORITY_MASK) |
        1);
    gGameDatalinkSubBgControl_04001008[2] = (uint16_t)(
        (gGameDatalinkSubBgControl_04001008[2] &
            ~DATALINK_SUB_BG_PRIORITY_MASK) |
        2);
    gGameDatalinkSubBgControl_04001008[3] = (uint16_t)(
        (gGameDatalinkSubBgControl_04001008[3] &
            ~DATALINK_SUB_BG_PRIORITY_MASK) |
        3);

    gGameDatalinkSubDisplayConfigOwner.screen_base_blocks_fc[0] = 4;
    gGameDatalinkSubDisplayConfigOwner.screen_base_blocks_fc[1] = 8;
    gGameDatalinkSubDisplayConfigOwner.screen_base_blocks_fc[2] = 10;
    gGameDatalinkSubDisplayConfigOwner.screen_base_blocks_fc[3] = 6;
}

/* 0x020ad668..0x020ad6bb */
void Game_SerializeDatalinkSceneState_020ad668(
    const Game_DatalinkSceneOwner *scene,
    uint8_t **cursor)
{
    const uint8_t *scene_bytes = Game_DatalinkConstSceneBytes(scene);

    Game_DatalinkStateStack_Serialize(
        (const Game_DatalinkStateStack *)(const void *)scene,
        cursor);
    MI_CpuCopy(
        scene_bytes + DATALINK_SCENE_SERIALIZED_COUNT_OFFSET,
        *cursor,
        1);
    ++*cursor;
    MI_CpuCopy(
        scene_bytes + DATALINK_SCENE_SERIALIZED_FLAG_OFFSET,
        *cursor,
        1);
    ++*cursor;
}

/* 0x020ad6bc..0x020ad70f */
void Game_DeserializeDatalinkSceneState_020ad6bc(
    Game_DatalinkSceneOwner *scene,
    const uint8_t **cursor)
{
    uint8_t *scene_bytes = Game_DatalinkSceneBytes(scene);

    Game_DatalinkStateStack_Deserialize(
        (Game_DatalinkStateStack *)(void *)scene,
        cursor);
    MI_CpuCopy(
        *cursor,
        scene_bytes + DATALINK_SCENE_SERIALIZED_COUNT_OFFSET,
        1);
    ++*cursor;
    MI_CpuCopy(
        *cursor,
        scene_bytes + DATALINK_SCENE_SERIALIZED_FLAG_OFFSET,
        1);
    ++*cursor;
}

/* 0x020ad710..0x020ad87b */
void Game_ReleaseDatalinkSceneResources_020ad710(
    Game_DatalinkSceneOwner *scene)
{
    uint8_t *scene_bytes = Game_DatalinkSceneBytes(scene);
    uint32_t graphics_owner_address = Game_ReadDatalinkSceneWord(
        scene_bytes,
        DATALINK_SCENE_GRAPHICS_OWNER_ADDRESS_OFFSET);
    unsigned int group_index;

    Game_ReleaseDatalinkSceneAllocation(
        scene_bytes, DATALINK_SCENE_OAM_BUFFER_ADDRESS_OFFSET);
    Game_ReleaseGraphicsResourceSet_0209d714(
        (void *)(uintptr_t)(graphics_owner_address +
            DATALINK_SCENE_GRAPHICS_OWNER_RESOURCE_OFFSET));
    Game_ReleaseGraphicsResourceSet_0209d714(
        scene_bytes + DATALINK_SCENE_INLINE_RESOURCE_SET_OFFSET);

    Game_ReleaseDatalinkSceneAllocation(
        scene_bytes, DATALINK_SCENE_AUXILIARY_A_ADDRESS_OFFSET);
    Game_ReleaseDatalinkSceneAllocation(
        scene_bytes, DATALINK_SCENE_AUXILIARY_B_ADDRESS_OFFSET);
    Game_ReleaseDatalinkSceneAllocation(
        scene_bytes, DATALINK_SCENE_AUXILIARY_C_ADDRESS_OFFSET);

    for (group_index = 0;
         group_index < DATALINK_SCENE_RESOURCE_GROUP_COUNT;
         ++group_index) {
        Game_ReleaseGraphicsResourceSlot_0209d690(group_index);
        Game_ReleaseDatalinkSceneResourceGroup(
            scene_bytes,
            DATALINK_SCENE_RESOURCE_GROUP_A_OFFSET +
                group_index * DATALINK_SCENE_RESOURCE_GROUP_STRIDE);
        Game_ReleaseDatalinkSceneResourceGroup(
            scene_bytes,
            DATALINK_SCENE_RESOURCE_GROUP_B_OFFSET +
                group_index * DATALINK_SCENE_RESOURCE_GROUP_STRIDE);
    }
    Game_ResetCellAnimationPool_0209bee4();
}
