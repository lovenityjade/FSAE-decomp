#include "game/manual_viewer_lifecycle.h"

#include "game/input_state.h"
#include "game/scrollable_tile_buffer.h"
#include "game/sound_manager.h"
#include "ntmv/m2d/resource_accessor.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static uint8_t sViewer[0x40628];
static char sEvents[8];
static unsigned int sEventCount;

static void Event(char event)
{
    sEvents[sEventCount++] = event;
    sEvents[sEventCount] = '\0';
}

void GameSoundManager_InitEmpty(GameSoundManager *manager)
{
    assert(manager == (GameSoundManager *)(sViewer + 0xfc));
    Event('S');
}

void GameScrollableTileBuffer_Init(GameScrollableTileBuffer *buffer)
{
    assert(buffer == (GameScrollableTileBuffer *)(sViewer + 0x40370));
    Event('B');
}

void GameTouchSampler_Init(GameTouchSampler *sampler)
{
    assert(sampler == (GameTouchSampler *)(sViewer + 0x403a6));
    Event('T');
}

NtmvM2dArcResourceAccessor *NtmvM2dArcResourceAccessor_Init(
    NtmvM2dArcResourceAccessor *accessor)
{
    assert(accessor == (NtmvM2dArcResourceAccessor *)(
        sViewer + (sEventCount == 3 ? 0x403f8 : 0x40500)));
    Event('A');
    return accessor;
}

static void AssertWordZero(uint32_t offset)
{
    assert(*(const uint32_t *)(sViewer + offset) == 0);
}

static void AssertHalfwordZero(uint32_t offset)
{
    assert(*(const uint16_t *)(sViewer + offset) == 0);
}

int main(void)
{
    static const uint32_t word_offsets[] = {
        0x10, 0x14,
        0x340, 0x344, 0x348,
        0x34c, 0x350, 0x354, 0x358, 0x35c, 0x360,
        0x364, 0x368,
        0x4039c, 0x40608,
        0xf0, 0xf4, 0xf8,
    };
    static const uint32_t halfword_offsets[] = {
        0x403a0, 0x403a2, 0x403a4,
        0x4060c, 0x4060e, 0x40610, 0x40612, 0x40614,
        0x4061c, 0x4061e,
    };
    unsigned int index;

    memset(sViewer, 0xff, sizeof(sViewer));
    sEvents[0] = '\0';
    assert(GameManualViewer_Construct_020bc588(
        (GameManualViewerPageFlow *)sViewer) ==
        (GameManualViewerPageFlow *)sViewer);
    assert(strcmp(sEvents, "SBTAA") == 0);

    for (index = 0;
         index < sizeof(word_offsets) / sizeof(word_offsets[0]);
         ++index) {
        AssertWordZero(word_offsets[index]);
    }
    for (index = 0;
         index < sizeof(halfword_offsets) / sizeof(halfword_offsets[0]);
         ++index) {
        AssertHalfwordZero(halfword_offsets[index]);
    }
    assert(sViewer[0x80] == 0);
    assert(sViewer[0xec] == 0);
    for (index = 0; index < 5; ++index) {
        assert(sViewer[0x40620 + index] == 0);
    }
    return 0;
}
