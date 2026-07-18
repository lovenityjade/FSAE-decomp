#include "game/datalink_scene_command.h"

#include "game/datalink_controller.h"

#include <stdint.h>

enum {
    DATALINK_SCENE_TWEEN_RECORDS_OFFSET = 0x43c,
    DATALINK_SCENE_TWEEN_RECORD_COUNT = 0x60,
    DATALINK_SCENE_ACTIVE_TWEEN_COUNT_OFFSET = 0x253c
};

extern void MI_CpuFill8(void *destination, uint8_t value, uint32_t size);

/* 0x020ad020..0x020ad06f; no literal pool follows this body. */
void Game_InitializeDatalinkSceneTweenRecords_020ad020(
    Game_DatalinkSceneOwner *scene)
{
    uint8_t *scene_bytes = (uint8_t *)scene;
    Game_DatalinkTweenRecord *records =
        (Game_DatalinkTweenRecord *)(void *)(
            scene_bytes + DATALINK_SCENE_TWEEN_RECORDS_OFFSET);
    unsigned int index;

    scene_bytes[DATALINK_SCENE_ACTIVE_TWEEN_COUNT_OFFSET] = 0;
    MI_CpuFill8(
        records,
        0,
        DATALINK_SCENE_TWEEN_RECORD_COUNT * sizeof(*records));
    for (index = 0; index < DATALINK_SCENE_TWEEN_RECORD_COUNT; ++index) {
        Game_InitializeDatalinkTweenRecord_020acbf4(
            &records[index],
            (uint8_t)index);
    }
}
