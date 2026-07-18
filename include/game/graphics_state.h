#ifndef FSAE_GAME_GRAPHICS_STATE_H
#define FSAE_GAME_GRAPHICS_STATE_H

#include <stdbool.h>
#include <stdint.h>

/* Fixed display-plane bits shared by both manual 2D engines. */
uint32_t Game_GetUiDisplayControlBits(void); /* 0x020bc390 */
bool Game_IsGraphicsDefaultDmaConfigured_020bc528(void);
void Game_GraphicsPreInitHook_02012c3c(void);

/* Applies one signed master-brightness value to both 2D engines. */
void Game_SetMasterBrightnessBoth(int32_t brightness); /* 0x020bc3a8 */

/* Restores the fixed main-engine alpha blend used by the menu renderer. */
void Game_SetDefaultMainBlendAlpha(void); /* 0x020bc3d4 */

/* Writes palette entry zero to both main and sub BG palettes. */
void Game_SetBgPaletteColor0Both(uint16_t color); /* 0x020bc3fc */

#endif
