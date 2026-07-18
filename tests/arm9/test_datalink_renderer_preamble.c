#include "game/datalink_renderer_preamble.h"

#include "game/datalink_phase_ten_finalizer.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

Game_DatalinkSceneOwner gGameDatalinkSceneOwner;

static const Game_DatalinkRenderDescriptor
    sExpectedDescriptors[GAME_DATALINK_RENDER_DESCRIPTOR_COUNT] = {
        {0x15, 6, 0, UINT32_C(0x0209CA38)},
        {0x16, 5, 0, UINT32_C(0x0209CAB0)},
        {0x08, 6, 0, UINT32_C(0x0209CAF0)},
        {0x0A, 6, 0, UINT32_C(0x0209CB40)},
        {0x02, 5, 0, UINT32_C(0x0209CB90)},
        {0x0D, 6, 0, UINT32_C(0x0209CBE0)},
        {0x0F, 6, 0, UINT32_C(0x0209CC30)},
        {0x1A, 4, 0, UINT32_C(0x0209CC80)},
        {0x1B, 4, 0, UINT32_C(0x0209CD18)},
        {0x1C, 4, 0, UINT32_C(0x0209CDB0)}
    };

static int sReady[3];
static int sQueriedSlots[3];
static size_t sQueryCount;
static int sCommands[3];
static size_t sCommandCount;

int Game_IsDatalinkPlayerSlotReady(int slot)
{
    assert(slot >= 0 && slot < 3);
    assert(sQueryCount < sizeof(sQueriedSlots) / sizeof(sQueriedSlots[0]));
    sQueriedSlots[sQueryCount++] = slot;
    return sReady[slot];
}

void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command
)
{
    assert(scene == &gGameDatalinkSceneOwner);
    assert(command >= 0 && command < 3);
    assert(sQueryCount == (size_t)command + 1U);
    assert(sCommandCount < sizeof(sCommands) / sizeof(sCommands[0]));
    sCommands[sCommandCount++] = command;
}

static void TestCopiesAllTenRawRomDescriptors(void)
{
    Game_DatalinkRenderDescriptor
        descriptors[GAME_DATALINK_RENDER_DESCRIPTOR_COUNT];

    memset(descriptors, 0xA5, sizeof(descriptors));

    Game_CopyDatalinkRenderDescriptors(descriptors);

    assert(memcmp(descriptors, sExpectedDescriptors, sizeof(descriptors)) == 0);
}

static void TestQueriesEverySlotAndCommandsOnlyReadySlots(void)
{
    sReady[0] = 1;
    sReady[1] = 0;
    sReady[2] = 1;
    sQueryCount = 0;
    sCommandCount = 0;

    Game_IssueReadyDatalinkSlotCommands();

    assert(sQueryCount == 3);
    assert(sQueriedSlots[0] == 0);
    assert(sQueriedSlots[1] == 1);
    assert(sQueriedSlots[2] == 2);
    assert(sCommandCount == 2);
    assert(sCommands[0] == 0);
    assert(sCommands[1] == 2);
}

static void TestNoReadySlotsStillPerformsEveryQuery(void)
{
    sReady[0] = 0;
    sReady[1] = 0;
    sReady[2] = 0;
    sQueryCount = 0;
    sCommandCount = 0;

    Game_IssueReadyDatalinkSlotCommands();

    assert(sQueryCount == 3);
    assert(sCommandCount == 0);
}

int main(void)
{
    TestCopiesAllTenRawRomDescriptors();
    TestQueriesEverySlotAndCommandsOnlyReadySlots();
    TestNoReadySlotsStillPerformsEveryQuery();
    return 0;
}
