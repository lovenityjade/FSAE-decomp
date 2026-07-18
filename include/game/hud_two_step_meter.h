#ifndef GAME_HUD_TWO_STEP_METER_H
#define GAME_HUD_TWO_STEP_METER_H

#include <stdint.h>

/*
 * Draws the upper member of a family of three two-step HUD indicators.
 * The role-specific name remains provisional until the owning HUD state is
 * recovered.
 */
void Game_DrawTopHudTwoStepMeter(int step, uint16_t *screen_tilemap);

#endif
