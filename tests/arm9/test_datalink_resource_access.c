#include "game/datalink_resource_access.h"

#include <assert.h>
#include <stdint.h>

static const uint8_t sPrimaryResourceData[128];
static const uint8_t sAlternateResourceData[128];

const uint8_t *gGameDatalinkPrimaryResourceBase =
    sPrimaryResourceData;
const uint8_t *gGameDatalinkAlternateResourceBase =
    sAlternateResourceData;

static uint32_t sLastResourceId;
static int sPrimaryOffsetCount;
static int sAlternateOffsetCount;
static int sPrimarySizeCount;
static int sAlternateSizeCount;

uint32_t Game_GetDatalinkPrimaryResourceOffset(uint32_t resource_id)
{
    sLastResourceId = resource_id;
    ++sPrimaryOffsetCount;
    return 12U;
}

uint32_t Game_GetDatalinkPrimaryResourceSize(uint32_t resource_id)
{
    sLastResourceId = resource_id;
    ++sPrimarySizeCount;
    return 27U;
}

uint32_t Game_GetDatalinkAlternateResourceOffset(uint32_t resource_id)
{
    sLastResourceId = resource_id;
    ++sAlternateOffsetCount;
    return 44U;
}

uint32_t Game_GetDatalinkAlternateResourceSize(uint32_t resource_id)
{
    sLastResourceId = resource_id;
    ++sAlternateSizeCount;
    return 61U;
}

static void ResetHarness(void)
{
    sLastResourceId = UINT32_C(0xFFFFFFFF);
    sPrimaryOffsetCount = 0;
    sAlternateOffsetCount = 0;
    sPrimarySizeCount = 0;
    sAlternateSizeCount = 0;
}

static void TestPrimaryResourceAddress(void)
{
    const void *address;

    ResetHarness();
    address = Game_GetDatalinkResourceAddress(5U, 0);

    assert(address == &sPrimaryResourceData[12]);
    assert(sLastResourceId == 5U);
    assert(sPrimaryOffsetCount == 1);
    assert(sAlternateOffsetCount == 0);
    assert(sPrimarySizeCount == 0);
    assert(sAlternateSizeCount == 0);
}

static void TestAlternateResourceAddress(void)
{
    const void *address;

    ResetHarness();
    address = Game_GetDatalinkResourceAddress(9U, -1);

    assert(address == &sAlternateResourceData[44]);
    assert(sLastResourceId == 9U);
    assert(sPrimaryOffsetCount == 0);
    assert(sAlternateOffsetCount == 1);
    assert(sPrimarySizeCount == 0);
    assert(sAlternateSizeCount == 0);
}

static void TestPrimaryResourceSize(void)
{
    ResetHarness();

    assert(Game_GetDatalinkResourceSize(11U, 0) == 27U);
    assert(sLastResourceId == 11U);
    assert(sPrimaryOffsetCount == 0);
    assert(sAlternateOffsetCount == 0);
    assert(sPrimarySizeCount == 1);
    assert(sAlternateSizeCount == 0);
}

static void TestAlternateResourceSize(void)
{
    ResetHarness();

    assert(Game_GetDatalinkResourceSize(13U, 1) == 61U);
    assert(sLastResourceId == 13U);
    assert(sPrimaryOffsetCount == 0);
    assert(sAlternateOffsetCount == 0);
    assert(sPrimarySizeCount == 0);
    assert(sAlternateSizeCount == 1);
}

int main(void)
{
    TestPrimaryResourceAddress();
    TestAlternateResourceAddress();
    TestPrimaryResourceSize();
    TestAlternateResourceSize();
    return 0;
}
