#include "game/datalink_scene_frame.h"

#include "game/datalink_controller.h"

#include <stdint.h>

enum {
    DATALINK_SCENE_STATE_DEPTH_OFFSET = 0x2c,
    DATALINK_SCENE_STATE_STACK_OFFSET = 0x0c,
    DATALINK_SCENE_STATE_STACK_STRIDE = 4,
    DATALINK_SCENE_RECORDS_OFFSET = 0x43c,
    DATALINK_SCENE_RECORD_STRIDE = 0x58,
    DATALINK_SCENE_ACTIVE_RECORD_COUNT_OFFSET = 0x253c,
    DATALINK_SCENE_PENDING_SOUND_OFFSET = 0x253e,
    DATALINK_SCENE_SOUND_MASK = 0x7f,
    DATALINK_SCENE_PUBLISHED_POSITION_COUNT = 4,
    DATALINK_SCENE_PUBLISHED_RECORD_BASE = 0x36,
    DATALINK_FX_SHIFT = 12
};

extern void Game_PlayDatalinkPendingSoundEffect_020827a8(int sound_id);

static uint8_t *Game_DatalinkSceneFrameBytes(
    Game_DatalinkSceneOwner *scene)
{
    return (uint8_t *)scene;
}

static const uint8_t *Game_DatalinkSceneFrameConstBytes(
    const Game_DatalinkSceneOwner *scene)
{
    return (const uint8_t *)scene;
}

static void Game_CopyDatalinkSceneFrameBytes(
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

static uint8_t Game_ReadDatalinkSceneByte(
    const Game_DatalinkSceneOwner *scene,
    unsigned int offset)
{
    return Game_DatalinkSceneFrameConstBytes(scene)[offset];
}

static void Game_WriteDatalinkSceneByte(
    Game_DatalinkSceneOwner *scene,
    unsigned int offset,
    uint8_t value)
{
    Game_DatalinkSceneFrameBytes(scene)[offset] = value;
}

static Game_DatalinkState *Game_GetDatalinkSceneCurrentState(
    Game_DatalinkSceneOwner *scene)
{
    int8_t depth = (int8_t)Game_ReadDatalinkSceneByte(
        scene, DATALINK_SCENE_STATE_DEPTH_OFFSET);
    uintptr_t state_address = 0;

    Game_CopyDatalinkSceneFrameBytes(
        &state_address,
        Game_DatalinkSceneFrameBytes(scene) +
            DATALINK_SCENE_STATE_STACK_OFFSET +
            depth * DATALINK_SCENE_STATE_STACK_STRIDE,
        sizeof(state_address)
    );
    return (Game_DatalinkState *)state_address;
}

static Game_DatalinkTweenRecord *Game_GetDatalinkSceneRecord(
    Game_DatalinkSceneOwner *scene,
    int index)
{
    return (Game_DatalinkTweenRecord *)(
        Game_DatalinkSceneFrameBytes(scene) +
        DATALINK_SCENE_RECORDS_OFFSET +
        index * DATALINK_SCENE_RECORD_STRIDE
    );
}

static void Game_UpdateDatalinkSceneCurrentState(
    Game_DatalinkSceneOwner *scene)
{
    Game_DatalinkState *state = Game_GetDatalinkSceneCurrentState(scene);

    state->vtable_00->slot_04(state);
}

static void Game_PublishDatalinkScenePlayerPositions(
    Game_DatalinkSceneOwner *scene)
{
    int index;

    for (index = 0;
         index < DATALINK_SCENE_PUBLISHED_POSITION_COUNT;
         ++index) {
        const Game_DatalinkTweenRecord *record =
            Game_GetDatalinkSceneRecord(
                scene,
                DATALINK_SCENE_PUBLISHED_RECORD_BASE + index);

        gGameDatalinkScenePublishedPositions[index].x =
            record->current_x_fx_30 >> DATALINK_FX_SHIFT;
        gGameDatalinkScenePublishedPositions[index].y =
            record->current_y_fx_34 >> DATALINK_FX_SHIFT;
    }
}

static void Game_UpdateDatalinkSceneRecords(
    Game_DatalinkSceneOwner *scene)
{
    uint8_t index = 0;

    while (index < Game_ReadDatalinkSceneByte(
               scene, DATALINK_SCENE_ACTIVE_RECORD_COUNT_OFFSET)) {
        Game_UpdateDatalinkTweenRecord_020acc2c(
            Game_GetDatalinkSceneRecord(scene, index));
        ++index;
    }
}

/* 0x020ad87c..0x020ad94f; literal at 0x020ad950 is excluded. */
void Game_UpdateDatalinkSceneFrame_020ad87c(
    Game_DatalinkSceneOwner *scene)
{
    uint8_t pending_sound;

    Game_ResetDatalinkSceneOamCount(scene);
    pending_sound = Game_ReadDatalinkSceneByte(
        scene, DATALINK_SCENE_PENDING_SOUND_OFFSET);
    if (pending_sound != 0U) {
        Game_PlayDatalinkPendingSoundEffect_020827a8(
            pending_sound & DATALINK_SCENE_SOUND_MASK);
        Game_WriteDatalinkSceneByte(
            scene, DATALINK_SCENE_PENDING_SOUND_OFFSET, 0);
        return;
    }

    Game_UpdateDatalinkSceneCurrentState(scene);
    Game_PublishDatalinkScenePlayerPositions(scene);
    Game_UpdateDatalinkSceneRecords(scene);
}
