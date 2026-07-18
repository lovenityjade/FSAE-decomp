#include "game/datalink_scene_command.h"

#include "game/datalink_controller.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

enum {
    TEST_SCENE_SIZE = 0x2540,
    TEST_RECORDS_OFFSET = 0x43c,
    TEST_RECORD_COUNT = 0x60,
    TEST_ACTIVE_COUNT_OFFSET = 0x253c
};

static union {
    uint32_t alignment;
    uint8_t bytes[TEST_SCENE_SIZE];
} sSceneStorage;

static unsigned int sClearCalls;
static unsigned int sInitializeCalls;

void MI_CpuFill8(void *destination, uint8_t value, uint32_t size)
{
    assert(destination == sSceneStorage.bytes + TEST_RECORDS_OFFSET);
    assert(value == 0);
    assert(size == TEST_RECORD_COUNT * 0x58);
    ++sClearCalls;
    memset(destination, value, size);
}

void Game_InitializeDatalinkTweenRecord_020acbf4(
    Game_DatalinkTweenRecord *tween,
    uint8_t slot_index)
{
    uint8_t *expected =
        sSceneStorage.bytes + TEST_RECORDS_OFFSET + slot_index * 0x58;
    unsigned int byte_index;

    assert((uint8_t *)(void *)tween == expected);
    assert(slot_index == sInitializeCalls);
    for (byte_index = 0; byte_index < sizeof(*tween); ++byte_index) {
        assert(expected[byte_index] == 0);
    }
    tween->slot_index_00 = slot_index;
    tween->initialized_01 = 1;
    tween->active_03 = 1;
    tween->interpolation_callback_address_10 = 0x020acab4;
    ++sInitializeCalls;
}

int main(void)
{
    Game_DatalinkSceneOwner *scene =
        (Game_DatalinkSceneOwner *)(void *)sSceneStorage.bytes;
    unsigned int index;

    assert(sizeof(Game_DatalinkTweenRecord) == 0x58);
    assert(TEST_RECORDS_OFFSET + TEST_RECORD_COUNT * 0x58 ==
        TEST_ACTIVE_COUNT_OFFSET);
    memset(&sSceneStorage, 0xa5, sizeof(sSceneStorage));

    Game_InitializeDatalinkSceneTweenRecords_020ad020(scene);

    assert(sClearCalls == 1);
    assert(sInitializeCalls == TEST_RECORD_COUNT);
    assert(sSceneStorage.bytes[TEST_RECORDS_OFFSET - 1] == 0xa5);
    assert(sSceneStorage.bytes[TEST_ACTIVE_COUNT_OFFSET] == 0);
    assert(sSceneStorage.bytes[TEST_ACTIVE_COUNT_OFFSET + 1] == 0xa5);
    for (index = 0; index < TEST_RECORD_COUNT; ++index) {
        Game_DatalinkTweenRecord *record =
            (Game_DatalinkTweenRecord *)(void *)(
                sSceneStorage.bytes + TEST_RECORDS_OFFSET + index * 0x58);

        assert(record->slot_index_00 == (uint8_t)index);
        assert(record->initialized_01 == 1);
        assert(record->active_03 == 1);
        assert(record->interpolation_callback_address_10 == 0x020acab4);
        assert(record->duration_fx_28 == 0);
        assert(record->cell_animation_address_54 == 0);
    }
    return 0;
}
