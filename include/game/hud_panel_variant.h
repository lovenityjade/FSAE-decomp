#ifndef GAME_HUD_PANEL_VARIANT_H
#define GAME_HUD_PANEL_VARIANT_H

#include <stdint.h>

/* Valid left variants are 0..6; valid right variants are 0..11. */
void Game_DrawLeftHudPanelVariant(int variant, uint16_t *screen_tilemap);
void Game_DrawRightHudPanelVariant(int variant, uint16_t *screen_tilemap);

#endif
