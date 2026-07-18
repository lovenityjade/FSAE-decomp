#include "game/graphics_state.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

static unsigned int sBrightnessCalls;
static unsigned int sPaletteCalls;
static int32_t sBrightness;

uint32_t gGameGraphicsDefaultDmaChannel_020dfb80;

void GXx_SetMasterBrightness_(uintptr_t register_address, int32_t brightness)
{
    const uintptr_t expected[2] = {0x0400006c, 0x0400106c};
    assert(sBrightnessCalls < 2);
    assert(register_address == expected[sBrightnessCalls]);
    sBrightness = brightness;
    ++sBrightnessCalls;
}

void G2x_SetBlendAlpha_(
    uintptr_t register_address,
    uint32_t first_planes,
    uint32_t second_planes,
    uint32_t eva,
    uint32_t evb)
{
    assert(register_address == 0x04000050);
    assert(first_planes == 1 && second_planes == 0x3e);
    assert(eva == 6 && evb == 10);
}

static void CheckPalette(const void *source, uint32_t offset, uint32_t size)
{
    assert(source != NULL);
    assert(*(const uint16_t *)source == 0x5a3c);
    assert(offset == 0 && size == 2);
    ++sPaletteCalls;
}

void GX_LoadBGPltt(const void *source, uint32_t offset, uint32_t size)
{
    assert(sPaletteCalls == 0);
    CheckPalette(source, offset, size);
}

void GXS_LoadBGPltt(const void *source, uint32_t offset, uint32_t size)
{
    assert(sPaletteCalls == 1);
    CheckPalette(source, offset, size);
}

int main(void)
{
    assert(Game_GetUiDisplayControlBits() == 0x00100010);
    gGameGraphicsDefaultDmaChannel_020dfb80 = 0;
    assert(Game_IsGraphicsDefaultDmaConfigured_020bc528());
    gGameGraphicsDefaultDmaChannel_020dfb80 = 3;
    assert(Game_IsGraphicsDefaultDmaConfigured_020bc528());
    gGameGraphicsDefaultDmaChannel_020dfb80 = 4;
    assert(Game_IsGraphicsDefaultDmaConfigured_020bc528());
    gGameGraphicsDefaultDmaChannel_020dfb80 = 7;
    assert(Game_IsGraphicsDefaultDmaConfigured_020bc528());
    gGameGraphicsDefaultDmaChannel_020dfb80 = 8;
    assert(!Game_IsGraphicsDefaultDmaConfigured_020bc528());
    gGameGraphicsDefaultDmaChannel_020dfb80 = UINT32_MAX;
    assert(!Game_IsGraphicsDefaultDmaConfigured_020bc528());
    Game_SetMasterBrightnessBoth(-9);
    assert(sBrightnessCalls == 2 && sBrightness == -9);
    Game_SetDefaultMainBlendAlpha();
    Game_SetBgPaletteColor0Both(0x5a3c);
    assert(sPaletteCalls == 2);
    return 0;
}
