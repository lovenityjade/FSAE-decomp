#include "game/datalink_scene_oam.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

enum {
    TEST_SCENE_SIZE = 0x2600,
    TEST_OAM_MANAGER_OFFSET = 0x2540
};

static union {
    void *alignment;
    uint8_t bytes[TEST_SCENE_SIZE];
} sSceneStorage;

static Game_DatalinkOamManager sManager;
static const Game_DatalinkOamEntry *sExpectedEntries;
static uint32_t sExpectedCount;
static int sSubmitCount;

void NNS_G2dEntryOamManagerOam(
    Game_DatalinkOamManager *manager,
    const Game_DatalinkOamEntry *oam_entries,
    uint32_t count)
{
    assert(manager == &sManager);
    assert(oam_entries == sExpectedEntries);
    assert(count == sExpectedCount);
    ++sSubmitCount;
}

static Game_DatalinkSceneOwner *Scene(void)
{
    return (Game_DatalinkSceneOwner *)sSceneStorage.bytes;
}

static Game_DatalinkOamEntry MakeEntry(
    uint32_t shape,
    uint32_t size,
    uint32_t x,
    uint32_t y)
{
    Game_DatalinkOamEntry entry;

    entry.attribute_0 = (uint16_t)((shape << 14) | (y & 0xFFU));
    entry.attribute_1 = (uint16_t)((size << 14) | (x & 0x1FFU));
    entry.attribute_2 = 0U;
    entry.affine_value = 0U;
    return entry;
}

static void ResetHarness(void)
{
    Game_DatalinkOamManager *manager = &sManager;

    memset(&sSceneStorage, 0, sizeof(sSceneStorage));
    memcpy(
        sSceneStorage.bytes + TEST_OAM_MANAGER_OFFSET,
        &manager,
        sizeof(manager)
    );
    sExpectedEntries = NULL;
    sExpectedCount = 0U;
    sSubmitCount = 0;
}

static void TestVisibleEntriesAreSubmitted(void)
{
    Game_DatalinkOamEntry entries[2];

    ResetHarness();
    entries[0] = MakeEntry(0U, 0U, 40U, 60U);
    entries[1] = MakeEntry(2U, 3U, 80U, 100U);
    sExpectedEntries = entries;
    sExpectedCount = 2U;

    Game_SubmitVisibleDatalinkSceneOams(Scene(), entries, 2);
    assert(sSubmitCount == 1);
}

static void TestAnyVisibleOriginKeepsTheGroup(void)
{
    Game_DatalinkOamEntry entries[2];

    ResetHarness();
    entries[0] = MakeEntry(0U, 0U, 300U, 220U);
    entries[1] = MakeEntry(1U, 2U, 20U, 30U);
    sExpectedEntries = entries;
    sExpectedCount = 2U;

    Game_SubmitVisibleDatalinkSceneOams(Scene(), entries, 2);
    assert(sSubmitCount == 1);
}

static void TestOffscreenOriginsAreRejected(void)
{
    Game_DatalinkOamEntry entry;

    ResetHarness();
    entry = MakeEntry(0U, 0U, 300U, 20U);
    Game_SubmitVisibleDatalinkSceneOams(Scene(), &entry, 1);
    assert(sSubmitCount == 0);

    ResetHarness();
    entry = MakeEntry(0U, 0U, 20U, 200U);
    Game_SubmitVisibleDatalinkSceneOams(Scene(), &entry, 1);
    assert(sSubmitCount == 0);
}

static void TestNonpositiveCountsAreRejected(void)
{
    Game_DatalinkOamEntry entry = MakeEntry(0U, 0U, 20U, 20U);

    ResetHarness();
    Game_SubmitVisibleDatalinkSceneOams(Scene(), &entry, 0);
    assert(sSubmitCount == 0);
    Game_SubmitVisibleDatalinkSceneOams(Scene(), &entry, -1);
    assert(sSubmitCount == 0);
}

int main(void)
{
    TestVisibleEntriesAreSubmitted();
    TestAnyVisibleOriginKeepsTheGroup();
    TestOffscreenOriginsAreRejected();
    TestNonpositiveCountsAreRejected();
    return 0;
}
