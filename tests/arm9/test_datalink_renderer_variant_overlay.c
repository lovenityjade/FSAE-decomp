#include "game/datalink_renderer_variant_overlay.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static uint16_t sSecondaryTilemap[1];
static uint16_t sPrimaryInitial[1];
static uint16_t sPrimaryAfterClear[1];
static volatile Game_DatalinkRenderPrimaryBinding *sBuffers;
static int sParticipantType;
static int sExpectedActiveIndex;
static size_t sTypeQueryCount;
static size_t sClearCount;
static size_t sPaletteCount;

int Game_GetDatalinkParticipantType(
    const Game_DatalinkFlowState *state,
    int participant_index
)
{
    (void)state;
    assert(participant_index == sExpectedActiveIndex);
    ++sTypeQueryCount;
    return sParticipantType;
}

void Game_ClearDatalinkRenderRectangle(
    uint16_t *tilemap,
    int x,
    int y,
    int width,
    int height
)
{
    assert(x == 2);
    assert(y == 10);
    assert(width == 0x1C);
    assert(height == 4);
    if (sClearCount == 0U) {
        assert(tilemap == sSecondaryTilemap);
        sBuffers->primary_buffer = (uint8_t *)(void *)sPrimaryAfterClear;
    } else {
        assert(sClearCount == 1U);
        assert(tilemap == sPrimaryAfterClear);
    }
    ++sClearCount;
}

void Game_SetDatalinkRenderRectanglePalette(
    uint16_t *tilemap,
    int x,
    int y,
    int width,
    int height,
    int palette
)
{
    assert(tilemap == sSecondaryTilemap);
    assert(x == 2);
    assert(y == 10);
    assert(width == 0x1C);
    assert(height == 4);
    assert(palette == sParticipantType);
    ++sPaletteCount;
}

static void ResetHarness(
    Game_DatalinkFlowState *state,
    Game_DatalinkRenderPrimaryBinding *buffers
)
{
    memset(state, 0, sizeof(*state));
    buffers->primary_buffer = (uint8_t *)(void *)sPrimaryInitial;
    sBuffers = buffers;
    sTypeQueryCount = 0U;
    sClearCount = 0U;
    sPaletteCount = 0U;
}

static void TestSelectedRowUsesPaletteFourWithoutTypeLookup(void)
{
    Game_DatalinkFlowState state;
    Game_DatalinkRenderPrimaryBinding buffers;

    ResetHarness(&state, &buffers);
    state.selected_participant_40 = 1;
    sParticipantType = 4;

    Game_RenderDatalinkRowVariantOverlay(
        &state,
        &buffers,
        sSecondaryTilemap,
        1
    );

    assert(sTypeQueryCount == 0U);
    assert(sClearCount == 0U);
    assert(sPaletteCount == 1U);
}

static void TestTypeOneClearsSecondaryThenReloadedPrimary(void)
{
    Game_DatalinkFlowState state;
    Game_DatalinkRenderPrimaryBinding buffers;

    ResetHarness(&state, &buffers);
    state.selected_participant_40 = 2;
    state.active_participant_index_2c = -3;
    sExpectedActiveIndex = -3;
    sParticipantType = 1;

    Game_RenderDatalinkRowVariantOverlay(
        &state,
        &buffers,
        sSecondaryTilemap,
        1
    );

    assert(sTypeQueryCount == 1U);
    assert(sClearCount == 2U);
    assert(sPaletteCount == 0U);
}

static void TestOtherTypeUsesPaletteThree(void)
{
    Game_DatalinkFlowState state;
    Game_DatalinkRenderPrimaryBinding buffers;

    ResetHarness(&state, &buffers);
    state.selected_participant_40 = 0;
    state.active_participant_index_2c = 5;
    sExpectedActiveIndex = 5;
    sParticipantType = 3;

    Game_RenderDatalinkRowVariantOverlay(
        &state,
        &buffers,
        sSecondaryTilemap,
        1
    );

    assert(sTypeQueryCount == 1U);
    assert(sClearCount == 0U);
    assert(sPaletteCount == 1U);
}

int main(void)
{
    TestSelectedRowUsesPaletteFourWithoutTypeLookup();
    TestTypeOneClearsSecondaryThenReloadedPrimary();
    TestOtherTypeUsesPaletteThree();
    return 0;
}
