#include "game/datalink_sprite_oam.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

Game_DatalinkOamManager gGameDatalinkOamManager;

static const uint8_t sCellData[1];
static Game_DatalinkOamEntry sGeneratedEntries[4];
static uint32_t sGeneratedCount;
static int sMakeCount;
static int sEntryCount;
static uint32_t sEnteredCount;

uint32_t NNS_G2dMakeCellToOams(
    Game_DatalinkOamEntry *output,
    uint32_t capacity,
    const void *cell,
    const void *affine_transform,
    const Game_DatalinkSpriteTranslation *translation,
    void *attribute_callback,
    void *callback_argument
)
{
    assert(capacity == 16U);
    assert(cell == sCellData);
    assert(affine_transform == NULL);
    assert(translation != NULL);
    assert(attribute_callback == NULL);
    assert(callback_argument == NULL);
    assert(sGeneratedCount <= capacity);
    memcpy(output, sGeneratedEntries,
           sGeneratedCount * sizeof(sGeneratedEntries[0]));
    ++sMakeCount;
    return sGeneratedCount;
}

void NNS_G2dEntryOamManagerOam(
    Game_DatalinkOamManager *manager,
    const Game_DatalinkOamEntry *oam_entries,
    uint32_t count
)
{
    assert(manager == &gGameDatalinkOamManager);
    assert(oam_entries != NULL);
    sEnteredCount = count;
    ++sEntryCount;
}

static Game_DatalinkOamEntry MakeEntry(
    uint32_t shape,
    uint32_t size,
    uint32_t x,
    uint32_t y
)
{
    Game_DatalinkOamEntry entry;

    entry.attribute_0 = (uint16_t)((shape << 14) | (y & 0xFFU));
    entry.attribute_1 = (uint16_t)((size << 14) | (x & 0x1FFU));
    entry.attribute_2 = UINT16_C(0xFFFF);
    entry.affine_value = 0U;
    return entry;
}

static void ResetHarness(
    Game_DatalinkSpriteObject *sprite,
    Game_DatalinkSpriteCellBinding *binding,
    Game_DatalinkOamEntry *output
)
{
    memset(sprite, 0, sizeof(*sprite));
    memset(binding, 0, sizeof(*binding));
    memset(output, 0, 4U * sizeof(output[0]));
    memset(sGeneratedEntries, 0, sizeof(sGeneratedEntries));
    binding->current_cell_30 = sCellData;
    sprite->active_03 = 1U;
    sprite->oam_priority_01 = 2U;
    sprite->cell_binding_54 = binding;
    sGeneratedCount = 0U;
    sMakeCount = 0;
    sEntryCount = 0;
    sEnteredCount = 0U;
}

static void TestInactiveAndYoungSpritesAreSkipped(void)
{
    Game_DatalinkSpriteObject sprite;
    Game_DatalinkSpriteCellBinding binding;
    Game_DatalinkOamEntry output[4];

    ResetHarness(&sprite, &binding, output);
    sprite.active_03 = 0U;
    assert(Game_SubmitDatalinkSpriteOams(output, 16U, &sprite) == 0);
    assert(sMakeCount == 0);

    ResetHarness(&sprite, &binding, output);
    sprite.age_gate_04 = 1U;
    sprite.age_05 = 59U;
    assert(Game_SubmitDatalinkSpriteOams(output, 16U, &sprite) == 0);
    assert(sMakeCount == 0);
}

static void TestVisibleSpriteAppliesPriorityAndSubmits(void)
{
    Game_DatalinkSpriteObject sprite;
    Game_DatalinkSpriteCellBinding binding;
    Game_DatalinkOamEntry output[4];

    ResetHarness(&sprite, &binding, output);
    sGeneratedEntries[0] = MakeEntry(0U, 1U, 10U, 20U);
    sGeneratedCount = 1U;

    assert(Game_SubmitDatalinkSpriteOams(output, 16U, &sprite) == 1);
    assert(output[0].attribute_2 == UINT16_C(0xFBFF));
    assert(sMakeCount == 1);
    assert(sEntryCount == 1);
    assert(sEnteredCount == 1U);
}

static void TestMultipleOamsUseFirstShapeDimensions(void)
{
    Game_DatalinkSpriteObject sprite;
    Game_DatalinkSpriteCellBinding binding;
    Game_DatalinkOamEntry output[4];

    ResetHarness(&sprite, &binding, output);
    sGeneratedEntries[0] = MakeEntry(1U, 0U, 100U, 180U);
    sGeneratedEntries[1] = MakeEntry(2U, 3U, 90U, 185U);
    sGeneratedCount = 2U;

    assert(Game_SubmitDatalinkSpriteOams(output, 16U, &sprite) == 2);
    assert(sEntryCount == 1);
    assert(sEnteredCount == 2U);
}

static void TestHorizontalAndWrappedBoundsAreRejected(void)
{
    Game_DatalinkSpriteObject sprite;
    Game_DatalinkSpriteCellBinding binding;
    Game_DatalinkOamEntry output[4];

    ResetHarness(&sprite, &binding, output);
    sGeneratedEntries[0] = MakeEntry(0U, 0U, 300U, 20U);
    sGeneratedCount = 1U;
    assert(Game_SubmitDatalinkSpriteOams(output, 16U, &sprite) == 0);
    assert(sEntryCount == 0);

    ResetHarness(&sprite, &binding, output);
    sGeneratedEntries[0] = MakeEntry(0U, 1U, 190U, 200U);
    sGeneratedCount = 1U;
    assert(Game_SubmitDatalinkSpriteOams(output, 16U, &sprite) == 0);
    assert(sEntryCount == 0);
}

static void TestActorBelowScreenRejectsWrappedTopOam(void)
{
    Game_DatalinkSpriteObject sprite;
    Game_DatalinkSpriteCellBinding binding;
    Game_DatalinkOamEntry output[4];

    ResetHarness(&sprite, &binding, output);
    sprite.translation_30.y_fx = 193 * 0x1000;
    sGeneratedEntries[0] = MakeEntry(0U, 0U, 20U, 10U);
    sGeneratedCount = 1U;

    assert(Game_SubmitDatalinkSpriteOams(output, 16U, &sprite) == 0);
    assert(sEntryCount == 0);
}

int main(void)
{
    TestInactiveAndYoungSpritesAreSkipped();
    TestVisibleSpriteAppliesPriorityAndSubmits();
    TestMultipleOamsUseFirstShapeDimensions();
    TestHorizontalAndWrappedBoundsAreRejected();
    TestActorBelowScreenRejectsWrappedTopOam();
    return 0;
}
