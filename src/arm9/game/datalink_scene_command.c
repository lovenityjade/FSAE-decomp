#include "game/datalink_scene_command.h"

#include "game/datalink_sprite_oam.h"

#include <stdint.h>

enum {
    DATALINK_SCENE_OBJECTS_OFFSET = 0x43c,
    DATALINK_SCENE_OBJECT_STRIDE = 0x58,
    DATALINK_SCENE_OAM_BUFFER_ADDRESS_OFFSET = 0x2544,
    DATALINK_SCENE_FRAME_OAM_COUNT_OFFSET = 0x25a8,
    DATALINK_SCENE_OAM_CAPACITY = 0x40
};

static uint8_t *Game_DatalinkSceneBytes(Game_DatalinkSceneOwner *scene)
{
    return (uint8_t *)scene;
}

static void Game_CopyDatalinkSceneBytes(
    void *destination,
    const void *source,
    unsigned int size)
{
    uint8_t *destination_bytes = (uint8_t *)destination;
    const uint8_t *source_bytes = (const uint8_t *)source;

    while (size != 0U) {
        *destination_bytes = *source_bytes;
        ++destination_bytes;
        ++source_bytes;
        --size;
    }
}

static uint16_t Game_ReadDatalinkSceneFrameOamCount(
    const Game_DatalinkSceneOwner *scene)
{
    uint16_t count;

    Game_CopyDatalinkSceneBytes(
        &count,
        (const uint8_t *)scene + DATALINK_SCENE_FRAME_OAM_COUNT_OFFSET,
        sizeof(count)
    );
    return count;
}

static void Game_WriteDatalinkSceneFrameOamCount(
    Game_DatalinkSceneOwner *scene,
    uint16_t count)
{
    Game_CopyDatalinkSceneBytes(
        Game_DatalinkSceneBytes(scene) +
            DATALINK_SCENE_FRAME_OAM_COUNT_OFFSET,
        &count,
        sizeof(count)
    );
}

static Game_DatalinkOamEntry *Game_GetDatalinkSceneOamBuffer(
    const Game_DatalinkSceneOwner *scene)
{
    Game_DatalinkOamEntry *oam_buffer;

    Game_CopyDatalinkSceneBytes(
        &oam_buffer,
        (const uint8_t *)scene + DATALINK_SCENE_OAM_BUFFER_ADDRESS_OFFSET,
        sizeof(oam_buffer)
    );
    return oam_buffer;
}

static Game_DatalinkSpriteObject *Game_GetDatalinkSceneObject(
    Game_DatalinkSceneOwner *scene,
    int command)
{
    return (Game_DatalinkSpriteObject *)(
        Game_DatalinkSceneBytes(scene) +
        DATALINK_SCENE_OBJECTS_OFFSET +
        command * DATALINK_SCENE_OBJECT_STRIDE
    );
}

/* 0x020ad954..0x020ad963 */
void Game_ResetDatalinkSceneOamCount(Game_DatalinkSceneOwner *scene)
{
    Game_WriteDatalinkSceneFrameOamCount(scene, 0);
}

/* 0x020ad964..0x020ad9a3 */
void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command)
{
    int16_t generated = (int16_t)Game_SubmitDatalinkSpriteOams(
        Game_GetDatalinkSceneOamBuffer(scene),
        DATALINK_SCENE_OAM_CAPACITY,
        Game_GetDatalinkSceneObject(scene, command)
    );
    uint16_t frame_oams = Game_ReadDatalinkSceneFrameOamCount(scene);

    Game_WriteDatalinkSceneFrameOamCount(
        scene,
        (uint16_t)(frame_oams + generated)
    );
}
