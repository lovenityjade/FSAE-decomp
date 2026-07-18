#include "game/datalink_scene_command.h"

#include "game/datalink_sprite_oam.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

enum {
    TEST_SCENE_SIZE = 0x2600,
    TEST_OBJECTS_OFFSET = 0x43c,
    TEST_OBJECT_STRIDE = 0x58,
    TEST_OAM_BUFFER_ADDRESS_OFFSET = 0x2544,
    TEST_FRAME_OAM_COUNT_OFFSET = 0x25a8
};

static union {
    void *alignment;
    uint8_t bytes[TEST_SCENE_SIZE];
} sSceneStorage;

static Game_DatalinkOamEntry sOamBuffer[0x40];
static const Game_DatalinkSpriteObject *sExpectedSprite;
static int sSubmitReturn;
static int sSubmitCalls;

int Game_SubmitDatalinkSpriteOams(
    Game_DatalinkOamEntry *oam_buffer,
    uint16_t capacity,
    const Game_DatalinkSpriteObject *sprite)
{
    assert(oam_buffer == sOamBuffer);
    assert(capacity == 0x40);
    assert(sprite == sExpectedSprite);
    ++sSubmitCalls;
    return sSubmitReturn;
}

static Game_DatalinkSceneOwner *Scene(void)
{
    return (Game_DatalinkSceneOwner *)sSceneStorage.bytes;
}

static void WriteOamBufferAddress(Game_DatalinkOamEntry *oam_buffer)
{
    memcpy(
        sSceneStorage.bytes + TEST_OAM_BUFFER_ADDRESS_OFFSET,
        &oam_buffer,
        sizeof(oam_buffer)
    );
}

static uint16_t ReadFrameOamCount(void)
{
    uint16_t count;

    memcpy(
        &count,
        sSceneStorage.bytes + TEST_FRAME_OAM_COUNT_OFFSET,
        sizeof(count)
    );
    return count;
}

static void WriteFrameOamCount(uint16_t count)
{
    memcpy(
        sSceneStorage.bytes + TEST_FRAME_OAM_COUNT_OFFSET,
        &count,
        sizeof(count)
    );
}

static const Game_DatalinkSpriteObject *ExpectedSpriteForCommand(
    int command)
{
    return (const Game_DatalinkSpriteObject *)(
        sSceneStorage.bytes +
        TEST_OBJECTS_OFFSET +
        command * TEST_OBJECT_STRIDE
    );
}

static void ResetScene(void)
{
    memset(&sSceneStorage, 0xa5, sizeof(sSceneStorage));
    memset(sOamBuffer, 0, sizeof(sOamBuffer));
    WriteOamBufferAddress(sOamBuffer);
    sExpectedSprite = NULL;
    sSubmitReturn = 0;
    sSubmitCalls = 0;
}

static void TestResetFrameOamCount(void)
{
    ResetScene();
    WriteFrameOamCount(0xbeef);

    Game_ResetDatalinkSceneOamCount(Scene());

    assert(ReadFrameOamCount() == 0);
    assert(sSubmitCalls == 0);
}

static void TestApplySceneCommandSubmitsSelectedObject(void)
{
    ResetScene();
    WriteFrameOamCount(10);
    sSubmitReturn = 7;
    sExpectedSprite = ExpectedSpriteForCommand(5);

    Game_ApplyDatalinkSceneCommand(Scene(), 5);

    assert(sSubmitCalls == 1);
    assert(ReadFrameOamCount() == 17);
}

static void TestApplySceneCommandAddsSignedLowHalfword(void)
{
    ResetScene();
    WriteFrameOamCount(10);
    sSubmitReturn = -3;
    sExpectedSprite = ExpectedSpriteForCommand(0x35);

    Game_ApplyDatalinkSceneCommand(Scene(), 0x35);

    assert(sSubmitCalls == 1);
    assert(ReadFrameOamCount() == 7);
}

int main(void)
{
    TestResetFrameOamCount();
    TestApplySceneCommandSubmitsSelectedObject();
    TestApplySceneCommandAddsSignedLowHalfword();
    return 0;
}
