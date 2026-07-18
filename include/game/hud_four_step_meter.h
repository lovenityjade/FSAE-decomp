#ifndef GAME_HUD_FOUR_STEP_METER_H
#define GAME_HUD_FOUR_STEP_METER_H

#include <stdint.h>

/*
 * Draws the four-step vertical indicator at the right edge of the HUD.
 * The gameplay role attached to the indicator remains provisional.
 */
void Game_DrawRightHudFourStepMeter(int step, uint16_t *screen_tilemap);

#endif
