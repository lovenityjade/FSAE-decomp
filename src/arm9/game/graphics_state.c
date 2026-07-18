#include "game/graphics_state.h"

#include <stddef.h>

enum {
    REG_G2_MASTER_BRIGHT = 0x0400006c,
    REG_G2S_MASTER_BRIGHT = 0x0400106c,
    REG_G2_BLEND_CONTROL = 0x04000050,
};

extern void GXx_SetMasterBrightness_(uintptr_t register_address, int32_t brightness);
extern void G2x_SetBlendAlpha_(
    uintptr_t register_address,
    uint32_t first_planes,
    uint32_t second_planes,
    uint32_t eva,
    uint32_t evb);
extern void GX_LoadBGPltt(const void *source, uint32_t offset, uint32_t size);
extern void GXS_LoadBGPltt(const void *source, uint32_t offset, uint32_t size);
extern uint32_t gGameGraphicsDefaultDmaChannel_020dfb80;

/* 0x02012c3c: retained empty pre-initialization hook. */
void Game_GraphicsPreInitHook_02012c3c(void)
{
}

/* 0x020bc390 */
uint32_t Game_GetUiDisplayControlBits(void)
{
    return 0x00100010;
}

/* 0x020bc528 */
bool Game_IsGraphicsDefaultDmaConfigured_020bc528(void)
{
    return gGameGraphicsDefaultDmaChannel_020dfb80 < 8;
}

/* 0x020bc3a8 */
void Game_SetMasterBrightnessBoth(int32_t brightness)
{
    GXx_SetMasterBrightness_(REG_G2_MASTER_BRIGHT, brightness);
    GXx_SetMasterBrightness_(REG_G2S_MASTER_BRIGHT, brightness);
}

/* 0x020bc3d4 */
void Game_SetDefaultMainBlendAlpha(void)
{
    G2x_SetBlendAlpha_(REG_G2_BLEND_CONTROL, 1, 0x3e, 6, 10);
}

/* 0x020bc3fc */
void Game_SetBgPaletteColor0Both(uint16_t color)
{
    GX_LoadBGPltt(&color, 0, sizeof(color));
    GXS_LoadBGPltt(&color, 0, sizeof(color));
}
