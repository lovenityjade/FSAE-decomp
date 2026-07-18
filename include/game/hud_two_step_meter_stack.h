#ifndef GAME_HUD_TWO_STEP_METER_STACK_H
#define GAME_HUD_TWO_STEP_METER_STACK_H

#include <stdint.h>

/*
 * Draw the middle and bottom members of the three-indicator HUD stack.
 * Their role-specific names remain provisional until the owning state is
 * recovered.
 */
void Game_DrawMiddleHudTwoStepMeter(int step, uint16_t *screen_tilemap);
void Game_DrawBottomHudTwoStepMeter(int step, uint16_t *screen_tilemap);

#endif
