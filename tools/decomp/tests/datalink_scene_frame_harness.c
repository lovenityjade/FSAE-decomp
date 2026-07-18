#include "game/datalink_scene_frame.h"

#include "game/datalink_controller.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

enum {
    TEST_SCENE_SIZE = 0x2600,
    TEST_STATE_STACK_OFFSET = 0x0c,
    TEST_STATE_DEPTH_OFFSET = 0x2c,
    TEST_RECORDS_OFFSET = 0x43c,
    TEST_RECORD_STRIDE = 0x58,
    TEST_ACTIVE_RECORD_COUNT_OFFSET = 0x253c,
    TEST_PENDING_SOUND_OFFSET = 0x253e
};

typedef enum TestEvent {
    TEST_EVENT_RESET,
    TEST_EVENT_SOUND,
    TEST_EVENT_STATE_UPDATE,
    TEST_EVENT_RECORD_UPDATE
} TestEvent;

volatile Game_DatalinkScenePublishedPosition
    gGameDatalinkScenePublishedPositions[4];

static union {
    void *alignment;
    uint8_t bytes[TEST_SCENE_SIZE];
} sSceneStorage;

static TestEvent sEvents[16];
static unsigned int sEventCount;
static int sSoundId;
static unsigned int sRecordUpdateCount;
static Game_DatalinkTweenRecord *sExpectedRecordBase;
static Game_DatalinkState sState;

static void RecordEvent(TestEvent event)
{
    assert(sEventCount < sizeof(sEvents) / sizeof(sEvents[0]));
    sEvents[sEventCount++] = event;
}

static Game_DatalinkSceneOwner *Scene(void)
{
    return (Game_DatalinkSceneOwner *)sSceneStorage.bytes;
}

static Game_DatalinkTweenRecord *Record(int index)
{
    return (Game_DatalinkTweenRecord *)(
        sSceneStorage.bytes + TEST_RECORDS_OFFSET + index * TEST_RECORD_STRIDE);
}

static void StoreCurrentState(Game_DatalinkState *state)
{
    uintptr_t state_address = (uintptr_t)state;

    memcpy(
        sSceneStorage.bytes + TEST_STATE_STACK_OFFSET,
        &state_address,
        sizeof(state_address)
    );
    sSceneStorage.bytes[TEST_STATE_DEPTH_OFFSET] = 0;
}

static void StateUpdate(Game_DatalinkState *state)
{
    assert(state == &sState);
    RecordEvent(TEST_EVENT_STATE_UPDATE);
}

static const Game_DatalinkStateVTable sStateVTable = {
    NULL,
    StateUpdate,
    NULL,
    NULL,
    NULL
};

void Game_ResetDatalinkSceneOamCount(Game_DatalinkSceneOwner *scene)
{
    assert(scene == Scene());
    RecordEvent(TEST_EVENT_RESET);
}

void Game_PlayDatalinkPendingSoundEffect_020827a8(int sound_id)
{
    sSoundId = sound_id;
    RecordEvent(TEST_EVENT_SOUND);
}

void Game_UpdateDatalinkTweenRecord_020acc2c(
    Game_DatalinkTweenRecord *tween)
{
    assert(tween == &sExpectedRecordBase[sRecordUpdateCount]);
    if (sRecordUpdateCount == 0) {
        sSceneStorage.bytes[TEST_ACTIVE_RECORD_COUNT_OFFSET] = 3;
    }
    assert(gGameDatalinkScenePublishedPositions[0].x == 10);
    assert(gGameDatalinkScenePublishedPositions[3].y == -80);
    ++sRecordUpdateCount;
    RecordEvent(TEST_EVENT_RECORD_UPDATE);
}

static void ResetHarness(void)
{
    memset(&sSceneStorage, 0, sizeof(sSceneStorage));
    memset((void *)gGameDatalinkScenePublishedPositions, 0,
           sizeof(gGameDatalinkScenePublishedPositions));
    memset(sEvents, 0, sizeof(sEvents));
    sEventCount = 0;
    sSoundId = -1;
    sRecordUpdateCount = 0;
    sExpectedRecordBase = Record(0);
    memset(&sState, 0, sizeof(sState));
    sState.vtable_00 = &sStateVTable;
    StoreCurrentState(&sState);
}

static void TestPendingSoundReturnsEarly(void)
{
    ResetHarness();
    sSceneStorage.bytes[TEST_PENDING_SOUND_OFFSET] = 0x85;
    sSceneStorage.bytes[TEST_ACTIVE_RECORD_COUNT_OFFSET] = 4;

    Game_UpdateDatalinkSceneFrame_020ad87c(Scene());

    assert(sEventCount == 2);
    assert(sEvents[0] == TEST_EVENT_RESET);
    assert(sEvents[1] == TEST_EVENT_SOUND);
    assert(sSoundId == 5);
    assert(sSceneStorage.bytes[TEST_PENDING_SOUND_OFFSET] == 0);
    assert(sRecordUpdateCount == 0);
}

static void TestStatePositionPublishAndRecordUpdates(void)
{
    int index;

    ResetHarness();
    sSceneStorage.bytes[TEST_ACTIVE_RECORD_COUNT_OFFSET] = 2;
    for (index = 0; index < 4; ++index) {
        Record(0x36 + index)->current_x_fx_30 =
            (10 + index) << 12;
        Record(0x36 + index)->current_y_fx_34 =
            (-20 * (index + 1)) << 12;
    }

    Game_UpdateDatalinkSceneFrame_020ad87c(Scene());

    assert(sEventCount == 5);
    assert(sEvents[0] == TEST_EVENT_RESET);
    assert(sEvents[1] == TEST_EVENT_STATE_UPDATE);
    assert(sEvents[2] == TEST_EVENT_RECORD_UPDATE);
    assert(sEvents[3] == TEST_EVENT_RECORD_UPDATE);
    assert(sEvents[4] == TEST_EVENT_RECORD_UPDATE);
    assert(sRecordUpdateCount == 3);
    for (index = 0; index < 4; ++index) {
        assert(gGameDatalinkScenePublishedPositions[index].x == 10 + index);
        assert(
            gGameDatalinkScenePublishedPositions[index].y ==
            -20 * (index + 1));
    }
}

int main(void)
{
    TestPendingSoundReturnsEarly();
    TestStatePositionPublishAndRecordUpdates();
    return 0;
}
